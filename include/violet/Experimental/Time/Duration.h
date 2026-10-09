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
//! # 🌺💜 `violet/Experimental/Time/Duration.h`
//! A signed span of time with nanosecond resolution.
//!
//! [`Duration`] is the unit of measurement for Violet's chrono framework. It is what you add
//! to an [`Instant`] or [`TimePoint`], what you get from subtracting two of them, and what timeouts
//! and sleeps take. It is a thin wrapper over [`std::chrono::nanoseconds`].
//!
//! ## Which time type do I want?
//! | Type          | Answers                       | Backed by                   | Monotonic |
//! | :------------ | :---------------------------- | --------------------------- | :-------- |
//! | [`Duration`]  | "how long?"                   | `std::chrono::nanoseconds`  | n/a       |
//! | [`Instant`]   | "how much time has passed?"   | `std::chrono::steady_clock` | yes       |
//! | [`TimePoint`] | "what time is it?"            | `std::chrono::system_clock` | no        |
//!
//! A `Duration` stores its tick count as an [`Int64`] number of nanoseconds, so it spans roughly
//! 292 years. Unlike Rust's [`std::time::Duration`], it is **signed**. Subtracting a later timestamp
//! from an earlier one produces a negative duration instead of failing.
//!
//! [`std::time::Duration`]: https://doc.rust-lang.org/std/time/struct.Duration.html
//!
//! ## Overflow
//! Arithmetic is checked in three tiers. Pick the one that matches how you want overflow handled:
//!
//! * **Operators** (`+`, `-`, `*`, unary `-`): overflow is a compile error during constant evaluation
//!   and a debug assertion at runtime. In release builds the check is skipped.
//! * **`Checked*`** ([`Duration::CheckedAdd`], [`Duration::CheckedSub`]): return [`Nothing`] on overflow.
//! * **`Saturating*`** ([`Duration::SaturatingAdd`], [`Duration::SaturatingSub`]): clamp to
//!   [`Duration::Max`] or [`Duration::Min`].
//!
//! ## Conversions
//! * **Construct** with [`Duration::Seconds`], [`Duration::Milliseconds`], etc., or with any
//!   `std::chrono::duration`. Sources finer than a nanosecond are truncated toward zero.
//! * **Read back** with `As*` ([`Duration::AsMillis`], [`Duration::AsSeconds`], ...), which truncate
//!   toward zero, or with [`Duration::Cast`] for an arbitrary `std::chrono::duration`.
//! * **Text**: [`Duration::FromStr`] parses strings like `"5s"` or `"250ms"`, and [`Duration::ToString`]
//!   (also used by `operator<<` and the formatter) produces them.

#pragma once

#include <violet/Experimental/Numeric.h>

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
#include "absl/time/time.h"
#endif

#include <chrono>
#include <ctime>

namespace violet::experimental::chrono {
namespace internals {

template<std::integral T>
    requires std::is_signed_v<T>
constexpr auto doCheckedAdd(T one, T two, CStr context) noexcept -> T
{
    auto result = numeric::CheckedAdd<T>(one, two);
    VIOLET_DEBUG_ASSERT_FMT(
        result.HasValue(), "duration arithemtic overflow at runtime ({} + {}): {}", one, two, context);

    return result.UnwrapUnchecked(Unsafe("checked at debug time"));
}

template<std::integral T>
    requires std::is_signed_v<T>
constexpr auto doCheckedSub(T one, T two, CStr context) noexcept -> T
{
    auto result = numeric::CheckedSub<T>(one, two);
    VIOLET_DEBUG_ASSERT_FMT(
        result.HasValue(), "duration arithemtic overflow at runtime ({} - {}): {}", one, two, context);

    return result.UnwrapUnchecked(Unsafe("checked at debug time"));
}

template<std::integral T>
    requires std::is_signed_v<T>
constexpr auto doCheckedMul(T one, T two, CStr context) noexcept -> T
{
    auto result = numeric::CheckedMul<T>(one, two);
    VIOLET_DEBUG_ASSERT_FMT(
        result.HasValue(), "duration arithemtic overflow at runtime ({} * {}): {}", one, two, context);

    return result.UnwrapUnchecked(Unsafe("checked at debug time"));
}

} // namespace internals

/// A span of time with nanosecond resolution.
///
/// Check the [module documentation](#) for more information.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("26.06.05") Duration final {
    /// The underlying standard-library representation, [`std::chrono::nanoseconds`].
    using std_type = std::chrono::nanoseconds;

    /// The integral type storing the nanosecond tick count, a signed 64-bit integer.
    using rep = Int64;

    /// Constructs a zero-length [`Duration`].
    constexpr VIOLET_IMPLICIT Duration() = default;

    /// Constructs a [`Duration`] from any [`std::chrono::duration`].
    ///
    /// The source duration is cast to nanosecond resolution. Coarser units (seconds,
    /// milliseconds, ...) convert exactly; finer-resolution sources are truncated toward
    /// zero by [`std::chrono::duration_cast`]. Implicit so that `std::chrono` literals can
    /// be passed wherever a [`Duration`] is expected.
    ///
    /// @param dur the source duration to wrap.
    template<typename Rep, typename Period>
    constexpr VIOLET_IMPLICIT Duration(std::chrono::duration<Rep, Period> dur)
        : n_ns(std::chrono::duration_cast<std_type>(dur))
    {
    }

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
    /// Constructs a [`Duration`] from [`absl::Duration`].
    NOELDOC_SINCE("current")
    VIOLET_IMPLICIT Duration(absl::Duration dur)
        : n_ns(absl::ToChronoNanoseconds(dur))
    {
    }
#endif

    /// Returns a zero-length [`Duration`], equivalent to a default-constructed one.
    constexpr static auto Zero() -> Duration
    {
        return {std_type::zero()};
    }

    /// Constructs a [`Duration`] spanning `ns` nanoseconds.
    constexpr static auto Nanoseconds(rep ns) -> Duration
    {
        return {std_type(ns)};
    }

    /// Constructs a [`Duration`] spanning `ns` microseconds.
    constexpr static auto Microseconds(rep ns) -> Duration
    {
        return {std::chrono::microseconds(ns)};
    }

    /// Constructs a [`Duration`] spanning `ns` milliseconds.
    constexpr static auto Milliseconds(rep ns) -> Duration
    {
        return {std::chrono::milliseconds(ns)};
    }

    /// Constructs a [`Duration`] spanning `ns` seconds.
    constexpr static auto Seconds(rep ns) -> Duration
    {
        return {std::chrono::seconds(ns)};
    }

    /// Constructs a [`Duration`] spanning `ns` minutes.
    constexpr static auto Minutes(rep ns) -> Duration
    {
        return {std::chrono::minutes(ns)};
    }

    /// Constructs a [`Duration`] spanning `ns` hours.
    constexpr static auto Hours(rep ns) -> Duration
    {
        return {std::chrono::hours(ns)};
    }

    /// Returns the largest representable [`Duration`].
    constexpr static auto Max() -> Duration
    {
        return {std_type::max()};
    }

    /// Returns the smallest (most negative) representable [`Duration`].
    constexpr static auto Min() -> Duration
    {
        return {std_type::min()};
    }

    /// Converts a [`time_t`] struct into Violet's `Duration` object, otherwise `Nothing` is returned
    /// if the conversion failed.
    NOELDOC_SINCE("current")
    constexpr static auto TryFrom(::time_t tm) noexcept -> Optional<Duration>
    {
        constexpr auto kMaxSeconds = std_type::max().count() / 1'000'000'000;
        constexpr auto kMinSeconds = std_type::min().count() / 1'000'000'000;

        if (tm > kMaxSeconds || tm < kMinSeconds) {
            return Nothing;
        }

        return {std::chrono::seconds(tm)};
    }

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
    /// Converts a [`absl::Duration`] struct into Violet's `Duration` object, otherwise `Nothing` is returned
    /// if the conversion failed.
    NOELDOC_SINCE("current")
    constexpr static auto TryFrom(absl::Duration dur) noexcept -> Optional<Duration>
    {
        constexpr auto kMax = absl::Nanoseconds(std::numeric_limits<Int64>::max());
        constexpr auto kMin = absl::Nanoseconds(std::numeric_limits<Int64>::min());
        if (dur > kMax || dur < kMin) {
            return Nothing;
        }

        return Duration(absl::ToInt64Nanoseconds(dur));
    }
#endif

    /// Parses a [`Duration`] from a human-readable string such as `"5s"` or `"250ms"`.
    ///
    /// @param input the textual duration to parse.
    /// @return the parsed [`Duration`], or an error describing why parsing failed.
    static auto FromStr(Str input) noexcept -> anyhow::Result<Duration>;

    /// Casts this [`Duration`] to an arbitrary [`std::chrono::duration`] type `Dur`.
    ///
    /// A thin wrapper over [`std::chrono::duration_cast`]; conversions to a coarser
    /// resolution truncate toward zero.
    ///
    /// ```cpp
    /// auto d = Duration::Milliseconds(5250);
    /// VIOLET_ASSERT0(d.Cast<std::chrono::seconds>().count() == 5);
    /// ```
    template<typename Dur>
    [[nodiscard]] constexpr auto Cast() const -> Dur
    {
        return std::chrono::duration_cast<Dur>(this->n_ns);
    }

    /// Returns the whole number of nanoseconds in this [`Duration`].
    [[nodiscard]] constexpr auto AsNanos() const -> rep
    {
        return this->n_ns.count();
    }

    /// Returns the whole number of microseconds, truncating any sub-microsecond remainder.
    [[nodiscard]] constexpr auto AsMicros() const -> rep
    {
        return this->Cast<std::chrono::microseconds>().count();
    }

    /// Returns the whole number of milliseconds, truncating any sub-millisecond remainder.
    [[nodiscard]] constexpr auto AsMillis() const -> rep
    {
        return this->Cast<std::chrono::milliseconds>().count();
    }

    /// Returns the whole number of seconds, truncating any sub-second remainder.
    [[nodiscard]] constexpr auto AsSeconds() const -> rep
    {
        return this->Cast<std::chrono::seconds>().count();
    }

    /// Returns the whole number of minutes, truncating any sub-minute remainder.
    [[nodiscard]] constexpr auto AsMinutes() const -> rep
    {
        return this->Cast<std::chrono::minutes>().count();
    }

    /// Returns the whole number of hours, truncating any sub-hour remainder.
    [[nodiscard]] constexpr auto AsHours() const -> rep
    {
        return this->Cast<std::chrono::hours>().count();
    }

    /// Returns `true` if this [`Duration`] is exactly zero-length.
    [[nodiscard]] constexpr auto Zeroed() const noexcept -> bool
    {
        return this->n_ns.count() == 0;
    }

    /// Formats this [`Duration`] as a human-readable string (e.g. `"5s"`, `"250ms"`).
    [[nodiscard]]
    NOELDOC_SINCE("26.07.03") auto ToString() const -> String;

    /// Writes the result of [`ToString`] to the output stream `os`.
    friend auto operator<<(std::ostream& os, const Duration& self) -> std::ostream&
    {
        return os << self.ToString();
    }

    /// Adds `other` to this [`Duration`], returning [`Nothing`] on overflow.
    ///
    /// The non-aborting counterpart to [`operator+`]: instead of failing on overflow,
    /// the result is reported through the [`Optional`].
    ///
    /// @param other the duration to add.
    /// @return the sum, or [`Nothing`] if it would overflow the nanosecond range.
    constexpr auto CheckedAdd(Duration other) const -> Optional<Duration>
    {
        return numeric::CheckedAdd<rep>(this->AsNanos(), other.AsNanos()).Map([](auto rep) -> Duration {
            return Duration(rep);
        });
    }

    /// Adds `other` to this [`Duration`], clamping to [`Max`]/[`Min`] instead of overflowing.
    ///
    /// On positive overflow the result saturates to [`Max`]; on negative overflow it
    /// saturates to [`Min`].
    ///
    /// @param other the duration to add.
    /// @return the saturated sum.
    [[nodiscard]]
    constexpr auto SaturatingAdd(Duration other) const -> Duration
    {
        return numeric::CheckedAdd<rep>(this->AsNanos(), other.AsNanos())
            .Map([](auto rep) -> Duration { return Duration(rep); })
            .UnwrapOr(this->AsNanos() > 0 ? Duration::Max() : Duration::Min());
    }

    /// Subtracts `other` from this [`Duration`], returning [`Nothing`] on overflow.
    ///
    /// The non-aborting counterpart to [`operator-`]: instead of failing on overflow,
    /// the result is reported through the [`Optional`].
    ///
    /// @param other the duration to subtract.
    /// @return the difference, or [`Nothing`] if it would overflow the nanosecond range.
    constexpr auto CheckedSub(Duration other) const -> Optional<Duration>
    {
        return numeric::CheckedSub<rep>(this->AsNanos(), other.AsNanos()).Map([](auto rep) -> Duration {
            return Duration(rep);
        });
    }

    /// Subtracts `other` from this [`Duration`], clamping to [`Max`]/[`Min`] instead of overflowing.
    ///
    /// On positive overflow the result saturates to [`Max`]; on negative overflow it
    /// saturates to [`Min`].
    ///
    /// @param other the duration to subtract.
    /// @return the saturated difference.
    [[nodiscard]]
    constexpr auto SaturatingSub(Duration other) const -> Duration
    {
        return numeric::CheckedSub<rep>(this->AsNanos(), other.AsNanos())
            .Map([](auto rep) -> Duration { return Duration(rep); })
            .UnwrapOr(this->AsNanos() > 0 ? Duration::Max() : Duration::Min());
    }

    /// Returns the underlying [`std::chrono::nanoseconds`] value.
    [[nodiscard]]
    constexpr auto ToStd() const noexcept -> std_type
    {
        return this->n_ns;
    }

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
    /// Returns a [`absl::Duration`] from this object.
    [[nodiscard]]
    NOELDOC_SINCE("current") constexpr auto ToAbsl() const noexcept -> absl::Duration
    {
        return absl::FromChrono(this->n_ns);
    }
#endif

    /// Returns a [`std::time_t`] from this object.
    [[nodiscard]]
    NOELDOC_SINCE("current") constexpr auto ToTimeT() const noexcept -> ::time_t
    {
        return static_cast<::time_t>(std::chrono::floor<std::chrono::seconds>(this->n_ns).count());
    }

    /// Returns `true` if this [`Duration`] is non-zero; the inverse of [`Zeroed`].
    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return !this->Zeroed();
    }

    /// Explicitly converts to the underlying [`std::chrono::nanoseconds`]; see [`ToStd`].
    constexpr VIOLET_EXPLICIT operator std_type() const noexcept
    {
        return this->ToStd();
    }

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
    /// Explicitly converts to [`absl::Duration`]
    NOELDOC_SINCE("current")
    constexpr VIOLET_EXPLICIT operator absl::Duration() const noexcept
    {
        return this->ToAbsl();
    }
#endif

    /// Explicitly converts to [`std::time_t`]
    NOELDOC_SINCE("current")
    constexpr VIOLET_EXPLICIT operator ::time_t() const noexcept
    {
        return this->ToTimeT();
    }

    /// Returns the sum of this [`Duration`] and `other`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at
    /// runtime. Use [`CheckedAdd`] or [`SaturatingAdd`] for non-aborting alternatives.
    constexpr auto operator+(Duration other) const -> Duration
    {
        return {std_type(internals::doCheckedAdd(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+"))};
    }

    /// Returns the difference of this [`Duration`] and `other`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at
    /// runtime. Use [`CheckedSub`] or [`SaturatingSub`] for non-aborting alternatives.
    constexpr auto operator-(Duration other) const -> Duration
    {
        return {std_type(internals::doCheckedSub(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator-"))};
    }

    /// Adds `other` into this [`Duration`] in place, with the same overflow checking as [`operator+`].
    constexpr auto operator+=(Duration other) -> Duration&
    {
        this->n_ns = std_type(internals::doCheckedAdd(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+="));

        return *this;
    }

    /// Subtracts `other` from this [`Duration`] in place, with the same overflow checking as [`operator-`].
    constexpr auto operator-=(Duration other) -> Duration&
    {
        this->n_ns = std_type(internals::doCheckedSub(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+="));

        return *this;
    }

    /// Returns this [`Duration`] scaled by the integer factor `ns`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at runtime.
    constexpr auto operator*(rep ns) const -> Duration
    {
        return {std_type(
            internals::doCheckedMul(this->AsNanos(), ns, "violet::experimental::chrono::Duration::operator*"))};
    }

    /// Returns this [`Duration`] divided by the integer divisor `ns`, truncating toward zero.
    ///
    /// Dividing by zero trips a debug-build assertion.
    constexpr auto operator/(rep ns) const -> Duration
    {
        VIOLET_DEBUG_ASSERT(ns != 0, "divide by zero");
        return Duration::Nanoseconds(this->AsNanos() / ns);
    }

    /// Returns the negation of this [`Duration`].
    ///
    /// Negating [`Min`] (i.e. `Int64::MIN` nanoseconds) overflows: a hard error during
    /// constant evaluation and a debug-build assertion at runtime.
    constexpr auto operator-() const -> Duration
    {
        VIOLET_DEBUG_ASSERT(this->AsNanos() != std::numeric_limits<rep>::min(), "negation of Int64::MIN");
        return Duration::Nanoseconds(-this->AsNanos());
    }

    constexpr auto operator<=>(const Duration&) const -> std::strong_ordering = default;

private:
    VIOLET_EXPLICIT Duration(Int64 ns) noexcept
        : n_ns(ns)
    {
    }

    std_type n_ns{};
};

} // namespace violet::experimental::chrono

VIOLET_FORMATTER_SINCE("26.07.03", violet::experimental::chrono::Duration);
