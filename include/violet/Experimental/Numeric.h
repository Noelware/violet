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

#if (defined(__cpp_lib_constexpr_charconv) && __cpp_lib_constexpr_charconv >= 202207L)
#define __violet_constexpr_charconv__ constexpr
#else
#define __violet_constexpr_charconv__ inline
#endif

/// Fallible, no-exception way to parse unsigned integrals from string input.
/// @param input input to parse
template<std::unsigned_integral N>
__violet_constexpr_charconv__ auto ParseUnsigned(Str input) -> anyhow::Result<N>;

template<>
__violet_constexpr_charconv__ auto ParseUnsigned<UInt64>(Str input) -> anyhow::Result<UInt64>
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

    return tmp;
}

#define __violet_impl_unsigned_parse__(type)                                                                           \
    template<>                                                                                                         \
    __violet_constexpr_charconv__ auto ParseUnsigned<type>(Str input) -> anyhow::Result<type>                          \
    {                                                                                                                  \
        auto value = VIOLET_TRY(ParseUnsigned<UInt64>(input));                                                         \
        constexpr auto kMax = std::numeric_limits<type>::max();                                                        \
        if (value > kMax) {                                                                                            \
            return Err(ANYHOW_FMT("failed to parse integral value [{}]", input)                                        \
                    .Context(                                                                                          \
                        std::format("wanted less than or equal to `{}', but received [{}] instead", kMax, value)));    \
        }                                                                                                              \
                                                                                                                       \
        return static_cast<type>(value);                                                                               \
    }

__violet_impl_unsigned_parse__(UInt8);
__violet_impl_unsigned_parse__(UInt16);
__violet_impl_unsigned_parse__(UInt32);

#undef __violet_impl_unsigned_parse__

/// Fallible, no-exception way to parse unsigned integrals from string input.
/// @param input input to parse
template<std::integral N>
__violet_constexpr_charconv__ auto ParseSigned(Str input) -> anyhow::Result<N>;

template<>
__violet_constexpr_charconv__ auto ParseSigned<Int64>(Str input) -> anyhow::Result<Int64>
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

    return tmp;
}

#define __violet_impl_signed_parse__(type)                                                                             \
    template<>                                                                                                         \
    __violet_constexpr_charconv__ auto ParseSigned<type>(Str input) -> anyhow::Result<type>                            \
    {                                                                                                                  \
        auto value = VIOLET_TRY(ParseSigned<Int64>(input));                                                            \
        constexpr auto kMin = std::numeric_limits<type>::min();                                                        \
        constexpr auto kMax = std::numeric_limits<type>::max();                                                        \
        if (value > kMax) {                                                                                            \
            return Err(ANYHOW_FMT("failed to parse integral value [{}]", input)                                        \
                    .Context(                                                                                          \
                        std::format("wanted less than or equal to `{}', but received [{}] instead", kMax, value)));    \
        }                                                                                                              \
                                                                                                                       \
        if (value < static_cast<Int64>(kMin)) {                                                                        \
            return Err(ANYHOW_FMT("failed to parse integral value [{}]", input)                                        \
                    .Context(                                                                                          \
                        std::format("wanted greater than or equal to `{}', but received [{}] instead", kMin, value))); \
        }                                                                                                              \
        return static_cast<type>(value);                                                                               \
    }

__violet_impl_signed_parse__(Int8);
__violet_impl_signed_parse__(Int16);
__violet_impl_signed_parse__(Int32);

#undef __violet_impl_signed_parse__

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

#undef __violet_constexpr_charconv__

} // namespace violet::numeric
