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
//! # 🌺💜 `violet/Experimental/Time/FakeClock.h`

#pragma once

#include <violet/Experimental/Mutex.h>
#include <violet/Experimental/Time/Clock.h>

namespace violet::experimental {

/// A [`Clock`] with manually-advanced time. Primarily used in unit tests.
///
/// Starts at a fixed epoch (wall: 2026-01-01T00:00:00Z, monotonic: zero) by default or provided by the constructor.
/// Time will only advance in this world when [`FakeClock::Advance`] or any of the `Set*` methods are called, so that
/// tests are fully determinstic.
///
/// ## Thread Safety
/// All methods are safe to call concurrently. Advancing time is atomic; concurrent [`FakeClock::Now()`]
/// calls are see the old or new value.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Time/FakeClock.h>
///
/// violet::experimental::FakeClock clock;
///
/// // actually advance time in this world
/// auto start = clock.Now();
/// clock.Advance(eous::chrono::Duration::Seconds(5));
///
/// VIOLET_ASSERT0((clock.Now() - start).AsSeconds(), 5);
/// ```
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") FakeClock final: public Clock {
    VIOLET_IMPLICIT FakeClock();
    VIOLET_IMPLICIT FakeClock(chrono::TimePoint tp);

    /// @see violet::experimental::Clock::Now()
    [[nodiscard]] auto Now() const -> chrono::Instant override;

    /// @see violet::experimental::Clock::WallNow()
    [[nodiscard]] auto WallNow() const -> chrono::TimePoint override;

    /// Advance both monotonic and wall clocks by `dur`.
    void Advance(chrono::Duration dur);

    /// Sets the monotonic time value to `time`. The wall-clock is unchanged.
    void SetMonotonic(chrono::Instant time);

    /// Set the wall-clock value to a specific value. The monotonic clock is unchanged.
    ///
    /// ## Remarks
    /// Setting wall-clock values backwards is allowed.
    void SetWall(chrono::TimePoint time);

private:
    mutable Mutex n_mux;
    chrono::Instant n_monotonic VIOLET_GUARDED_BY(n_mux);
    chrono::TimePoint n_wall VIOLET_GUARDED_BY(n_mux);
};

} // namespace violet::experimental
