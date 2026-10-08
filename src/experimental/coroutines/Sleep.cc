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

#include <violet/Experimental/Coroutines/Sleep.h>

namespace violet::experimental::coro {
namespace internals {

void SleepAwaiter::await_suspend(std::coroutine_handle<>) noexcept
{
    auto& handle = Handle::Current();
    auto* self = CurrentTask();
    VIOLET_DEBUG_ASSERT(self != nullptr, "`Sleep` awaited outside of a runtime");

    RetainRawTask(Unsafe("we are running as the current task"), self);

    this->n_entry.Task = self;
    this->n_entry.Fire = &SleepAwaiter::fire;
    this->n_driver = handle.Timers;
    this->n_driver->Register(this->n_entry, handle.Clock->Now() + this->n_duration);
}

} // namespace internals

auto Sleep(chrono::Duration dur) noexcept -> internals::SleepAwaiter
{
    auto& handle = Handle::Current();
    VIOLET_DEBUG_ASSERT(handle.Timers != nullptr,
        "timers API was not enabled at runtime. Call `EnableTimers()` in `Runtime::Builder` to enable it!");

    return internals::SleepAwaiter{dur};
}

} // namespace violet::experimental::coro
