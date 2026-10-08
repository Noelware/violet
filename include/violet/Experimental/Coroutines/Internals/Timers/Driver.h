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

#pragma once

#include <violet/Experimental/Coroutines/Internals/Timers/Wheel.h>
#include <violet/Experimental/Time/Instant.h>

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals::timers {

struct VIOLET_API Driver final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Driver);

    VIOLET_EXPLICIT Driver(chrono::Instant base) noexcept
        : n_base(base)
    {
    }

    ~Driver()
    {
        VIOLET_DEBUG_ASSERT(
            this->n_wheel.Empty(), "timers::Driver destroyed with pending timers; call `Clear()` first.");
    }

    void Register(TimerEntry& entry, chrono::Instant deadline) noexcept;
    void Cancel(TimerEntry& entry) noexcept;

    [[nodiscard]]
    auto NextDeadline() const noexcept -> Optional<chrono::Instant>;

    auto FireExpired(chrono::Instant now) -> UInt;
    void Clear() noexcept;

private:
    [[nodiscard]]
    auto getDeadlineTick(chrono::Instant deadline) const noexcept -> UInt64;

    [[nodiscard]]
    auto getNowTick(chrono::Instant now) const noexcept -> UInt64;

    chrono::Instant n_base;
    TimerWheel n_wheel;
};

} // namespace experimental::coro::internals::timers
} // namespace NOELDOC_HIDE violet
