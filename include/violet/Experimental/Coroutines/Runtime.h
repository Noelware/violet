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
//
//! # 🌺💜 `violet/Experimental/Coroutines/Runtime.h`

#pragma once

#include <violet/Experimental/Coroutines/Internals/Scheduler.h>
#include <violet/Experimental/Coroutines/Internals/Timers/Driver.h>
#include <violet/Experimental/Coroutines/JoinHandle.h>
#include <violet/Experimental/Coroutines/Task.h>
#include <violet/Experimental/Time/Clock.h>
#include <violet/Experimental/Unique.h>
#include <violet/SourceLocation.h>

namespace violet::experimental::coro {

struct Runtime;

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Handle final {
    NOELDOC_HIDE const struct Clock* Clock;
    NOELDOC_HIDE internals::timers::Driver* Timers;

    static auto Current(SourceLocation loc = std::source_location::current()) noexcept -> Handle&;
    static auto TryCurrent() noexcept -> Optional<Handle>;

    template<typename T>
    auto Spawn(Task<T> task) noexcept -> JoinHandle<T>;

private:
    friend struct Runtime;

    VIOLET_EXPLICIT Handle(
        internals::Scheduler* scheduler, internals::timers::Driver* timers, const struct Clock* clock) noexcept
        : Clock(clock)
        , Timers(timers)
        , n_scheduler(scheduler)
    {
    }

    internals::Scheduler* n_scheduler = nullptr;
};

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") HandleGuard final {
    VIOLET_DISALLOW_COPY_AND_MOVE(HandleGuard);

    ~HandleGuard();

private:
    friend struct Runtime;

    VIOLET_EXPLICIT HandleGuard(Handle* handle) noexcept;

    Handle* n_previous = nullptr;
};

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Runtime final {
    struct Builder;

    struct NOELDOC_HIDE ctor_key final {
    private:
        friend struct Builder;
        VIOLET_EXPLICIT ctor_key() noexcept = default;
    };

    VIOLET_DISALLOW_COPY_AND_MOVE(Runtime);
    ~Runtime();

    NOELDOC_HIDE VIOLET_EXPLICIT Runtime(ctor_key, const Clock& clock, ptr::Unique<internals::timers::Driver> timers,
        ptr::Unique<internals::Scheduler> scheduler) noexcept;

    template<typename T>
    auto BlockOn(Task<T> task) -> T;

    template<typename T>
    auto Spawn(Task<T> task) -> JoinHandle<T>
    {
        return this->n_handle.Spawn(VIOLET_MOVE(task));
    }

    auto Handle() noexcept -> struct Handle&;

private:
    friend struct Builder;

    const Clock* n_clock;
    ptr::Unique<internals::timers::Driver> n_timers;
    ptr::Unique<internals::Scheduler> n_scheduler;
    struct Handle n_handle;
};

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Runtime::Builder final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Builder);
    ~Builder() = default;

    static auto CurrentThread() -> Builder;

    template<typename ClockType>
        requires std::derived_from<std::remove_cvref_t<ClockType>, Clock>
    auto WithClock(const ClockType& clock) & noexcept -> Builder&
    {
        this->n_clock = std::addressof(clock);
        return *this;
    }

    template<typename ClockType>
        requires std::derived_from<std::remove_cvref_t<ClockType>, Clock>
    auto WithClock(const ClockType& clock) && noexcept -> Builder&&
    {
        this->n_clock = std::addressof(clock);
        return VIOLET_MOVE(*this);
    }

    auto EnableTimers() & noexcept -> Builder&;
    auto EnableTimers() && noexcept -> Builder&&;

    auto EnableAll() & noexcept -> Builder&;
    auto EnableAll() && noexcept -> Builder&&;

    [[nodiscard]]
    auto Build() && -> ptr::Unique<Runtime>;

private:
    VIOLET_EXPLICIT Builder(ptr::Unique<internals::Scheduler> scheduler) noexcept
        : n_scheduler(VIOLET_MOVE(scheduler))
    {
    }

    const Clock* n_clock = std::addressof(Clock::System());
    ptr::Unique<internals::timers::Driver> n_timers;
    ptr::Unique<internals::Scheduler> n_scheduler;
};

template<typename T>
auto Runtime::BlockOn(Task<T> task) -> T
{
    VIOLET_ASSERT(!Handle::TryCurrent(), "`Runtime::BlockOn` can't be called from inside a runtime");

    HandleGuard guard(std::addressof(this->n_handle));
    JoinHandle<T> root = this->n_handle.Spawn(VIOLET_MOVE(task));

    this->n_scheduler->BlockOn(root.n_task, {*this->n_clock, this->n_timers.Get()});
    return root.Take();
}

template<typename T>
auto Handle::Spawn(Task<T> task) noexcept -> JoinHandle<T>
{
    auto spawned = internals::MkSpawnedTask(VIOLET_MOVE(task));
    internals::RawTask* raw = spawned.Header();

    raw->Owner = this->n_scheduler;
    raw->State.store(internals::kTaskStateScheduled, std::memory_order_relaxed);

    internals::RetainRawTask(Unsafe("the owned list will own the first reference"), raw);
    this->n_scheduler->Tasks.Insert(raw);

    internals::RetainRawTask(Unsafe("we are reference #2: the run queue"), raw);
    this->n_scheduler->Push(raw);

    return JoinHandle(VIOLET_MOVE(spawned));
}

} // namespace violet::experimental::coro
