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

#include <violet/Violet.h>

#if VIOLET_USE_RTTI
#include <violet/Support/Demangle.h>
#endif

#ifndef VIOLET_HAS_EXCEPTIONS
#include <cstdio>
#include <source_location>
#endif

#include <expected>
#include <type_traits>

namespace violet {

template<typename T, typename E>
struct Result;

/// Creates a new `Result<T, E>` that contains a successful value.
/// @tparam T The underlying success type to construct.
/// @tparam E The underlying error type to pass.
/// @tparam Args The arguments to pass to `T`'s constructor.
/// @param args The arguments to pass to `T`'s constructor.
/// @return An `Optional<T>` with the value constructed.
template<typename T, typename E, typename... Args>
constexpr static auto Ok(Args&&... args) -> Result<T, E>
{
    static_assert(std::is_object_v<T>, "`Result<T, E>` requires `T` to be a object type or `void`");
    static_assert(std::is_object_v<E>, "`Result<T, E>` requires `E` to be an object type");
    static_assert(!std::is_reference_v<T>, "`Result<T, E>` must not wrap a reference type");
    static_assert(!std::is_reference_v<E>, "`Result<T, E>` must not wrap a reference type");
    static_assert(!std::is_array_v<T>, "`Result<T, E>` must not wrap an array type");
    static_assert(!std::is_array_v<E>, "`Result<T, E>` must not wrap an array type");
    static_assert(
        !std::is_const_v<E> && !std::is_volatile_v<E>, "`Result<T, E>` must not have a cv-qualified error type");
    static_assert(std::is_destructible_v<T>, "`Result<T, E>` requires T to be destructible");
    static_assert(std::is_destructible_v<E>, "`Result<T, E>` requires E to be destructible");
    static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>,
        "`Result<T, E>` requires T to be movable or copyable");
    static_assert(std::is_move_constructible_v<E> || std::is_copy_constructible_v<E>,
        "`Result<T, E>` requires E to be movable or copyable");
    static_assert(sizeof(E) > 0, "`Result<T, E>` requires E to be a complete type");

    return Result<T, E>(std::in_place_index<0>, VIOLET_FWD(Args, args)...);
}

/// Concept for detecting nested [`violet::Result`] types.
///
/// **nested_result** determines whether a type `T` is a **nested `Result<Result<U, E>, E>`** for some `U`.
/// This is useful for implementing functions like [`Result::transpose`] where only nested [`Result`]s
/// should be normalized.
///
/// ## Example
/// ```cpp
/// #include <violet/Container/Result.h>
///
/// using inner = violet::Result<int, std::string>;
/// using outer = violet::Result<inner, std::string>;
///
/// static_assert(violet::nested_result<outer, std::string>);
/// static_assert(!violet::nested_result<int, std::string>);
/// ```
template<typename T, typename E>
concept nested_result = instanceof_v<Result, T> && std::same_as<typename T::error_type, E>;

/// Type trait that detects whether a type `T` is a `Result<..., ...>`.
///
/// ## Example
/// ```cpp
/// #include <violet/Container/Result.h>
/// #include <type_traits>
///
/// static_assert(violet::is_result_v<violet::Result<int, std::string>>);
/// static_assert(!violet::is_result_v<int>);
/// ```
template<typename T>
struct is_result: std::false_type {};

template<typename T, typename E>
struct is_result<Result<T, E>>: std::true_type {};

template<typename T>
static constexpr bool is_result_v = is_result<T>::value;

/// A tagged error variant.
///
/// `Err<E>` is a lightweight, non-zero abstraction used to explicitly construct `violet::Result<T, E>`
/// or `std::expected<T, E>` in their error states.
///
/// > [!NOTE]
/// > This is a backport-compatible equivalent of C++23's `std::unexpected` as Violet targets C++20 as the
/// > MSCPPV (Minimum Supported C++ Version)
///
/// ## Design
/// - `Err<E>` is **not** implicitly convertible to `E`
/// - It exists solely to disambiguate error construction
/// - Prevents accidental success-path conversions
///
/// ## Invariants
/// - `E` must not be `void`
/// - The contained error is always engaged
template<typename E>
struct VIOLET_API Err final {
    static_assert(!std::is_void_v<E>, "`Err<void>` is ill-formed");
    static_assert(std::is_object_v<E>, "`Err<E>` requires `E` to be a object type");
    static_assert(!std::is_reference_v<E>, "`Err<E>` must not wrap a reference type");
    static_assert(!std::is_array_v<E>, "`Err<E>` must not wrap an array type");
    static_assert(!std::is_const_v<E> && !std::is_volatile_v<E>, "`Err<E>` must not be cv-qualified");
    static_assert(!std::is_pointer_v<E>, "`Err<E>` should not wrap raw pointer types");
    static_assert(!std::is_function_v<E>, "`Err<E>` must not wrap function types");
    static_assert(std::is_move_constructible_v<E> || std::is_copy_constructible_v<E>,
        "`Err<E>` requires `E` to be movable or copyable");
    static_assert(sizeof(E) > 0, "`Err<E>` requires `E` to be a complete type");
    static_assert(!std::same_as<E, Err<E>>, "`Err<Err<E>>` is ill-formed");
    static_assert(!std::same_as<E, std::in_place_t>, "`Err<E>` must not wrap `std::in_place_t`");

#if VIOLET_REQUIRE_STL(202302L)
    static_assert(!std::same_as<E, std::unexpect_t>, "`Err<E>` must not wrap `std::unexpect_t`");
#endif

    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Err);

    /// Constructs from a const lvalue error.
    constexpr VIOLET_EXPLICIT Err(const E& error) noexcept(std::is_nothrow_copy_constructible_v<E>)
        : n_value(error)
    {
    }

    /// Constructs from an rvalue error.
    constexpr VIOLET_EXPLICIT Err(E&& error) noexcept(std::is_nothrow_move_constructible_v<E>)
        : n_value(VIOLET_MOVE(error))
    {
    }

    /// Constructs the contained error in-place.
    ///
    /// This constructor is disabled when it would collide with copy/move construction from `E`.
    template<typename... Args>
        requires(std::is_constructible_v<E, Args...>
            && !(sizeof...(Args) == 1 && (std::same_as<std::decay_t<Args>, E> || ...)))
    constexpr VIOLET_EXPLICIT Err(Args&&... args) noexcept(std::is_nothrow_constructible_v<E, Args...>)
        : n_value(VIOLET_FWD(Args, args)...)
    {
    }

    /// [**lvalue**] Access the contained error.
    constexpr auto Error() & noexcept -> E&
    {
        return this->n_value;
    }

    /// [**rvalue**] Access the contained error.
    constexpr auto Error() && noexcept -> E&&
    {
        return VIOLET_MOVE(this->n_value);
    }

    /// [const **lvalue**] Access the contained error.
    constexpr auto Error() const& noexcept -> const E&
    {
        return this->n_value;
    }

    /// [const **rvalue**] Access the contained error.
    constexpr auto Error() const&& noexcept -> const E&&
    {
        return VIOLET_MOVE(this->n_value);
    }

    constexpr auto operator==(const Err& other) const noexcept -> bool
        requires(requires { this->Error() == other.Error(); })
    {
        return this->Error() == other.Error();
    }

    constexpr auto operator!=(const Err& other) const noexcept -> bool
        requires(requires { this->Error() != other.Error(); })
    {
        return this->Error() != other.Error();
    }

    template<typename T>
    constexpr VIOLET_EXPLICIT operator violet::Result<T, E>() const noexcept
    {
        return Result<T, E>(std::in_place_index<1>, this->n_value);
    }

#if VIOLET_REQUIRE_STL(202302L)
    template<typename T>
    constexpr VIOLET_EXPLICIT operator std::expected<T, E>() const noexcept
    {
        return std::expected<T, E>(std::unexpect, this->n_value);
    }

    constexpr VIOLET_EXPLICIT operator std::unexpected<E>() const noexcept
    {
        return std::unexpected<E>(this->n_value);
    }
#endif

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return violet::ToString(Error());
    }

    constexpr friend auto operator<<(std::ostream& os, const Err<E>& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    E n_value;
};

/// Representation of a successful or failed state, analgous of Rust's [`std::result::Result`].
///
/// [`std::result::Result`]: https://doc.rust-lang.org/1.90.0/std/result/enum.Result.html
///
/// ## Example
/// ```cpp
/// #include <violet/Container/Result.h>
///
/// auto res = violet::Ok<int, std::string>(42);
/// violet::println("the answer to life is: {}", res);
/// ```
template<typename T, typename E>
struct [[nodiscard("always check the error state")]] VIOLET_API Result final {
    static_assert(
        std::is_object_v<T> || std::is_void_v<T>, "`Result<T, E>` requires `T` to be a object type or `void`");
    static_assert(std::is_object_v<E>, "`Result<T, E>` requires `E` to be an object type");
    static_assert(!std::is_reference_v<T>, "`Result<T, E>` must not wrap a reference type");
    static_assert(!std::is_reference_v<E>, "`Result<T, E>` must not wrap a reference type");
    static_assert(!std::is_array_v<T>, "`Result<T, E>` must not wrap an array type");
    static_assert(!std::is_array_v<E>, "`Result<T, E>` must not wrap an array type");
    static_assert(
        !std::is_const_v<E> && !std::is_volatile_v<E>, "`Result<T, E>` must not have a cv-qualified error type");
    static_assert(std::is_destructible_v<T>, "`Result<T, E>` requires T to be destructible");
    static_assert(std::is_destructible_v<E>, "`Result<T, E>` requires E to be destructible");
    static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>,
        "`Result<T, E>` requires T to be movable or copyable");
    static_assert(std::is_move_constructible_v<E> || std::is_copy_constructible_v<E>,
        "`Result<T, E>` requires E to be movable or copyable");
    static_assert(sizeof(E) > 0, "`Result<T, E>` requires E to be a complete type");

    using value_type = T;
    using error_type = E;

    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Result);

    constexpr VIOLET_IMPLICIT Result(const T& ok)
        : n_ok(true)
    {
        ::new (&this->n_storage.Value) T(ok);
    }

    constexpr VIOLET_IMPLICIT Result(T&& ok) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_ok(true)
    {
        ::new (&this->n_storage.Value) T(VIOLET_MOVE(ok));
    }

    template<typename... Args>
        requires(std::is_constructible_v<T, Args...>)
    constexpr VIOLET_EXPLICIT Result(std::in_place_index_t<0L>, Args&&... args)
        : Result(T(VIOLET_FWD(Args, args)...))
    {
    }

    template<typename... Args>
        requires(std::is_constructible_v<E, Args...>)
    constexpr VIOLET_EXPLICIT Result(std::in_place_index_t<1L>, Args&&... args)
        : Result(violet::Err<E>(VIOLET_FWD(Args, args)...))
    {
    }

    constexpr VIOLET_IMPLICIT Result(const violet::Err<E>& err)
    {
        ::new (&this->n_storage.Error) violet::Err(err);
    }

    constexpr VIOLET_IMPLICIT Result(violet::Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        ::new (&this->n_storage.Error) violet::Err(VIOLET_MOVE(err));
    }

    ~Result()
    {
        this->destroy();
    }

    constexpr VIOLET_IMPLICIT Result(const Result& other)
        : n_ok(other.n_ok)
    {
        if (this->n_ok) {
            ::new (&this->n_storage.Value) T(other.n_storage.Value);
        } else {
            ::new (&this->n_storage.Error) violet::Err<E>(other.n_storage.Error);
        }
    }

    constexpr auto operator=(const Result& other) -> Result&
    {
        if (this != &other) {
            this->destroy();

            this->n_ok = other.n_ok;
            if (this->n_ok) {
                ::new (&this->n_storage.Value) T(other.n_storage.Value);
            } else {
                ::new (&this->n_storage.Error) violet::Err<E>(other.n_storage.Error);
            }
        }

        return *this;
    }

    constexpr VIOLET_IMPLICIT Result(Result&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>)
        : n_ok(std::exchange(other.n_ok, false))
    {
        if (this->n_ok) {
            ::new (&this->n_storage.Value) T(VIOLET_MOVE(other.n_storage.Value));
        } else {
            ::new (&this->n_storage.Error) violet::Err<E>(VIOLET_MOVE(other.n_storage.Error));
        }
    }

    constexpr auto operator=(Result&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>) -> Result&
    {
        if (this != &other) {
            // If both are same variant, move-assign; else destroy + reconstruct.
            if (this->n_ok && other.n_ok) {
                this->n_storage.Value = VIOLET_MOVE(other.n_storage.Value);
            } else if (!this->n_ok && !other.n_ok) {
                this->n_storage.Error = VIOLET_MOVE(other.n_storage.Error);
            } else {
                this->destroy();
                this->n_ok = other.n_ok;

                if (other.n_ok) {
                    ::new (&this->n_storage.Value) T(VIOLET_MOVE(other.n_storage.Value));
                } else {
                    ::new (&this->n_storage.Error) violet::Err<E>(VIOLET_MOVE(other.n_storage.Error));
                }
            }
        }

        return *this;
    }

    [[nodiscard]] constexpr auto Ok() const noexcept -> bool
    {
        return this->n_ok;
    }

    [[nodiscard]] constexpr auto Err() const noexcept -> bool
    {
        return !this->Ok();
    }

    constexpr auto Value() noexcept -> T&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "`Result<T, E>` invariant reached");
        return this->n_storage.Value;
    }

    constexpr auto Value() const noexcept -> const T&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "`Result<T, E>` invariant reached");
        return this->n_storage.Value;
    }

    constexpr auto Error() noexcept -> E&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` invariant reached");
        return this->n_storage.Error.Error();
    }

    constexpr auto Error() const noexcept -> const E&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` invariant reached");
        return this->n_storage.Error.Error();
    }

    template<typename U>
        requires(nested_result<Result<U, E>, E>)
    constexpr auto Transpose() && -> Result<U, E>
    {
        if (this->Ok()) {
            return VIOLET_MOVE(Value());
        }

        return Err<E>(VIOLET_MOVE(Error()));
    }

    template<typename Fun>
        requires(callable<Fun>)
    constexpr auto AndThen(Fun&& fun) && -> Result<std::invoke_result_t<Fun, T>, E>
    {
        using Ret = std::invoke_result_t<Fun, T>;
        if (this->Ok()) {
            return Ok<Ret, E>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(Value())));
        }

        return Err<E>(VIOLET_MOVE(Error()));
    }

    template<typename Fun>
        requires(callable<Fun> && callable_returns<Fun, void, const T&>)
    constexpr auto Inspect(Fun&& fun) noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<T>())))
        -> Result&
    {
        if (this->Ok()) {
            std::invoke(VIOLET_FWD(Fun, fun), Value());
        }

        return *this;
    }

    template<typename Fun>
        requires(callable<Fun> && callable_returns<Fun, void, const E&>)
    constexpr auto InspectErr(Fun&& fun) noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<E>())))
        -> Result&
    {
        if (this->Ok()) {
            std::invoke(VIOLET_FWD(Fun, fun), Error());
        }

        return *this;
    }

    constexpr auto IntoErr() && noexcept -> E
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` tried to consume an inexistent error");
        return VIOLET_MOVE(this->n_storage.Error.Error());
    }

    template<typename Pred>
        requires(callable<Pred> && callable_returns<Pred, bool, const T&>)
    constexpr auto OkAnd(Pred&& pred) const noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<T>())))
        -> bool
    {
        return this->Ok() && std::invoke(VIOLET_FWD(Pred, pred), Value());
    }

    template<typename Pred>
        requires(callable<Pred> && callable_returns<Pred, bool, const E&>)
    constexpr auto ErrAnd(Pred&& pred) const noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<E>())))
        -> bool
    {
        return this->Err() && std::invoke(VIOLET_FWD(Pred, pred), Error());
    }

    constexpr auto Unwrap(std::source_location loc = std::source_location::current()) &
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return this->n_storage.Value;
        }

        panicUnexpectly("tried to `Unwrap()` a error variant", loc);
    }

    constexpr auto Unwrap(std::source_location loc = std::source_location::current()) &&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->n_storage.Value);
        }

        panicUnexpectly("tried to `Unwrap()` a error variant", loc);
    }

    constexpr auto Unwrap(std::source_location loc = std::source_location::current()) const&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return this->n_storage.Value;
        }

        panicUnexpectly("tried to `Unwrap()` a error variant", loc);
    }

    constexpr auto Unwrap(std::source_location loc = std::source_location::current()) const&&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->n_storage.Value);
        }

        panicUnexpectly("tried to `Unwrap()` a error variant", loc);
    }

    constexpr auto UnwrapErr(std::source_location loc = std::source_location::current()) &
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return this->n_storage.Error.Error();
        }

        panicUnexpectly("tried to `Unwrap()` an ok variant", loc);
    }

    constexpr auto UnwrapErr(std::source_location loc = std::source_location::current()) &&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return VIOLET_MOVE(this->n_storage.Error.Error());
        }

        panicUnexpectly("tried to `Unwrap()` an ok variant", loc);
    }

    constexpr auto UnwrapErr(std::source_location loc = std::source_location::current()) const&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return this->n_storage.Error.Error();
        }

        panicUnexpectly("tried to `Unwrap()` an ok variant", loc);
    }

    constexpr auto UnwrapErr(std::source_location loc = std::source_location::current()) const&&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> E
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->n_storage.Error.Error());
        }

        panicUnexpectly("tried to `Unwrap()` an ok variant", loc);
    }

    constexpr auto Except(Str message, std::source_location loc = std::source_location::current()) &
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return this->n_storage.Value;
        }

        panicUnexpectly(message, loc);
    }

    constexpr auto Except(Str message, std::source_location loc = std::source_location::current()) &&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->n_storage.Value);
        }

        panicUnexpectly(message, loc);
    }

    constexpr auto Except(Str message, std::source_location loc = std::source_location::current()) const&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return this->n_storage.Value;
        }

        panicUnexpectly(message, loc);
    }

    constexpr auto Except(Str message, std::source_location loc = std::source_location::current()) const&&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->n_storage.Value);
        }

        panicUnexpectly(message, loc);
    }

    constexpr auto UnwrapOr(T&& defaultValue) & noexcept -> T
    {
        return this->Ok() ? this->n_storage.Value : VIOLET_MOVE(defaultValue);
    }

    constexpr auto UnwrapOr(T&& defaultValue) && noexcept -> T
    {
        return this->Ok() ? VIOLET_MOVE(this->n_storage.Value) : VIOLET_MOVE(defaultValue);
    }

    constexpr auto UnwrapOr(T&& defaultValue) const& noexcept -> T
    {
        return this->Ok() ? this->n_storage.Value : VIOLET_MOVE(defaultValue);
    }

    constexpr auto UnwrapOr(T&& defaultValue) const&& noexcept -> T
    {
        return this->Ok() ? VIOLET_MOVE(this->n_storage.Value) : VIOLET_MOVE(defaultValue);
    }

    constexpr auto operator*() & -> T&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Dereferencing a Result in Err state");
        return this->n_storage.Value;
    }

    constexpr auto operator*() && -> T&&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Dereferencing a Result in Err state");
        return VIOLET_MOVE(this->n_storage.Value);
    }

    constexpr auto operator*() const& -> const T&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Dereferencing a Result in Err state");
        return this->n_storage.Value;
    }

    constexpr auto operator*() const&& -> const T&&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Dereferencing a Result in Err state");
        return VIOLET_MOVE(this->n_storage.Value);
    }

    constexpr auto operator->() -> T*
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Accessing a Result in Err state");
        return std::addressof(this->n_storage.Value);
    }

    constexpr auto operator->() const -> const T*
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Accessing a Result in Err state");
        return std::addressof(this->n_storage.Value);
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->Ok();
    }

#if VIOLET_REQUIRE_STL(202302L)
    constexpr VIOLET_EXPLICIT operator std::expected<T, E>() const noexcept
    {
        return this->Ok() ? std::expected<T, E>(Value()) : std::expected<T, E>(std::unexpect, Error());
    }

    constexpr VIOLET_EXPLICIT operator std::unexpected<E>() const noexcept
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "tried to convert a Ok `Result<T, E>` into `std::unexpected<E>`");
        return std::unexpected(Error());
    }
#endif

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        if (this->Ok()) {
            return violet::ToString(Value());
        }

        return violet::ToString(Error());
    }

    constexpr friend auto operator<<(std::ostream& os, const Result<T, E>& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    mutable bool n_ok = false;

    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
    union storage_t {
        T Value;
        violet::Err<E> Error;

        constexpr storage_t() noexcept {}
        ~storage_t() {}
    } n_storage;

    void destroy() noexcept(std::is_nothrow_destructible_v<T> && std::is_nothrow_destructible_v<E>)
    {
        if (this->n_ok) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                this->n_storage.Value.~T();
            }
        } else {
            if constexpr (!std::is_trivially_destructible_v<E>) {
                this->n_storage.Error.~Err<E>();
            }
        }
    }

    [[noreturn]] VIOLET_COLD static void panicUnexpectly(Str message, [[maybe_unused]] const std::source_location& loc)
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
    {
#ifdef VIOLET_HAS_EXCEPTIONS
        throw std::logic_error(std::format("panic in `Result<T, E>` [{}:{}:{} ({})]: {}", loc.file_name(), loc.line(),
            loc.column(), util::DemangleCXXName(loc.function_name()), message));
#else
        std::println(stderr, "panic in `Result<T, E>` [{}:{}:{} ({})]: {}", loc.file_name(), loc.line(), loc.column(),
            util::DemangleCXXName(loc.function_name()), message);

        std::unreachable();
#endif
    }
};

template<typename E>
struct Result<void, E> final {
    static_assert(std::is_object_v<E>, "`Result<T, E>` requires `E` to be an object type");
    static_assert(!std::is_reference_v<E>, "`Result<T, E>` must not wrap a reference type");
    static_assert(!std::is_array_v<E>, "`Result<T, E>` must not wrap an array type");
    static_assert(
        !std::is_const_v<E> && !std::is_volatile_v<E>, "`Result<T, E>` must not have a cv-qualified error type");
    static_assert(std::is_destructible_v<E>, "`Result<T, E>` requires E to be destructible");
    static_assert(std::is_move_constructible_v<E> || std::is_copy_constructible_v<E>,
        "`Result<T, E>` requires E to be movable or copyable");
    static_assert(sizeof(E) > 0, "`Result<T, E>` requires E to be a complete type");

    using value_type = void;
    using error_type = E;

    constexpr VIOLET_IMPLICIT Result() noexcept = default;
    constexpr VIOLET_IMPLICIT Result(const violet::Err<E>& err)
        : n_value(new violet::Err<E>(err))
    {
    }

    constexpr VIOLET_IMPLICIT Result(violet::Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<E>)
        : n_value(new violet::Err<E>(VIOLET_MOVE(err)))
    {
    }

    template<typename U>
        requires(!std::same_as<std::decay_t<U>, Result<void, E>> && std::convertible_to<U, E>)
    constexpr VIOLET_IMPLICIT Result(const U& err)
        : n_value(new violet::Err<E>(err))
    {
    }

    template<typename U>
        requires(!std::same_as<std::decay_t<U>, Result<void, E>> && std::convertible_to<U, E>)
    constexpr VIOLET_IMPLICIT Result(U&& err)
        : n_value(new violet::Err<E>(VIOLET_FWD(U, err)))
    {
    }

    ~Result()
    {
        delete this->n_value;
    }

    constexpr VIOLET_IMPLICIT Result(const Result& other)
        : n_value(other.n_value ? new violet::Err<E>(*other.n_value) : nullptr)
    {
    }

    constexpr auto operator=(const Result& other) noexcept -> Result&
    {
        if (this != &other) {
            delete this->n_value;
            this->n_value = other.n_value ? new violet::Err<E>(*other.n_value) : nullptr;
        }

        return *this;
    }

    constexpr VIOLET_IMPLICIT Result(Result&& other) noexcept
        : n_value(std::exchange(other.n_value, nullptr))
    {
    }

    constexpr auto operator=(Result&& other) noexcept -> Result&
    {
        if (this != &other) {
            delete this->n_value;
            this->n_value = std::exchange(other.n_value, nullptr);
        }

        return *this;
    }

    [[nodiscard]] constexpr auto Ok() const noexcept -> bool
    {
        return this->n_value == nullptr;
    }

    [[nodiscard]] constexpr auto Err() const noexcept -> bool
    {
        return this->n_value != nullptr;
    }

    constexpr auto Error() noexcept -> E&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` invariant reached");
        return this->n_value->Error();
    }

    constexpr auto Error() const noexcept -> const E&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` invariant reached");
        return this->n_value->Error();
    }

    template<typename Fun>
        requires(callable<Fun>)
    constexpr auto AndThen(Fun&& fun) && -> Result<std::invoke_result_t<Fun>, E>
    {
        using Ret = std::invoke_result_t<Fun>;
        if (this->Ok()) {
            return Ok<Ret, E>(std::invoke(VIOLET_FWD(Fun, fun)));
        }

        return Err<E>(VIOLET_MOVE(Error()));
    }

    template<typename Fun>
        requires(callable<Fun> && callable_returns<Fun, void>)
    constexpr auto Inspect(Fun&& fun) noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun)))) -> Result&
    {
        if (this->Ok()) {
            std::invoke(VIOLET_FWD(Fun, fun));
        }

        return *this;
    }

    template<typename Fun>
        requires(callable<Fun> && callable_returns<Fun, void, const E&>)
    constexpr auto InspectErr(Fun&& fun) noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<E>())))
        -> Result&
    {
        if (this->Ok()) {
            std::invoke(VIOLET_FWD(Fun, fun), Error());
        }

        return *this;
    }

    constexpr auto IntoErr() && noexcept -> E
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<void, E>` tried to consume an inexistent error");
        return VIOLET_MOVE(this->n_value);
    }

    template<typename Pred>
        requires(callable<Pred> && callable_returns<Pred, bool>)
    constexpr auto OkAnd(Pred&& pred) const noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred)))) -> bool
    {
        return this->Ok() && std::invoke(VIOLET_FWD(Pred, pred));
    }

    template<typename Pred>
        requires(callable<Pred> && callable_returns<Pred, bool, const E&>)
    constexpr auto ErrAnd(Pred&& pred) const noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<E>())))
        -> bool
    {
        return this->Err() && std::invoke(VIOLET_FWD(Pred, pred), Error());
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->Ok();
    }

#if VIOLET_REQUIRE_STL(202302L)
    constexpr VIOLET_EXPLICIT operator std::expected<void, E>() const noexcept
    {
        return this->Ok() ? std::expected<void, E>(std::in_place) : std::expected<void, E>(std::unexpect, Error());
    }

    constexpr VIOLET_EXPLICIT operator std::unexpected<E>() const noexcept
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "tried to convert a Ok `Result<void, E>` into `std::unexpected<E>`");
        return std::unexpected<E>(Error());
    }
#endif

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        if (this->Ok()) {
            return "<void type>";
        }

        return violet::ToString(Error());
    }

    constexpr friend auto operator<<(std::ostream& os, const Result<void, E>& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    violet::Err<E>* n_value = nullptr;
};

} // namespace violet
