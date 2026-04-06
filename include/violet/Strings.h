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
VIOLET_API auto TrimStart(Str input, Fun&& fun = std::isspace) noexcept(
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
VIOLET_API auto TrimEnd(Str input, Fun&& fun = std::isspace) noexcept(
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
VIOLET_API auto Trim(Str input, Fun&& fun = std::isspace) noexcept(
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
VIOLET_API auto SplitOnce(Str input, char delim) noexcept -> Pair<Str, Optional<Str>>;

/// Joins a collection of elements into a single string with a delimiter.
///
/// ## Example
///
/// ```cpp
/// #include <violet/Strings.h>
///
/// using namespace violet::strings;
///
/// auto basic = Join(Vec<String>{"Bob", "Alice"}, ", ");
/// // => "Bob, Alice"
///
/// auto mapped = Join(Vec<String>{"Bob", "Alice"}, ", ", [](const String& name) -> String {
///     return std::format("'{}'", name);
/// });
/// // => "'Bob', 'Alice'"
/// ```
template<std::ranges::input_range Range>
    requires(violet::Stringify<std::ranges::range_value_t<Range>>)
VIOLET_API auto Join(const Range& range, Str delim) noexcept -> violet::String
{
    return Join(range, delim, [](const auto& value) -> String { return violet::ToString(value); });
}

/// Joins a collection of elements into a single string with a delimiter with a designated mapper.
template<std::ranges::input_range Range, typename Fun>
#if defined(VIOLET_GCC)
    requires(requires(Fun fn, const std::ranges::range_value_t<Range>& v) {
        { std::invoke(fn, v) } -> std::convertible_to<violet::String>;
    })
#else
    requires(callable<Fun, const std::ranges::range_value_t<Range>&>,
        callable_returns<Fun, violet::String, const std::ranges::range_value_t<Range>&>)
#endif
VIOLET_API auto Join(const Range& range, Str delim, Fun&& mapper) noexcept -> violet::String
{
    String result;
    bool first = true;
    auto onElement = VIOLET_FWD(Fun, mapper);

    for (const auto& element: range) {
        if (!first) {
            result.append(delim);
        }

        result.append(std::invoke(onElement, element));
        first = false;
    }

    return result;
}

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
struct VIOLET_API Split final: public violet::Iterator<Split> {
    /// Creates a `Split` iterator over `input` using the default delimiter.
    /// @param input string view to iterate over
    VIOLET_IMPLICIT Split(Str input) noexcept;

    /// Creates a `Split` iterator over `input` using `delim` as the separator
    /// @param input string view to iterate over
    /// @param delim delimiter character
    VIOLET_IMPLICIT Split(Str input, char delim) noexcept;

    /// Advances the iterator and returns the next substring.
    /// @returns the next segement if available, or [`Nothing`] if iteration is complete.
    VIOLET_API auto Next() noexcept -> Optional<Str>;

private:
    Str n_input;
    char n_delim;
    UInt n_pos = 0;
    UInt n_revPos = 0;
};

static_assert(Iterable<Split>, "`Split` is not a valid iterable");

/// A fixed-count, splitting iterator over a string slice.
///
/// `SplitN<N>` will split the input string atmost `N` times using a given delimiter
/// or `' '` as the default delimiter. After `N` splits, the remainder of the string is returned
/// as the last element.
///
/// ### Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// auto it = violet::strings::SplitN<2>("a:b:c:d", ':');
///
/// auto a = it.Next();
/// assert(a.HasValueAnd([](const Str& input) -> bool { return input == "a"; }));
///
/// auto b = it.Next();
/// assert(b.HasValueAnd([](const Str& input) -> bool { return input == "b"; }));
///
/// auto c = it.Next();
/// assert(!e.HasValue());
/// ```
template<UInt N>
struct VIOLET_API SplitN final: public Iterator<SplitN<N>> {
    /// Construct the iterator with a given input using the default delimiter (`' '`)
    VIOLET_IMPLICIT SplitN(Str input) noexcept
        : SplitN<N>(input, ' ')
    {
    }

    /// Construct the iterator with a given input and delimiter.
    /// @param input input to split
    /// @param delim the delimiter to check
    VIOLET_IMPLICIT SplitN(Str input, char delim) noexcept
        : n_input(input)
        , n_delim(delim)
    {
    }

    /// Advances the iterator and returns the next substring.
    /// @returns the next segement if available, or [`Nothing`] if iteration is complete.
    VIOLET_API auto Next() noexcept -> Optional<Str>
    {
        if (this->n_pos >= this->n_input.size()) {
            return Nothing;
        }

        if (this->n_splits >= N) {
            auto remainder = this->n_input.substr(this->n_pos);
            this->n_pos = this->n_input.size() + 1;

            return remainder;
        }

        auto next = this->n_input.find(this->n_delim, this->n_pos);
        if (next == Str::npos) {
            auto piece = this->n_input.substr(this->n_pos);
            this->n_pos = this->n_input.size() + 1;
            this->n_splits++;

            return piece;
        }

        auto piece = this->n_input.substr(this->n_pos, next - this->n_pos);
        this->n_pos = next + 1;
        this->n_splits++;

        return piece;
    }

private:
    Str n_input;
    char n_delim;
    UInt n_pos = 0;
    UInt n_splits = 0;
};

/// A lazy, zero-copyable iterator over lines (`\n`, `\r`, `\r\n`) in a string.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// auto text = "hello\nworld\r\nfoo\rbar"sv;
/// auto lines = violet::strings::Lines(text).Count();
/// assert(lines == 4);
/// ```
struct VIOLET_API Lines final: public Iterator<Lines> {
    using Item = Str;

    constexpr Lines() noexcept = default;
    constexpr VIOLET_IMPLICIT Lines(Str text) noexcept
        : n_remaining(text)
    {
    }

    VIOLET_API constexpr auto Next() noexcept -> Optional<Item>
    {
        if (this->n_done) {
            return Nothing;
        }

        if (this->n_remaining.empty()) {
            this->n_done = true;
            if (this->n_hadTrailingNewline) {
                return Str();
            }

            return Nothing;
        }

        auto pos = findNewline(this->n_remaining);
        if (pos == Str::npos) {
            auto line = this->n_remaining;
            this->n_remaining = Str(this->n_remaining.data() + this->n_remaining.size(), 0);
            this->n_done = true;

            return line;
        }

        auto line = this->n_remaining.substr(0, pos);
        if (this->n_remaining[pos] == '\r') {
            pos++;
            if (pos < this->n_remaining.size() && this->n_remaining[pos] == '\n') {
                pos++; // \r\n
            }
        } else {
            pos++;
        }

        this->n_hadTrailingNewline = true;
        this->n_remaining = this->n_remaining.substr(pos);
        return line;
    }

    [[nodiscard]] constexpr auto SizeHint() const noexcept -> violet::SizeHint
    {
        if (this->n_done) {
            return { 0, Some<UInt>(0) };
        }

        return { 1, Nothing };
    }

private:
    constexpr static auto findNewline(Str sv) noexcept -> UInt
    {
        for (UInt i = 0; i < sv.size(); i++) {
            if (sv[i] == '\n' || sv[i] == '\r') {
                return i;
            }
        }

        return Str::npos;
    }

    Str n_remaining;
    bool n_done = false;
    bool n_hadTrailingNewline = false;
};

} // namespace violet::strings
