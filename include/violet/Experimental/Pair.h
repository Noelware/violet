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

// TODO(@auguwu/Noel): add conversion operators from std::pair <-> violet::Pair
#ifndef VIOLET_IMPLEMENT_BACKWARDS_STL_PAIR
#define VIOLET_IMPLEMENT_BACKWARDS_STL_PAIR 0
#endif

#if VIOLET_IMPLEMENT_BACKWARDS_STL_PAIR
#include <tuple>
#endif

namespace violet::experimental {

template<typename T, typename U>
struct NOELDOC_EXPERIMENTAL_SINCE("current") Pair final {
    static_assert(!std::is_void_v<T> && !std::is_void_v<U>, "`Pair`: cannot hold `void`");
    static_assert(!std::is_reference_v<T> && !std::is_reference_v<U>, "`Pair`: cannot hold reference members");
    static_assert(!std::is_function_v<T> && !std::is_function_v<U>,
        "`Pair`: cannot hold function types; store a function pointer instead");

    static_assert(!std::is_abstract_v<T> && !std::is_abstract_v<U>,
        "`Pair`: cannot hold an abstract class; store a pointer or `violet::experimental::Own<T>`");

    static_assert(!std::is_array_v<T> && !std::is_array_v<U>,
        "`Pair`: doesn't support raw array members; use `std::array<T, N>` or `violet::experimental::Slice<T, N>`");

    static_assert(std::is_destructible_v<T> && std::is_destructible_v<U>, "`Pair`: members must be destructible");

    using first_type = T;
    using second_type = U;

    VIOLET_NO_UNIQUE_ADDRESS T First{};
    VIOLET_NO_UNIQUE_ADDRESS U Second{};

    constexpr VIOLET_IMPLICIT Pair() noexcept(
        std::is_nothrow_default_constructible_v<T> && std::is_nothrow_default_constructible_v<U>)
        requires(std::default_initializable<T> && std::default_initializable<U>)
    = default;

    constexpr VIOLET_EXPLICIT(!std::convertible_to<const T&, T> || !std::convertible_to<const U&, U>)
        Pair(const T& first, const U& second) noexcept(
            std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_constructible_v<U>)
        requires(std::copy_constructible<T> && std::copy_constructible<U>)
        : First(first)
        , Second(second)
    {
    }

    template<typename X = T, typename Y = U>
        requires(std::constructible_from<T, X> && std::constructible_from<U, Y>)
    constexpr VIOLET_EXPLICIT(!std::convertible_to<X, T> || !std::convertible_to<Y, U>) Pair(
        X&& first, Y&& second) noexcept(std::is_nothrow_constructible_v<T, X> && std::is_nothrow_constructible_v<U, Y>)
        : First(VIOLET_FWD(X, first))
        , Second(VIOLET_FWD(Y, second))
    {
    }

    template<typename X, typename Y>
        requires(std::constructible_from<T, const X&> && std::constructible_from<U, const Y&>)
    constexpr VIOLET_EXPLICIT(!std::convertible_to<const X&, T> || !std::convertible_to<const Y&, U>)
        Pair(const Pair<X, Y>& other) noexcept(
            std::is_nothrow_constructible_v<T, const X&> && std::is_nothrow_constructible_v<U, const Y&>)
        : First(other.First)
        , Second(other.Second)
    {
    }

    template<typename X, typename Y>
        requires(std::constructible_from<T, X> && std::constructible_from<U, Y>)
    constexpr VIOLET_EXPLICIT(!std::convertible_to<X, T> || !std::convertible_to<Y, U>) Pair(
        Pair<X, Y>&& other) noexcept(std::is_nothrow_constructible_v<T, X> && std::is_nothrow_constructible_v<U, Y>)
        : First(VIOLET_MOVE(other.First))
        , Second(VIOLET_MOVE(other.Second))
    {
    }

    constexpr static auto New(T first, U second) -> Pair<std::unwrap_ref_decay_t<T>, std::unwrap_ref_decay_t<U>>
    {
        return Pair(VIOLET_MOVE(first), VIOLET_MOVE(second));
    }

    constexpr void swap(Pair& other) noexcept(std::is_nothrow_swappable_v<T> && std::is_nothrow_swappable_v<U>)
        requires(std::swappable<T> && std::swappable<U>)
    {
        std::ranges::swap(this->First, other.First);
        std::ranges::swap(this->Second, other.Second);
    }

    constexpr friend void swap(Pair& lhs, Pair& rhs) noexcept(noexcept(lhs.swap(rhs)))
        requires(std::swappable<T> && std::swappable<U>)
    {
        lhs.swap(rhs);
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
        return Pair<return_type, second_type>(
            std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->First)), VIOLET_MOVE(this->Second));
    }

    template<typename Fun>
        requires callable<Fun, const T>
    [[nodiscard]]
    constexpr auto MapFirst(Fun&& fun) const&& VIOLET_NOEXCEPT_FUN(fun, const T) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, const T>;
        return Pair<return_type, second_type>(
            std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->First)), VIOLET_MOVE(this->Second));
    }

    template<typename Fun>
        requires callable<Fun, U&>
    [[nodiscard]]
    constexpr auto MapSecond(Fun&& fun)
        & VIOLET_NOEXCEPT_FUN(fun, U&) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, U&>;
        return Pair<first_type, return_type>(this->First, std::invoke(VIOLET_FWD(Fun, fun), this->Second));
    }

    template<typename Fun>
        requires callable<Fun, const U&>
    [[nodiscard]]
    constexpr auto MapSecond(Fun&& fun) const& VIOLET_NOEXCEPT_FUN(fun, const U&) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, const U&>;
        return Pair<first_type, return_type>(this->First, std::invoke(VIOLET_FWD(Fun, fun), this->Second));
    }

    template<typename Fun>
        requires callable<Fun, U>
    [[nodiscard]]
    constexpr auto MapSecond(Fun&& fun)
        && VIOLET_NOEXCEPT_FUN(fun, U) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, U>;
        return Pair<first_type, return_type>(
            VIOLET_MOVE(this->First), std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->Second)));
    }

    template<typename Fun>
        requires callable<Fun, const U>
    [[nodiscard]]
    constexpr auto MapSecond(Fun&& fun) const&& VIOLET_NOEXCEPT_FUN(fun, const U) -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Fun, const U>;
        return Pair<first_type, return_type>(
            VIOLET_MOVE(this->First), std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->Second)));
    }
};

template<typename First, typename Second>
Pair(First, Second) -> Pair<First, Second>;

template<typename T1, typename T2, typename U1, typename U2>
    requires(std::equality_comparable_with<T1, U1> && std::equality_comparable_with<T2, U2>)
constexpr auto operator==(const Pair<T1, T2>& lhs, const Pair<U1, U2>& rhs) -> bool
{
    return lhs.First == rhs.First && lhs.Second == rhs.Second;
}

template<typename T1, typename T2, typename U1, typename U2>
    requires(std::three_way_comparable_with<T1, U1> && std::three_way_comparable_with<T2, U2>)
constexpr auto operator<=>(const Pair<T1, T2>& lhs, const Pair<U1, U2>& rhs)
    -> std::common_comparison_category_t<std::compare_three_way_result_t<T1, U1>,
        std::compare_three_way_result_t<T2, U2>>
{
    if (auto cmp = lhs.First <=> rhs.First; cmp != 0) {
        return cmp;
    }

    return lhs.Second <=> rhs.Second;
}

template<UInt I, typename T, typename U>
    requires(I < 2)
constexpr auto get(Pair<T, U>& pair) noexcept -> std::conditional_t<I == 0, T, U>&
{
    if constexpr (I == 0) {
        return pair.First;
    } else {
        return pair.Second;
    }
}

template<UInt I, typename T, typename U>
    requires(I < 2)
constexpr auto get(const Pair<T, U>& pair) noexcept -> const std::conditional_t<I == 0, T, U>&
{
    if constexpr (I == 0) {
        return pair.First;
    } else {
        return pair.Second;
    }
}

template<UInt I, typename T, typename U>
    requires(I < 2)
constexpr auto get(Pair<T, U>&& pair) noexcept -> std::conditional_t<I == 0, T, U>&&
{
    if constexpr (I == 0) {
        return VIOLET_MOVE(pair.First);
    } else {
        return VIOLET_MOVE(pair.Second);
    }
}

template<UInt I, typename T, typename U>
    requires(I < 2)
constexpr auto get(const Pair<T, U>&& pair) noexcept -> const std::conditional_t<I == 0, T, U>&&
{
    if constexpr (I == 0) {
        return VIOLET_MOVE(pair.First);
    } else {
        return VIOLET_MOVE(pair.Second);
    }
}

template<typename X, typename T, typename U>
    requires(std::same_as<X, T> != std::same_as<X, U>)
constexpr auto get(Pair<T, U>& pair) noexcept -> X&
{
    return get<std::same_as<X, T> ? 0 : 1>(pair);
}

template<typename X, typename T, typename U>
    requires(std::same_as<X, T> != std::same_as<X, U>)
constexpr auto get(const Pair<T, U>& pair) noexcept -> const X&
{
    return get<std::same_as<X, T> ? 0 : 1>(pair);
}

template<typename X, typename T, typename U>
    requires(std::same_as<X, T> != std::same_as<X, U>)
constexpr auto get(Pair<T, U>&& pair) noexcept -> X&&
{
    return get<std::same_as<X, T> ? 0 : 1>(VIOLET_MOVE(pair));
}

template<typename X, typename T, typename U>
    requires(std::same_as<X, T> != std::same_as<X, U>)
constexpr auto get(const Pair<T, U>&& pair) noexcept -> const X&&
{
    return get<std::same_as<X, T> ? 0 : 1>(VIOLET_MOVE(pair));
}

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
struct std::tuple_size<violet::experimental::Pair<First, Second>>: public std::integral_constant<violet::UInt, 2> { };

template<violet::UInt I, typename First, typename Second>
struct std::tuple_element<I, violet::experimental::Pair<First, Second>> {
    static_assert(I < 2, "`Pair` only consists of two types");
};

template<typename First, typename Second>
struct std::tuple_element<0L, violet::experimental::Pair<First, Second>> {
    using type = First;
};

template<typename First, typename Second>
struct std::tuple_element<1L, violet::experimental::Pair<First, Second>> {
    using type = Second;
};
