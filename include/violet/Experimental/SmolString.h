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
//! # 🌺💜 `violet/Experimental/SmolString.h`

#pragma once

#include <violet/Language/Macros.h>
#include <violet/Language/Policy.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <format>
#include <string_view>

namespace violet::experimental {

/// A small, non-owning, fixed-capacity string type.
///
/// `SmolString` will store up to `N` characters inline with no heap allocation,
/// making it sutable for use as NTTP (non-type template parameter), compile-time
/// string manipulation, and latency-sensitive contexts where small-string optimizations
/// are desired.
///
/// @tparam N maximum number of chatacters the string can hold (excluding null terminator)
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/SmolString.h>
/// #include <violet/Violet.h>
///
/// using violet::experimental::SmolString;
///
/// constexpr SmolString hello("hello");
/// hello.Append(", world!");
///
/// VIOLET_ASSERT(hello == "hello, world!", "`hello` didn't construct correctly");
/// VIOLET_ASSERT(hello.Size() == 13, "size failure");
/// ```
template<size_t N>
struct SmolString final {
    /// Construct a empty smol string.
    constexpr VIOLET_IMPLICIT SmolString() noexcept = default;

    /// Construct a [`SmolString`] from a string literal.
    ///
    /// ## Constraints
    /// - `M <= N + 1` :: literal string size cannot (minus null terminator) cannot exceed
    ///   the capacity.
    ///
    /// @param M size of the string; deduced from the array size (including null-terminator).
    template<size_t M>
        requires((M <= N + 1))
    constexpr VIOLET_IMPLICIT SmolString(const char (&str)[M]) noexcept
        : n_size(M - 1)
    {
        std::copy_n(str, this->n_size, this->n_data.begin());
    }

    /// Construct a [`SmolString`] from any type convertible to [`std::string_view`].
    ///
    /// ## Panics
    /// In debug builds (without `NDEBUG` being defined), it'll assert that `sv.size() <= N`.
    ///
    /// @param sv the string view itself
    template<std::convertible_to<std::string_view> Str>
        requires(!std::same_as<std::decay_t<Str>, const char*>)
    constexpr VIOLET_IMPLICIT SmolString(Str sv) noexcept(noexcept(sv.size() < N))
        : n_size(sv.size())
    {
        assert(sv.size() < N);

        std::copy_n(sv.data(), this->n_size, this->n_data.begin());
    }

    /// Returns the size of this string.
    [[nodiscard]] constexpr auto Size() const noexcept -> size_t
    {
        return this->n_size;
    }

    /// Returns the capacity (excluding the null terminator) that this smol string can carry.
    [[nodiscard]] constexpr auto Capacity() const noexcept -> size_t
    {
        return N;
    }

    /// Returns **true** if this smol string is empty.
    [[nodiscard]] constexpr auto Empty() const noexcept -> bool
    {
        return this->n_size == 0;
    }

    /// Pushes a single character into this smol string.
    ///
    /// ## Panics (Debug)
    /// This method will panic if the size of the string is greater than or equal
    /// to the capacity.
    constexpr void Push(char ch)
    {
        assert(this->n_size <= N && "string is full");
        this->n_data[this->n_size++] = ch;
    }

    /// Pushes a [`std::string_view`] into this smol string.
    ///
    /// ## Panics (Debug)
    /// This method will panic if the size of the string plus the string itself is greater than or equal
    /// to the capacity.
    constexpr auto Append(std::string_view sv) -> SmolString&
    {
        assert(this->n_size + sv.size() <= N && "overflow");

        std::copy_n(sv.data(), sv.size(), this->n_data.data() + this->n_size);
        this->n_size += sv.size();

        return *this;
    }

    /// Appends a formatted string into this smol string.
    ///
    /// **Note**: This method *will always* allocate!
    ///
    /// ## Panics (Debug)
    /// This method will panic if the size of the string plus the string itself is greater than or equal
    /// to the capacity.
    template<typename... Args>
    constexpr auto AppendFormatted(std::format_string<Args...> fmt, Args&&... args) -> SmolString&
    {
        auto remaining = N - this->n_size;
        auto result = std::format_to_n(this->n_data.data() + this->n_size, remaining, fmt, VIOLET_FWD(Args, args)...);

        assert(this->n_size + result.size <= N && "formatted output exceeded capacity");
        this->n_size += result.size;

        return *this;
    }

    /// Support for C++ iterators. Resolves to [`std::array<char, N>::data`].
    constexpr auto begin() noexcept -> char*
    {
        return this->n_data.data();
    }

    /// Support for C++ iterators. Resolves to [`std::array<char, N>::data`] to the size of the string itself.
    constexpr auto end() noexcept -> char*
    {
        return this->n_data.data() + this->n_size;
    }

    /// Support for C++ iterators. Resolves to [`std::array<char, N>::data`].
    [[nodiscard]] constexpr auto begin() const noexcept -> const char*
    {
        return this->n_data.data();
    }

    /// Support for C++ iterators. Resolves to [`std::array<char, N>::data`] to the size of the string itself.
    [[nodiscard]] constexpr auto end() const noexcept -> const char*
    {
        return this->n_data.data() + this->n_size;
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return !this->Empty();
    }

    constexpr VIOLET_EXPLICIT operator std::string_view() const noexcept
    {
        return { this->n_data.data(), this->n_size };
    }

    /// ## Notes
    /// You can disable assertions on the subscript operator with defining
    /// `VIOLET_NO_ASSERT_SUBSCRIPT`. (GCC/Clang: `-DVIOLET_NO_ASSERT_SUBSCRIPT`, MSVC: `/DVIOLET_NO_ASSERT_SUBSCRIPT`)
    constexpr auto operator[](size_t idx) -> char&
    {
#ifndef VIOLET_NO_ASSERT_SUBSCRIPT
        assert(idx <= this->n_size && "out-of-bounds");
#endif

        return this->n_data[idx];
    }

    /// ## Notes
    /// You can disable assertions on the subscript operator with defining
    /// `VIOLET_NO_ASSERT_SUBSCRIPT`. (GCC/Clang: `-DVIOLET_NO_ASSERT_SUBSCRIPT`, MSVC: `/DVIOLET_NO_ASSERT_SUBSCRIPT`)
    constexpr auto operator[](size_t idx) const -> const char&
    {
#ifndef VIOLET_NO_ASSERT_SUBSCRIPT
        assert(idx <= this->n_size && "out-of-bounds");
#endif

        return this->n_data[idx];
    }

    constexpr friend auto operator==(const SmolString& self, std::string_view sv) -> bool
    {
        return static_cast<std::string_view>(self) == sv;
    }

    constexpr friend auto operator!=(const SmolString& self, std::string_view sv) -> bool
    {
        return !(self == sv);
    }

    constexpr friend auto operator==(std::string_view self, const SmolString& smol) -> bool
    {
        return smol == self;
    }

    constexpr friend auto operator!=(std::string_view self, const SmolString& smol) -> bool
    {
        return !(self == smol);
    }

    constexpr auto operator<=>(const SmolString&) const noexcept -> std::strong_ordering = default;

private:
    std::array<char, N> n_data{};
    size_t n_size = 0;
};

template<std::size_t M>
SmolString(const char (&)[M]) -> SmolString<M - 1>;

} // namespace violet::experimental
