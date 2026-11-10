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

#pragma once

#include <violet/Language/Macros.h>

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
#include "absl/strings/has_absl_stringify.h"
#include "absl/strings/str_cat.h"
#endif

#include <concepts>
#include <string>
#include <type_traits>

namespace violet {
namespace stringify_internal {

template<typename T>
concept is_char_type = std::same_as<std::remove_cv_t<T>, char> || std::same_as<std::remove_cv_t<T>, wchar_t>
    || std::same_as<std::remove_cv_t<T>, char8_t> || std::same_as<std::remove_cv_t<T>, char16_t>
    || std::same_as<std::remove_cv_t<T>, char32_t>;

template<typename T>
concept is_numeric
    = (std::integral<T> && !std::same_as<std::remove_cv_t<T>, bool> && !is_char_type<T>) || std::floating_point<T>;

template<typename T>
concept has_to_string_member = requires(const std::remove_cvref_t<T> ty) {
    { ty.ToString() } -> std::convertible_to<std::string>;
};

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
template<typename T>
concept HasAbslStringify = absl::HasAbslStringify<std::remove_cvref_t<T>>::value;
#endif

} // namespace stringify_internal

/// Returns `str` unchanged.
///
/// Exists so that [`violet::ToString`] can be called on any value in generic code
/// without special-casing strings.
NOELDOC_SINCE("26.02")
inline auto ToString(std::string str) -> std::string
{
    return str;
}

/// Converts a boolean to `"true"` or `"false"`.
///
/// ## Remarks
/// Since "26.11", this overload only accepts an actual `bool`. Earlier versions took
/// `bool` by value, which allowed implicit conversions: pointers, `char`, `short` and
/// unscoped enums all silently became `"true"` or `"false"`.
///
/// ## Example
/// ```cpp
/// violet::ToString(true);  // "true"
/// violet::ToString(false); // "false"
/// ```
template<std::same_as<bool> T>
NOELDOC_SINCE("26.02")
auto ToString(T value) -> std::string
{
    return value ? "true" : "false";
}

/// Returns the string `"nullptr"` for null pointer types.
NOELDOC_SINCE("26.02")
constexpr auto ToString(std::nullptr_t) -> std::string
{
    return "nullptr";
}

/// Copies a null-terminated character string into a [`std::basic_string`] of the same
/// character type.
///
/// A null pointer produces an empty string, not undefined behaviour.
///
/// ## Remarks
/// Only character types are accepted. Passing any other pointer type is a compile error instead of
/// producing `"true"`.
///
/// Only the `char` instantiation returns a [`std::string`] and therefore satisfies
/// [`Stringify`]. The `wchar_t`, `char8_t`, `char16_t` and `char32_t` instantiations
/// return their own string types.
///
/// ## Example
/// ```cpp
/// violet::ToString("hello");      // std::string{"hello"}
/// violet::ToString(L"hello");     // std::wstring{L"hello"}
/// violet::ToString((const char*)nullptr); // std::string{}
/// ```
template<stringify_internal::is_char_type Char>
NOELDOC_SINCE("26.11")
auto ToString(const Char* str) -> std::basic_string<Char>
{
    return str != nullptr ? std::basic_string<Char>{str} : std::basic_string<Char>{};
}

/// Converts a single character into a one-character [`std::basic_string`].
///
/// ## Example
/// ```cpp
/// violet::ToString('a'); // std::string{"a"}
/// ```
template<stringify_internal::is_char_type Char>
NOELDOC_SINCE("26.11")
auto ToString(Char ch) -> std::basic_string<Char>
{
    return std::basic_string<Char>(1, ch);
}

/// Converts an integer or floating-point value to its decimal representation using
/// [`std::to_string`].
///
/// Accepts every integral type except `bool` and the character types (which have their
/// own overloads), plus every floating-point type. `short`, `signed char` and
/// `unsigned char` are promoted and formatted as numbers.
///
/// ## Remarks
/// The exact output for floating-point values depends on the standard library's
/// [`std::to_string`].
///
/// ## Example
/// ```cpp
/// violet::ToString(42);         // "42"
/// violet::ToString(-7LL);       // "-7"
/// violet::ToString(short{ 5 }); // "5"
/// ```
template<stringify_internal::is_numeric T>
NOELDOC_SINCE("26.11")
auto ToString(T value) -> std::string
{
    return std::to_string(value);
}

/// Stringifies any type that has a `ToString()` member function.
///
/// The member must be callable on a `const` instance and its result must convert to
/// [`std::string`].
///
/// ## Remarks
/// Since "26.11":
/// * the result is always a [`std::string`]. Earlier versions used `decltype(auto)`,
///   which could return a reference into the argument and dangle when called on a
///   temporary.
///
/// * if a type has both a `ToString()` member and an `AbslStringify` extension, the
///   member wins. Earlier versions rejected such types entirely.
///
/// ## Example
/// ```cpp
/// struct Point {
///     int x, y;
///     auto ToString() const -> std::string { return std::format("({}, {})", x, y); }
/// };
///
/// violet::ToString(Point{ 1, 2 }); // "(1, 2)"
/// ```
template<stringify_internal::has_to_string_member T>
NOELDOC_SINCE("26.02")
auto ToString(const T& val) -> std::string
{
    return val.ToString();
}

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)

/// Stringifies any type that provides an Abseil `AbslStringify` extension, using
/// [`absl::StrCat`].
///
/// Only available when Violet is built with Abseil (marked with `VIOLET_FEATURE_ABSEIL`). Types that also have a
/// `ToString()` member use that member instead.
///
/// ## Example
/// ```cpp
/// struct Id {
///     int value;
///
///     template<typename Sink>
///     friend void AbslStringify(Sink& sink, const Id& id) { absl::Format(&sink, "#%d", id.value); }
/// };
///
/// violet::ToString(Id{ 7 }); // "#7"
/// ```
template<stringify_internal::HasAbslStringify T>
    requires(!stringify_internal::has_to_string_member<T>)
NOELDOC_SINCE("26.11")
auto ToString(const T& value) -> std::string
{
    return absl::StrCat(value);
}

#endif

/// C++ concept satisfied when `violet::ToString(value)` is valid for a `const T&` and
/// produces something convertible to [`std::string`].
///
/// `Char` defaults to `char`, so `Stringify<T>` means "stringifies to a [`std::string`]".
/// Pass another character type to ask for a different string type, for example `Stringify<T, wchar_t>` for
/// [`std::wstring`]. `Char` can't be a string view or any other non-owning type, so a satisfied `Stringify` always
/// yields an owning string.
///
/// For the default `Char = char`, this covers:
/// * `std::string`, `const char*` and `char`
/// * `bool` and `std::nullptr_t`
/// * integer and floating-point types
/// * types with a `const`-callable `ToString()` member
/// * types with an `AbslStringify` extension, when built with Abseil (marked with `VIOLET_FEATURE_ABSEIL`).
///
/// Wide and Unicode character types and strings (`wchar_t`, `char8_t`, `char16_t`,
/// `char32_t`) don't satisfy `Stringify<T>`. They satisfy `Stringify<T, Char>` for their
/// own `Char` instead. No other overload currently produces a non-`char` string, so
/// `Stringify<int, wchar_t>` is `false`.
///
/// ## Remarks
/// Since "26.11", this concept is defined in terms of [`violet::ToString`] itself and
/// cv/ref-qualifiers on `T` are ignored, so `Stringify<const Foo&>` is equivalent to
/// `Stringify<Foo>`. Earlier versions only checked for a `ToString()` member and could
/// fail to compile because of a recursive constraint.
///
/// To make your own type satisfy `Stringify`, give it a `const` `ToString()` member or an
/// `AbslStringify` extension (if built with Abseil). Adding a new `violet::ToString` overload after
/// this header will **not** work: the concept only sees overloads declared above it.
///
/// @since 26.02
template<typename T, typename Char = char>
concept Stringify = stringify_internal::is_char_type<Char> && requires(const std::remove_cvref_t<T>& value) {
    { violet::ToString(value) } -> std::convertible_to<std::basic_string<Char>>;
};

} // namespace violet
