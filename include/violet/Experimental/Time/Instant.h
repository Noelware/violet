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
//! # 🌺💜 `violet/Experimental/Time/Instant.h`

#pragma once

#include <violet/Experimental/Time/Duration.h>

#include <chrono>

namespace violet::experimental {
struct Clock;
}

namespace violet::experimental::chrono {

/// A monotonic timestamp.
///
/// Backed by [`std::chrono::steady_clock`], which guarantees the clock is strictly
/// non-decreasing and unaffected by wall-clock adjustments (NTP, manual time changes,
/// daylight savings). Use [`Instant`] for timeouts, deadlines, and elapsed-time measurements;
/// anywhere you don't care *what time it is* but *how much time has passed*
///
/// ## Interaction with `TimePoint`
/// **Instant** is deliberately not convertible to or from a [`TimePoint`] object. The two represent
/// fundamentally different notions of time: an `Instant` is meaningful only within a single process'
/// lifetime (it is not serializable, send it over the network, or compare across reboots). A [`TimePoint`]
/// is a wall-clock moment that can jump, skew, or go backwards.
///
/// ## Examples
/// ```cpp
/// #include <violet/Experimental/Time/Clock.h>
/// #include <violet/Experimental/Time/Instant.h>
///
/// using namespace violet::experimental;
/// using namespace violet::experimental::chrono;
///
/// auto deadline = Clock::System().Now() + Duration::Seconds(30);
/// while (Clock::System().Now() < deadline) {
///     doWork();
/// }
/// ```
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") Instant final {
    /// The C++-backed type for this object.
    using std_type = std::chrono::steady_clock::time_point;

    constexpr VIOLET_IMPLICIT Instant() noexcept = default;
    constexpr VIOLET_IMPLICIT Instant(std_type tp)
        : n_tp(tp)
    {
    }

    /// Returns the C++-backed implementation that [`Instant`] uses.
    [[nodiscard]] constexpr auto ToStd() const -> std_type
    {
        return this->n_tp;
    }

    /// The duration since `earlier`. If `earlier` is later than `this`, it returns zero.
    [[nodiscard]] constexpr auto DurationSince(Instant earlier) const -> Duration
    {
        return this->n_tp < earlier.n_tp ? Duration::Zero() : Duration(this->n_tp - earlier.n_tp);
    }

    /// The elapsed time since this instant, measured against a given `clock`. Returns
    /// a zeroed duration if this instant is in the future.
    [[nodiscard]] auto Elapsed(const Clock& clock) const -> Duration;

    constexpr auto operator+(Duration dur) const -> Instant
    {
        return { this->n_tp + dur.ToStd() };
    }

    constexpr auto operator-(Duration dur) const -> Instant
    {
        return { this->n_tp - dur.ToStd() };
    }

    constexpr auto operator-(Instant other) const -> Duration
    {
        return { this->n_tp - other.n_tp };
    }

    constexpr auto operator+=(Duration dur) -> Instant&
    {
        this->n_tp += dur.ToStd();
        return *this;
    }

    constexpr auto operator<=>(const Instant& other) const -> std::strong_ordering
    {
        return this->n_tp <=> other.n_tp;
    }

    constexpr auto operator<=>(const std_type& other) const -> std::strong_ordering
    {
        return this->n_tp <=> other;
    }

private:
    std_type n_tp;
};
} // namespace violet::experimental::chrono
