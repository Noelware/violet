// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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
//! # 🌺💜 `violet/container/Optional.h`
//! This header file contains the `Optional` type, a wrapper for C++'s `optional`
//! interface but resembles more with Rust's [`std::option::Option`] instead.
//!
//! [`std::option::Option`]: https://doc.rust-lang.org/stable/std/option/enum.Option.html

#pragma once

#include "violet/violet.h"

#include <cstddef>
#include <optional>
#include <ostream>
#include <type_traits>
#include <utility>

namespace Noelware::Violet {

constexpr auto Nothing = std::nullopt; ///< Newtype for [`std::nullopt`].

/// A **optional** value.
template<typename T>
struct Optional final {
    /// Alias to get the type `T` of this [`Optional`].
    using Type = T;

    /// Creates a empty, disengaged optional value.
    constexpr Optional() noexcept = default;

    /// Creates a empty, disengaged optional value from a [`Nothing`]/[`std::nullopt`] value.
    constexpr VIOLET_IMPLICIT Optional(std::nullopt_t) noexcept {}

    /// @hidden
    /// @internal
    template<typename... Args>
    constexpr VIOLET_IMPLICIT Optional(std::in_place_t, Args&&... args)
    {
        this->construct(VIOLET_FWD(Args, args)...);
    }

    /// Constructs a [`Optional`] from a copy/move value.
    /// @param other The copied/moved value.
    template<typename U = T>
    constexpr VIOLET_EXPLICIT Optional(U&& other)
        requires(
            // Requires `U` is not the same as `Optional<U>` (fwds to `Optional<U>&&` constructor)
            !std::is_same_v<std::remove_cvref_t<U>, Optional> &&

            // Requires `U` is not the same as `std::optional<T>` (fwds to `std::optional<T>&&` constructor)
            !std::is_same_v<std::remove_cvref_t<U>, std::optional<T>> &&

            // Another safety check to ensure that `T` is constructible from `U`.
            std::constructible_from<T, U>)
    {
        this->construct(VIOLET_FWD(U, other));
    }

    /// Copy constructor.
    /// @param other The other optional to copy from.
    constexpr VIOLET_IMPLICIT Optional(const Optional& other)
    {
        if (other.n_hasValue) {
            this->construct(*other.ptr());
        }
    }

    /// Move constructor.
    ///
    /// ## Remarks
    /// This constructor is enabled if `T` is move constructible.
    ///
    /// @param other The other optional to move from.
    constexpr VIOLET_IMPLICIT Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::is_move_constructible_v<T>
    {
        if (other.n_hasValue) {
            this->construct(VIOLET_MOVE(*other.ptr()));
            other.Reset();
        }
    }

    /// Copy constructor from a [`std::optional`].
    ///
    /// ## Remarks
    /// This constructor is enabled if `T` is copy constructible.
    ///
    /// @param other The other [`std::optional`] to copy.
    constexpr VIOLET_IMPLICIT Optional(const std::optional<T>& other)
        requires std::is_copy_constructible_v<T>
    {
        if (other.has_value()) {
            this->construct(*other);
        }
    }

    /// Move constructor from a [`std::optional`].
    ///
    /// ## Remarks
    /// This constructor is enabled if `T` is move constructible.
    ///
    /// @param other The other [`std::optional`] to move.
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr VIOLET_IMPLICIT Optional(std::optional<T>&& other)
        requires std::is_move_constructible_v<T>
    {
        if (other.has_value()) {
            this->construct(VIOLET_MOVE(*other));
        }
    }

    /// The destructor for this [`Optional`].
    ~Optional() noexcept
    {
        destroy();
    }

    /// Assigns empty state to this [`Optional`].
    constexpr auto operator=(std::nullopt_t) noexcept -> Optional&
    {
        Reset();
        return *this;
    }

    /// Copy-assigns from an another [`Optional`].
    ///
    /// ## Remarks
    /// This assignment is avaliable if `T` is copy assignable or constructible.
    ///
    /// @param other The other [`Optional`] to copy.
    constexpr auto operator=(const Optional& other) -> Optional&
        requires std::is_copy_assignable_v<T> || std::is_copy_constructible_v<T>
    {
        if (this == &other) {
            return *this;
        }

        if (this->n_hasValue && other.n_hasValue) {
            *ptr() = *other.ptr();
        } else if (this->n_hasValue && !other.n_hasValue) {
            Reset();
        } else if (!this->n_hasValue && other.n_hasValue) {
            this->construct(*other.ptr());
        }

        return *this;
    }

    /// Move-assigns from an other [`Optional`].
    ///
    /// ## Remarks
    /// - This assignment is avaliable if `T` is move assignable or constructible.
    /// - This assignment doesn't throw any exceptions if the move assignable or
    ///   constructible doesn't throw any exceptions.
    ///
    /// @param other The other [`Optional`] to move.
    constexpr auto operator=(Optional&& other) noexcept(
        std::is_nothrow_move_assignable_v<T> || std::is_nothrow_move_constructible_v<T>) -> Optional&
        requires std::is_move_assignable_v<T> || std::is_move_constructible_v<T>
    {
        if (this == &other) {
            return *this;
        }

        if (this->n_hasValue && other.n_hasValue) {
            *ptr() = VIOLET_MOVE(*other.ptr());
            other.Reset();
        } else if (this->n_hasValue && !other.n_hasValue) {
            Reset();
        } else if (!this->n_hasValue && other.n_hasValue) {
            this->construct(VIOLET_MOVE(*other.ptr()));
            other.Reset();
        }

        return *this;
    }

    /// Assigns a value into this [`Optional`], constructing it if the [`Optional`]
    /// is empty.
    template<typename U = T>
    constexpr auto operator=(U&& other) noexcept -> Optional&
        // clang-format off
        requires(
            // Ensures that `U` is not `Optional` (fwds to `operator==(Optional<U>&&)` decl)
            !std::is_same_v<std::remove_cvref_t<U>, Optional> &&

            // Ensures that `U` is not `std::optional<T>` (fwds to `operator==(std::optional<T>&&)` decl)
            !std::is_same_v<std::remove_cvref_t<U>, std::optional<T>> &&

            // Safety check to ensure that `T&` is constructible from `U`.
            std::assignable_from<T&, U>)
    // clang-format on
    {
        if (this->n_hasValue) {
            *ptr() = VIOLET_FWD(U, other);
        } else {
            this->construct(VIOLET_FWD(U, other));
        }

        return *this;
    }

    /// Copy-assigns from an another [`std::optional`].
    ///
    /// ## Remarks
    /// This assignment is avaliable if `T` is copy assignable or constructible.
    ///
    /// @param other The other [`std::optional`] to copy.
    constexpr auto operator=(const std::optional<T>& other) -> Optional&
        requires std::is_copy_constructible_v<T> || std::is_copy_assignable_v<T>
    {
        if (other.has_value() && this->n_hasValue) {
            *ptr() = *other;
        } else if (other.has_value() && !this->n_hasValue) {
            this->construct(*other);
        } else {
            Reset();
        }

        return *this;
    }

    /// Move-assigns from an other [`std::optional`].
    ///
    /// ## Remarks
    /// - This assignment is avaliable if `T` is move assignable or constructible.
    /// - This assignment doesn't throw any exceptions if the move assignable or
    ///   constructible doesn't throw any exceptions.
    ///
    /// @param other The other [`std::optional`] to move.
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    constexpr auto operator=(std::optional<T>&& other) -> Optional&
        requires std::is_move_constructible_v<T> || std::is_move_assignable_v<T>
    {
        if (other.has_value() && this->n_hasValue) {
            *ptr() = VIOLET_MOVE(*other);
        } else if (other.has_value() && !this->n_hasValue) {
            this->construct(VIOLET_MOVE(*other));
        } else {
            Reset();
        }

        return *this;
    }

    /// Allows to access the contained value by pointer.
    ///
    /// ## Remarks
    /// This [`Optional`] will terminate if this [`Optional`] is empty. It is recommended to use
    /// the [`Optional::ValueOr`] function to provide a default if you don't want to terminate.
    ///
    /// @returns A pointer to the contained value while preserving const-ness.
    constexpr auto operator->() const -> const T*
    {
        assert(this->n_hasValue);
        return ptr();
    }

    /// Allows to access the contained value by pointer.
    ///
    /// ## Remarks
    /// This [`Optional`] will terminate if this [`Optional`] is empty. It is recommended to use
    /// the [`Optional::ValueOr`] function to provide a default if you don't want to terminate.
    ///
    /// @returns A pointer to the contained value.
    constexpr auto operator->() -> T*
    {
        assert(this->n_hasValue);
        return ptr();
    }

    /// Allows deferencing this [`Optional`] to retrieve the contained value by const-ness lvalue.
    ///
    /// ## Remarks
    /// This [`Optional`] will terminate if this [`Optional`] is empty. It is recommended to use
    /// the [`Optional::ValueOr`] function to provide a default if you don't want to terminate.
    ///
    /// @returns A rvalue reference to the contained value while preserving const-ness for this lvalue return.
    constexpr auto operator*() const& -> const T&
    {
        assert(this->n_hasValue);
        return *ptr();
    }

    /// Allows deferencing this [`Optional`] to retrieve the contained value by lvalue.
    ///
    /// ## Remarks
    /// This [`Optional`] will terminate if this [`Optional`] is empty. It is recommended to use
    /// the [`Optional::ValueOr`] function to provide a default if you don't want to terminate.
    ///
    /// @returns A lvalue reference to the contained value.
    constexpr auto operator*() & -> T&
    {
        assert(this->n_hasValue);
        return *ptr();
    }

    /// Allows deferencing this [`Optional`] to retrieve the contained value by const-ness rvalue.
    ///
    /// ## Remarks
    /// This [`Optional`] will terminate if this [`Optional`] is empty. It is recommended to use
    /// the [`Optional::ValueOr`] function to provide a default if you don't want to terminate.
    ///
    /// @returns A rvalue reference to the contained value while preserving const-ness.
    constexpr auto operator*() const&& -> const T&
    {
        assert(this->n_hasValue);
        return *ptr();
    }

    /// Allows deferencing this [`Optional`] to retrieve the contained value by rvalue.
    ///
    /// ## Remarks
    /// This [`Optional`] will terminate if this [`Optional`] is empty. It is recommended to use
    /// the [`Optional::ValueOr`] function to provide a default if you don't want to terminate.
    ///
    /// @returns A rvalue reference to the contained value.
    constexpr auto operator*() && -> T&
    {
        assert(this->n_hasValue);
        return *ptr();
    }

    /// Returns the contained value by lvalue reference.
    ///
    /// @throws [std::bad_optional_access] Only thrown if this [`Optional`] is empty.
    /// @returns A lvalue reference to the contained value.
    constexpr auto Value() & -> T&
    {
        if (!this->n_hasValue) {
            throw std::bad_optional_access{};
        }

        return *ptr();
    }

    /// Returns the contained value by lvalue reference.
    ///
    /// @throws [std::bad_optional_access] Only thrown if this [`Optional`] is empty.
    /// @returns A lvalue reference to the contained value while preserving const-ness for the lvalue return.
    constexpr auto Value() const& -> const T&
    {
        if (!this->n_hasValue) {
            throw std::bad_optional_access{};
        }

        return *ptr();
    }

    /// Returns the contained value by rvalue reference.
    ///
    /// @throws [std::bad_optional_access] Only thrown if this [`Optional`] is empty.
    /// @returns A rvalue reference to the contained value.
    constexpr auto Value() && -> T&&
    {
        if (!this->n_hasValue) {
            throw std::bad_optional_access{};
        }

        return *ptr();
    }

    /// Returns the contained value by rvalue reference.
    ///
    /// @throws [std::bad_optional_access] Only thrown if this [`Optional`] is empty.
    /// @returns A reference to the contained value while preserving const-ness for the rvalue return.
    constexpr auto Value() const&& -> const T&&
    {
        if (!this->n_hasValue) {
            throw std::bad_optional_access{};
        }

        return *ptr();
    }

    /// [**lvalue**] Returns the contained value if present, otherwise `default_value`
    /// is returned instead.
    ///
    /// @param default_value The value to return if this [`Optional`] is empty.
    /// @return if engaged, returns the contained value in this [`Optional`] or `default_value` otherwise.
    constexpr auto ValueOr(const T& default_value) const& -> T
    {
        return n_hasValue ? *ptr() : default_value;
    }

    /// [**rvalue**] Returns the contained value if present, otherwise `default_value`
    /// is returned instead.
    ///
    /// @param default_value The value to return if this [`Optional`] is empty.
    /// @return if engaged, returns the contained value in this [`Optional`] or `default_value` otherwise.
    constexpr auto ValueOr(T&& default_value) && -> T
    {
        return n_hasValue ? VIOLET_MOVE(*ptr()) : VIOLET_MOVE(default_value);
    }

    VIOLET_IMPL_EQUALITY_SINGLE(Optional, lhs, rhs, {
        if (lhs.n_hasValue != rhs.n_hasValue) {
            return false;
        }

        if (!lhs.n_hasValue) {
            return true;
        }

        return *lhs.ptr() == *rhs.ptr();
    });

    VIOLET_IMPL_EQUALITY(const Optional&, std::nullopt_t, lhs, , { return !lhs.HasValue(); });
    VIOLET_IMPL_EQUALITY(const Optional&, const std::optional<T>&, lhs, rhs, {
        if (lhs.n_hasValue != rhs.has_value()) {
            return false;
        }

        if (!lhs.n_hasValue) {
            return true;
        }

        return *lhs.ptr() == *rhs;
    });

    /// Returns **true** if this [`Optional`] contains a value.
    constexpr VIOLET_IMPLICIT operator bool() const
    {
        return this->n_hasValue;
    }

    /// Allows translating this [`Optional`] into a [`std::optional`].
    constexpr VIOLET_IMPLICIT operator std::optional<T>()
    {
        return this->n_hasValue ? std::optional<T>(*ptr()) : Nothing;
    }

    /// Returns **true** if this [`Optional`] contains a value.
    constexpr auto HasValue() const noexcept -> bool
    {
        return this->n_hasValue;
    }

    /// Applies the `fun`ction if this [`Optional`] contains a value.
    /// @param func The predicate to call.
    template<typename Fun>
    constexpr auto HasValueAnd(Fun&& fun) const noexcept -> bool
    {
        return HasValue() && std::invoke(VIOLET_FWD(Fun, fun), *ptr());
    }

    /// [**lvalue**] Calls the `fun`ction if this [`Optional`] contains a value
    /// and maps it to a new value.
    template<typename Fun>
    constexpr auto Map(Fun&& fun) const& noexcept -> Optional<T>
    {
        using U = std::invoke_result_t<Fun, const T&>;
        return HasValue() ? Optional<U>(std::in_place, VIOLET_FWD(Fun, fun)(*ptr())) : Nothing;
    }

    /// [**rvalue**] Calls the `fun`ction if this [`Optional`] contains a value
    /// and maps it to a new value.
    template<typename Fun>
    constexpr auto Map(Fun&& fun) const&& noexcept -> Optional<T>
    {
        using U = std::invoke_result_t<Fun, const T&>;
        return HasValue() ? Optional<U>(std::in_place, VIOLET_FWD(Fun, fun)(VIOLET_MOVE(*ptr()))) : Nothing;
    }

    /// [**lvalue**] Calls the `fun`ction if this [`Optional`] contains a value
    /// and inspects the value of it.
    template<typename Fun>
    constexpr auto Inspect(Fun&& fun) const& noexcept -> Optional<T>
    {
        if (HasValue()) {
            VIOLET_FWD(Fun, fun)(*ptr());
        }

        return *this;
    }

    /// [**rvalue**] Calls the `fun`ction if this [`Optional`] contains a value
    /// and inspects the value of it.
    template<typename Fun>
    constexpr auto Inspect(Fun&& fun) const&& noexcept -> Optional<T>
    {
        if (HasValue()) {
            VIOLET_FWD(Fun, fun)(VIOLET_MOVE(*ptr()));
        }

        return *this;
    }

    /// Moves the contained value out and leaves this [`Optional`] empty.
    /// @return A [`Optional`] containing the previous value, or a empty one if none.
    constexpr auto Take() noexcept -> Optional<T>
    {
        if (!HasValue()) {
            return {};
        }

        Optional tmp(std::in_place, VIOLET_MOVE(*ptr()));
        Reset();

        return tmp;
    }

    /// Replaces the current value of this [`Optional`] into a new one, constructing in-place.
    /// @param args Arguments forwarded to construct the new value
    /// @return Returns the old value of this [`Optional`] or a empty one if none.
    template<typename... Args>
    constexpr auto Replace(Args&&... args) -> T&
    {
        if (HasValue()) {
            Reset();
        }

        this->construct(VIOLET_FWD(Args, args)...);
        return *ptr();
    }

    /// Destroys the contained value (if present) and leaves this [`Optional`] empty.
    constexpr auto Reset()
    {
        destroy();
    }

    VIOLET_OSTREAM_IMPL(const Optional&)
    {
        if (!self.HasValue()) {
            return os << "«no value»";
        }

        const auto& value = self.Value();

        // clang-format off
        if constexpr (requires {
            { os << value } -> std::same_as<std::ostream&>;
        }) {
            // clang-format on
            return os << value;
        }

        const auto& type = typeid(T);
        return os << "«type '" << type.name() << '@' << type.hash_code() << "' not streamable»";
    }

private:
    alignas(T) Array<std::byte, sizeof(T)> n_storage; ///< storage for this [`Optional`].
    mutable bool n_hasValue = false; ///< marker to indicate if this `Optional` has a value.

    constexpr auto ptr() noexcept -> T*
    {
        // SAFETY: Since all valid bits inside of the pointer (as a `std::any<uint8, {sizeof})`),
        //         the only way we can do this without any errors is with `reinterpret_cast`.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return std::launder(reinterpret_cast<T*>(this->n_storage.data()));
    }

    constexpr auto ptr() const noexcept -> const T*
    {
        // SAFETY: Since all valid bits inside of the pointer (as a `std::any<uint8, {sizeof})`),
        //         the only way we can do this without any errors is with `reinterpret_cast`.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return std::launder(reinterpret_cast<const T*>(this->n_storage.data()));
    }

    template<typename... Args>
    auto construct(Args&&... args)
    {
        ::new (this->n_storage.data()) T(VIOLET_FWD(Args, args)...);
        this->n_hasValue = true;
    }

    auto destroy()
    {
        if (this->n_hasValue) {
            ptr()->~T();
            this->n_hasValue = false;
        }
    }
};

/// Constructs a new [`Optional`] with in-place arguments, similar to Rust's
/// [`std::option::Option::Some`].
///
/// [`std::option::Option::Some`]: https://doc.rust-lang.org/stable/std/option/enum.Option.html#variant-Some
template<typename T, typename... Args>
constexpr static auto Some(Args&&... args) -> Optional<T>
{
    return { std::in_place, VIOLET_FWD(Args, args)... };
}

} // namespace Noelware::Violet
