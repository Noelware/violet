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
//! # 🌺💜 `violet/Experimental/Time/Clock.h`

#pragma once

#include <violet/Experimental/Time/Duration.h>
#include <violet/Experimental/Time/Instant.h>
#include <violet/Experimental/Time/TimePoint.h>

namespace violet::experimental {

/// A source of time.
///
/// ## Thread Safety
/// Implementations must be safe to call concurrently from any thread. Both the built-in
/// system clock, wall clock and mock-fake clock are thread-safe.
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") Clock {
    virtual ~Clock() = default;

    /// Returns the system-implementation clock.
    static auto System() noexcept -> Clock&;

    /// Current wall-clock timestamp. Subject to adjustment.
    [[nodiscard]] virtual auto WallNow() const -> chrono::TimePoint = 0;

    /// Current monotonic timestamp. Never goes backwards within a
    /// single process.
    [[nodiscard]] virtual auto Now() const -> chrono::Instant = 0;
};

/// A real clock backed by C++'s [`std::chrono::steady_clock`] and [`std::chrono::system_clock`]
/// primitives.
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") SystemClock final: public Clock {
    /// @see violet::experimental::Clock::Now()
    [[nodiscard]] auto Now() const -> chrono::Instant override;

    /// @see violet::experimental::Clock::WallNow()
    [[nodiscard]] auto WallNow() const -> chrono::TimePoint override;
};

} // namespace violet::experimental
