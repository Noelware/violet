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
//! # 🌺💜 `violet/Experimental/Pair.h`

#pragma once

#include <violet/Violet.h>

namespace violet::experimental {

template<typename T, typename U>
struct NOELDOC_EXPERIMENTAL_SINCE("current") Pair final {
    static_assert(VIOLET_TRAIT_FOR_ALL_TYPES(!std::is_void_v, T, U), "`Pair`: cannot hold `void`");
    static_assert(VIOLET_TRAIT_FOR_ALL_TYPES(!std::is_reference_v, T, U), "`Pair`: cannot hold reference members");
    static_assert(VIOLET_TRAIT_FOR_ALL_TYPES(!std::is_function_v, T, U),
        "`Pair`: cannot hold function types; store a function pointer instead");

    static_assert(VIOLET_TRAIT_FOR_ALL_TYPES(!std::is_abstract_v, T, U),
        "`Pair`: cannot hold an abstract class; store a pointer or `violet::experimental::Own<T>`");

    static_assert(VIOLET_TRAIT_FOR_ALL_TYPES(!std::is_array_v, T, U),
        "`Pair`: doesn't support raw array members; use `std::array<T, N>` or `violet::experimental::Slice<T, N>`");

    static_assert(VIOLET_TRAIT_FOR_ALL_TYPES(std::is_destructible_v, T, U), "`Pair`: members must be destructible");

    using first_type = T;
    using second_type = U;

    VIOLET_NO_UNIQUE_ADDRESS T First;
    VIOLET_NO_UNIQUE_ADDRESS U Second;

    constexpr VIOLET_IMPLICIT Pair() noexcept(VIOLET_TRAIT_FOR_ALL_TYPES(std::is_nothrow_default_constructible_v, T, U))
        requires(VIOLET_TRAIT_FOR_ALL_TYPES(std::default_initializable, T, U))
    = default;

    constexpr VIOLET_IMPLICIT Pair(const T& first, const U& second)
        : First(first)
        , Second(second)
    {
    }

    template<typename X = T, typename Y = U>
        requires(std::constructible_from<T, X> && std::constructible_from<U, Y>)
    constexpr VIOLET_IMPLICIT Pair(X&& first, Y&& second)
        : First(VIOLET_FWD(X, first))
        , Second(VIOLET_FWD(Y, second))
    {
    }

    template<typename X, typename Y>
        requires(std::constructible_from<T, const X&> && std::constructible_from<U, const Y&>)
    constexpr VIOLET_EXPLICIT(!std::convertible_to<const X&, T> || !std::convertible_to<const Y&, U>)
        Pair(const Pair<X, Y>& other) noexcept(VIOLET_TRAIT_FOR_ALL_TYPES(std::is_nothrow_copy_constructible_v, X, Y))
        : First(other.First)
        , Second(other.Second)
    {
    }

    template<typename X, typename Y>
        requires(std::constructible_from<T, X> && std::constructible_from<U, Y>)
    constexpr VIOLET_EXPLICIT(!std::convertible_to<X, T> && !std::convertible_to<Y, U>) Pair(
        Pair<X, Y>&& other) noexcept(std::is_nothrow_constructible_v<T, X> && std::is_nothrow_constructible_v<U, Y>)
        : First(VIOLET_MOVE(other).First)
        , Second(VIOLET_MOVE(other).Second)
    {
    }

    constexpr static auto New(T first, U second) -> Pair<std::unwrap_ref_decay_t<T>, std::unwrap_ref_decay_t<U>>
    {
        return Pair(VIOLET_MOVE(first), VIOLET_MOVE(second));
    }

    template<typename Fun>
        requires callable<Fun, T&>
    [[nodiscard]]
    constexpr auto MapFirst(Fun&& fun)
        & VIOLET_NOEXCEPT_FUN(fun, T&) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, T&>;
        return Pair<return_type, second_type>(std::invoke(VIOLET_FWD(Fun, fun), this->First), this->Second);
    }

    template<typename Fun>
        requires callable<Fun, const T&>
    [[nodiscard]]
    constexpr auto MapFirst(Fun&& fun) const& VIOLET_NOEXCEPT_FUN(fun, const T&) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, const T&>;
        return Pair<return_type, second_type>(std::invoke(VIOLET_FWD(Fun, fun), this->First), this->Second);
    }

    template<typename Fun>
        requires callable<Fun, T>
    [[nodiscard]]
    constexpr auto MapFirst(Fun&& fun)
        && VIOLET_NOEXCEPT_FUN(fun, T) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, T>;

        auto self = VIOLET_MOVE(*this);
        return Pair<return_type, second_type>(std::invoke(VIOLET_FWD(Fun, fun), self.First), self.Second);
    }

    template<typename Fun>
        requires callable<Fun, T>
    [[nodiscard]]
    constexpr auto MapFirst(Fun&& fun) const&& VIOLET_NOEXCEPT_FUN(fun, T) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, T>;

        auto self = VIOLET_MOVE(*this);
        return Pair<return_type, second_type>(std::invoke(VIOLET_FWD(Fun, fun), self.First), self.Second);
    }

    template<typename Fun>
        requires callable<Fun, T&>
    [[nodiscard]]
    constexpr auto MapSecond(Fun&& fun)
        & VIOLET_NOEXCEPT_FUN(fun, T&) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, T&>;
        return Pair<first_type, return_type>(this->First, std::invoke(VIOLET_FWD(Fun, fun), this->Second));
    }

    template<typename Fun>
        requires callable<Fun, const T&>
    [[nodiscard]]
    constexpr auto MapSecond(Fun&& fun) const& VIOLET_NOEXCEPT_FUN(fun, const T&) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, const T&>;
        return Pair<first_type, return_type>(this->First, std::invoke(VIOLET_FWD(Fun, fun), this->Second));
    }

    template<typename Fun>
        requires callable<Fun, T>
    [[nodiscard]]
    constexpr auto MapSecond(Fun&& fun)
        && VIOLET_NOEXCEPT_FUN(fun, T) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, T>;

        auto self = VIOLET_MOVE(*this);
        return Pair<first_type, return_type>(self.First, std::invoke(VIOLET_FWD(Fun, fun), self.Second));
    }

    template<typename Fun>
        requires callable<Fun, T>
    [[nodiscard]]
    constexpr auto MapSecond(Fun&& fun) const&& VIOLET_NOEXCEPT_FUN(fun, T) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, T>;

        auto self = VIOLET_MOVE(*this);
        return Pair<first_type, return_type>(self.First, std::invoke(VIOLET_FWD(Fun, fun), self.Second));
    }
};

template<typename First, typename Second>
Pair(First, Second) -> Pair<First, Second>;

} // namespace violet::experimental

template<typename First, typename Second>
struct std::formatter<violet::experimental::Pair<First, Second>, char> final: public std::formatter<violet::String> {
    constexpr formatter() noexcept = default;

    template<typename FormatCx>
    auto format(const violet::experimental::Pair<First, Second>& pair, FormatCx& cx) const
    {
        // formats as '{<first>, <second>}'
        return std::formatter<violet::String>::format(std::format("{{{}, {}}}", pair.First, pair.Second), cx);
    }
};

template<typename First, typename Second>
struct std::tuple_size<violet::experimental::Pair<First, Second>> final
    : public std::integral_constant<violet::UInt, 2> { };

template<violet::UInt I, typename First, typename Second>
struct std::tuple_element<I, violet::experimental::Pair<First, Second>> final {
    static_assert(I < 2, "`Pair` only consists of two types");
};

template<typename First, typename Second>
struct std::tuple_element<0L, violet::experimental::Pair<First, Second>> final {
    using type = First;
};

template<typename First, typename Second>
struct std::tuple_element<1L, violet::experimental::Pair<First, Second>> final {
    using type = Second;
};
