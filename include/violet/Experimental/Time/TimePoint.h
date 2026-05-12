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
//! # 🌺💜 `violet/Experimental/Time/TimePoint.h`

#pragma once

#include <violet/Experimental/Time/Duration.h>

namespace violet::experimental::chrono {

/// A wall-clock timestamp.
///
/// Backed by [`std::chrono::system_clock::time_point`], which represents
/// a point on the real-world timelime. Subjected to NTP adjustments,
/// manual clock changes, and leap-second smearing; it is *not* monotonic. **TimePoint**
/// is meant to be used for log timestamps, user-facing formatting,
/// or anything that needs to mean the same thing across processes or restarts.
///
/// ## Representation
/// **TimePoint** internally stores a [`system_clock::time_point`] with the platform's
/// native resolution (nanoseconds on Linux/macOS, 100ns on Windows). All conversions
/// to and from [`Int64`](Int64) Unix epoch are computed in nanoseconds.
///
/// [`system_clock::time_point`]: std::chrono::system_clock::time_point
struct TimePoint final {
    using std_type = std::chrono::system_clock::time_point;

    constexpr VIOLET_IMPLICIT TimePoint() noexcept = default;
    constexpr VIOLET_IMPLICIT TimePoint(std_type tp) noexcept
        : n_ns_since_epoch(std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count())
    {
    }

    constexpr static auto FromUnixSeconds(Int64 secs) -> TimePoint
    {
        return TimePoint(secs * 1'000'000'000);
    }

    constexpr static auto FromUnixMillis(Int64 millis) -> TimePoint
    {
        return TimePoint(millis * 1'000'000);
    }

    constexpr static auto FromUnixNanos(Int64 nanos) -> TimePoint
    {
        return TimePoint(nanos);
    }

    [[nodiscard]] constexpr auto ToStd() const -> std_type
    {
        return std_type(
            std::chrono::duration_cast<typename std_type::duration>(std::chrono::nanoseconds(this->n_ns_since_epoch)));
    }

    [[nodiscard]] constexpr auto ToUnixSeconds() const -> Int64
    {
        if (this->n_ns_since_epoch == 0) {
            return 0;
        }

        return this->n_ns_since_epoch / 1'000'000'000;
    }

    [[nodiscard]] constexpr auto ToUnixMillis() const -> Int64
    {
        if (this->n_ns_since_epoch == 0) {
            return 0;
        }

        return this->n_ns_since_epoch / 1'000'000;
    }

    [[nodiscard]] constexpr auto ToUnixNanos() const -> Int64
    {
        return this->n_ns_since_epoch;
    }

    constexpr auto CheckedAdd(Duration dur) const -> Optional<TimePoint>
    {
        auto result = numeric::CheckedAdd(this->n_ns_since_epoch, dur.AsNanos());
        if (result.HasValue()) {
            return TimePoint(result.Value());
        }

        return Nothing;
    }

    constexpr auto CheckedSub(Duration dur) const -> Optional<TimePoint>
    {
        auto result = numeric::CheckedSub(this->n_ns_since_epoch, dur.AsNanos());
        if (result.HasValue()) {
            return TimePoint(result.Value());
        }

        return Nothing;
    }

    constexpr auto CheckedDurationSince(TimePoint earlier) const -> Optional<Duration>
    {
        auto result = numeric::CheckedSub(this->n_ns_since_epoch, earlier.n_ns_since_epoch);
        if (result.HasValue()) {
            return Duration::Nanoseconds(result.Value());
        }

        return Nothing;
    }

    static auto FromISO8601(Str input) -> anyhow::Result<TimePoint>;
    [[nodiscard]] auto IntoISO8601() const -> String;

    constexpr auto operator+(Duration dur) const -> TimePoint
    {
        return TimePoint(detail::doCheckedAdd(this->n_ns_since_epoch, dur.AsNanos(), "TimePoint::operator+(Duration)"));
    }

    constexpr auto operator+(TimePoint tp) const -> Duration
    {
        return Duration::Nanoseconds(
            detail::doCheckedAdd(this->n_ns_since_epoch, tp.n_ns_since_epoch, "TimePoint::operator-(TimePoint)"));
    }

    constexpr auto operator-(Duration dur) const -> TimePoint
    {
        return TimePoint(detail::doCheckedSub(this->n_ns_since_epoch, dur.AsNanos(), "TimePoint::operator-(Duration)"));
    }

    constexpr auto operator-(TimePoint tp) const -> Duration
    {
        return Duration::Nanoseconds(
            detail::doCheckedSub(this->n_ns_since_epoch, tp.n_ns_since_epoch, "TimePoint::operator-(TimePoint)"));
    }

    constexpr auto operator+=(Duration dur) -> TimePoint&
    {
        this->n_ns_since_epoch
            = detail::doCheckedAdd(this->n_ns_since_epoch, dur.AsNanos(), "TimePoint::operator+=(Duration)");

        return *this;
    }

    constexpr auto operator-=(Duration dur) -> TimePoint&
    {
        this->n_ns_since_epoch
            = detail::doCheckedSub(this->n_ns_since_epoch, dur.AsNanos(), "TimePoint::operator+=(Duration)");

        return *this;
    }

    constexpr auto operator<=>(const TimePoint&) const -> std::strong_ordering = default;

private:
    constexpr VIOLET_EXPLICIT TimePoint(Int64 ns)
        : n_ns_since_epoch(ns)
    {
    }

    Int64 n_ns_since_epoch{ };
};

} // namespace violet::experimental::chrono
