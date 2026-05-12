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
struct Duration final {
    using std_type = std::chrono::nanoseconds;
    using rep = Int64;

    constexpr VIOLET_IMPLICIT Duration() = default;

    template<typename Rep, typename Period>
    constexpr VIOLET_IMPLICIT Duration(std::chrono::duration<Rep, Period> dur)
        : n_ns(std::chrono::duration_cast<std_type>(dur))
    {
    }

    constexpr static auto Zero() -> Duration
    {
        return { std_type::zero() };
    }

    constexpr static auto Nanoseconds(rep ns) -> Duration
    {
        return { std_type(ns) };
    }

    constexpr static auto Microseconds(rep ns) -> Duration
    {
        return { std::chrono::microseconds(ns) };
    }

    constexpr static auto Milliseconds(rep ns) -> Duration
    {
        return { std::chrono::milliseconds(ns) };
    }

    constexpr static auto Seconds(rep ns) -> Duration
    {
        return { std::chrono::seconds(ns) };
    }

    constexpr static auto Minutes(rep ns) -> Duration
    {
        return { std::chrono::minutes(ns) };
    }

    constexpr static auto Hours(rep ns) -> Duration
    {
        return { std::chrono::hours(ns) };
    }

    constexpr static auto Max() -> Duration
    {
        return { std_type::max() };
    }

    constexpr static auto Min() -> Duration
    {
        return { std_type::min() };
    }

    static auto FromStr(Str input) noexcept -> anyhow::Result<Duration>;

    template<typename Dur>
    [[nodiscard]] constexpr auto Cast() const -> Dur
    {
        return std::chrono::duration_cast<Dur>(this->n_ns);
    }

    [[nodiscard]] constexpr auto AsNanos() const -> rep
    {
        return this->n_ns.count();
    }

    [[nodiscard]] constexpr auto AsMicros() const -> rep
    {
        return this->Cast<std::chrono::microseconds>().count();
    }

    [[nodiscard]] constexpr auto AsMillis() const -> rep
    {
        return this->Cast<std::chrono::milliseconds>().count();
    }

    [[nodiscard]] constexpr auto AsSeconds() const -> rep
    {
        return this->Cast<std::chrono::seconds>().count();
    }

    [[nodiscard]] constexpr auto AsMinutes() const -> rep
    {
        return this->Cast<std::chrono::minutes>().count();
    }

    [[nodiscard]] constexpr auto AsHours() const -> rep
    {
        return this->Cast<std::chrono::hours>().count();
    }

    [[nodiscard]] constexpr auto Zeroed() const noexcept -> bool
    {
        return this->n_ns.count() == 0;
    }

    constexpr auto CheckedAdd(Duration other) const -> violet::Optional<Duration>
    {
        auto result = numeric::CheckedAdd<rep>(this->AsNanos(), other.AsNanos());
        if (result.HasValue()) {
            return violet::Some(Duration::Nanoseconds(result.Value()));
        }

        return violet::Nothing;
    }

    [[nodiscard]] constexpr auto SaturatingAdd(Duration other) const -> Duration
    {
        auto result = numeric::CheckedAdd<rep>(this->AsNanos(), other.AsNanos());
        if (result.HasValue()) {
            return Duration::Nanoseconds(result.Value());
        }

        return this->AsNanos() > 0 ? Duration::Max() : Duration::Min();
    }

    constexpr auto CheckedSub(Duration other) const -> violet::Optional<Duration>
    {
        auto result = numeric::CheckedSub<rep>(this->AsNanos(), other.AsNanos());
        if (result.HasValue()) {
            return violet::Some(Duration::Nanoseconds(result.Value()));
        }

        return violet::Nothing;
    }

    [[nodiscard]] constexpr auto SaturatingSub(Duration other) const -> Duration
    {
        auto result = numeric::CheckedSub<rep>(this->AsNanos(), other.AsNanos());
        if (result.HasValue()) {
            return Duration::Nanoseconds(result.Value());
        }

        return this->AsNanos() > 0 ? Duration::Max() : Duration::Min();
    }

    [[nodiscard]] constexpr auto ToStd() const noexcept -> std_type
    {
        return this->n_ns;
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return !this->Zeroed();
    }

    constexpr VIOLET_EXPLICIT operator std_type() const noexcept
    {
        return this->ToStd();
    }

    constexpr auto operator+(Duration other) const -> Duration
    {
        return { std_type(detail::doCheckedAdd(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+")) };
    }

    constexpr auto operator-(Duration other) const -> Duration
    {
        return { std_type(detail::doCheckedSub(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator-")) };
    }

    constexpr auto operator+=(Duration other) -> Duration&
    {
        this->n_ns = std_type(detail::doCheckedAdd(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+="));

        return *this;
    }

    constexpr auto operator-=(Duration other) -> Duration&
    {
        this->n_ns = std_type(detail::doCheckedSub(
            this->AsNanos(), other.AsNanos(), "violet::experimental::chrono::Duration::operator+="));

        return *this;
    }

    constexpr auto operator*(rep ns) const -> Duration
    {
        return { std_type(
            detail::doCheckedMul(this->AsNanos(), ns, "violet::experimental::chrono::Duration::operator*")) };
    }

    constexpr auto operator/(rep ns) const -> Duration
    {
        VIOLET_DEBUG_ASSERT(ns != 0, "divide by zero");
        return Duration::Nanoseconds(this->AsNanos() / ns);
    }

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
