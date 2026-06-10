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
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") TimePoint final {
    /// The underlying standard-library representation, [`std::chrono::system_clock::time_point`].
    using std_type = std::chrono::system_clock::time_point;

    /// Constructs a [`TimePoint`] at the Unix epoch (`1970-01-01T00:00:00Z`).
    constexpr VIOLET_IMPLICIT TimePoint() noexcept = default;

    /// Constructs a [`TimePoint`] from a [`std::chrono::system_clock::time_point`].
    ///
    /// The source is normalized to nanoseconds since the Unix epoch; platforms whose clock
    /// is coarser than nanoseconds (e.g. 100ns on Windows) widen exactly. Implicit so a
    /// `system_clock` time point can be passed wherever a [`TimePoint`] is expected.
    ///
    /// @param tp the wall-clock time point to wrap.
    constexpr VIOLET_IMPLICIT TimePoint(std_type tp) noexcept
        : n_ns_since_epoch(std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count())
    {
    }

    /// Constructs a [`TimePoint`] `secs` seconds after the Unix epoch.
    constexpr static auto FromUnixSeconds(Int64 secs) -> TimePoint
    {
        return TimePoint(secs * 1'000'000'000);
    }

    /// Constructs a [`TimePoint`] `millis` milliseconds after the Unix epoch.
    constexpr static auto FromUnixMillis(Int64 millis) -> TimePoint
    {
        return TimePoint(millis * 1'000'000);
    }

    /// Constructs a [`TimePoint`] `nanos` nanoseconds after the Unix epoch.
    constexpr static auto FromUnixNanos(Int64 nanos) -> TimePoint
    {
        return TimePoint(nanos);
    }

    /// Converts back to a [`std::chrono::system_clock::time_point`].
    ///
    /// On platforms whose clock is coarser than nanoseconds, the value is truncated to the
    /// native resolution.
    [[nodiscard]] constexpr auto ToStd() const -> std_type
    {
        return std_type(
            std::chrono::duration_cast<typename std_type::duration>(std::chrono::nanoseconds(this->n_ns_since_epoch)));
    }

    /// Returns the whole number of seconds since the Unix epoch, truncating toward zero.
    [[nodiscard]] constexpr auto ToUnixSeconds() const -> Int64
    {
        if (this->n_ns_since_epoch == 0) {
            return 0;
        }

        return this->n_ns_since_epoch / 1'000'000'000;
    }

    /// Returns the whole number of milliseconds since the Unix epoch, truncating toward zero.
    [[nodiscard]] constexpr auto ToUnixMillis() const -> Int64
    {
        if (this->n_ns_since_epoch == 0) {
            return 0;
        }

        return this->n_ns_since_epoch / 1'000'000;
    }

    /// Returns the number of nanoseconds since the Unix epoch.
    [[nodiscard]] constexpr auto ToUnixNanos() const -> Int64
    {
        return this->n_ns_since_epoch;
    }

    /// Adds `dur` to this [`TimePoint`], returning [`Nothing`] on overflow.
    ///
    /// The non-aborting counterpart to [`operator+`].
    ///
    /// @param dur the duration to advance by.
    /// @return the resulting time point, or [`Nothing`] if it would overflow the epoch range.
    constexpr auto CheckedAdd(Duration dur) const -> Optional<TimePoint>
    {
        auto result = numeric::CheckedAdd(this->n_ns_since_epoch, dur.AsNanos());
        if (result.HasValue()) {
            return TimePoint(result.Value());
        }

        return Nothing;
    }

    /// Subtracts `dur` from this [`TimePoint`], returning [`Nothing`] on overflow.
    ///
    /// The non-aborting counterpart to [`operator-`].
    ///
    /// @param dur the duration to rewind by.
    /// @return the resulting time point, or [`Nothing`] if it would overflow the epoch range.
    constexpr auto CheckedSub(Duration dur) const -> Optional<TimePoint>
    {
        auto result = numeric::CheckedSub(this->n_ns_since_epoch, dur.AsNanos());
        if (result.HasValue()) {
            return TimePoint(result.Value());
        }

        return Nothing;
    }

    /// Returns the [`Duration`] elapsed from `earlier` up to this [`TimePoint`].
    ///
    /// The non-aborting counterpart to `operator-(TimePoint)`. The result is negative when
    /// `earlier` is actually later than `this`.
    ///
    /// @param earlier the reference time point to measure from.
    /// @return the signed elapsed duration, or [`Nothing`] if the difference would overflow.
    constexpr auto CheckedDurationSince(TimePoint earlier) const -> Optional<Duration>
    {
        auto result = numeric::CheckedSub(this->n_ns_since_epoch, earlier.n_ns_since_epoch);
        if (result.HasValue()) {
            return Duration::Nanoseconds(result.Value());
        }

        return Nothing;
    }

    /// Parses a [`TimePoint`] from an ISO 8601 / RFC 3339 timestamp (e.g. `"2026-06-09T12:00:00Z"`).
    ///
    /// @param input the textual timestamp to parse.
    /// @return the parsed [`TimePoint`], or an error describing why parsing failed.
    static auto FromISO8601(Str input) -> anyhow::Result<TimePoint>;

    /// Formats this [`TimePoint`] as an ISO 8601 / RFC 3339 timestamp string.
    [[nodiscard]] auto IntoISO8601() const -> String;

    /// Returns the [`TimePoint`] reached by advancing this one by `dur`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at
    /// runtime; see [`CheckedAdd`] for a non-aborting alternative.
    constexpr auto operator+(Duration dur) const -> TimePoint
    {
        return TimePoint(detail::doCheckedAdd(this->n_ns_since_epoch, dur.AsNanos(), "TimePoint::operator+(Duration)"));
    }

    /// Returns the [`Duration`] formed by summing the two epoch offsets of `this` and `tp`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at runtime.
    constexpr auto operator+(TimePoint tp) const -> Duration
    {
        return Duration::Nanoseconds(
            detail::doCheckedAdd(this->n_ns_since_epoch, tp.n_ns_since_epoch, "TimePoint::operator-(TimePoint)"));
    }

    /// Returns the [`TimePoint`] reached by rewinding this one by `dur`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at
    /// runtime; see [`CheckedSub`] for a non-aborting alternative.
    constexpr auto operator-(Duration dur) const -> TimePoint
    {
        return TimePoint(detail::doCheckedSub(this->n_ns_since_epoch, dur.AsNanos(), "TimePoint::operator-(Duration)"));
    }

    /// Returns the [`Duration`] elapsed from `tp` up to this [`TimePoint`].
    ///
    /// Negative when `tp` is later than `this`. Overflow is a hard error during constant
    /// evaluation and a debug-build assertion at runtime; see [`CheckedDurationSince`] for a
    /// non-aborting alternative.
    constexpr auto operator-(TimePoint tp) const -> Duration
    {
        return Duration::Nanoseconds(
            detail::doCheckedSub(this->n_ns_since_epoch, tp.n_ns_since_epoch, "TimePoint::operator-(TimePoint)"));
    }

    /// Advances this [`TimePoint`] by `dur` in place, with the same overflow checking as [`operator+`].
    constexpr auto operator+=(Duration dur) -> TimePoint&
    {
        this->n_ns_since_epoch
            = detail::doCheckedAdd(this->n_ns_since_epoch, dur.AsNanos(), "TimePoint::operator+=(Duration)");

        return *this;
    }

    /// Rewinds this [`TimePoint`] by `dur` in place, with the same overflow checking as [`operator-`].
    constexpr auto operator-=(Duration dur) -> TimePoint&
    {
        this->n_ns_since_epoch
            = detail::doCheckedSub(this->n_ns_since_epoch, dur.AsNanos(), "TimePoint::operator+=(Duration)");

        return *this;
    }

    constexpr auto operator<=>(const TimePoint& other) const -> std::strong_ordering
    {
        return this->n_ns_since_epoch <=> other.n_ns_since_epoch;
    }

    constexpr auto operator<=>(const std_type& other) const -> std::strong_ordering
    {
        return this->n_ns_since_epoch <=> other.time_since_epoch().count();
    }

private:
    constexpr VIOLET_EXPLICIT TimePoint(Int64 ns)
        : n_ns_since_epoch(ns)
    {
    }

    Int64 n_ns_since_epoch{ };
};

} // namespace violet::experimental::chrono
