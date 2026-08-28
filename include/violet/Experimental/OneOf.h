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
//! # 🌺💜 `violet/Experimental/OneOf.h`

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Experimental/Slice.h>

namespace violet::experimental {
namespace oneof_internal {

/// A recursive union that never enters a valueless state. Every member is
/// formally active from the union's perspective; construction, destruction,
/// and access are managed manually via placement-new and explicit destructor
/// calls, tracked by the owning [`OneOf`]'s active index.
///
/// The empty specialisation `storage<>` acts as the recursion terminator and carries no data.
template<typename... Ts>
union storage;

template<typename T, typename... Ts>
union storage<T, Ts...> final { // NOLINT(cppcoreguidelines-special-member-functions)
    T Head;
    storage<Ts...> Tail;

    constexpr storage() noexcept { }
    constexpr ~storage() noexcept { }
};

template<>
union storage<> final {
};

/// Returns a reference to the element at compile-time index `I` inside
/// `storage`.
///
/// ## Panics
/// Out-of-bounds `I` is a hard compile error, the recursion hits `storage<>` which has no `Head` member.
template<UInt Index, typename... Ts>
constexpr auto GetElement(storage<Ts...>& storage) noexcept -> auto&
{
    if constexpr (Index == 0) {
        return storage.Head;
    } else {
        return GetElement<Index - 1>(storage.Tail);
    }
}

/// Returns a const reference to the element at compile-time index `I` inside
/// `storage`.
///
/// ## Panics
/// Out-of-bounds `I` is a hard compile error, the recursion hits `storage<>` which has no `Head` member.
template<UInt Index, typename... Ts>
constexpr auto GetElement(const storage<Ts...>& storage) noexcept -> const auto&
{
    if constexpr (Index == 0) {
        return storage.Head;
    } else {
        return GetElement<Index - 1>(storage.Tail);
    }
}

/// Placement-constructs the element at compile-time index `Index` inside `storage`, activating every
/// intermediate `Tail` union along the way.
///
/// This is required for the construction to be valid in a constant expression: a nested union member can
/// only be placement-constructed once its immediate parent union has itself been made active, which a plain
/// `::new (std::addressof(GetElement<Index>(storage))) T(...)` does not do for `Index > 0`.
template<UInt Index, typename... Ts, typename... Args>
constexpr void ConstructElement(storage<Ts...>& storage, Args&&... args)
{
    if constexpr (Index == 0) {
        ::new (std::addressof(storage.Head)) pack_element_t<0, Ts...>(VIOLET_FWD(Args, args)...);
    } else {
        ::new (std::addressof(storage.Tail)) std::decay_t<decltype(storage.Tail)>();
        ConstructElement<Index - 1>(storage.Tail, VIOLET_FWD(Args, args)...);
    }
}

/// Calls the destructor of the element at runtime index `active` inside `storage`, then returns. Does nothing if
/// `Index >= sizeof...(Ts)`.
template<UInt Index, typename... Ts>
constexpr void DestroyActiveElementInStorage(UInt active, storage<Ts...>& storage)
{
    if constexpr (Index < sizeof...(Ts)) {
        if (active == Index) {
            std::destroy_at(std::addressof(GetElement<Index>(storage)));
            return;
        }

        DestroyActiveElementInStorage<Index + 1, Ts...>(active, storage);
    }
}

/// Builds a constexpr jump table of `sizeof...(Is)` function pointers.
///
/// Each entry `table[I]` calls `visitor` with the element at index `I`
/// inside `storage`. Passing `const Storage` as the `Storage` template
/// argument produces a table whose entries accept a `const Storage&`,
/// resolving to the const overload of `getElementInStorage` automatically.
///
/// There is intentionally only one overload, const-ness is encoded in
/// the `Storage` type parameter so overload resolution is never ambiguous.
template<typename ReturnType, typename Visitor, typename Storage, UInt... Is>
constexpr auto CreateVisitorDispatchTable(std::index_sequence<Is...>)
    -> Slice<ReturnType (*)(Visitor&&, Storage&), sizeof...(Is)>
{
    using function = ReturnType (*)(Visitor&&, Storage&);
    return Slice<function, sizeof...(Is)>{
        // clang-format off
        +[](Visitor&& visitor, Storage& storage) -> ReturnType {
            return std::invoke(VIOLET_MOVE(visitor), GetElement<Is>(storage));
        }...
        // clang-format on
    };
}

template<typename ReturnType, typename Visitor, typename Storage, typename... Ts>
constexpr inline auto DispatchTableFor
    = CreateVisitorDispatchTable<ReturnType, Visitor, Storage>(std::index_sequence_for<Ts...>{});

} // namespace oneof_internal

/// A well-behaved, empty type that is usable as a placeholder alternative in
/// a [`OneOf`] object to represent the absense of a value.
///
/// **Mono** is comparable, hashable, and trivally copyable. All comparisons between two **Mono**
/// objects are equal.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/OneOf.h>
///
/// violet::experimental::OneOf<violet::experimental::Mono, int> x;
/// ```
struct VIOLET_API Mono final {
    constexpr auto operator<=>(Mono) const noexcept -> std::strong_ordering
    {
        return std::strong_ordering::equal;
    }

    constexpr auto operator==(Mono) const noexcept -> bool
    {
        return true;
    }
};

template<typename... Fs>
struct Overloaded final: std::decay_t<Fs>... {
    using std::decay_t<Fs>::operator()...;
};

template<typename... Ts>
struct OneOf {
    static_assert(sizeof...(Ts) > 0, "`OneOf` requires atleast one type to be present");
    static_assert((std::is_destructible_v<Ts> && ...), "`OneOf` requires all types to be destructible");

    template<UInt Index>
    using TypeAt = pack_element_t<Index, Ts...>;

    template<typename T>
        requires pack_contains_v<T, Ts...>
    constexpr static UInt IndexOf = pack_index_v<T, Ts...>;

    constexpr VIOLET_IMPLICIT OneOf() noexcept
        requires(std::default_initializable<TypeAt<0L>>)
        : OneOf(static_cast<UInt>(0))
    {
        oneof_internal::ConstructElement<0>(this->n_storage);
    }

    template<typename U>
        requires(pack_contains_v<U, Ts...> && (!std::same_as<U, OneOf>))
    constexpr VIOLET_IMPLICIT OneOf(U&& value) noexcept(std::is_nothrow_move_constructible_v<U>)
        : OneOf(IndexOf<U>)
    {
        oneof_internal::ConstructElement<IndexOf<U>>(this->n_storage, VIOLET_FWD(U, value));
    }

    template<typename U>
        requires(pack_contains_v<U, Ts...> && (!std::same_as<U, OneOf>))
    constexpr auto operator=(U&& value) -> OneOf&
    {
        OneOf tmp(VIOLET_FWD(U, value));
        swap(*this, tmp);

        return *this;
    }

    constexpr VIOLET_IMPLICIT OneOf(const OneOf& other)
        requires(std::is_copy_constructible_v<Ts> && ...)
        : OneOf(other.n_index)
    {
        other.Visit([&](const auto& value) -> void {
            using type = std::decay_t<decltype(value)>;
            oneof_internal::ConstructElement<IndexOf<type>>(this->n_storage, value);
        });
    }

    VIOLET_IMPLICIT OneOf(const OneOf&)
        requires(!(std::is_copy_constructible_v<Ts> && ...))
    = delete; // reason: one of `T` in `Ts` is not copy-constructible

    constexpr auto operator=(const OneOf& other) -> OneOf&
        requires(std::is_copy_constructible_v<Ts> && ...)
    {
        if (this != &other) {
            OneOf tmp(other);
            swap(*this, tmp);
        }

        return *this;
    }

    auto operator=(const OneOf& other) noexcept -> OneOf
        requires(!(std::is_copy_constructible_v<Ts> && ...))
    = delete; // cannot assign a copy because one of `T` in `Ts` is not copy-constructible

    constexpr VIOLET_IMPLICIT OneOf(OneOf&& other) noexcept
        : OneOf(other.n_index)
    {
        VIOLET_MOVE(other).Visit([&](auto&& value) -> void {
            using type = std::decay_t<decltype(value)>;
            oneof_internal::ConstructElement<IndexOf<type>>(this->n_storage, VIOLET_MOVE(value));
        });
    }

    constexpr auto operator=(OneOf&& other) noexcept -> OneOf&
    {
        if (this != &other) {
            OneOf tmp(VIOLET_MOVE(other));
            swap(*this, tmp);
        }

        return *this;
    }

    constexpr ~OneOf()
    {
        oneof_internal::DestroyActiveElementInStorage<0, Ts...>(this->n_index, this->n_storage);
    }

    template<typename T, typename... Args>
        requires(pack_contains_v<T, Ts...> && std::is_constructible_v<T, Args...>)
    constexpr static auto New(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) -> OneOf
    {
        return OneOf::make<IndexOf<T>>(T(VIOLET_FWD(Args, args)...));
    }

    [[nodiscard]] constexpr auto Index() const noexcept -> UInt
    {
        return this->n_index;
    }

    template<typename T>
    [[nodiscard]] constexpr auto Holds() const noexcept -> bool
    {
        return this->n_index == IndexOf<T>;
    }

    template<typename T>
        requires pack_contains_v<T, Ts...>
    constexpr auto Get() noexcept -> Optional<std::reference_wrapper<T>>
    {
        if (this->n_index != IndexOf<T>) {
            return Nothing;
        }

        return Some(std::ref(oneof_internal::GetElement<IndexOf<T>>(this->n_storage)));
    }

    template<typename T>
        requires pack_contains_v<T, Ts...>
    constexpr auto GetUnchecked(Unsafe) noexcept -> T&
    {
        VIOLET_ASSUME(this->n_index == IndexOf<T>);
        return std::ref(oneof_internal::GetElement<IndexOf<T>>(this->n_storage));
    }

    template<typename T>
        requires pack_contains_v<T, Ts...>
    constexpr auto Get() const noexcept -> Optional<std::reference_wrapper<const T>>
    {
        if (this->n_index != IndexOf<T>) {
            return Nothing;
        }

        return Some(std::cref(oneof_internal::GetElement<IndexOf<T>>(this->n_storage)));
    }

    template<typename T>
        requires pack_contains_v<T, Ts...>
    constexpr auto GetUnchecked(Unsafe) const noexcept -> T&
    {
        VIOLET_ASSUME(this->n_index == IndexOf<T>);
        return std::ref(oneof_internal::GetElement<IndexOf<T>>(this->n_storage));
    }

    template<typename Visitor>
    constexpr auto Visit(Visitor&& visitor) & -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Visitor, std::tuple_element_t<0, std::tuple<Ts...>>&>;
        static_assert((std::is_same_v<return_type, std::invoke_result_t<Visitor, Ts&>> && ...),
            "all `OneOf::Visit` alternatives must return the same type");

        return std::invoke(
            // clang-format off
            oneof_internal::DispatchTableFor<return_type, std::decay_t<Visitor>, oneof_internal::storage<Ts...>, Ts...>[this->n_index],
            VIOLET_FWD(Visitor, visitor), this->n_storage
            // clang-format on
        );
    }

    template<typename Visitor>
    constexpr auto Visit(Visitor&& visitor) const& -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Visitor, std::tuple_element_t<0, std::tuple<Ts...>>&>;
        static_assert((std::is_same_v<return_type, std::invoke_result_t<Visitor, const Ts&>> && ...),
            "all `OneOf::Visit` alternatives must return the same type");

        return std::invoke(
            // clang-format off
            oneof_internal::DispatchTableFor<return_type, std::decay_t<Visitor>, const oneof_internal::storage<Ts...>, Ts...>[this->n_index],
            VIOLET_FWD(Visitor, visitor), this->n_storage
            // clang-format on
        );
    }

    template<typename Visitor>
    constexpr auto Visit(Visitor&& visitor) && -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Visitor, std::tuple_element_t<0, std::tuple<Ts...>>&>;
        static_assert((std::is_same_v<return_type, std::invoke_result_t<Visitor, Ts&&>> && ...),
            "all `OneOf::Visit` alternatives must return the same type");

        return std::invoke(
            // clang-format off
            oneof_internal::DispatchTableFor<return_type, std::decay_t<Visitor>, oneof_internal::storage<Ts...>, Ts...>[this->n_index],
            VIOLET_FWD(Visitor, visitor), this->n_storage
            // clang-format on
        );
    }

    template<typename Visitor>
    constexpr auto Visit(Visitor&& visitor) const&& -> decltype(auto)
    {
        using return_type = std::invoke_result_t<Visitor, std::tuple_element_t<0, std::tuple<Ts...>>&>;
        static_assert((std::is_same_v<return_type, std::invoke_result_t<Visitor, const Ts&&>> && ...),
            "all `OneOf::Visit` alternatives must return the same type");

        return std::invoke(
            // clang-format off
            oneof_internal::DispatchTableFor<return_type, std::decay_t<Visitor>, const oneof_internal::storage<Ts...>, Ts...>[this->n_index],
            VIOLET_FWD(Visitor, visitor), this->n_storage
            // clang-format on
        );
    }

    template<typename... Fs>
    constexpr auto Match(Fs&&... funcs) & -> decltype(auto)
    {
        return this->Visit(Overloaded<Fs...>{VIOLET_FWD(Fs, funcs)...});
    }

    template<typename... Fs>
    constexpr auto Match(Fs&&... funcs) const& -> decltype(auto)
    {
        return this->Visit(Overloaded<Fs...>{VIOLET_FWD(Fs, funcs)...});
    }

    template<typename... Fs>
    constexpr auto Match(Fs&&... funcs) && -> decltype(auto)
    {
        return VIOLET_MOVE(*this).Visit(Overloaded<Fs...>{VIOLET_FWD(Fs, funcs)...});
    }

    constexpr friend void swap(OneOf& o1, OneOf& o2) noexcept
    {
        if (o1.n_index == o2.n_index) {
            o1.Visit([&](auto& av) -> void {
                using type = std::decay_t<decltype(av)>;

                type tmp(VIOLET_MOVE(av));
                std::destroy_at(std::addressof(av));
                std::construct_at(std::addressof(av),
                    VIOLET_MOVE(o2.GetUnchecked<type>(Unsafe("already checked if we are at the same index"))));

                std::construct_at(
                    std::addressof(oneof_internal::GetElement<IndexOf<type>>(o2.n_storage)), VIOLET_MOVE(tmp));
            });
        } else {
            OneOf tmp(VIOLET_MOVE(o1));
            std::destroy_at(&o1);
            std::construct_at(&o1, VIOLET_MOVE(o2));

            std::destroy_at(&o2);
            std::construct_at(&o2, VIOLET_MOVE(tmp));
        }
    }

    constexpr auto operator==(const OneOf& other) const noexcept -> bool
        requires(std::equality_comparable<Ts> && ...)
    {
        if (this->Index() != other.Index()) {
            return false;
        }

        return this->Visit([&](const auto& value) -> bool {
            using type = std::decay_t<decltype(value)>;
            return value == *other.Get<type>();
        });
    }

    constexpr auto operator!=(const OneOf& other) const noexcept -> bool
        requires(std::equality_comparable<Ts> && ...)
    {
        return !(*this == other);
    }

private:
    UInt n_index = 0;
    oneof_internal::storage<Ts...> n_storage;

    constexpr VIOLET_EXPLICIT OneOf(UInt index) noexcept
        : n_index(index)
    {
    }

    template<UInt Index, typename U>
    constexpr static auto make(U&& value) noexcept(std::is_nothrow_move_constructible_v<U>) -> OneOf
    {
        OneOf result(Index);
        oneof_internal::ConstructElement<Index>(result.n_storage, VIOLET_FWD(U, value));

        return result;
    }
};

} // namespace violet::experimental

template<>
struct std::hash<violet::experimental::Mono> final {
    constexpr auto operator()(violet::experimental::Mono) const noexcept -> violet::UInt
    {
        return 0;
    }
};
