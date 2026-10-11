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

#include <violet/Experimental/Time/Clock.h>
#include <violet/Experimental/Time/Instant.h>
#include <violet/Experimental/Unique.h>
#include <violet/IO/Experimental/Error.h>

namespace violet::experimental::coro::internals::io {

struct Op;

struct DriverOptions { };

struct Driver {
    virtual ~Driver() = default;

    virtual auto TryNow(Op& op) noexcept -> bool = 0;
    virtual void Submit(Op& op) noexcept = 0;
    virtual void Orphan(Op* op) noexcept = 0;

    virtual void Park(Optional<chrono::Instant> deadline, const Clock& clock) noexcept = 0;
    virtual void Unpark() noexcept = 0;
};

/// Milliseconds to wait until `deadline`, rounded *up* so a park never ends before it (avoids spinning).
inline auto MillisUntil(chrono::Instant deadline, const Clock& clock) noexcept -> Int64
{
    const chrono::Instant now = clock.Now();
    if (deadline <= now) {
        return 0;
    }

    return ((deadline - now).AsNanos() + 999'999) / 1'000'000;
}

VIOLET_API
auto CreateDriver(DriverOptions& options) noexcept -> violet::io::experimental::Result<ptr::Unique<Driver>>;

} // namespace violet::experimental::coro::internals::io
