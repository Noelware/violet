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

#include <violet/Container/Optional.h>
#include <violet/Iterator.h>
#include <violet/Violet.h>

namespace violet::strings {

/// Removes any leading ASCII whitespace from `input`.
///
/// Whitespace characters are those for which [`std::isspace`] would return
/// **true** in the default C locale (i.e, spaces, `\t`, `\n`, `\r\`, etc.) or
/// any implementation from the `fun` parameter.
///
/// This is a zero-allocation function, the return [`Str`] is a view into
/// the original [`input`].
///
/// @param input string view to trim
/// @param fun the function to call to determine if a character is whitespace, defaults to [`std::isspace`].
/// @returns view of `input` with leading whitespace removed
template<typename Fun = int (*)(int)>
    requires(callable<Fun, unsigned char> && std::convertible_to<std::invoke_result_t<Fun&, unsigned char>, bool>)
auto TrimStart(Str input, Fun&& fun = std::isspace) noexcept(
    noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<unsigned char>()))) -> Str
{
    auto&& pred = VIOLET_FWD(Fun, fun);
    while (!input.empty() && std::invoke(pred, static_cast<unsigned char>(input.front()))) {
        input.remove_prefix(1);
    }

    return input;
}

/// Removes trailing ASCII whitespace from `input`.
///
/// Whitespace characters are those for which [`std::isspace`] would return
/// **true** in the default C locale (i.e, spaces, `\t`, `\n`, `\r\`, etc.) or
/// any implementation from the `fun` parameter.
///
/// This is a zero-allocation function, the return [`Str`] is a view into
/// the original [`input`].
///
/// @param input string view to trim
/// @param fun the function to call to determine if a character is whitespace, defaults to [`std::isspace`].
/// @returns view of `input` with leading whitespace removed
template<typename Fun = int (*)(int)>
    requires(callable<Fun, unsigned char> && std::convertible_to<std::invoke_result_t<Fun&, unsigned char>, bool>)
auto TrimEnd(Str input, Fun&& fun = std::isspace) noexcept(
    noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<unsigned char>()))) -> Str
{
    auto&& pred = VIOLET_FWD(Fun, fun);
    while (!input.empty() && std::invoke(pred, static_cast<unsigned char>(input.back()))) {
        input.remove_suffix(1);
    }

    return input;
}

/// Removes leading and trailing ASCII whitespace from `input`.
///
/// This is a zero-allocation function, the return [`Str`] is a view into
/// the original [`input`].
///
/// @param input string view to trim
/// @param fun the function to call to determine if a character is whitespace, defaults to [`std::isspace`].
/// @returns view of `input` with leading whitespace removed
template<typename Fun = int (*)(int)>
    requires(callable<Fun, unsigned char> && std::convertible_to<std::invoke_result_t<Fun&, unsigned char>, bool>)
auto Trim(Str input, Fun&& fun = std::isspace) noexcept(
    noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<unsigned char>()))) -> Str
{
    auto&& pred = VIOLET_FWD(Fun, fun);
    return TrimEnd(TrimStart(input, pred), pred);
}

/// Splits `input` at the first occurence of `delim`.
///
/// The string is divided into at most two parts:
/// - The substring before the first occurrence of `delim`.
/// - The substring after the first occurrence of `delim`, if any.
///
/// The delimiter itself is not included in either result.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// using violet::strings::SplitOnce;
///
/// auto [lhs, rhs] = SplitOnce("key:value", ':');
/// VIOLET_ASSERT(lhs == "key");
/// VIOLET_ASSERT(rhs.HasValueAnd([](const Str& value) -> bool { return value == "value"; }));
/// ```
///
/// @param input string view to split
/// @param delim delimiter character
auto SplitOnce(Str input, char delim) noexcept -> Pair<Str, Optional<Str>>;

/// An iterator that yields substrings of `input` separated by a delimiter.
///
/// `Split` produces non-owning views into the original string. It does not
/// allocate or copy the underlying data.
///
/// By default, the delimiter is implementation-defined (typically space),
/// or may be explicitly specified.
///
/// This type satisfies [`violet::Iterable`].
///
/// # Examples
/// ```cpp
/// #include <violet/Strings.h>
///
/// for (auto part: violet::strings::Split("a,b,c", ',')) {
///     // Yields: "a", "b", "c"
/// }
/// ```
///
/// Empty segments may be yielded if consecutive delimiters are present,
/// depending on implementation semantics.
struct Split final: public violet::Iterator<Split> {
    /// Creates a `Split` iterator over `input` using the default delimiter.
    /// @param input string view to iterate over
    VIOLET_IMPLICIT Split(Str input) noexcept;

    /// Creates a `Split` iterator over `input` using `delim` as the separator
    /// @param input string view to iterate over
    /// @param delim delimiter character
    VIOLET_IMPLICIT Split(Str input, char delim) noexcept;

    /// Advances the iterator and returns the next substring.
    /// @returns the next segement if available, or [`Nothing`] if iteration is complete.
    auto Next() noexcept -> Optional<Str>;

private:
    Str n_input;
    char n_delim;
    UInt n_pos = 0;
    UInt n_revPos = 0;
};

static_assert(Iterable<Split>, "`Split` is not a valid iterable object");

} // namespace violet::strings
