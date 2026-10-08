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
//! # 🌺💜 `violet/Experimental/Coroutines/Sleep.h`

#pragma once

#include <violet/Experimental/Coroutines/Internals/Timers/Wheel.h>
#include <violet/Experimental/Coroutines/Runtime.h>

namespace violet::experimental::coro {
namespace NOELDOC_HIDE internals {

struct VIOLET_API SleepAwaiter final {
    VIOLET_DISALLOW_COPY_AND_MOVE(SleepAwaiter);

    VIOLET_EXPLICIT SleepAwaiter(chrono::Duration dur) noexcept
        : n_duration(dur)
    {
    }

    ~SleepAwaiter()
    {
        if (this->n_driver != nullptr) {
            this->n_driver->Cancel(this->n_entry);
        }
    }

    [[nodiscard]]
    static auto await_ready() noexcept -> bool
    {
        return false;
    }

    static void await_resume() noexcept { }

    void await_suspend(std::coroutine_handle<>) noexcept;

private:
    struct ent final: TimerEntry {
        RawTask* Task = nullptr;
    };

    static void fire(TimerEntry* entry) noexcept
    {
        RawTask* task = std::exchange(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
            static_cast<ent*>(entry)->Task, nullptr);

        WakeupTask(Unsafe("timer entry holds a reference"), task);
        ReleaseRawTask(Unsafe("dropping the timer entry's reference"), task);
    }

    chrono::Duration n_duration;
    timers::Driver* n_driver = nullptr;
    ent n_entry;
};

} // namespace NOELDOC_HIDE internals

/// Suspends the current task until `duration` has elapsed on the runtime's clock.
///
/// ## Example
/// ```cpp
/// co_await Sleep(Duration::Milliseconds(50));
/// ```
auto Sleep(chrono::Duration dur) noexcept -> internals::SleepAwaiter;

} // namespace violet::experimental::coro
