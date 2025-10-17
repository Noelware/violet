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
//! # 🌺💜 `violet/container/Result.h`
//! This header file contains the `Optional` type, a wrapper for C++'s `expected`
//! interface but resembles more with Rust's [`std::result::Result`] instead.
//!
//! [`std::result::Result`]: https://doc.rust-lang.org/stable/std/result/enum.Result.html

#pragma once

#include "violet/support/Demangle.h"
#include "violet/violet.h"

#include <expected>
#include <type_traits>

namespace Noelware::Violet {

template<typename E>
using Err = std::unexpected<E>; ///< Newtype for [`std::unexpected`]

/// Representation of a successful or failed state, analgous of Rust's [`std::result::Result`].
///
/// [`std::result::Result`]: https://doc.rust-lang.org/stable/std/result/enum.Result.html
template<typename T, typename E>
struct [[nodiscard("always check its error state")]] Result final {
    /// Type-alias for the success state's type.
    using ValueType = T;

    /// Type-alias for the failed state's type.
    using ErrorType = E;

    Result() = delete; /// do not allow construction of a default `Result`.

    constexpr VIOLET_IMPLICIT Result(const T& ok)
        : n_isOk(true)
    {
        ::new (&this->n_storage.ok) T(ok);
    }

    constexpr VIOLET_IMPLICIT Result(T&& ok) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_isOk(true)
    {
        ::new (&this->n_storage.ok) T(VIOLET_MOVE(ok));
    }

    constexpr VIOLET_IMPLICIT Result(const Err<E>& err)
    {
        ::new (&this->n_storage.error) Err<E>(err);
    }

    constexpr VIOLET_IMPLICIT Result(Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        ::new (&this->n_storage.error) Err<E>(VIOLET_MOVE(err));
    }

    template<typename U>
    constexpr VIOLET_IMPLICIT Result(const U& err)
        requires(std::is_convertible_v<U, E>)
    {
        ::new (&this->n_storage.error) Err<E>(err);
    }

    template<typename U>
    constexpr VIOLET_IMPLICIT Result(U&& err) noexcept(std::is_nothrow_move_constructible_v<E>)
        requires(std::is_convertible_v<U, E>)
    {
        ::new (&this->n_storage.error) Err<E>(VIOLET_FWD(U, err));
    }

    ~Result()
    {
        destroy();
    }

    constexpr Result(const Result<T, E>& other)
        : n_isOk(other.n_isOk)
    {
        if (other.n_isOk) {
            ::new (&this->n_storage.ok) T(other.n_storage.ok);
        } else {
            ::new (&this->n_storage.error) Err<E>(other.n_storage.error);
        }
    }

    constexpr Result(Result&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>)
        : n_isOk(other.n_isOk)
    {
        if (other.n_isOk) {
            ::new (&this->n_storage.ok) T(VIOLET_MOVE(other.n_storage.ok));
        } else {
            ::new (&this->n_storage.error) Err<E>(VIOLET_MOVE(other.n_storage.error));
        }
    }

    constexpr auto operator=(const Result& other) -> Result&
    {
        if (this == &other) {
            return *this;
        }

        destroy();

        this->n_isOk = other.n_isOk;
        if (this->n_isOk) {
            ::new (&this->n_storage.ok) T(other.n_storage.ok);
        } else {
            ::new (&this->n_storage.error) Err<E>(other.n_storage.error);
        }

        return *this;
    }

    constexpr auto operator=(Result&& other) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_assignable_v<E>) -> Result&
    {
        if (this == &other) {
            return *this;
        }

        destroy();

        this->n_isOk = other.n_isOk;
        if (this->n_isOk) {
            ::new (&this->n_storage.ok) T(VIOLET_MOVE(other.n_storage.ok));
        } else {
            ::new (&this->n_storage.error) Err<E>(VIOLET_MOVE(other.n_storage.error));
        }

        return *this;
    }

    constexpr VIOLET_IMPLICIT operator bool() const noexcept
    {
        return this->n_isOk;
    }

    constexpr VIOLET_IMPLICIT operator std::expected<T, E>()
    {
        return this->n_isOk ? std::expected<T, E>(*Value()) : Err(*Error());
    }

    constexpr VIOLET_IMPLICIT operator std::unexpected<E>()
    {
        assert(!this->n_isOk);
        return Err(*Error());
    }

    [[nodiscard]] constexpr auto IsOk() const noexcept -> bool
    {
        return this->n_isOk;
    }

    [[nodiscard]] constexpr auto IsErr() const noexcept -> bool
    {
        return !this->n_isOk;
    }

    [[nodiscard]] constexpr auto Value() const -> const T&
    {
        assert(IsOk());
        return this->n_storage.ok;
    }

    [[nodiscard]] constexpr auto Value() -> T&
    {
        assert(IsOk());
        return this->n_storage.ok;
    }

    [[nodiscard]] constexpr auto Error() const noexcept -> const E&
    {
        assert(IsErr());
        return this->n_storage.error.error();
    }

    [[nodiscard]] constexpr auto Error() noexcept -> E&
    {
        assert(IsErr());
        return this->n_storage.error.error();
    }

    template<typename Fun>
    constexpr auto Map(Fun&& fun) & -> Result<std::invoke_result_t<Fun, T&>, E>
    {
        using U = std::invoke_result_t<Fun, T&>;
        return IsOk() ? Result<U, E>(std::invoke(VIOLET_FWD(Fun, fun), Value())) : Result<U, E>(Error());
    }

    template<typename Fun>
    constexpr auto Map(Fun&& fun) && -> Result<std::invoke_result_t<Fun, T&&>, E>
    {
        using U = std::invoke_result_t<Fun, T&&>;
        return IsOk() ? Result<U, E>(std::invoke(VIOLET_FWD(Fun, fun), Value())) : Result<U, E>(Error());
    }

    template<typename U>
        requires std::is_default_constructible_v<T>
    constexpr auto MapOr(U&& def) const&
    {
        return IsOk() ? Value() : VIOLET_FWD(U, def);
    }

    template<typename U>
        requires std::is_default_constructible_v<T>
    constexpr auto MapOr(U&& def) &&
    {
        return IsOk() ? Value() : VIOLET_FWD(U, def);
    }

    template<typename Fun>
    constexpr auto MapError(Fun&& fun) const& -> Result<std::invoke_result_t<Fun, const E&>, E>
    {
        using U = std::invoke_result_t<Fun, const E&>;
        return IsErr() ? Result<T, U>(std::invoke(VIOLET_FWD(Fun, fun), Error())) : Result<T, U>(Error());
    }

    template<typename Fun>
    constexpr auto MapError(Fun&& fun) && -> Result<std::invoke_result_t<Fun, T&&>, E>
    {
        using U = std::invoke_result_t<Fun, T&&>;
        return IsErr() ? Result<T, U>(std::invoke(VIOLET_FWD(Fun, fun), Error())) : Result<T, U>(Value());
    }

    VIOLET_OSTREAM_IMPL(const Result&)
    {
        if (self.IsOk()) {
            const auto& value = self.Value();

            // clang-format off
            if constexpr (requires {
                { os << value } -> std::same_as<std::ostream&>;
            }) {
                // clang-format on
                return os << value;
            }

            const auto& type = typeid(T);
            return os << "«ok variant type '" << Utility::DemangleCXXName(type.name()) << '@' << type.hash_code()
                      << "' not streamable»";
        }

        const auto& value = self.Error();

        // clang-format off
        if constexpr (requires {
            { os << value } -> std::same_as<std::ostream&>;
        }) {
            // clang-format on
            return os << value;
        }

        const auto& type = typeid(E);
        return os << "«error variant type '" << Utility::DemangleCXXName(type.name()) << '@' << type.hash_code()
                  << "' not streamable»";
    }

private:
    mutable bool n_isOk = false; ///< marker to indicate if this result is in its success state

    union Storage { // NOLINT(cppcoreguidelines-special-member-functions)
        T ok;
        Err<E> error;

        Storage() {}
        ~Storage() {}
    } n_storage;

    void destroy()
    {
        if (this->n_isOk) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                this->n_storage.ok.~T();
            }
        } else {
            if constexpr (!std::is_trivially_destructible_v<E>) {
                this->n_storage.error.~Err<E>();
            }
        }
    }
};

/// A specialization for `void`-returned types of a [`Result`].
template<typename E>
struct [[nodiscard("always check the error state before discarding")]] Result<void, E> final {
    /// Type-alias for the success state's type (`void`).
    using ValueType = void;

    /// Type-alias for the failed state's type.
    using ErrorType = E;

    /// Constructs a successful (`Ok`) result.
    Result() noexcept = default;

    /// Constructs an error result from an `Err<E>`.
    /// @param err The error value to store.
    constexpr VIOLET_IMPLICIT Result(const Err<E>& err)
        : n_error(new Err<E>(err))
    {
    }

    /// Constructs an error result from an rvalue `Err<E>`.
    /// @param err The error value to move into storage.
    constexpr VIOLET_IMPLICIT Result(Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<E>)
        : n_error(new Err<E>(VIOLET_MOVE(err)))
    {
    }

    /// Constructs an error result from a convertible type.
    /// @param err The error value to store (converted to `E`).
    template<typename U>
    constexpr VIOLET_IMPLICIT Result(const U& err)
        requires std::is_convertible_v<U, E>
        : n_error(new Err<E>(err))
    {
    }

    /// Constructs an error result from a movable convertible type.
    /// @param err The error value to move into storage (converted to `E`).
    template<typename U>
    constexpr VIOLET_IMPLICIT Result(U&& err)
        requires std::is_convertible_v<U, E>
        : n_error(new Err<E>(VIOLET_FWD(U, err)))
    {
    }

    ~Result()
    {
        // Releases the error, if error.
        delete this->n_error;
    }

    /// Copy constructor.
    /// Duplicates the error value if present.
    constexpr Result(const Result& other)
        : n_error(other.n_error ? new Err<E>(*other.n_error) : nullptr)
    {
    }

    /// Move constructor.
    /// Moves the error pointer if present; leaves `other` as `Ok`.
    constexpr Result(Result&& other) noexcept
        : n_error(other.n_error)
    {
        other.n_error = nullptr;
    }

    /// Copy assignment operator.
    /// Replaces current error state with copy of `other`.
    constexpr auto operator=(const Result& other) -> Result&
    {
        if (this == &other) {
            return *this;
        }

        delete this->n_error;
        this->n_error = other.n_error ? new Err<E>(*other.n_error) : nullptr;

        return *this;
    }

    /// Move assignment operator.
    /// Moves error state from `other`, leaving `other` as `Ok`.
    constexpr auto operator=(Result&& other) noexcept -> Result&
    {
        if (this == &other) {
            return *this;
        }

        delete this->n_error;
        this->n_error = other.n_error;
        other.n_error = nullptr;

        return *this;
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_error == nullptr;
    }

    constexpr VIOLET_IMPLICIT operator std::expected<void, E>() noexcept
    {
        return this->IsOk() ? std::expected<void, E>({}) : Err(*Error());
    }

    constexpr VIOLET_IMPLICIT operator std::unexpected<E>() noexcept
    {
        assert(this->IsErr());
        return Err(*Error());
    }

    /// Returns true if the result is a success (`Ok`).
    [[nodiscard]] constexpr auto IsOk() const noexcept -> bool
    {
        return n_error == nullptr;
    }

    /// Returns true if the result is a failure (`Err`).
    [[nodiscard]] constexpr auto IsErr() const noexcept -> bool
    {
        return n_error != nullptr;
    }

    /// Returns a reference to the error value.
    /// Asserts if the result is `Ok`.
    [[nodiscard]] constexpr auto Error() const -> const E&
    {
        assert(IsErr());
        return n_error->error();
    }

    /// Returns a reference to the error value.
    /// Asserts if the result is `Ok`.
    [[nodiscard]] constexpr auto Error() -> E&
    {
        assert(IsErr());
        return n_error->error();
    }

    VIOLET_OSTREAM_IMPL(const Result&)
    {
        if (self.IsOk()) {
            return os << "«result of 'void' type»";
        }

        const auto& value = self.Error();

        // clang-format off
        if constexpr (requires {
            { os << value } -> std::same_as<std::ostream&>;
        }) {
            // clang-format on
            return os << value;
        }

        const auto& type = typeid(E);
        return os << "«error variant type '" << Utility::DemangleCXXName(type.name()) << '@' << type.hash_code()
                  << "' not streamable»";
    }

private:
    Err<E>* n_error = nullptr; ///< pointer to an error, if one exists.
};

} // namespace Noelware::Violet
