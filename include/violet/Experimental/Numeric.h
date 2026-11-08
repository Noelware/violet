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
#include <violet/Container/Result.h>
#include <violet/Experimental/OneOf.h>
#include <violet/Experimental/Slice.h>

#include <charconv>
#include <cstring>
#include <limits>

#if defined(__cpp_lib_constexpr_charconv) && __cpp_lib_constexpr_charconv >= 202207L
#define VIOLET_FEATURE_CONSTEXPR_CHARCONV 1
#define __violet_constexpr_charconv__ constexpr
#else
#define VIOLET_FEATURE_CONSTEXPR_CHARCONV 0
#define __violet_constexpr_charconv__
#endif

#if VIOLET_PLATFORM(APPLE_MACOS)
#define VIOLET_FEATURE_FROM_CHARS_FLOAT 0
#elif defined(VIOLET_STD_LIBCXX) && VIOLET_STDLIB(LIBCXX) && _LIBCPP_VERSION >= 170000
#define VIOLET_FEATURE_FROM_CHARS_FLOAT 1
#elif defined(VIOLET_STD_GNU) && VIOLET_STDLIB(GNU) && defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
#define VIOLET_FEATURE_FROM_CHARS_FLOAT 1
#else
#define VIOLET_FEATURE_FROM_CHARS_FLOAT 0
#endif // VIOLET_PLATFORM(APPLE_MACOS)

#if !VIOLET_FEATURE(FROM_CHARS_FLOAT)
#if VIOLET_PLATFORM(APPLE_MACOS)
#include <xlocale.h>
#elif defined(_GNU_SOURCE) && (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200809L)
#include <locale.h>
#else
#error platform does not support `from_chars<float>` or `from_chars<double>`.
#endif
#endif

#ifdef VIOLET_USE_PORTABLE_ARITHEMTIC
#warning `VIOLET_USE_PORTABLE_ARITHEMTIC` is deprecated as of current; define `VIOLET_FEATURE_PORTABLE_ARITHEMTIC` instead
#undef VIOLET_USE_PORTABLE_ARITHEMTIC
#define VIOLET_FEATURE_PORTABLE_ARITHEMTIC 1
#elif !defined(VIOLET_FEATURE_PORTABLE_ARITHEMTIC)
#define VIOLET_FEATURE_PORTABLE_ARITHEMTIC 0
#endif

namespace violet::experimental::numeric {
namespace error {

struct TrailingGarbage final {
    [[nodiscard]]
    auto ToString() const noexcept -> CStr
    {
        return "trailing garbage in input";
    }
};

struct InputTooLong final {
    String Input;

    [[nodiscard]]
    auto ToString() const noexcept -> String
    {
        return std::format("input too large: {}", this->Input);
    }
};

} // namespace error

struct Error final: public OneOf<std::error_code, error::TrailingGarbage, error::InputTooLong> {
    using OneOf::OneOf;

    [[nodiscard]]
    auto ToString() const -> String
    {
        return this->Match([](const std::error_code& ec) -> String { return ec.message(); },
            [](const auto& err) -> String { return err.ToString(); });
    }
};

/// Returns **true** if the possiblity of adding `one` and `two` could overflow. This is meant
/// to mimic the `__builtin_add_overflow` compiler builtin function for compilers that don't
/// support it.
template<std::signed_integral N>
constexpr auto WouldAddingOverflow(N one, N two) noexcept -> bool
{
    constexpr N kMax = std::numeric_limits<N>::max();
    constexpr N kMin = std::numeric_limits<N>::min();

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
template<std::signed_integral N>
constexpr auto WouldSubtractingOverflow(N one, N two) noexcept -> bool
{
    constexpr N kMax = std::numeric_limits<N>::max();
    constexpr N kMin = std::numeric_limits<N>::min();

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
template<std::signed_integral N>
constexpr auto WouldMultiplicationOverflow(N one, N two) -> bool
{
    constexpr N kMax = std::numeric_limits<N>::max();
    constexpr N kMin = std::numeric_limits<N>::min();

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
//
/// ## Implementation
/// By default, if the compiler provides the `__builtin_add_overflow` intrinstic (Clang, GCC
/// and most recent MSVC), it'll opt into using the intrinstic, which can be lowered to a single
/// instruction plus a flag-register check on x86_64 and ARM64. Otherwise, if `VIOLET_FEATURE_PORTABLE_ARITHEMTIC`
/// is defiend or the compiler doesn't support `__builtin_add_overflow`, it'll use the slower, but efficient
/// [`WouldAddingOverflow`].
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// namespace violetx = violet::experimental;
/// using namespace violetx::numeric;
///
/// VIOLET_ASSERT0(CheckedAdd<Int64>(100, 200) == Some(300));
/// VIOLET_ASSERT0(!CheckedAdd<Int64>(std::numeric_limits<Int64>::max(), 1));
/// VIOLET_ASSERT0(!CheckedAdd<Int64>(std::numeric_limits<Int64>::min(), 1));
/// ```
template<std::signed_integral N>
constexpr auto CheckedAdd(N one, N two) noexcept -> Optional<N>
{
#if VIOLET_HAS_BUILTIN(add_overflow) && !VIOLET_FEATURE(PORTABLE_ARITHEMTIC)
    N result = 0;
    if (__builtin_add_overflow(one, two, &result)) {
        return Nothing;
    }

    return result;
#else
    if (WouldAddingOverflow(one, two)) {
        return Nothing;
    }

    return static_cast<N>(one + two);
#endif
}

/// Subtraction that returns [`Nothing`] on signed integer overflow.
///
/// This will perform `one - two` using the most efficient available mechanism
/// without invoking undefined behaviour on overflow. On overflow, the result
/// is [`Nothing`]; otherwise, it is `Some(sum)`.
//
/// ## Implementation
/// By default, if the compiler provides the `__builtin_sub_overflow` intrinstic (Clang, GCC
/// and most recent MSVC), it'll opt into using the intrinstic, which can be lowered to a single
/// instruction plus a flag-register check on x86_64 and ARM64. Otherwise, if `VIOLET_FEATURE_PORTABLE_ARITHEMTIC`
/// is defiend or the compiler doesn't support `__builtin_sub_overflow`, it'll use the slower, but efficient
/// [`WouldSubtractingOverflow`].
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// namespace violetx = violet::experimental;
/// using namespace violetx::numeric;
///
/// VIOLET_ASSERT0(CheckedSub<Int64>(300, 100) == Some(200));
/// ```
template<std::signed_integral N>
constexpr auto CheckedSub(N one, N two) noexcept -> Optional<N>
{
#if VIOLET_HAS_BUILTIN(sub_overflow) && !VIOLET_FEATURE(PORTABLE_ARITHEMTIC)
    N result = 0;
    if (__builtin_sub_overflow(one, two, &result)) {
        return Nothing;
    }

    return result;
#else
    if (WouldSubtractingOverflow(one, two)) {
        return Nothing;
    }

    return static_cast<N>(one - two);
#endif
}

/// Multiplication that returns [`Nothing`] on signed integer overflow.
///
/// This will perform `one * two` using the most efficient available mechanism
/// without invoking undefined behaviour on overflow. On overflow, the result
/// is [`Nothing`]; otherwise, it is `Some(sum)`.
//
/// ## Implementation
/// By default, if the compiler provides the `__builtin_mul_overflow` intrinstic (Clang, GCC
/// and most recent MSVC), it'll opt into using the intrinstic, which can be lowered to a single
/// instruction plus a flag-register check on x86_64 and ARM64. Otherwise, if `VIOLET_FEATURE_PORTABLE_ARITHEMTIC`
/// is defiend or the compiler doesn't support `__builtin_mul_overflow`, it'll use the slower, but efficient
/// [`WouldSubtractingOverflow`].
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// namespace violetx = violet::experimental;
/// using namespace violetx::numeric;
///
/// VIOLET_ASSERT0(CheckedMul<Int64>(7, 6) == Some(42));
/// ```
template<std::signed_integral N>
constexpr auto CheckedMul(N one, N two) noexcept -> Optional<N>
{
#if VIOLET_HAS_BUILTIN(mul_overflow) && !VIOLET_FEATURE(PORTABLE_ARITHEMTIC)
    N result = 0;
    if (__builtin_mul_overflow(one, two, &result)) {
        return Nothing;
    }

    return result;
#else
    if (WouldMultiplicationOverflow(one, two)) {
        return Nothing;
    }

    return static_cast<N>(one * two);
#endif
}

#define __from_chars__(T, decl, it, end)                                                                               \
    T __tmpvar{};                                                                                                      \
    auto __start = (it);                                                                                               \
    auto __end = (end);                                                                                                \
    auto [ptr, ec] = ::std::from_chars(__start, __end, __tmpvar);                                                      \
    if (ec != ::std::errc{}) {                                                                                         \
        return ::violet::Err<::violet::experimental::numeric::Error>(::std::make_error_code(ec));                      \
    }                                                                                                                  \
                                                                                                                       \
    if (ptr != __end) {                                                                                                \
        return ::violet::Err<::violet::experimental::numeric::Error>(error::TrailingGarbage{});                        \
    }                                                                                                                  \
                                                                                                                       \
    decl = __tmpvar;

#if VIOLET_FEATURE(FROM_CHARS_FLOAT)

/// Fallible, no-exception way to parse a double-precision floating-point value
/// from string input.
///
/// The full input must be consumed; trailing garbage results in an error.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// namespace violetx = violet::experimental;
///
/// auto value = violetx::numeric::Parse<float>("3.14");
/// VIOLET_ASSERT0(value.Ok() && *value == 3.14);
///
/// auto bad = violetx::numeric::Parse<double>("3.14xyz");
/// VIOLET_ASSERT0(bad.Err());
/// ```
///
/// @param input string slice containing the textual representation to parse
template<std::floating_point F>
constexpr auto Parse(Str input) noexcept -> Result<F, Error>
{
    __from_chars__(F, const auto value, input.data(), input.data() + input.size());
    return value;
}

#else

namespace internals {

inline auto locale() -> locale_t
{
    static locale_t cloc = ::newlocale(LC_NUMERIC_MASK, "C", nullptr);
    return cloc;
}

} // namespace internals

/// Fallible, no-exception way to parse a double-precision floating-point value
/// from string input.
///
/// The full input must be consumed; trailing garbage results in an error.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Numeric.h>
///
/// namespace violetx = violet::experimental;
///
/// auto value = violetx::numeric::Parse<float>("3.14");
/// VIOLET_ASSERT0(value.Ok() && *value == 3.14);
///
/// auto bad = violetx::numeric::Parse<double>("3.14xyz");
/// VIOLET_ASSERT0(bad.Err());
/// ```
///
/// @param input string slice containing the textual representation to parse
template<std::floating_point F>
auto Parse(Str input) noexcept -> Result<F, Error>
{
    Slice<char, 128> buf;
    if (input.size() >= buf.Size()) {
        return Err(error::InputTooLong{
            .Input = String(input),
        });
    }

    ::memcpy(buf.Data(), input.data(), input.size());
    buf.AssumeInit(Unsafe("memcpy operation"), input.size());
    buf[input.size()] = '\0';

    char* end = nullptr;
    errno = 0;

    double raw = ::strtod_l(buf.Data(), &end, internals::locale());
    Int32 saved = errno;

    if (end == buf.Data()) {
        return Err(std::make_error_code(std::errc::invalid_argument));
    }

    if (end != buf.Data() + input.size()) {
        return Err(error::TrailingGarbage{});
    }

    if (saved == ERANGE) {
        return Err(std::make_error_code(std::errc::result_out_of_range));
    }

    if constexpr (std::same_as<F, float>) {
        constexpr auto kMax = static_cast<double>(std::numeric_limits<float>::max());
        constexpr auto kLowest = static_cast<double>(std::numeric_limits<float>::lowest());

        if (raw > kMax || raw < kLowest) {
            return Err(std::make_error_code(std::errc::result_out_of_range));
        }

        return static_cast<float>(raw);
    } else {
        return static_cast<F>(raw);
    }
}

#endif

/// Fallible, no-exception way to parse unsigned integrals from string input.
/// @param input input to parse
template<std::unsigned_integral N>
__violet_constexpr_charconv__ auto Parse(Str input) -> Result<N, Error>
{
    __from_chars__(UInt64, const auto value, input.data(), input.data() + input.size());
    if constexpr (!std::same_as<N, UInt64>) {
        constexpr auto kMax = std::numeric_limits<N>::max();
        if (value > kMax) {
            return Err(std::make_error_code(std::errc::result_out_of_range));
        }
    }

    return static_cast<N>(value);
}

/// Fallible, no-exception way to parse signed integrals from string input.
/// @param input input to parse
template<std::signed_integral N>
__violet_constexpr_charconv__ auto Parse(Str input) -> Result<N, Error>
{
    __from_chars__(Int64, const auto value, input.data(), input.data() + input.size());
    if constexpr (!std::same_as<N, Int64>) {
        constexpr auto kMin = std::numeric_limits<N>::min();
        constexpr auto kMax = std::numeric_limits<N>::max();

        if (value > kMax || value < kMin) {
            return Err(std::make_error_code(std::errc::result_out_of_range));
        }
    }

    return static_cast<N>(value);
}

} // namespace violet::experimental::numeric

#undef __violet_constexpr_charconv__
