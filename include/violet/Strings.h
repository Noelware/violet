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

namespace violet::strings {

/// Resolves the character type of `StringType`.
///
/// This is `StringType::character` for Violet's string types, or `StringType::value_type` for the
/// standard library's string types.
///
/// ## Remarks
/// For any other type (i.e, `const char*`), the trait has no `type` member. Overloads constrained on
/// [`character_t`] are dropped from overload resolution rather than causing a hard error.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// static_assert(std::same_as<violet::strings::character_t<std::string>, char>);
/// static_assert(std::same_as<violet::strings::character_t<std::u16string_view>, char16_t>);
/// ```
///
/// @tparam StringType the string type to inspect
template<typename StringType>
struct NOELDOC_SINCE("current") character { };

template<typename StringType>
    requires(
        requires { typename StringType::value_type; } && !requires { typename StringType::character; })
struct NOELDOC_SINCE("current") character<StringType> {
    using type = StringType::value_type;
};

template<typename StringType>
    requires requires { typename StringType::character; }
struct NOELDOC_SINCE("current") character<StringType> {
    using type = StringType::character;
};

/// Shorthand for `character<StringType>::type`.
/// @since current
template<typename StringType>
using character_t = character<StringType>::type;

/// Resolves the unsigned counterpart of [`character_t`], which is what character predicates
/// like [`std::isspace`] are called with.
///
/// ## Remarks
/// As with [`character`], this has no `type` member if `StringType` isn't a string type.
///
/// @tparam StringType the string type to inspect
template<typename StringType>
struct NOELDOC_SINCE("current") unsigned_char { };

template<typename StringType>
    requires requires { typename character_t<StringType>; }
struct NOELDOC_SINCE("current") unsigned_char<StringType> {
    using type = std::make_unsigned_t<character_t<StringType>>;
};

/// Shorthand for `unsigned_char<StringType>::type`.
/// @since current
template<typename StringType>
using unsigned_char_t = unsigned_char<StringType>::type;

/// Whether `StringType` is a non-owning string view (any [`std::basic_string_view`]).
/// @tparam StringType the string type to inspect
template<typename StringType>
struct NOELDOC_SINCE("current") is_view_type: std::false_type { };

template<typename CharT, typename Traits>
struct NOELDOC_SINCE("current") is_view_type<std::basic_string_view<CharT, Traits>>: std::true_type { };

/// Shorthand for `is_view_type<StringType>::value`.
template<typename StringType>
NOELDOC_SINCE("current")
[[maybe_unused]] inline constexpr static bool is_view_type_v = is_view_type<StringType>::value;

/// Resolves the non-owning view type of `StringType`.
///
/// [`std::basic_string`] and [`std::basic_string_view`] both resolve to [`std::basic_string_view`].
/// Any other type has to provide a `view_type` member.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// static_assert(std::same_as<violet::strings::view_type<std::string>, std::string_view>);
/// ```
///
/// @tparam StringType the string type to inspect
template<typename StringType>
struct NOELDOC_SINCE("current") view {
    using type = StringType::view_type;
};

template<typename CharT, typename Traits, typename Alloc>
struct NOELDOC_SINCE("current") view<std::basic_string<CharT, Traits, Alloc>> {
    using type = std::basic_string_view<CharT, Traits>;
};

template<typename CharT, typename Traits>
struct NOELDOC_SINCE("current") view<std::basic_string_view<CharT, Traits>> {
    using type = std::basic_string_view<CharT, Traits>;
};

/// Shorthand for `view<StringType>::type`.
/// @since current
template<typename StringType>
using view_type = view<StringType>::type;

/// Resolves the owning string type of `StringType`.
///
/// This is the inverse of [`view`]. [`std::basic_string_view`] and [`std::basic_string`] both resolve to
/// [`std::basic_string`]. Any other type has to provide an `owned_type` member.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// static_assert(std::same_as<violet::strings::owned_type<std::string_view>, std::string>);
/// ```
///
/// @tparam StringType the string type to inspect
template<typename StringType>
struct NOELDOC_SINCE("current") owned {
    using type = StringType::owned_type;
};

template<typename CharT, typename Traits>
struct NOELDOC_SINCE("current") owned<std::basic_string_view<CharT, Traits>> {
    using type = std::basic_string<CharT, Traits>;
};

template<typename CharT, typename Traits>
struct NOELDOC_SINCE("current") owned<std::basic_string<CharT, Traits>> {
    using type = std::basic_string<CharT, Traits>;
};

/// Shorthand for `owned<StringType>::type`.
/// @since current
template<typename StringType>
using owned_type = owned<StringType>::type;

/// Whether `StringType` is an owning string (any [`std::basic_string`]).
///
/// @tparam StringType the string type to inspect
template<typename StringType>
struct NOELDOC_SINCE("current") is_owned_type: std::false_type { };

template<typename CharT, typename Traits, typename Alloc>
struct NOELDOC_SINCE("current") is_owned_type<std::basic_string<CharT, Traits, Alloc>>: std::true_type { };

/// Shorthand for `is_owned_type<StringType>::value`.
template<typename StringType>
NOELDOC_SINCE("current")
[[maybe_unused]] inline constexpr static bool is_owned_type_v = is_owned_type<StringType>::value;

/// Removes leading whitespace from `input`.
///
/// Whitespace characters are those for which `fun` returns **true**. By default, that's [`std::isspace`] in the
/// default C locale (i.e, spaces, `\t`, `\n`, `\r`, etc.).
///
/// ## Remarks
/// - Views ([`Str`], any [`std::basic_string_view`]) are trimmed without allocating; the result points into the
///   same buffer as `input`.
/// - Owned strings (any [`std::basic_string`]) are trimmed in place and returned by value.
/// - Types that provide their own `TrimStart` or `trim_start` method are dispatched to it.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// using namespace std::string_view_literals;
///
/// assert(violet::strings::TrimStart("  hello  "sv) == "hello  ");
/// assert(violet::strings::TrimStart("//usr"sv, [](unsigned char ch) { return ch == '/'; }) == "usr");
/// ```
///
/// @param input string to trim
/// @param fun predicate that returns **true** for characters to remove, defaults to [`std::isspace`]
/// @return `input` with leading whitespace removed
template<typename StringType = Str, typename Fun = int (*)(int)>
    requires(callable<Fun, unsigned_char_t<StringType>>
        && std::convertible_to<std::invoke_result_t<Fun&, unsigned_char_t<StringType>>, bool>)
auto TrimStart(StringType input, Fun&& fun = std::isspace) noexcept(
    VIOLET_NOEXCEPT_FUN(fun, unsigned_char_t<StringType>)) -> StringType
{
    auto pred = VIOLET_FWD(Fun, fun);
    if constexpr (requires { input.trim_start(pred); }) {
        return input.trim_start(pred);
    }

    if constexpr (requires { input.TrimStart(pred); }) {
        return input.TrimStart(pred);
    }

    if constexpr (is_view_type_v<StringType> && requires {
                      { input.front() };
                      { input.empty() };
                      { input.remove_prefix(1) };
                  }) {
        while (!input.empty() && std::invoke(pred, static_cast<unsigned_char_t<StringType>>(input.front()))) {
            input.remove_prefix(1);
        }

        return input;
    }

    if constexpr (is_owned_type_v<StringType>
        && requires { input.erase(0, std::declval<typename StringType::size_type>()); }) {
        const auto size = input.size();
        UInt skip = 0;

        while (skip < size && std::invoke(pred, static_cast<unsigned_char_t<StringType>>(input[skip]))) {
            ++skip;
        }

        input.erase(0, skip);

        return input;
    }

    // At this point, we don't know what to do.
    VIOLET_UNREACHABLE();
}

/// Removes trailing whitespace from `input`.
///
/// Whitespace characters are those for which `fun` returns **true**. By default, that's [`std::isspace`] in the
/// default C locale (i.e, spaces, `\t`, `\n`, `\r`, etc.).
///
/// ## Remarks
/// - Views ([`Str`], any [`std::basic_string_view`]) are trimmed without allocating; the result points into the
///   same buffer as `input`.
/// - Owned strings (any [`std::basic_string`]) are trimmed in place and returned by value.
/// - Types that provide their own `TrimEnd` or `trim_end` method are dispatched to it.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// using namespace std::string_view_literals;
///
/// assert(violet::strings::TrimEnd("  hello  "sv) == "  hello");
/// assert(violet::strings::TrimEnd("/usr/lib//"sv, [](unsigned char ch) { return ch == '/'; }) == "/usr/lib");
/// ```
///
/// @param input string to trim
/// @param fun predicate that returns **true** for characters to remove, defaults to [`std::isspace`]
/// @return `input` with trailing whitespace removed
template<typename StringType = Str, typename Fun = int (*)(int)>
    requires(callable<Fun, unsigned_char_t<StringType>>
        && std::convertible_to<std::invoke_result_t<Fun&, unsigned_char_t<StringType>>, bool>)
auto TrimEnd(StringType input, Fun&& fun = std::isspace) noexcept(
    noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<unsigned_char_t<StringType>>()))) -> StringType
{
    auto pred = VIOLET_FWD(Fun, fun);
    if constexpr (requires { input.TrimEnd(pred); }) {
        return input.TrimEnd(pred);
    }

    if constexpr (requires { input.trim_end(pred); }) {
        return input.trim_end(pred);
    }

    if constexpr (is_view_type_v<StringType> && requires {
                      { input.front() };
                      { input.empty() };
                      { input.remove_suffix(1) };
                  }) {
        while (!input.empty() && std::invoke(pred, static_cast<unsigned_char_t<StringType>>(input.back()))) {
            input.remove_suffix(1);
        }

        return input;
    }

    if constexpr (is_owned_type_v<StringType>
        && requires { input.erase(std::declval<typename StringType::size_type>()); }) {
        auto end = input.size();
        while (end > 0 && std::invoke(pred, static_cast<unsigned_char_t<StringType>>(input[end - 1]))) {
            --end;
        }

        input.erase(end);
        return input;
    }

    // At this point, we don't know what to do.
    VIOLET_UNREACHABLE();
}

/// Removes leading and trailing whitespace from `input`.
///
/// Whitespace characters are those for which `fun` returns **true**. By default, that's [`std::isspace`] in the
/// default C locale (i.e, spaces, `\t`, `\n`, `\r`, etc.).
///
/// ## Remarks
/// - Views ([`Str`], any [`std::basic_string_view`]) are trimmed without allocating; the result points into the
///   same buffer as `input`.
/// - Owned strings (any [`std::basic_string`]) are returned by value. Assigning the result to a [`Str`]
///   (`Str s = Trim(someString)`) leaves a view into a destroyed temporary.
/// - Types that provide their own `Trim` or `trim` method are dispatched to it.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// using namespace std::string_view_literals;
///
/// assert(violet::strings::Trim("  hello  "sv) == "hello");
/// assert(violet::strings::Trim(std::string("\thello\n")) == "hello");
/// ```
///
/// @param input string to trim
/// @param fun predicate that returns **true** for characters to remove, defaults to [`std::isspace`]
/// @return `input` with leading and trailing whitespace removed
template<typename StringType = Str, typename Fun = int (*)(int)>
    requires(callable<Fun, unsigned_char_t<StringType>>
        && std::convertible_to<std::invoke_result_t<Fun&, unsigned_char_t<StringType>>, bool>)
auto Trim(StringType input, Fun&& fun = std::isspace) noexcept(
    noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<unsigned_char_t<StringType>>()))) -> StringType
{
    auto pred = VIOLET_FWD(Fun, fun);
    if constexpr (requires { input.Trim(pred); }) {
        return input.Trim(pred);
    }

    if constexpr (requires { input.trim(pred); }) {
        return input.trim(pred);
    }

    return TrimEnd(TrimStart(input, pred), pred);
}

/// Removes leading whitespace from the C-style string `input`.
///
/// This is the overload for string literals and `const CharT*`: `input` is trimmed as a
/// [`std::basic_string_view`] without allocating, and the result points into `input`.
///
/// @param input NUL-terminated string to trim
/// @param fun predicate that returns **true** for characters to remove, defaults to [`std::isspace`]
/// @return a view of `input` with leading whitespace removed
template<typename CharT, typename Fun = int (*)(int)>
    requires(callable<Fun, std::make_unsigned_t<CharT>>
        && std::convertible_to<std::invoke_result_t<Fun&, std::make_unsigned_t<CharT>>, bool>)
auto TrimStart(const CharT* input, Fun&& fun = std::isspace) noexcept(
    std::is_nothrow_invocable_v<Fun&, std::make_unsigned_t<CharT>>) -> std::basic_string_view<CharT>
{
    return TrimStart(std::basic_string_view<CharT>(input), VIOLET_FWD(Fun, fun));
}

/// Removes trailing whitespace from the C-style string `input`.
///
/// This is the overload for string literals and `const CharT*`: `input` is trimmed as a
/// [`std::basic_string_view`] without allocating, and the result points into `input`.
///
/// @param input NUL-terminated string to trim
/// @param fun predicate that returns **true** for characters to remove, defaults to [`std::isspace`]
/// @return a view of `input` with trailing whitespace removed
template<typename CharT, typename Fun = int (*)(int)>
    requires(callable<Fun, std::make_unsigned_t<CharT>>
        && std::convertible_to<std::invoke_result_t<Fun&, std::make_unsigned_t<CharT>>, bool>)
auto TrimEnd(const CharT* input, Fun&& fun = std::isspace) noexcept(
    std::is_nothrow_invocable_v<Fun&, std::make_unsigned_t<CharT>>) -> std::basic_string_view<CharT>
{
    return TrimEnd(std::basic_string_view<CharT>(input), VIOLET_FWD(Fun, fun));
}

/// Removes leading and trailing whitespace from the C-style string `input`.
///
/// This is the overload for string literals and `const CharT*`: `input` is trimmed as a
/// [`std::basic_string_view`] without allocating, and the result points into `input`.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// auto trimmed = violet::strings::Trim("  hello  ");
/// assert(trimmed == "hello");
/// ```
///
/// @param input NUL-terminated string to trim
/// @param fun predicate that returns **true** for characters to remove, defaults to [`std::isspace`]
/// @return a view of `input` with leading and trailing whitespace removed
template<typename CharT, typename Fun = int (*)(int)>
    requires(callable<Fun, std::make_unsigned_t<CharT>>
        && std::convertible_to<std::invoke_result_t<Fun&, std::make_unsigned_t<CharT>>, bool>)
auto Trim(const CharT* input, Fun&& fun = std::isspace) noexcept(
    std::is_nothrow_invocable_v<Fun&, std::make_unsigned_t<CharT>>) -> std::basic_string_view<CharT>
{
    return Trim(std::basic_string_view<CharT>(input), VIOLET_FWD(Fun, fun));
}

/// Splits `input` at the first occurrence of `delim`.
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
/// @return the part before `delim`, and the part after it if `delim` was found
VIOLET_API
auto SplitOnce(Str input, char delim) noexcept -> Pair<Str, Optional<Str>>;

/// Joins a collection of elements into a single string, separated by `delim`.
///
/// Each element is converted with [`violet::ToString`].
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// auto joined = violet::strings::Join(Vec<String>{"Bob", "Alice"}, ", ");
/// assert(joined == "Bob, Alice");
/// ```
///
/// @param range elements to join
/// @param delim separator placed between each element
/// @return the joined string
template<std::ranges::input_range Range>
    requires(Stringify<std::ranges::range_value_t<Range>>)
auto Join(const Range& range, Str delim) noexcept -> String
{
    return Join(range, delim, [](const auto& value) -> String { return violet::ToString(value); });
}

/// Joins a collection of elements into a single string, separated by `delim`, mapping each element through
/// `mapper`.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// auto mapped = violet::strings::Join(Vec<String>{"Bob", "Alice"}, ", ", [](const String& name) -> String {
///     return std::format("'{}'", name);
/// });
///
/// assert(mapped == "'Bob', 'Alice'");
/// ```
///
/// @param range elements to join
/// @param delim separator placed between each element
/// @param mapper converts each element into a [`String`]
/// @return the joined string
template<std::ranges::input_range Range, typename Fun>
#if VIOLET_COMPILER(GCC)
    requires(requires(Fun fn, const std::ranges::range_value_t<Range>& v) {
        { std::invoke(fn, v) } -> std::convertible_to<violet::String>;
    })
#else
    requires(callable<Fun, const std::ranges::range_value_t<Range>&>,
        callable_returns<Fun, violet::String, const std::ranges::range_value_t<Range>&>)
#endif
auto Join(const Range& range, Str delim, Fun&& mapper) noexcept -> String
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

/// Replaces all occurrences of `pattern` in `input` with `replacement`.
///
/// If `pattern` is empty or isn't found, a copy of `input` is returned unchanged.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// auto result = violet::strings::ReplaceAll("hello %d world!", "%d", "420");
/// assert(result == "hello 420 world!");
/// ```
///
/// @param input string to search
/// @param pattern substring to replace
/// @param replacement string to put in place of each `pattern`
/// @return a new [`String`] with every `pattern` replaced
NOELDOC_SINCE("26.07")
constexpr auto ReplaceAll(Str input, Str pattern, Str replacement) -> String
{
    if (pattern.empty()) {
        return String(input);
    }

    String result;
    UInt pos = 0;

    while (true) {
        auto found = input.find(pattern, pos);
        if (found == Str::npos) {
            result.append(input.substr(pos));
            break;
        }

        result.append(input.substr(pos, found - pos));
        result.append(replacement);
        pos = found + pattern.size();
    }

    return result;
}

static_assert(ReplaceAll("hello %d world!", "%d", "420") == "hello 420 world!");
static_assert(ReplaceAll("%d-%d-%d", "%d", "x") == "x-x-x");
static_assert(ReplaceAll("hello world", "%d", "42") == "hello world");
static_assert(ReplaceAll("hello", "", "42") == "hello");
static_assert(ReplaceAll("", "%t", "42").empty());
static_assert(ReplaceAll("a%tb%tc", "%t", "") == "abc");
static_assert(ReplaceAll("%t%t", "%t", "ab") == "abab");

/// An iterator that yields the substrings of `input` separated by a delimiter.
///
/// Every substring is a view into `input`; nothing is allocated or copied. The delimiter defaults to `' '`.
///
/// ## Remarks
/// - Consecutive delimiters yield empty substrings (`"a,,b"` yields `"a"`, `""`, `"b"`).
/// - A trailing delimiter does not yield a trailing empty substring (`"a,"` yields only `"a"`).
///
/// This type satisfies [`violet::Iterable`].
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// for (auto part: violet::strings::Split("a,b,c", ',')) {
///     // Yields: "a", "b", "c"
/// }
/// ```
struct VIOLET_API Split final: public violet::Iterator<Split> {
    /// Creates a `Split` iterator over `input`, separated by `' '`.
    /// @param input string view to iterate over
    VIOLET_IMPLICIT Split(Str input) noexcept;

    /// Creates a `Split` iterator over `input`, separated by `delim`.
    /// @param input string view to iterate over
    /// @param delim delimiter character
    VIOLET_IMPLICIT Split(Str input, char delim) noexcept;

    /// Advances the iterator and returns the next substring.
    /// @return the next substring, or [`Nothing`] once iteration is complete
    auto Next() noexcept -> Optional<Str>;

private:
    Str n_input;
    char n_delim;
    UInt n_pos = 0;
    UInt n_revPos = 0;
};

static_assert(Iterable<Split>, "`Split` is not a valid iterable");

/// An iterator that splits a string at most `N` times.
///
/// After `N` splits, the rest of the string, delimiters included, is yielded as the last substring. The
/// delimiter defaults to `' '`. Every substring is a view into the input.
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// auto it = violet::strings::SplitN<2>("a:b:c:d", ':');
///
/// assert(it.Next().HasValueAnd([](const Str& part) -> bool { return part == "a"; }));
/// assert(it.Next().HasValueAnd([](const Str& part) -> bool { return part == "b"; }));
/// assert(it.Next().HasValueAnd([](const Str& part) -> bool { return part == "c:d"; }));
/// assert(!it.Next().HasValue());
/// ```
///
/// @tparam N the maximum number of splits
template<UInt N>
struct SplitN final: public Iterator<SplitN<N>> {
    /// Creates a `SplitN` iterator over `input`, separated by `' '`.
    /// @param input string view to iterate over
    VIOLET_IMPLICIT SplitN(Str input) noexcept
        : SplitN<N>(input, ' ')
    {
    }

    /// Creates a `SplitN` iterator over `input`, separated by `delim`.
    /// @param input string view to iterate over
    /// @param delim delimiter character
    VIOLET_IMPLICIT SplitN(Str input, char delim) noexcept
        : n_input(input)
        , n_delim(delim)
    {
    }

    /// Advances the iterator and returns the next substring.
    /// @return the next substring, or [`Nothing`] once iteration is complete
    auto Next() noexcept -> Optional<Str>
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

/// A lazy, zero-copy iterator over the lines of a string.
///
/// Lines can end in `\n`, `\r`, or `\r\n`. Line endings are not included in the yielded lines, and every line is a
/// view into the input.
///
/// ## Remarks
/// If the text ends with a line ending, a final empty line is yielded (`"a\n"` yields `"a"`, `""`).
///
/// ## Example
/// ```cpp
/// #include <violet/Strings.h>
///
/// using namespace std::string_view_literals;
///
/// auto lines = violet::strings::Lines("hello\nworld\r\nfoo\rbar"sv).Count();
/// assert(lines == 4);
/// ```
struct VIOLET_API Lines final: public Iterator<Lines> {
    using Item = Str;

    /// Creates an empty iterator that yields nothing.
    constexpr Lines() noexcept = default;

    /// Creates an iterator over the lines of `text`.
    /// @param text the text to iterate over
    constexpr VIOLET_IMPLICIT Lines(Str text) noexcept
        : n_remaining(text)
    {
    }

    /// Advances the iterator and returns the next line.
    /// @return the next line, or [`Nothing`] once iteration is complete
    constexpr auto Next() noexcept -> Optional<Item>
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

    /// Returns the bounds on the number of lines left: at least one until the iterator is done.
    [[nodiscard]]
    constexpr auto SizeHint() const noexcept -> violet::SizeHint
    {
        if (this->n_done) {
            return {0, Some<UInt>(0)};
        }

        return {1, Nothing};
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
