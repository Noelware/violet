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
//! # 🌺💜 `violet/Experimental/Tuple.h`

#pragma once

#include <violet/Violet.h>

// TODO(@auguwu/Noel): add conversion operators from std::tuple <-> violet::Tuple
#ifndef VIOLET_IMPLEMENT_BACKWARDS_STL_TUPLE
#define VIOLET_IMPLEMENT_BACKWARDS_STL_TUPLE 0
#endif

#if VIOLET_IMPLEMENT_BACKWARDS_STL_TUPLE
#include <tuple>
#endif

namespace violet::experimental {

template<typename... Ts>
class Tuple;

namespace tuple_internal {
template<UInt Index, typename T>
struct Leaf {
    VIOLET_NO_UNIQUE_ADDRESS T Value;

    constexpr Leaf() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires(std::default_initializable<T>)
        : Value()
    {
    }

    template<typename U>
        requires(std::constructible_from<T, U &&>)
    constexpr VIOLET_EXPLICIT Leaf(U&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : Value(VIOLET_FWD(U, other))
    {
    }
};

template<typename Sequence, typename... Ts>
struct Storage;

template<UInt... Is, typename... Ts>
struct Storage<std::index_sequence<Is...>, Ts...>: Leaf<Is, Ts>... {
    constexpr Storage() = default;

    template<typename... Us>
        requires(sizeof...(Us) == sizeof...(Ts))
    constexpr VIOLET_EXPLICIT Storage(Us&&... us)
        : Leaf<Is, Ts>(VIOLET_FWD(Us, us))...
    {
    }
};

template<typename X, typename... Ts>
consteval auto IndexOf() noexcept -> UInt
{
    UInt index = 0;
    (void)((!std::same_as<X, Ts> && (++index, true)) && ...);

    return index;
}

struct from_tag_t final { };
constexpr inline from_tag_t from_tag{};

} // namespace tuple_internal

template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("current") is_tuple final: public std::false_type { };

template<typename... Ts>
struct is_tuple<Tuple<Ts...>> final: public std::true_type { };

#ifdef VIOLET_IMPLEMENT_BACKWARDS_STL_TUPLE
template<typename... Ts>
struct is_tuple<std::tuple<Ts...>> final: public std::true_type { };
#endif

template<typename T>
NOELDOC_EXPERIMENTAL_SINCE("current")
constexpr inline bool is_tuple_v = is_tuple<T>::value;

/// @since current
template<typename T>
concept tuple_like = is_tuple_v<std::remove_cvref_t<T>>;

template<typename... Ts>
class NOELDOC_EXPERIMENTAL_SINCE("current") Tuple final
    : tuple_internal::Storage<std::index_sequence_for<Ts...>, Ts...> {
    using Storage = tuple_internal::Storage<std::index_sequence_for<Ts...>, Ts...>;

    template<UInt I>
    using element_type = pack_element_t<I, Ts...>;

    template<UInt I>
    using leaf_type = tuple_internal::Leaf<I, element_type<I>>;

public:
    constexpr VIOLET_IMPLICIT Tuple()
        requires(std::default_initializable<Ts> && ...)
    = default;

    constexpr ~Tuple() = default;

    template<typename... Us>
        requires((sizeof...(Us) == sizeof...(Ts)) && (sizeof...(Ts) >= 1) && (std::constructible_from<Ts, Us &&> && ...)
            && !(sizeof...(Us) == 1 && (std::same_as<std::remove_cvref_t<Us>, Tuple> && ...)))
    constexpr VIOLET_EXPLICIT((!std::convertible_to<Us&&, Ts> || ...)) Tuple(Us&&... us)
        : Storage(VIOLET_FWD(Us, us)...)
    {
    }

    template<typename... Us>
        requires((sizeof...(Us) == sizeof...(Ts)) && (std::constructible_from<Ts, const Us&> && ...)
            && (!std::same_as<Tuple<Us...>, Tuple>))
    constexpr VIOLET_EXPLICIT((!std::convertible_to<const Us&, Ts> || ...)) Tuple(const Tuple<Us...>& other)
        : Tuple(tuple_internal::from_tag, other, std::index_sequence_for<Ts...>())
    {
    }

    template<typename... Us>
        requires((sizeof...(Us) == sizeof...(Ts)) && (std::constructible_from<Ts, Us &&> && ...)
            && (!std::same_as<Tuple<Us...>, Tuple>))
    constexpr VIOLET_EXPLICIT((!std::convertible_to<Us&&, Ts> || ...)) Tuple(Tuple<Us...>&& other)
        : Tuple(tuple_internal::from_tag, VIOLET_MOVE(other), std::index_sequence_for<Ts...>())
    {
    }

    constexpr VIOLET_IMPLICIT Tuple(const Tuple&) = default;
    constexpr VIOLET_IMPLICIT Tuple(Tuple&&) noexcept = default;

    constexpr auto operator=(const Tuple&) -> Tuple& = default;
    constexpr auto operator=(Tuple&&) noexcept -> Tuple& = default;

    constexpr auto operator=(const Tuple& other) -> Tuple&
        requires((std::is_reference_v<Ts> || ...) && (std::assignable_from<Ts&, const Ts&> && ...))
    {
        this->assignFrom(other, std::index_sequence_for<Ts...>());
        return *this;
    }

    template<typename... Us>
        requires((sizeof...(Us) == sizeof...(Ts)) && (std::assignable_from<Ts&, const Us&> && ...)
            && (!std::same_as<Tuple<Us...>, Tuple>))
    constexpr auto operator=(const Tuple<Us...>& other) -> Tuple&
    {
        this->assignFrom(other, std::index_sequence_for<Ts...>());
        return *this;
    }

    template<typename... Us>
        requires((sizeof...(Us) == sizeof...(Ts)) && (std::assignable_from<Ts&, Us &&> && ...)
            && (!std::same_as<Tuple<Us...>, Tuple>))
    constexpr auto operator=(Tuple<Us...>&& other) -> Tuple&
    {
        this->assignFrom(VIOLET_MOVE(other), std::index_sequence_for<Ts...>());
        return *this;
    }

    [[nodiscard]]
    constexpr static auto Size() noexcept -> UInt
    {
        return sizeof...(Ts);
    }

    template<typename... Us>
    [[nodiscard]]
    constexpr static auto New(Us&&... us) -> Tuple<std::unwrap_ref_decay_t<Us>...>
    {
        return Tuple<std::unwrap_ref_decay_t<Us>...>(VIOLET_FWD(Us, us)...);
    }

    template<typename... Us>
    [[nodiscard]]
    constexpr static auto FwdAs(Us&&... us) noexcept -> Tuple<Us&&...>
    {
        return Tuple<Us&&...>(VIOLET_FWD(Us, us)...);
    }

    template<UInt I>
        requires(I < Tuple::Size())
    constexpr auto At() & noexcept -> element_type<I>&
    {
        return static_cast<leaf_type<I>&>(*this).Value;
    }

    template<UInt I>
        requires(I < Tuple::Size())
    constexpr auto At() const& noexcept -> const element_type<I>&
    {
        return static_cast<const leaf_type<I>&>(*this).Value;
    }

    template<UInt I>
        requires(I < Tuple::Size())
    constexpr auto At() && noexcept -> element_type<I>&&
    {
        return static_cast<element_type<I>&&>(static_cast<leaf_type<I>&>(*this).Value);
    }

    template<UInt I>
        requires(I < Tuple::Size())
    constexpr auto At() const&& noexcept -> const element_type<I>&&
    {
        return static_cast<const element_type<I>&&>(static_cast<const leaf_type<I>&>(*this).Value);
    }

    template<typename Fun>
    constexpr auto Apply(Fun&& fun) & -> decltype(auto)
    {
        return Tuple::applyTo(*this, VIOLET_FWD(Fun, fun));
    }

    template<typename Fun>
    constexpr auto Apply(Fun&& fun) const& -> decltype(auto)
    {
        return Tuple::applyTo(*this, VIOLET_FWD(Fun, fun));
    }

    template<typename Fun>
    constexpr auto Apply(Fun&& fun) && -> decltype(auto)
    {
        return Tuple::applyTo(VIOLET_MOVE(*this), VIOLET_FWD(Fun, fun));
    }

    template<typename Fun>
    constexpr auto Apply(Fun&& fun) const&& -> decltype(auto)
    {
        return Tuple::applyTo(VIOLET_MOVE(*this), VIOLET_FWD(Fun, fun));
    }

    template<typename... Us>
    constexpr auto Concat(Tuple<Us...>&& other) && -> Tuple<Ts..., Us...>
    {
        return [&]<UInt... Is, UInt... Js>(std::index_sequence<Is...>, std::index_sequence<Js...>) -> auto {
            return Tuple<Ts..., Us...>(
                VIOLET_MOVE(*this).template At<Is>()..., VIOLET_MOVE(other).template At<Js>()...);
        }(std::index_sequence_for<Ts...>(), std::index_sequence_for<Us...>());
    }

    constexpr void swap(Tuple& other) noexcept((std::is_nothrow_swappable_v<Ts> && ...))
        requires(std::swappable<Ts> && ...)
    {
        [&]<UInt... Is>(std::index_sequence<Is...>) -> void {
            (std::ranges::swap(this->template At<Is>(), other.template At<Is>()), ...);
        }(std::index_sequence_for<Ts...>());
    }

    constexpr friend void swap(Tuple& lhs, Tuple& rhs) noexcept(noexcept(lhs.swap(rhs)))
        requires(std::swappable<Ts> && ...)
    {
        lhs.swap(rhs);
    }

private:
    template<typename Other, UInt... Is>
    constexpr VIOLET_IMPLICIT Tuple(tuple_internal::from_tag_t, Other&& other, std::index_sequence<Is...>)
        : Storage(VIOLET_FWD(Other, other).template At<Is>()...)
    {
    }

    template<typename Other, UInt... Is>
    constexpr void assignFrom(Other&& other, std::index_sequence<Is...>)
    {
        ((this->template At<Is>() = VIOLET_FWD(Other, other).template At<Is>()), ...);
    }

    template<typename Self, typename Fun>
    constexpr static auto applyTo(Self&& self, Fun&& fun) -> decltype(auto)
    {
        return [&]<UInt... Is>(std::index_sequence<Is...>) -> decltype(auto) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_FWD(Self, self).template At<Is>()...);
        }(std::index_sequence_for<Ts...>());
    }
};

template<typename... Us>
Tuple(Us...) -> Tuple<Us...>;

template<typename... Ts, typename... Us>
    requires(sizeof...(Ts) == sizeof...(Us)) && (std::equality_comparable_with<Ts, Us> && ...)
constexpr auto operator==(const Tuple<Ts...>& lhs, const Tuple<Us...>& rhs) -> bool
{
    return [&]<UInt... Is>(std::index_sequence<Is...>) -> bool {
        return ((lhs.template At<Is>() == rhs.template At<Is>()) && ...);
    }(std::index_sequence_for<Ts...>());
}

template<typename... Ts, typename... Us>
    requires(sizeof...(Ts) == sizeof...(Us)) && (std::three_way_comparable_with<Ts, Us> && ...)
constexpr auto operator<=>(const Tuple<Ts...>& lhs, const Tuple<Us...>& rhs)
    -> std::common_comparison_category_t<std::compare_three_way_result_t<Ts, Us>...>
{
    using category = std::common_comparison_category_t<std::compare_three_way_result_t<Ts, Us>...>;

    category result = std::strong_ordering::equal;
    [&]<UInt... Is>(std::index_sequence<Is...>) -> void {
        (void)(((result = lhs.template At<Is>() <=> rhs.template At<Is>()) != 0) || ...);
    }(std::index_sequence_for<Ts...>());

    return result;
}

template<UInt I, typename... Ts>
    requires(I < sizeof...(Ts))
constexpr auto get(Tuple<Ts...>& tuple) noexcept -> pack_element_t<I, Ts...>&
{
    return tuple.template At<I>();
}

template<UInt I, typename... Ts>
    requires(I < sizeof...(Ts))
constexpr auto get(const Tuple<Ts...>& tuple) noexcept -> const pack_element_t<I, Ts...>&
{
    return tuple.template At<I>();
}

template<UInt I, typename... Ts>
    requires(I < sizeof...(Ts))
constexpr auto get(Tuple<Ts...>&& tuple) noexcept -> pack_element_t<I, Ts...>&&
{
    return VIOLET_MOVE(tuple).template At<I>();
}

template<UInt I, typename... Ts>
    requires(I < sizeof...(Ts))
constexpr auto get(const Tuple<Ts...>&& tuple) noexcept -> const pack_element_t<I, Ts...>&&
{
    return VIOLET_MOVE(tuple).template At<I>();
}

template<typename X, typename... Ts>
    requires((std::same_as<X, Ts> + ...) == 1)
constexpr auto get(Tuple<Ts...>& tuple) noexcept -> X&
{
    return tuple.template At<tuple_internal::IndexOf<X, Ts...>()>();
}

template<typename X, typename... Ts>
    requires((std::same_as<X, Ts> + ...) == 1)
constexpr auto get(const Tuple<Ts...>& tuple) noexcept -> const X&
{
    return tuple.template At<tuple_internal::IndexOf<X, Ts...>()>();
}

template<typename X, typename... Ts>
    requires((std::same_as<X, Ts> + ...) == 1)
constexpr auto get(Tuple<Ts...>&& tuple) noexcept -> X&&
{
    return VIOLET_MOVE(tuple).template At<tuple_internal::IndexOf<X, Ts...>()>();
}

template<typename X, typename... Ts>
    requires((std::same_as<X, Ts> + ...) == 1)
constexpr auto get(const Tuple<Ts...>&& tuple) noexcept -> const X&&
{
    return VIOLET_MOVE(tuple).template At<tuple_internal::IndexOf<X, Ts...>()>();
}

} // namespace violet::experimental

template<typename... Ts>
    requires((std::formattable<Ts, char> && ...))
struct std::formatter<violet::experimental::Tuple<Ts...>, char> final: public std::formatter<violet::String> {
    constexpr formatter() noexcept = default;

    constexpr auto parse(std::format_parse_context& pcx)
    {
        return pcx.begin();
    }

    template<typename FormatCx>
    auto format(const violet::experimental::Tuple<Ts...>& tuple, FormatCx& cx) const
    {
        // formats as '(T1, T2, Tn...)'
        auto out = cx.out();
        *out++ = '(';

        tuple.Apply([](const auto&... elems) -> void {
            bool first = true;
            ((out = std::format_to(out, "{}{}", first ? "" : ", ", elems), first = false), ...);
        });

        *out++ = ')';
        return out;
    }
};

template<typename... Ts>
struct std::tuple_size<violet::experimental::Tuple<Ts...>>: public std::integral_constant<violet::UInt, sizeof...(Ts)> {
};

template<violet::UInt I, typename... Ts>
struct std::tuple_element<I, violet::experimental::Tuple<Ts...>> {
    using type = violet::pack_element_t<I, Ts...>;
};
