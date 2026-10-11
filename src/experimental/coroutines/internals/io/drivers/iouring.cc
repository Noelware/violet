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

#if !(VIOLET_PLATFORM(LINUX) && VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER))
#error "platform was not built for Linux or the iouring driver feature was not enabled"
#else

#include <liburing.h>
#include <violet/Experimental/Coroutines/Internals/IO/Drivers/IoUring.h>
#include <violet/Experimental/Coroutines/Internals/IO/Op.h>

#include <cerrno>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace violet::io::experimental;

namespace violet::experimental::coro::internals::io {
namespace {

constexpr UInt32 kDefaultEntries = 256;

auto propagateError(Int32 negated) noexcept -> Error
{
    return Error::FromOSError(-negated);
}

} // namespace

void IoUringDriver::RingDeleter::operator()(io_uring* ring) const noexcept
{
    ::io_uring_queue_exit(ring);
    delete ring;
}

auto IoUringDriver::New(DriverOptions&) noexcept -> violet::io::experimental::Result<ptr::Unique<Driver>>
{
    Descriptor eventfd = Descriptor::FromRaw(::eventfd(0, EFD_CLOEXEC));
    if (!eventfd.Valid()) {
        return Err(Error::OSError());
    }

    auto uninit = ptr::Unique<io_uring>::New();
    if (const auto rc = ::io_uring_queue_init(kDefaultEntries, uninit.Get(), 0); rc < 0) {
        return Err(propagateError(rc));
    }

    ptr::Unique<io_uring, RingDeleter> ring(uninit.Leak().Get());
    return ptr::Unique<Driver>::New<IoUringDriver>(VIOLET_MOVE(ring), VIOLET_MOVE(eventfd));
}

IoUringDriver::~IoUringDriver()
{
    this->n_closing = true;
    if (this->n_wakeArmed) {
        auto* sqe = this->acquireSqe();
        ::io_uring_prep_cancel(sqe, &this->n_wakeValue, 0);
        ::io_uring_sqe_set_data(sqe, nullptr);
    }

    while (this->n_inflight > 0 || this->n_wakeArmed) {
        ::io_uring_submit_and_wait(this->n_ring.Get(), 1);
        this->drain();
    }
}

auto IoUringDriver::TryNow(Op&) noexcept -> bool
{
    return false;
}

void IoUringDriver::Submit(Op& op) noexcept
{
    auto* sqe = this->acquireSqe();
    op.PrepareUring(*sqe);
    ::io_uring_sqe_set_data(sqe, &op);

    op.Status = OpStatus::Inflight;
    this->n_inflight += 1;
}

void IoUringDriver::Orphan(Op* op) noexcept
{
    op->Waiter = Waker{};

    auto* sqe = this->acquireSqe();
    ::io_uring_prep_cancel(sqe, op, 0);
    ::io_uring_sqe_set_data(sqe, nullptr);
}

void IoUringDriver::Park(Optional<chrono::Instant> deadline, const Clock& clock) noexcept
{
    Optional<__kernel_timespec> timeout;
    if (deadline.HasValue()) {
        const Int64 millis = MillisUntil(*deadline, clock);
        timeout = __kernel_timespec{.tv_sec = millis / 1000, .tv_nsec = (millis % 1000) * 1'000'000};
    }

    io_uring_cqe* cqe = nullptr;
    const Int32 rc = ::io_uring_submit_and_wait_timeout(
        this->n_ring.Get(), &cqe, 1, timeout.HasValue() ? std::addressof(timeout.Value()) : nullptr, nullptr);

    VIOLET_DEBUG_ASSERT(
        rc >= 0 || rc == -ETIME || rc == -EINTR || rc == -EBUSY, "`io_uring_submit_and_wait_timeout` failed");

    this->drain();
}

void IoUringDriver::Unpark() noexcept
{
    constexpr UInt64 one = 1;
    static_cast<void>(::write(this->n_eventfd.Get(), &one, sizeof(one)));
}

auto IoUringDriver::acquireSqe() noexcept -> io_uring_sqe*
{
    auto* sqe = ::io_uring_get_sqe(this->n_ring.Get());
    if (sqe == nullptr) {
        // The queue was full, so we'll hand off what we currently have to the kernel and retry.
        ::io_uring_submit(this->n_ring.Get());
        sqe = ::io_uring_get_sqe(this->n_ring.Get());
    }

    VIOLET_ASSERT(sqe != nullptr, "io_uring submission queue exhausted");
    return sqe;
}

void IoUringDriver::armWake() noexcept
{
    auto* sqe = this->acquireSqe();
    ::io_uring_prep_read(sqe, this->n_eventfd.Get(), &this->n_wakeValue, sizeof(this->n_wakeValue), 0);
    ::io_uring_sqe_set_data(sqe, &this->n_wakeValue);
    this->n_wakeArmed = true;
}

void IoUringDriver::drain() noexcept
{
    UInt32 head = 0;
    UInt32 seen = 0;

    io_uring_cqe* cqe = nullptr;
    io_uring_for_each_cqe(this->n_ring.Get(), head, cqe)
    {
        this->complete(::io_uring_cqe_get_data(cqe), cqe->res);
        seen++;
    }

    ::io_uring_cq_advance(this->n_ring.Get(), seen);
}

void IoUringDriver::complete(void* data, Int32 res) noexcept
{
    if (data == nullptr) {
        return;
    }

    if (data == &this->n_wakeValue) {
        this->n_wakeArmed = false;
        if (!this->n_closing) {
            this->armWake();
        }

        return;
    }

    auto* op = static_cast<Op*>(data);
    this->n_inflight--;

    CompleteOperation(Unsafe("io_uring hands each operation exactly once"), op, op->NormalizeUringResult(res));
}

} // namespace violet::experimental::coro::internals::io

#endif
