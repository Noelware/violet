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

#pragma once

#include <violet/Experimental/Numeric.h>

#include <chrono>

namespace violet::experimental::chrono {
namespace detail {

    template<std::integral T>
        requires std::is_signed_v<T>
    constexpr auto doCheckedAdd(T one, T two, CStr context) -> T
    {
        Optional<T> result = numeric::CheckedAdd(one, two);
        if VIOLET_IF_CONSTEVAL {
#if VIOLET_FEATURE(EXCEPTIONS)
            if (!result.HasValue()) {
                throw "duration arithemtic overflow at compilation time";
            }
#else
            VIOLET_ASSERT_FMT(result.HasValue(), "duration arithemtic overflow at compilation time: {}", context);
#endif
        } else {
            VIOLET_DEBUG_ASSERT_FMT(
                result.HasValue(), "duration arithemtic overflow at runtime ({} + {}): {}", one, two, context);
        }

        return result.UnwrapUnchecked(Unsafe("checked beforehand"));
    }

    template<std::integral T>
        requires std::is_signed_v<T>
    constexpr auto doCheckedSub(T one, T two, CStr context) -> T
    {
        Optional<T> result = numeric::CheckedSub(one, two);
        if VIOLET_IF_CONSTEVAL {
#if VIOLET_FEATURE(EXCEPTIONS)
            if (!result.HasValue()) {
                throw "duration arithemtic overflow at compilation time";
            }
#else
            VIOLET_ASSERT_FMT(result.HasValue(), "duration arithemtic overflow at compilation time: {}", context);
#endif
        } else {
            VIOLET_DEBUG_ASSERT_FMT(
                result.HasValue(), "duration arithemtic overflow at runtime ({} - {}): {}", one, two, context);
        }

        return result.UnwrapUnchecked(Unsafe("checked beforehand"));
    }

    template<std::integral T>
        requires std::is_signed_v<T>
    constexpr auto doCheckedMul(T one, T two, CStr context) -> T
    {
        Optional<T> result = numeric::CheckedMul(one, two);
        if VIOLET_IF_CONSTEVAL {
#if VIOLET_FEATURE(EXCEPTIONS)
            if (!result.HasValue()) {
                throw "duration arithemtic overflow at compilation time";
            }
#else
            VIOLET_ASSERT_FMT(result.HasValue(), "duration arithemtic overflow at compilation time: {}", context);
#endif
        } else {
            VIOLET_DEBUG_ASSERT_FMT(
                result.HasValue(), "duration arithemtic overflow at runtime ({} * {}): {}", one, two, context);
        }

        return result.UnwrapUnchecked(Unsafe("checked beforehand"));
    }
} // namespace detail

/// A span of time with nanosecond resolution.
///
/// **Duration** is a thin wrapper over C++'s [`std::chrono::nanoseconds`] with a preferred,
/// simpler API that is cheap to copy, trivially comparable, and safe to use in arithmetic. All operations
/// are checked in debug builds.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Time/Duration.h>
///
/// using violet::experimental::chrono::Duration;
/// using namespace std::chrono_literals;
///
/// // From C++'s `std::chrono_literals`
/// Duration timeout(5s);
/// auto total = timeout + Duration::From(250ms);
/// VIOLET_ASSERT0(total.Cast<std::chrono::milliseconds>() == 5250);
///
/// // From static functions
/// Duration timeout = Duration::Seconds(5);
/// auto total = timeout + Duration::Milliseconds(250);
/// VIOLET_ASSERT0(timeout.AsMillis() == 5250);
/// ```
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") Duration final {
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

    /// Returns a zero-length [`Duration`], equivalent to a default-constructed one.
    constexpr static auto Zero() -> Duration
    {
        return { std_type::zero() };
    }

    /// Constructs a [`Duration`] spanning `ns` nanoseconds.
    constexpr static auto Nanoseconds(rep ns) -> Duration
    {
        return { std_type(ns) };
    }

    /// Constructs a [`Duration`] spanning `ns` microseconds.
    constexpr static auto Microseconds(rep ns) -> Duration
    {
        return { std::chrono::microseconds(ns) };
    }

    /// Constructs a [`Duration`] spanning `ns` milliseconds.
    constexpr static auto Milliseconds(rep ns) -> Duration
    {
        return { std::chrono::milliseconds(ns) };
    }

    /// Constructs a [`Duration`] spanning `ns` seconds.
    constexpr static auto Seconds(rep ns) -> Duration
    {
        return { std::chrono::seconds(ns) };
    }

    /// Constructs a [`Duration`] spanning `ns` minutes.
    constexpr static auto Minutes(rep ns) -> Duration
    {
        return { std::chrono::minutes(ns) };
    }

    /// Constructs a [`Duration`] spanning `ns` hours.
    constexpr static auto Hours(rep ns) -> Duration
    {
        return { std::chrono::hours(ns) };
    }

    /// Returns the largest representable [`Duration`].
    constexpr static auto Max() -> Duration
    {
        return { std_type::max() };
    }

    /// Returns the smallest (most negative) representable [`Duration`].
    constexpr static auto Min() -> Duration
    {
        return { std_type::min() };
    }

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
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto ToString() const -> violet::String;
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
    constexpr auto CheckedAdd(Duration other) const -> violet::Optional<Duration>
    {
        auto result = numeric::CheckedAdd<rep>(this->AsNanos(), other.AsNanos());
        if (result.HasValue()) {
            return violet::Some(Duration::Nanoseconds(result.Value()));
        }

        return violet::Nothing;
    }

    /// Adds `other` to this [`Duration`], clamping to [`Max`]/[`Min`] instead of overflowing.
    ///
    /// On positive overflow the result saturates to [`Max`]; on negative overflow it
    /// saturates to [`Min`].
    ///
    /// @param other the duration to add.
    /// @return the saturated sum.
    [[nodiscard]] constexpr auto SaturatingAdd(Duration other) const -> Duration
    {
        auto result = numeric::CheckedAdd<rep>(this->AsNanos(), other.AsNanos());
        if (result.HasValue()) {
            return Duration::Nanoseconds(result.Value());
        }

        return this->AsNanos() > 0 ? Duration::Max() : Duration::Min();
    }

    /// Subtracts `other` from this [`Duration`], returning [`Nothing`] on overflow.
    ///
    /// The non-aborting counterpart to [`operator-`]: instead of failing on overflow,
    /// the result is reported through the [`Optional`].
    ///
    /// @param other the duration to subtract.
    /// @return the difference, or [`Nothing`] if it would overflow the nanosecond range.
    constexpr auto CheckedSub(Duration other) const -> violet::Optional<Duration>
    {
        auto result = numeric::CheckedSub<rep>(this->AsNanos(), other.AsNanos());
        if (result.HasValue()) {
            return violet::Some(Duration::Nanoseconds(result.Value()));
        }

        return violet::Nothing;
    }

    /// Subtracts `other` from this [`Duration`], clamping to [`Max`]/[`Min`] instead of overflowing.
    ///
    /// On positive overflow the result saturates to [`Max`]; on negative overflow it
    /// saturates to [`Min`].
    ///
    /// @param other the duration to subtract.
    /// @return the saturated difference.
    [[nodiscard]] constexpr auto SaturatingSub(Duration other) const -> Duration
    {
        auto result = numeric::CheckedSub<rep>(this->AsNanos(), other.AsNanos());
        if (result.HasValue()) {
            return Duration::Nanoseconds(result.Value());
        }

        return this->AsNanos() > 0 ? Duration::Max() : Duration::Min();
    }

    /// Returns the underlying [`std::chrono::nanoseconds`] value.
    [[nodiscard]] constexpr auto ToStd() const noexcept -> std_type
    {
        return this->n_ns;
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

    /// Returns the sum of this [`Duration`] and `other`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at
    /// runtime. Use [`CheckedAdd`] or [`SaturatingAdd`] for non-aborting alternatives.
    constexpr auto operator+(Duration other) const -> Duration
    {
        return { std_type(detail::doCheckedAdd(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+")) };
    }

    /// Returns the difference of this [`Duration`] and `other`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at
    /// runtime. Use [`CheckedSub`] or [`SaturatingSub`] for non-aborting alternatives.
    constexpr auto operator-(Duration other) const -> Duration
    {
        return { std_type(detail::doCheckedSub(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator-")) };
    }

    /// Adds `other` into this [`Duration`] in place, with the same overflow checking as [`operator+`].
    constexpr auto operator+=(Duration other) -> Duration&
    {
        this->n_ns = std_type(detail::doCheckedAdd(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+="));

        return *this;
    }

    /// Subtracts `other` from this [`Duration`] in place, with the same overflow checking as [`operator-`].
    constexpr auto operator-=(Duration other) -> Duration&
    {
        this->n_ns = std_type(detail::doCheckedSub(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+="));

        return *this;
    }

    /// Returns this [`Duration`] scaled by the integer factor `ns`.
    ///
    /// Overflow is a hard error during constant evaluation and a debug-build assertion at runtime.
    constexpr auto operator*(rep ns) const -> Duration
    {
        return { std_type(
            detail::doCheckedMul(this->AsNanos(), ns, "violet::experimental::chrono::Duration::operator*")) };
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
        if VIOLET_IF_CONSTEVAL {
#if VIOLET_FEATURE(EXCEPTIONS)
            if (this->AsNanos() == std::numeric_limits<rep>::min()) {
                throw "overflow detected in `operator-()`";
            }
#else
            VIOLET_DEBUG_ASSERT(this->AsNanos() != std::numeric_limits<rep>::min(), "negation of Int64::MIN");
#endif
        } else {
            VIOLET_DEBUG_ASSERT(this->AsNanos() != std::numeric_limits<rep>::min(), "negation of Int64::MIN");
        }

        return Duration::Nanoseconds(-this->AsNanos());
    }

    constexpr auto operator<=>(const Duration&) const -> std::strong_ordering = default;

private:
    std_type n_ns{ };
};

} // namespace violet::experimental::chrono

VIOLET_FORMATTER_SINCE("26.07.03", violet::experimental::chrono::Duration);
