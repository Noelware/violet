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

#if VIOLET_FEATURE(PTHREADS)

#include <violet/Defer.h>
#include <violet/Experimental/Threading/Thread.h>

#include <bit>
#include <pthread.h>
#include <unistd.h>

namespace violet::experimental::threading {
namespace {

static_assert(std::is_integral_v<pthread_t> || std::is_pointer_v<pthread_t>, "`pthread_t` must be a scalar");
static_assert(sizeof(pthread_t) <= sizeof(std::uintptr_t), "`pthread_t` must fit in `std::uintptr_t`");
static_assert(sizeof(pthread_t) <= sizeof(void*), "`pthread_t` must fit in `void*`");

constinit std::atomic<void*> g_mainThreadState;

auto toId(pthread_t native) noexcept -> uintptr_t
{
    return std::bit_cast<uintptr_t>(native);
}

auto toNative(pthread_t native) noexcept -> void*
{
    return std::bit_cast<void*>(native);
}

auto fromNative(void* native) noexcept -> pthread_t
{
    return std::bit_cast<pthread_t>(native);
}

auto isMainThread() noexcept -> bool
{
#if VIOLET_PLATFORM(APPLE_MACOS)
    return ::pthread_main_np() != 0;
#elif VIOLET_PLATFORM(LINUX)
    // the initial thread's kernel ID is always the PID if we're the "main thread"
    return ::gettid() == ::getpid();
#else
    return true;
#endif
}

void setThreadName(const String& name) noexcept
{
    if (name.empty()) {
        return;
    }

#if VIOLET_PLATFORM(LINUX)
    String truncated(name.substr(0, 15));
    ::pthread_setname_np(::pthread_self(), truncated.c_str());
#elif VIOLET_PLATFORM(APPLE_MACOS)
    ::pthread_setname_np(name.c_str());
#endif
}

auto normalizeStackSize(UInt bytes) noexcept -> UInt
{
    const auto page = static_cast<UInt>(::sysconf(_SC_PAGESIZE));
    const UInt size = std::max(bytes, static_cast<UInt>(PTHREAD_STACK_MIN));

    // macOS requires the stack size to be a multiple of the page size
    return (size + page - 1) / page * page;
}

} // namespace

struct Thread::start final {
    Own<State> State;
    ptr::Unique<Runnable> Task;
};

thread_local Own<Thread::State> Thread::tCurrent = nullptr;
const bool Thread::s_mainThreadInitialized = (static_cast<void>(Thread::getMainThreadState()), true);

auto Thread::getMainThreadState() noexcept -> const Own<State>&
{
    VIOLET_LIKELY_IF(void* state = g_mainThreadState.load(std::memory_order_acquire); state != nullptr)
    {
        return *static_cast<const Own<State>*>(state);
    }

    VIOLET_DEBUG_ASSERT(isMainThread(), "violet's main thread state was initialized off the main thread");

    auto* fresh = new Own<State>(Own<State>::New(ThreadId(toId(::pthread_self())), Some("main")));
    void* expected = nullptr;

    if (!g_mainThreadState.compare_exchange_strong(
            expected, fresh, std::memory_order_acq_rel, std::memory_order_acquire)) {
        // we lost the race; someone else published first
        delete fresh;
        return *static_cast<const Own<State>*>(expected);
    }

    return *fresh;
}

auto ThreadId::Current() noexcept -> ThreadId
{
    return ThreadId(toId(::pthread_self()));
}

auto ThreadId::Main() noexcept -> ThreadId
{
    return Thread::getMainThreadState()->ID;
}

auto Thread::Current() noexcept -> Thread
{
    if (tCurrent == nullptr) {
        const auto& mainThread = Thread::getMainThreadState();
        tCurrent = mainThread->ID == ThreadId::Current() ? mainThread : Own<State>::New(ThreadId::Current(), Nothing);
    }

    return Thread(tCurrent);
}

auto Thread::Main() noexcept -> Thread
{
    return Thread(getMainThreadState());
}

auto Thread::Id() const noexcept -> ThreadId
{
    if (this->n_nativeThread != nullptr) {
        return ThreadId(toId(fromNative(this->n_nativeThread)));
    }

    return this->n_state->ID;
}

auto Thread::OSName() const noexcept -> Str
{
    Slice<char, 64> buf;
    ::pthread_getname_np(::pthread_self(), buf.Data(), buf.Size());
    buf.AssumeInit(Unsafe("`pthread_getname_np` will always write atleast [buf.Size()] bytes"), buf.Size());

    return {buf.Data()};
}

void Thread::Join()
{
    VIOLET_ASSERT(this->n_joinable, "thread handle is not joinable");

    const pthread_t thread = fromNative(this->n_nativeThread);
    VIOLET_ASSERT(::pthread_equal(thread, ::pthread_self()) == 0, "a thread cannot join itself");

    [[maybe_unused]]
    const auto rc = ::pthread_join(thread, nullptr);
    VIOLET_DEBUG_ASSERT_FMT(rc == 0, "`pthread_join` failed: {}", io::experimental::Error::FromOSError(rc));

    this->n_joinable = false;
}

void Thread::Detach()
{
    VIOLET_ASSERT(this->n_joinable, "thread handle is not joinable");

    [[maybe_unused]]
    const auto rc = ::pthread_detach(fromNative(this->n_nativeThread));
    VIOLET_DEBUG_ASSERT_FMT(rc == 0, "`pthread_join` failed: {}", io::experimental::Error::FromOSError(rc));

    this->n_joinable = false;
}

auto Thread::trampoline(void* arg) noexcept -> void*
{
    auto* ctx = static_cast<start*>(arg);
    tCurrent = ctx->State;
    tCurrent->ID = ThreadId(toId(::pthread_self()));

    if (auto name = tCurrent->Name; name.HasValue()) {
        setThreadName(*name);
    }

    {
        const auto token = tCurrent->Cts.Token();
        ctx->Task->Run(token);
    }

    delete ctx;
    tCurrent = nullptr;

    return nullptr;
}

auto Thread::Builder::spawn(ptr::Unique<Runnable> runnable) -> io::experimental::Result<Thread>
{
    pthread_attr_t attrs;
    if (const auto rc = ::pthread_attr_init(&attrs); rc != 0) {
        return Err(io::experimental::Error::FromOSError(rc));
    }

    VIOLET_DEFER({ ::pthread_attr_destroy(&attrs); });
    if (auto stackSize = this->n_stackSize; stackSize.HasValue()) {
        if (const auto rc = ::pthread_attr_setstacksize(&attrs, normalizeStackSize(*stackSize)); rc != 0) {
            return Err(io::experimental::Error::FromOSError(rc));
        }
    }

    // the thread ID is set by the trampoline
    auto state = Own<State>::New(ThreadId(0), VIOLET_MOVE(this->n_name));
    auto* ctx = new start(state, VIOLET_MOVE(runnable));

    pthread_t native{};
    const auto rc = ::pthread_create(&native, &attrs, &Thread::trampoline, ctx);
    if (rc != 0) {
        delete ctx;
        return Err(io::experimental::Error::FromOSError(rc));
    }

    Thread thread(VIOLET_MOVE(state), toNative(native));
    thread.n_joinable = true;

    return thread;
}

} // namespace violet::experimental::threading

#endif
