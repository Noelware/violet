// 🌺💜 Violet: Extended C++ standard library
// Copyright (c) 2025-2026 Noelware, LLC. <team@noelware.org>, et al.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <violet/Language/Macros.h>

#if !VIOLET_PLATFORM(LINUX)
#error "the epoll driver is only available on Linux"
#else

#include <violet/Experimental/Coroutines/Internals/IO/Drivers/Epoll.h>
#include <violet/Experimental/Coroutines/Internals/IO/Op.h>
#include <violet/Experimental/Slice.h>

#include <cerrno>
#include <climits>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace violet::io::experimental;

namespace violet::experimental::coro::internals::io {
namespace {

constexpr auto hasAny(Interest lhs, Interest rhs) noexcept -> bool
{
    return (static_cast<UInt8>(lhs) & static_cast<UInt8>(rhs)) != 0;
}

constexpr auto epollToInterest(UInt32 events) noexcept -> Interest
{
    if ((events & (EPOLLERR | EPOLLHUP)) != 0) {
        return Interest::ReadWrite;
    }

    UInt8 ready = 0;
    if ((events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) != 0) {
        ready |= static_cast<UInt8>(Interest::Read);
    }

    if ((events & EPOLLOUT) != 0) {
        ready |= static_cast<UInt8>(Interest::Write);
    }

    return static_cast<Interest>(ready);
}

} // namespace

EpollDriver::~EpollDriver()
{
    for (const auto& [fd, ent]: this->n_fds) {
        VIOLET_DEBUG_ASSERT(ent.Head == nullptr, "`EpollDriver` destroyed with operations still waiting");
    }
}

auto EpollDriver::New(DriverOptions&) noexcept -> violet::io::experimental::Result<ptr::Unique<Driver>>
{
    auto epollFd = ::epoll_create1(EPOLL_CLOEXEC);
    if (epollFd < 0) {
        return Err(violet::io::experimental::Error::OSError());
    }

    auto epoll = Descriptor::FromRaw(epollFd);

    auto eventfdRaw = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventfdRaw < 0) {
        return Err(violet::io::experimental::Error::OSError());
    }

    auto eventfd = Descriptor::FromRaw(eventfdRaw);

    epoll_event ev{.events = EPOLLIN, .data = {.fd = eventfd.Get()}};
    if (::epoll_ctl(epoll.Get(), EPOLL_CTL_ADD, eventfd.Get(), &ev) != 0) {
        return Err(violet::io::experimental::Error::OSError());
    }

    return ptr::Unique<Driver>::New<EpollDriver>(VIOLET_MOVE(epoll), VIOLET_MOVE(eventfd));
}

auto EpollDriver::TryNow(Op& op) noexcept -> bool
{
    if (op.ClosesDescriptor) {
        this->forget(op.Fd);
    }

    const Attempt attempt = op.TryOnce();
    if (!attempt.Done) {
        return false;
    }

    op.Result = attempt.Result;
    op.Status = OpStatus::Done;

    return true;
}

void EpollDriver::Submit(Op& op) noexcept
{
    op.Status = OpStatus::Inflight;

    auto& ent = this->n_fds[op.Fd];
    link(ent, op);

    auto res = this->arm(op.Fd, ent);
    if (res.Ok()) {
        return;
    }

    unlink(ent, op);
    if (ent.Head == nullptr && !ent.Registered) {
        (void)this->n_fds.Remove(op.Fd);
    }

    auto result = res.Error().RawOSError().Unwrap();
    if (result == EPERM) {
        const auto attempt = op.TryOnce();
        result = attempt.Done ? static_cast<Int32>(attempt.Result) : -EAGAIN;
    }

    CompleteOperation(Unsafe("`op` was unlinked"), &op, result);
}

void EpollDriver::Orphan(Op* op) noexcept
{
    if (auto it = this->n_fds.Get(op->Fd); it.HasValue()) {
        unlink(*it, *op);
    }

    op->Waiter = Waker{};
    delete op;
}

void EpollDriver::Park(Optional<chrono::Instant> deadline, const Clock& clock) noexcept
{
    Int32 timeout = -1;
    if (deadline.HasValue()) {
        timeout = static_cast<Int32>(std::min<Int64>(MillisUntil(*deadline, clock), INT_MAX));
    }

    Slice<epoll_event, 128> events;
    const Int32 count = ::epoll_wait(this->n_epoll.Get(), events.Data(), static_cast<Int32>(events.Size()), timeout);
    if (count < 0) {
        VIOLET_DEBUG_ASSERT(errno == EINTR, "`epoll_wait` failed");
        return;
    }

    events.AssumeInit(Unsafe("kernel has given us `count` elements into our data pointer"), count);
    for (Int32 i = 0; i < count; i++) {
        const auto& event = events[static_cast<UInt>(i)];
        if (event.data.fd == this->n_eventfd.Get()) {
            UInt64 drained = 0;
            while (::read(this->n_eventfd.Get(), &drained, sizeof(drained)) > 0) { }

            continue;
        }

        this->dispatch(event.data.fd, event.events);
    }
}

void EpollDriver::Unpark() noexcept
{
    constexpr UInt64 one = 1;
    static_cast<void>(::write(this->n_eventfd.Get(), &one, sizeof(one)));
}

void EpollDriver::dispatch(RawFd fd, UInt32 events) noexcept
{
    auto it = this->n_fds.Get(fd);
    if (!it.HasValue()) {
        return;
    }

    auto& ent = it.Value();
    const Interest ready = epollToInterest(events);

    for (Op* op = ent.Head; op != nullptr;) {
        auto* next = op->Next;
        if (hasAny(op->Wanted, ready)) {
            const auto attempt = op->TryOnce();
            if (attempt.Done) {
                unlink(ent, *op);
                CompleteOperation(Unsafe("`op` was unlinked"), op, attempt.Result);
            }
        }

        op = next;
    }

    if (ent.Head != nullptr) {
        if (const auto res = this->arm(fd, ent); res.Ok()) {
            while (auto* op = ent.Head) {
                unlink(ent, *op);
                CompleteOperation(Unsafe("`op` was unlinked"), op, 0);
            }
        }
    }
}

auto EpollDriver::arm(RawFd fd, entry& ent) noexcept -> violet::io::experimental::Result<void>
{
    UInt32 interest = EPOLLONESHOT;
    for (const Op* op = ent.Head; op != nullptr; op = op->Next) {
        if (hasAny(op->Wanted, Interest::Read)) {
            interest |= EPOLLIN | EPOLLRDHUP;
        }

        if (hasAny(op->Wanted, Interest::Write)) {
            interest |= EPOLLOUT;
        }
    }

    epoll_event ev{.events = interest, .data = {.fd = fd}};
    if (::epoll_ctl(this->n_epoll.Get(), ent.Registered ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &ev) != 0) {
        return Err(Error::OSError());
    }

    ent.Registered = true;
    return {};
}

void EpollDriver::forget(RawFd fd) noexcept
{
    auto it = this->n_fds.Get(fd);
    if (!it.HasValue()) {
        return;
    }

    while (auto* op = it->Head) {
        unlink(*it, *op);
        CompleteOperation(Unsafe("`op` was unlinked"), op, -ECANCELED);
    }

    if (it->Registered) {
        ::epoll_ctl(this->n_epoll.Get(), EPOLL_CTL_DEL, fd, nullptr);
    }

    (void)this->n_fds.Remove(fd);
}

void EpollDriver::link(entry& ent, Op& op) noexcept
{
    op.Previous = ent.Tail;
    op.Next = nullptr;

    (ent.Tail != nullptr ? ent.Tail->Next : ent.Head) = &op;
    ent.Tail = &op;
}

void EpollDriver::unlink(entry& ent, Op& op) noexcept
{
    (op.Previous != nullptr ? op.Previous->Next : ent.Head) = op.Next;
    (op.Next != nullptr ? op.Next->Previous : ent.Tail) = op.Previous;
    op.Previous = nullptr;
    op.Next = nullptr;
}

auto CreateDriver(DriverOptions& options) noexcept -> violet::io::experimental::Result<ptr::Unique<Driver>>
{
    return EpollDriver::New(options);
}

} // namespace violet::experimental::coro::internals::io

#endif
