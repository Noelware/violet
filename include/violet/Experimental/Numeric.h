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
//! # 🌺💜 `violet/Experimental/Numeric.h`

#pragma once

#include <violet/Container/Optional.h>
#include <violet/anyhow.h>

#include <charconv>

#if (defined(__cpp_lib_constexpr_charconv) && __cpp_lib_constexpr_charconv >= 202207L)
#define __violet_constexpr_charconv__ constexpr
#else
#define __violet_constexpr_charconv__ inline
#endif

#if VIOLET_PLATFORM(APPLE_MACOS)
#define __violet_has_from_chars_float__ 0
#elif defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 170000
#define __violet_has_from_chars_float__ 1
#elif defined(__GLIBCXX__) && defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
#define __violet_has_from_chars_float__ 1
#else
#define __violet_has_from_chars_float__ 0
#endif

#if __violet_has_from_chars_float__ == 0
#if VIOLET_PLATFORM(APPLE_MACOS)
#include <xlocale.h>
#elif defined(_GNU_SOURCE) || (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L)
#include <locale.h>
#else
#error                                                                                                                 \
    "For platforms that don't suppport `from_chars<float/double>`, `_GNU_SOURCE` or `_POSIX_C_SOURCE >= 200809L` must be defined"
#endif
#endif

namespace violet::numeric {

/// Returns **true** if the possiblity of adding `one` and `two` could overflow. This is meant
/// to mimic the `__builtin_add_overflow` compiler builtin function for compilers that don't
/// support it.
template<std::integral T>
    requires(std::is_signed_v<T>)
constexpr auto WouldAddingOverflow(T one, T two) -> bool
{
    constexpr T kMax = std::numeric_limits<T>::max();
    constexpr T kMin = std::numeric_limits<T>::min();

    if (two > 0 && one > kMax - two) {
        return true;
    }

    if (two < 0 && one < kMin - two) {
        return true;
    }

    return false;
}

/// Returns **true** if the possiblity of subtracting `one` and `two` could overflow. This is meant
/// to mimic the `__builtin_sub_overflow` compiler builtin function for compilers that don't
/// support it.
template<std::integral T>
    requires(std::is_signed_v<T>)
constexpr auto WouldSubtractingOverflow(T one, T two) -> bool
{
    constexpr T kMax = std::numeric_limits<T>::max();
    constexpr T kMin = std::numeric_limits<T>::min();

    if (two < 0 && one > kMax + two) {
        return true;
    }

    if (two > 0 && one < kMin + two) {
        return true;
    }

    return false;
}

/// Returns **true** if the possiblity of multiplying `one` by `two` could overflow. This is meant
/// to mimic the `__builtin_sub_overflow` compiler builtin function for compilers that don't
/// support it.
template<std::integral T>
    requires(std::is_signed_v<T>)
constexpr auto WouldMultiplicationOverflow(T one, T two) -> bool
{
    constexpr T kMax = std::numeric_limits<T>::max();
    constexpr T kMin = std::numeric_limits<T>::min();

    if (one == 0 || two == 0) {
        return false;
    }

    if (one == kMin && two == -1) {
        return true;
    }

    if (two == kMin && one == -1) {
        return true;
    }

    if (one > 0 && two > 0) {
        return one > kMax / two;
    }

    if (one < 0 && two < 0) {
        return one < kMax / two;
    }

    if (one > 0) {
        return two < kMin / one;
    }

    return one < kMin / two;
}

/// Addition that returns [`Nothing`] on signed integer overflow.
///
/// This will perform `one + two` using the most efficient available mechanism
/// without invoking undefined behaviour on overflow. On overflow, the result
/// is [`Nothing`]; otherwise, it is `Some(sum)`.
///
/// ## Implementation
/// By default, if the compiler provides the `__builtin_add_overflow` intrinstic (Clang, GCC
/// and most recent MSVC), it'll opt into using the intrinstic, which can be lowered to a single
/// instruction plus a flag-register check on x86_64 and ARM64.
///
/// Otherwise, if `VIOLET_USE_PORTABLE_ARITHEMTIC` is defined or if the compiler doesn't support the builtin intrinstic,
/// then it'll use the slow, but efficient [`WouldAddingOverflow`].
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// VIOLET_ASSERT0(violet::numeric::CheckedAdd<Int64>(100, 200) == Some(300));
/// VIOLET_ASSERT0(!violet::numeric::CheckedAdd<Int64>(std::numeric_limits<Int64>::max(), 1));
/// VIOLET_ASSERT0(!violet::numeric::CheckedAdd<Int64>(std::numeric_limits<Int64>::min(), 1));
/// ```
template<std::integral T>
    requires(std::is_signed_v<T>)
constexpr auto CheckedAdd(T one, T two) -> Optional<T>
{
#if !defined(VIOLET_USE_PORTABLE_ARITHEMTIC) && VIOLET_HAS_BUILTIN(add_overflow)
    T result = 0;
    if (__builtin_add_overflow(one, two, &result)) {
        return Nothing;
    }

    return result;
#else
    if (WouldAddingOverflow(one, two)) {
        return Nothing;
    }

    return static_cast<T>(one + two);
#endif
}

/// Subtraction that returns [`Nothing`] on signed integer overflow.
///
/// This will perform `one - two` using the most efficient available mechanism
/// without invoking undefined behaviour on overflow. On overflow, the result
/// is [`Nothing`]; otherwise, it is `Some(sum)`.
///
/// ## Implementation
/// By default, if the compiler provides the `__builtin_sub_overflow` intrinstic (Clang, GCC
/// and most recent MSVC), it'll opt into using the intrinstic, which can be lowered to a single
/// instruction plus a flag-register check on x86_64 and ARM64.
///
/// Otherwise, if `VIOLET_USE_PORTABLE_ARITHEMTIC` is defined or if the compiler doesn't support the builtin intrinstic,
/// then it'll use the slow, but efficient [`WouldSubtracingOverflow`].
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// VIOLET_ASSERT0(violet::numeric::CheckedSub<Int64>(300, 100) == Some(200));
/// ```
template<std::integral T>
    requires(std::is_signed_v<T>)
constexpr auto CheckedSub(T one, T two) -> Optional<T>
{
#if !defined(VIOLET_USE_PORTABLE_ARITHEMTIC) && VIOLET_HAS_BUILTIN(sub_overflow)
    T result = 0;
    if (__builtin_sub_overflow(one, two, &result)) {
        return Nothing;
    }

    return result;
#else
    if (WouldSubtractingOverflow(one, two)) {
        return Nothing;
    }

    return static_cast<T>(one - two);
#endif
}

/// Multiplication that returns [`Nothing`] on signed integer overflow.
///
/// This will perform `one * two` using the most efficient available mechanism
/// without invoking undefined behaviour on overflow. On overflow, the result
/// is [`Nothing`]; otherwise, it is `Some(mul)`.
///
/// ## Implementation
/// By default, if the compiler provides the `__builtin_mul_overflow` intrinstic (Clang, GCC
/// and most recent MSVC), it'll opt into using the intrinstic, which can be lowered to a single
/// instruction plus a flag-register check on x86_64 and ARM64.
///
/// Otherwise, if `VIOLET_USE_PORTABLE_ARITHEMTIC` is defined or if the compiler doesn't support the builtin intrinstic,
/// then it'll use the slow, but efficient [`WouldMultiplyingOverflow`].
///
/// ## Example
/// ```cpp
/// #include "violet/util/Integrals.h"
///
/// VIOLET_ASSERT0(violet::numeric::CheckedMul<Int64>(7, 6) == Some(42));
/// ```
template<std::integral T>
    requires(std::is_signed_v<T>)
constexpr auto CheckedMul(T one, T two) -> Optional<T>
{
#if !defined(VIOLET_USE_PORTABLE_ARITHEMTIC) && VIOLET_HAS_BUILTIN(mul_overflow)
    T result = 0;
    if (__builtin_mul_overflow(one, two, &result)) {
        return Nothing;
    }

    return result;
#else
    if (WouldMultiplicationOverflow(one, two)) {
        return Nothing;
    }

    return static_cast<T>(one * two);
#endif
}

#if __violet_has_from_chars_float__

/// Fallible, no-exception way to parse a double-precision floating-point value
/// from string input.
///
/// The full input must be consumed; trailing garbage results in an error.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// auto value = violet::numeric::Parse<float>("3.14");
/// VIOLET_ASSERT0(value.Ok() && *value == 3.14);
///
/// auto bad = violet::numeric::Parse<double>("3.14xyz");
/// VIOLET_ASSERT0(bad.Err());
/// ```
///
/// @param input string slice containing the textual representation to parse
template<std::floating_point T>
__violet_constexpr_charconv__ auto Parse(Str input) -> anyhow::Result<T>
{
    CStr it = input.data();
    CStr end = input.data() + input.size();

    T tmp{ };
    auto [ptr, ec] = std::from_chars(it, end, tmp);
    if (ec != std::errc{ }) {
        std::error_code code(std::make_error_code(ec));
        return Err(ANYHOW_FMT("failed to parse double-precise floating point integral value: {}", input)
                .Context(code.message()));
    }

    if (ptr != end) {
        return Err(ANYHOW_FMT("trailing garbage in input: {}", input));
    }

    return tmp;
}

#else

namespace detail {

    inline auto getCLocale() -> locale_t
    {
        static locale_t cloc = ::newlocale(LC_NUMERIC_MASK, "C", nullptr);
        return cloc;
    }

} // namespace detail

/// Fallible, no-exception way to parse a double-precision floating-point value
/// from string input.
///
/// The full input must be consumed; trailing garbage results in an error.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// auto value = violet::numeric::Parse<float>("3.14");
/// VIOLET_ASSERT0(value.Ok() && *value == 3.14);
///
/// auto bad = violet::numeric::Parse<double>("3.14xyz");
/// VIOLET_ASSERT0(bad.Err());
/// ```
///
/// @param input string slice containing the textual representation to parse
template<std::floating_point N>
inline auto Parse(Str input) -> anyhow::Result<N>
{
    Array<char, 128> buf;
    if (input.size() >= sizeof(buf)) {
        return Err(ANYHOW_FMT("input too long for floating-point parse: {}", input));
    }

    ::memcpy(buf.data(), input.data(), input.size());
    buf[input.size()] = '\0';

    char* end = nullptr;
    errno = 0; // reset errno from previous errors

    double raw = ::strtod_l(buf.data(), &end, detail::getCLocale());
    Int32 saved = errno;

    if (end == buf.data()) {
        return Err(ANYHOW_FMT("failed to parse floating-point value: {}", input));
    }

    if (end != buf.data() + input.size()) {
        return Err(ANYHOW_FMT("trailing garbage in input: {}", input));
    }

    if (saved == ERANGE) {
        return Err(ANYHOW_FMT("floating-point value out of range: {}", input));
    }

    if constexpr (std::same_as<N, float>) {
        constexpr auto kMax = static_cast<double>(std::numeric_limits<float>::max());
        constexpr auto kLowest = static_cast<double>(std::numeric_limits<float>::lowest());

        if (raw > kMax || raw < kLowest) {
            return Err(ANYHOW_FMT("value out of range for float: {}", input));
        }

        return static_cast<float>(raw);
    } else {
        return static_cast<N>(raw);
    }
}

#endif

/// Fallible, no-exception way to parse unsigned integrals from string input.
/// @param input input to parse
template<std::unsigned_integral N>
__violet_constexpr_charconv__ auto Parse(Str input) -> anyhow::Result<N>
{
    CStr it = input.data();
    CStr end = input.data() + input.size();

    UInt64 tmp{ };
    auto [ptr, ec] = std::from_chars(it, end, tmp);
    if (ec != std::errc{ }) {
        std::error_code code(std::make_error_code(ec));
        return Err(ANYHOW_FMT("failed to parse integral value: {}", input).Context(code.message()));
    }

    if (ptr != end) {
        return Err(ANYHOW_FMT("trailing garbage in input: {}", input));
    }

    if constexpr (!std::same_as<N, UInt64>) {
        constexpr auto kMax = std::numeric_limits<N>::max();
        if (tmp > kMax) {
            return Err(ANYHOW_FMT("failed to parse integral value: {}", input)
                    .Context(std::format("wanted less than or equal to: {}, but got {}", kMax, tmp)));
        }
    }

    return static_cast<N>(tmp);
}

/// Fallible, no-exception way to parse unsigned integrals from string input.
/// @param input input to parse
template<std::integral N>
__violet_constexpr_charconv__ auto Parse(Str input) -> anyhow::Result<N>
{
    CStr it = input.data();
    CStr end = input.data() + input.size();

    Int64 tmp{ };
    auto [ptr, ec] = std::from_chars(it, end, tmp);
    if (ec != std::errc{ }) {
        std::error_code code(std::make_error_code(ec));
        return Err(ANYHOW_FMT("failed to parse integral value: {}", input).Context(code.message()));
    }

    if (ptr != end) {
        return Err(ANYHOW_FMT("trailing garbage in input: {}", input));
    }

    if constexpr (!std::same_as<N, Int64>) {
        constexpr auto kMin = std::numeric_limits<N>::min();
        constexpr auto kMax = std::numeric_limits<N>::max();
        if (tmp > kMax) {
            return Err(ANYHOW_FMT("failed to parse integral value: {}", input)
                    .Context(std::format("wanted less than: {}, but got {}", kMax, tmp)));
        }

        if (tmp < static_cast<Int64>(kMin)) {
            return Err(ANYHOW_FMT("failed to parse integral value: {}", input)
                    .Context(std::format("wanted greater than: {}, but got {}", kMin, tmp)));
        }
    }

    return static_cast<N>(tmp);
}

} // namespace violet::numeric

#undef __violet_constexpr_charconv__
#undef __violet_has_from_chars_float__
