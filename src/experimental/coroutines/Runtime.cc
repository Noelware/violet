// 🌺💜 Violet: Extended C standard library
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

#include <violet/Experimental/Coroutines/Internals/Schedulers/SingleThreadedScheduler.h>
#include <violet/Experimental/Coroutines/Runtime.h>

namespace violet::experimental::coro {
namespace {
thread_local Handle* tCurrentHandle = nullptr;
}

auto Handle::Current(SourceLocation loc) noexcept -> Handle&
{
    VIOLET_DEBUG_ASSERT_LOC(tCurrentHandle != nullptr, "no runtime has been established", loc);
    return *tCurrentHandle;
}

auto Handle::TryCurrent() noexcept -> Optional<Handle>
{
    if (tCurrentHandle == nullptr) {
        return Nothing;
    }

    return *tCurrentHandle;
}

HandleGuard::HandleGuard(Handle* handle) noexcept
    : n_previous(std::exchange(tCurrentHandle, handle))
{
}

HandleGuard::~HandleGuard()
{
    tCurrentHandle = this->n_previous;
}

Runtime::Runtime(ctor_key, const Clock& clock, ptr::Unique<internals::timers::Driver> timers,
    ptr::Unique<internals::Scheduler> scheduler) noexcept
    : n_clock(std::addressof(clock))
    , n_timers(VIOLET_MOVE(timers))
    , n_scheduler(VIOLET_MOVE(scheduler))
    , n_handle(this->n_scheduler.Get(), this->n_timers.Get(), this->n_clock)
{
}

Runtime::~Runtime()
{
    VIOLET_DEBUG_ASSERT(tCurrentHandle != std::addressof(this->n_handle),
        "Runtime can't be destroyed from inside its own `BlockOn` call");

    if (this->n_timers != nullptr) {
        this->n_timers->Clear();
    }

    // if (this->n_io != nullptr) this->n_io->DoCompletionOfAllTasks();

    // Cancell all pending tasks.
    this->n_scheduler->CancelAll();
}

auto Runtime::Handle() noexcept -> struct Handle&
{
    return this->n_handle;
}

auto Runtime::Builder::CurrentThread() -> Builder
{
    return Builder(ptr::Unique<internals::Scheduler>::New<internals::SingleThreadedScheduler>());
}

auto Runtime::Builder::EnableTimers() & noexcept -> Builder&
{
    this->n_timers.Reset(new internals::timers::Driver(this->n_clock->Now()));
    return *this;
}

auto Runtime::Builder::EnableTimers() && noexcept -> Builder&&
{
    this->n_timers.Reset(new internals::timers::Driver(this->n_clock->Now()));
    return VIOLET_MOVE(*this);
}

auto Runtime::Builder::Build() && -> ptr::Unique<Runtime>
{
    return ptr::Unique<Runtime>::New(
        ctor_key{}, *this->n_clock, VIOLET_MOVE(this->n_timers), VIOLET_MOVE(this->n_scheduler));
}

} // namespace violet::experimental::coro
