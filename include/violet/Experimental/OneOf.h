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
#include <violet/Violet.h>

namespace violet::experimental {

namespace detail {

    /// A recursive union that never enters a valueless state. Every member is
    /// formally active from the union's perspective; construction, destruction,
    /// and access are managed manually via placement-new and explicit destructor
    /// calls, tracked by the owning [`OneOf`]'s active index.
    ///
    /// The empty specialisation `valueless_storage<>` acts as the recursion
    /// terminator and carries no data.
    template<typename... Ts>
    union valueless_storage;

    /// Returns a reference to the element at compile-time index `I` inside
    /// `storage`.
    ///
    /// # Panics
    /// Out-of-bounds `I` is a hard compile error — the recursion hits
    /// `valueless_storage<>` which has no `Head` member.
    template<UInt I, typename... Ts>
    constexpr auto getElementInStorage(valueless_storage<Ts...>& storage) noexcept -> auto&;

    /// Returns a const reference to the element at compile-time index `I`
    /// inside `storage`.
    template<UInt I, typename... Ts>
    constexpr auto getElementInStorage(const valueless_storage<Ts...>& storage) noexcept -> const auto&;

    /// Calls the destructor of the element at runtime index `active` inside
    /// `storage`, then returns. Does nothing if `Index >= sizeof...(Ts)`.
    template<UInt Index, typename... Ts>
    void destroyActiveElementInStorage(UInt active, valueless_storage<Ts...>& storage);

    /// Builds a constexpr jump table of `sizeof...(Is)` function pointers.
    ///
    /// Each entry `table[I]` calls `visitor` with the element at index `I`
    /// inside `storage`. Passing `const Storage` as the `Storage` template
    /// argument produces a table whose entries accept a `const Storage&`,
    /// resolving to the const overload of `getElementInStorage` automatically.
    ///
    /// There is intentionally only one overload — const-ness is encoded in
    /// the `Storage` type parameter so overload resolution is never ambiguous.
    template<typename Ret, typename Visitor, typename Storage, UInt... Is>
    constexpr auto createVisitorTable(std::index_sequence<Is...>) -> Array<Ret (*)(Visitor&&, Storage&), sizeof...(Is)>;

} // namespace detail

/// A discriminated union similar to [`std::visit`] but possess no valueless_by_exception state,
/// O(1) jump-table visitation, and much more.
template<typename... Ts>
struct OneOf final {
    static_assert(sizeof...(Ts) > 0, "`OneOf` requires atleast one type to be present");
    static_assert((std::is_destructible_v<Ts> && ...), "`OneOf` requires all types to be destructible");

    template<UInt Index>
    using TypeAt = pack_element_t<Index, Ts...>;

    template<typename T>
        requires(pack_contains_v<std::decay_t<T>, Ts...>)
    constexpr static UInt IndexOf = pack_index_v<std::decay_t<T>, Ts...>;

    template<typename T, typename... Args>
        requires(pack_contains_v<T, Ts...> && std::is_constructible_v<T, Args...>)
    static auto New(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) -> OneOf
    {
        OneOf result;
        ::new (std::addressof(detail::getElementInStorage<IndexOf<T>>(result.n_storage))) T(VIOLET_FWD(Args, args)...);

        result.n_index = IndexOf<T>;
        return result;
    }

    template<typename T>
        requires(pack_contains_v<std::decay_t<T>, Ts...> && (!std::is_same_v<std::decay_t<T>, OneOf>))
    VIOLET_IMPLICIT OneOf(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_index(IndexOf<T>)
    {
        ::new (std::addressof(detail::getElementInStorage<IndexOf<T>>(this->n_storage)))
            std::decay_t<T>(VIOLET_FWD(T, value));
    }

    VIOLET_IMPLICIT OneOf(const OneOf& other)
        : n_index(other.n_index)
    {
        other.Visit([&](const auto& value) -> void {
            using T = std::decay_t<decltype(value)>;
            ::new (std::addressof(detail::getElementInStorage<IndexOf<T>>(this->n_storage))) T(value);
        });
    }

    auto operator=(const OneOf& other) -> OneOf&
    {
        if (this != &other) {
            OneOf tmp(other);
            swap(*this, tmp);
        }

        return *this;
    }

    VIOLET_IMPLICIT OneOf(OneOf&& other) noexcept
        : n_index(other.n_index)
    {
        VIOLET_MOVE(other).Visit([&](auto&& value) -> void {
            using T = std::decay_t<decltype(value)>;
            ::new (std::addressof(detail::getElementInStorage<IndexOf<T>>(this->n_storage))) T(VIOLET_MOVE(value));
        });
    }

    auto operator=(OneOf&& other) noexcept -> OneOf&
    {
        if (this != &other) {
            OneOf tmp(VIOLET_MOVE(other));
            swap(*this, tmp);
        }

        return *this;
    }

    template<typename T>
        requires(pack_contains_v<std::decay_t<T>, Ts...> && (!std::is_same_v<std::decay_t<T>, OneOf>))
    auto operator=(T&& value) -> OneOf&
    {
        OneOf tmp(VIOLET_FWD(T, value));
        swap(*this, tmp);

        return *this;
    }

    ~OneOf()
    {
        detail::destroyActiveElementInStorage<0, Ts...>(this->n_index, this->n_storage);
    }

    /// Returns the zero-based index of the currently active type.
    [[nodiscard]] constexpr auto Index() const noexcept -> UInt
    {
        return this->n_index;
    }

    /// Returns `true` if the currently active type is `T`.
    template<typename T>
    [[nodiscard]] constexpr auto Holds() const noexcept -> bool
    {
        return this->n_index == IndexOf<T>;
    }

    /// Returns `Some(reference)` if the active type is `T`, or `Nothing` if
    /// it is not. Never throws.
    template<typename T>
        requires(pack_contains_v<T, Ts...>)
    constexpr auto Get() noexcept -> Optional<T&>
    {
        if (this->n_index != IndexOf<T>) {
            return Nothing;
        }

        return Optional<T&>(*std::addressof(detail::getElementInStorage<IndexOf<T>>(this->n_storage)));
    }

    /// Const overload of [`Get`]. Returns `Optional<const T&>`.
    template<typename T>
        requires(pack_contains_v<T, Ts...>)
    constexpr auto Get() const noexcept -> Optional<const T&>
    {
        if (this->n_index != IndexOf<T>) {
            return Nothing;
        }

        return Optional<const T&>(*std::addressof(detail::getElementInStorage<IndexOf<T>>(this->n_storage)));
    }

    /// Dispatches a visitor implementation to the active element via a constexpr jump table.
    ///
    /// `Visitor` must be callable with every type in `Ts&...` and all
    /// branches must share a common return type. Missing cases are a hard
    /// error at the `std::common_type_t` deduction.
    template<typename Visitor>
    auto Visit(Visitor&& visitor) & -> decltype(auto)
    {
        using return_type_t = std::common_type_t<std::invoke_result_t<Visitor, Ts&>...>;

        constexpr static auto DISPATCH_TABLE
            = detail::createVisitorTable<return_type_t, Visitor, storage_t>(std::index_sequence_for<Ts...>{});

        return DISPATCH_TABLE[this->n_index](VIOLET_FWD(Visitor, visitor), this->n_storage);
    }

    /// Const lvalue overload of [`Visit`]. Passes `const T&` to the visitor.
    template<typename Visitor>
    auto Visit(Visitor&& visitor) const& -> decltype(auto)
    {
        using return_type_t = std::common_type_t<std::invoke_result_t<Visitor, const Ts&>...>;

        constexpr static auto DISPATCH_TABLE
            = detail::createVisitorTable<return_type_t, Visitor, const storage_t>(std::index_sequence_for<Ts...>{});

        return DISPATCH_TABLE[this->n_index](VIOLET_FWD(Visitor, visitor), this->n_storage);
    }

    /// Rvalue overload of [`Visit`]. Passes `T&&` to the visitor, enabling
    /// move-only types to be consumed.
    template<typename Visitor>
    auto Visit(Visitor&& visitor) && -> decltype(auto)
    {
        using return_type_t = std::common_type_t<std::invoke_result_t<Visitor, Ts&&>...>;

        constexpr static auto DISPATCH_TABLE
            = detail::createVisitorTable<return_type_t, Visitor, storage_t>(std::index_sequence_for<Ts...>{});

        return DISPATCH_TABLE[this->n_index](VIOLET_FWD(Visitor, visitor), this->n_storage);
    }

    /// An exhausitive pattern match utility. Builds a loca overload set from `funs` and
    /// forwards it to a visitor implementation.
    ///
    /// Every type of `Ts...` must have a corresponding callable in `funs`, a missing
    /// case is a hard compile-time error.
    ///
    /// ## Example
    /// ```cpp
    /// #include <violet/Experimental/OneOf.h>
    ///
    /// using violet::experimental::OneOf;
    ///
    /// OneOf<int, float, double> x = 42;
    /// auto value = x.Match(
    ///      [](int value) -> int { return value; },
    ///      [](float) -> int     { return 0; },
    ///      [](double) -> int    { return 0; }
    /// );
    ///
    /// assert(value == 42);
    /// ```
    template<typename... Fs>
    auto Match(Fs&&... funs) & -> decltype(auto)
    {
        struct overload_t: std::decay_t<Fs>... {
            using std::decay_t<Fs>::operator()...;
        };

        return this->Visit(overload_t{ VIOLET_FWD(Fs, funs)... });
    }

    /// Const lvalue overload of [`Match`].
    template<typename... Fs>
    auto Match(Fs&&... funs) const& -> decltype(auto)
    {
        struct overload_t: std::decay_t<Fs>... {
            using std::decay_t<Fs>::operator()...;
        };

        return this->Visit(overload_t{ VIOLET_FWD(Fs, funs)... });
    }

    /// Rvalue overload of [`Match`]. Forwards `*this` as an rvalue into
    /// [`Visit`], enabling move-only types to be consumed in-place.
    template<typename... Fs>
    auto Match(Fs&&... funs) && -> decltype(auto)
    {
        struct overload_t: std::decay_t<Fs>... {
            using std::decay_t<Fs>::operator()...;
        };

        return VIOLET_MOVE(*this).Visit(overload_t{ VIOLET_FWD(Fs, funs)... });
    }

    /// Swaps the contents of `o1` and `o2`.
    ///
    /// If both hold the same type, delegates to that type's `swap`.
    /// Otherwise, moves each side through a temporary so neither object is
    /// ever left in a partially-constructed state.
    friend void swap(OneOf& o1, OneOf& o2) noexcept
    {
        if (o1.n_index == o2.n_index) {
            o1.Visit([&](auto& av) -> void {
                using T = std::decay_t<decltype(av)>;

                // Move through a temporary — requires only move-constructibility,
                // not move-assignability, so types like tracked_object work fine.
                T tmp(VIOLET_MOVE(av));
                ::new (std::addressof(av)) T(VIOLET_MOVE(*o2.template Get<T>()));
                ::new (std::addressof(*o2.template Get<T>())) T(VIOLET_MOVE(tmp));
            });
        } else {
            OneOf tmp(VIOLET_MOVE(o1));
            ::new (&o1) OneOf(VIOLET_MOVE(o2));
            ::new (&o2) OneOf(VIOLET_MOVE(tmp));
        }
    }

    auto operator==(const OneOf& other) const -> bool
        requires(std::equality_comparable<Ts> && ...)
    {
        if (this->Index() != other.Index()) {
            return false;
        }

        return this->Visit([&](const auto& value) -> bool {
            using T = std::decay_t<decltype(value)>;
            return value == *other.template Get<T>();
        });
    }

    [[nodiscard]] auto operator!=(const OneOf& other) const -> bool
        requires(std::equality_comparable<Ts> && ...)
    {
        return !(*this == other);
    }

private:
    VIOLET_IMPLICIT OneOf() noexcept = default;

    using storage_t = detail::valueless_storage<Ts...>;

    template<UInt Index, typename U>
    static auto make(U&& value) noexcept(std::is_nothrow_move_constructible_v<U>) -> OneOf
    {
        OneOf result;
        ::new (std::addressof(detail::getElementInStorage<Index>(result.n_storage)))
            TypeAt<Index>(VIOLET_FWD(U, value));

        result.n_index = Index;
        return result;
    }

    storage_t n_storage;
    UInt n_index = 0;
};

template<typename T>
OneOf(T) -> OneOf<T>;

} // namespace violet::experimental

namespace violet::experimental::detail {

template<typename T, typename... Ts>
union valueless_storage<T, Ts...> final { // NOLINT(cppcoreguidelines-special-member-functions)
    T Head;
    valueless_storage<Ts...> Tail;

    valueless_storage() {};
    ~valueless_storage() {};
};

template<>
union valueless_storage<> final {
};

template<UInt I, typename... Ts>
constexpr auto getElementInStorage(valueless_storage<Ts...>& storage) noexcept -> auto&
{
    if constexpr (I == 0)
        return storage.Head;
    else
        return getElementInStorage<I - 1>(storage.Tail);
}

template<UInt I, typename... Ts>
constexpr auto getElementInStorage(const valueless_storage<Ts...>& storage) noexcept -> const auto&
{
    if constexpr (I == 0)
        return storage.Head;
    else
        return getElementInStorage<I - 1>(storage.Tail);
}

template<UInt Index, typename... Ts>
void destroyActiveElementInStorage(UInt active, valueless_storage<Ts...>& storage)
{
    if constexpr (Index < sizeof...(Ts)) {
        if (active == Index) {
            getElementInStorage<Index>(storage).~pack_element_t<Index, Ts...>();
            return;
        }

        destroyActiveElementInStorage<Index + 1, Ts...>(active, storage);
    }
}

/// Single overload — const-ness of the storage parameter is encoded in the
/// `Storage` type argument, eliminating the ambiguity that arises when two
/// overloads differ only in return type.
template<typename Ret, typename Visitor, typename Storage, UInt... Is>
constexpr auto createVisitorTable(std::index_sequence<Is...>) -> Array<Ret (*)(Visitor&&, Storage&), sizeof...(Is)>
{
    using FnPtr = Ret (*)(Visitor&&, Storage&);
    return Array<FnPtr, sizeof...(Is)>{ +[](Visitor&& visitor, Storage& storage) -> Ret {
        auto vis = VIOLET_MOVE(visitor);
        return vis(getElementInStorage<Is>(storage));
    }... };
}
} // namespace violet::experimental::detail
