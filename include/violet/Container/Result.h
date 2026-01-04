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

#include "violet/Violet.h"

#if VIOLET_USE_RTTI
#include "violet/Support/Demangle.h"
#endif

#ifndef VIOLET_HAS_EXCEPTIONS
#include <source_location>
#endif

#include <expected>
#include <initializer_list>
#include <type_traits>

namespace violet {

/* --=-- START :: internals --=--*/

#ifndef VIOLET_HAS_EXCEPTIONS
VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void resultUnwrapFail(const std::source_location& loc);

VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void resultUnwrapErrFail(const std::source_location& loc);

VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void resultUnwrapFail(CStr message, const std::source_location& loc);
#else
VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void resultUnwrapFail();

VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void resultUnwrapErrFail();

VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void resultUnwrapFail(CStr message);
#endif

/* --=--  END :: internals  --=-- */

template<typename T>
struct Optional;

template<typename T, typename E>
struct Result;

/// Backport of C++23's `std::unexpected`.
// -+- TODO(@auguwu): alias this as `std::unexpected` once we drop C++20 support -+-
template<typename E>
struct Err final {
    static_assert(!std::is_void_v<E>, "`E` cannot be used with `void`");
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Err);

    constexpr VIOLET_EXPLICIT Err(const E& err)
        : n_error(err)
    {
    }

    constexpr VIOLET_IMPLICIT Err(E&& err)
        : n_error(VIOLET_MOVE(err))
    {
    }

    template<typename... Args>
        requires(!std::is_same_v<std::decay_t<Args>..., E> && std::is_constructible_v<E, Args...>)
    constexpr VIOLET_IMPLICIT Err(Args&&... args)
        : n_error(VIOLET_FWD(Args, args)...)
    {
    }

    template<typename U, typename... Args>
        requires std::is_constructible_v<E, std::initializer_list<U>&, Args&&...>
    constexpr VIOLET_EXPLICIT Err(std::initializer_list<U> list, Args&&... args)
        : n_error(list, VIOLET_FWD(Args, args)...)
    {
    }

    constexpr auto Error() & noexcept -> E&
    {
        return n_error;
    }

    constexpr auto Error() const& noexcept -> const E&
    {
        return n_error;
    }

    constexpr auto Error() && noexcept -> E&&
    {
        return VIOLET_MOVE(n_error);
    }

    constexpr auto Error() const&& noexcept -> const E&&
    {
        return VIOLET_MOVE(n_error);
    }

    constexpr auto operator==(const Err& other)
    {
        return this->Error() == other.Error();
    }

    constexpr auto operator!=(const Err& other)
    {
        return this->Error() != other.Error();
    }

    constexpr auto operator<(const Err& other)
    {
        return this->Error() < other.Error();
    }

    constexpr auto operator<=(const Err& other)
    {
        return this->Error() <= other.Error();
    }

    constexpr auto operator>(const Err& other)
    {
        return this->Error() > other.Error();
    }

    constexpr auto operator>=(const Err& other)
    {
        return this->Error() != other.Error();
    }

    template<typename T>
    constexpr VIOLET_EXPLICIT operator violet::Result<T, E>() noexcept
    {
        return violet::Result<T, E>(std::in_place_index<1L>, Error());
    }

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202302L) || __cplusplus >= 202302L
    template<typename T>
    constexpr VIOLET_EXPLICIT operator std::expected<T, E>() & noexcept
    {
        return std::expected<T, E>(std::unexpect, VIOLET_MOVE(Error()));
    }

    template<typename T>
    constexpr VIOLET_EXPLICIT operator std::expected<T, E>() && noexcept
    {
        return std::expected<T, E>(std::unexpect, VIOLET_MOVE(Error()));
    }

    constexpr VIOLET_EXPLICIT operator std::unexpected<E>() noexcept
    {
        return std::unexpected<E>(Error());
    }
#endif

private:
    E n_error;
};

template<typename E>
Err(E) -> Err<E>;

/// Creates a new `Result<T, E>` that contains a successful value.
///
/// @tparam T The underlying success type to construct.
/// @tparam E The underlying error type to pass.
/// @tparam Args The arguments to pass to `T`'s constructor.
/// @param args The arguments to pass to `T`'s constructor.
/// @return An `Optional<T>` with the value constructed.
template<typename T, typename E, typename... Args>
constexpr static auto Ok(Args&&... args) -> Result<T, E>
{
    static_assert(!std::is_void_v<T>, "`void` types cannot be constructed. use `{}` for void-like results");

    return Result<T, E>(std::in_place_index<0>, VIOLET_FWD(Args, args)...);
}

/// Representation of a successful or failed state, analgous of Rust's [`std::result::Result`].
///
/// [`std::result::Result`]: https://doc.rust-lang.org/1.90.0/std/result/enum.Result.html
template<typename T, typename E>
struct [[nodiscard("always check the error state")]] VIOLET_API Result final {
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Result);

    using value_type = T;
    using error_type = E;

    constexpr VIOLET_IMPLICIT Result(const T& ok)
        : n_ok(true)
    {
        ::new (&this->n_storage.ok) T(ok);
    }

    constexpr VIOLET_IMPLICIT Result(T&& ok) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_ok(true)
    {
        ::new (&this->n_storage.ok) T(VIOLET_MOVE(ok));
    }

    /// @internal
    template<typename... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr VIOLET_EXPLICIT Result(std::in_place_index_t<0L>, Args&&... args)
        : n_ok(true)
    {
        ::new (&this->n_storage.ok) T(VIOLET_FWD(Args, args)...);
    }

    /// @internal
    template<typename... Args>
        requires std::is_constructible_v<E, Args&&...>
    constexpr VIOLET_EXPLICIT Result(std::in_place_index_t<1L>, Args&&... args)
        : n_ok(true)
    {
        ::new (&this->n_storage.ok) violet::Err<E>(VIOLET_FWD(Args, args)...);
    }

    constexpr VIOLET_IMPLICIT Result(const violet::Err<E>& err)
    {
        ::new (&this->n_storage.err) violet::Err<E>(err);
    }

    constexpr VIOLET_IMPLICIT Result(violet::Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<E>)
    {
        ::new (&this->n_storage.err) violet::Err<E>(VIOLET_MOVE(err));
    }

    ~Result()
    {
        destroy();
    }

    constexpr VIOLET_IMPLICIT Result(const Result& other)
        : n_ok(other.n_ok)
    {
        if (other.n_ok) {
            ::new (&this->n_storage.ok) T(other.n_storage.ok);
        } else {
            ::new (&this->n_storage.err) violet::Err<E>(other.n_storage.err);
        }
    }

    constexpr auto operator=(const Result& other) noexcept -> Result&
    {
        if (this != &other) {
            destroy();

            this->n_ok = other.n_ok;
            if (other.n_ok) {
                ::new (&this->n_storage.ok) T(other.n_storage.ok);
            } else {
                ::new (&this->n_storage.err) violet::Err<E>(other.n_storage.err);
            }
        }

        return *this;
    }

    constexpr Result(Result&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>)
        : n_ok(other.n_ok)
    {
        if (other.n_ok) {
            ::new (&this->n_storage.ok) T(VIOLET_MOVE(other.n_storage.ok));
        } else {
            ::new (&this->n_storage.err) violet::Err<E>(VIOLET_MOVE(other.n_storage.err));
        }
    }

    constexpr auto operator=(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_move_constructible_v<E> && std::is_nothrow_move_assignable_v<T>
        && std::is_nothrow_move_assignable_v<E>) -> Result&
    {
        if (this != &other) {
            // If both are same variant, move-assign; else destroy + reconstruct.
            if (this->n_ok && other.n_ok) {
                this->n_storage.ok = VIOLET_MOVE(other.n_storage.ok);
            } else if (!this->n_ok && !other.n_ok) {
                this->n_storage.err = VIOLET_MOVE(other.n_storage.err);
            } else {
                destroy();
                this->n_ok = other.n_ok;
                if (other.n_ok) {
                    ::new (&this->n_storage.ok) T(VIOLET_MOVE(other.n_storage.ok));
                } else {
                    ::new (&this->n_storage.err) violet::Err<E>(VIOLET_MOVE(other.n_storage.err));
                }
            }
        }

        return *this;
    }

    constexpr auto Ok() const noexcept -> bool
    {
        return this->n_ok;
    }

    constexpr auto Err() const noexcept -> bool
    {
        return !this->Ok();
    }

    constexpr auto IntoOpt() const noexcept -> Optional<T>
    {
        return Ok() ? Optional<T>(Value()) : std::nullopt;
    }

    constexpr auto Value() noexcept -> T&
    {
        VIOLET_DEBUG_ASSERT(Ok(), "`Result<T, E>` doesn't contain any value");
        return this->n_storage.ok;
    }

    constexpr auto Value() const noexcept -> const T&
    {
        VIOLET_DEBUG_ASSERT(Ok(), "`Result<T, E>` doesn't contain any value");
        return this->n_storage.ok;
    }

    constexpr auto Error() noexcept -> E&
    {
        VIOLET_DEBUG_ASSERT(Err(), "`Result<T, E>` contains a error");
        return this->n_storage.err.Error();
    }

#ifdef VIOLET_HAS_EXCEPTIONS
    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    ///
    /// @throws std::logic_error If the error variant was present in this `Result`.
    constexpr auto Unwrap() && -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail();

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    ///
    /// @throws std::logic_error If the error variant was present in this `Result`.
    constexpr auto Unwrap() & -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail();

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    ///
    /// @throws std::logic_error If the error variant was present in this `Result`.
    constexpr auto Unwrap() const& -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return Value();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail();

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

#else
    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Optional` is empty. If you want to provide a
    /// default value, use `UnwrapOr` or `UnwrapOrElse`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    constexpr auto Unwrap(const std::source_location& loc = std::source_location::current()) && -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail(loc);

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Optional` is empty. If you want to provide a
    /// default value, use `UnwrapOr` or `UnwrapOrElse`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    constexpr auto Unwrap(const std::source_location& loc = std::source_location::current()) & -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail(loc);

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Optional` is empty. If you want to provide a
    /// default value, use `UnwrapOr` or `UnwrapOrElse`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    constexpr auto Unwrap(const std::source_location& loc = std::source_location::current()) const& -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return Value();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail(loc);

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

#endif

#ifdef VIOLET_HAS_EXCEPTIONS
    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    ///
    /// @throws std::logic_error If the error variant was present in this `Result`.
    constexpr auto UnwrapErr() && -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return Error();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapErrFail();

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    ///
    /// @throws std::logic_error If the error variant was present in this `Result`.
    constexpr auto UnwrapErr() & -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return Error();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapErrFail();

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    ///
    /// @throws std::logic_error If the error variant was present in this `Result`.
    constexpr auto UnwrapErr() const& -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return Error();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapErrFail();

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

#else
    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Optional` is empty. If you want to provide a
    /// default value, use `UnwrapOr` or `UnwrapOrElse`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    constexpr auto UnwrapErr(const std::source_location& loc = std::source_location::current()) && -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return Error();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapErrFail(loc);

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Optional` is empty. If you want to provide a
    /// default value, use `UnwrapOr` or `UnwrapOrElse`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    constexpr auto UnwrapErr(const std::source_location& loc = std::source_location::current()) & -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return Error();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapErrFail(loc);

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming this `Result`.
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Optional` is empty. If you want to provide a
    /// default value, use `UnwrapOr` or `UnwrapOrElse`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<String, UInt32>("hello world");
    /// String str = res.Unwrap();
    /// ```
    constexpr auto UnwrapErr(const std::source_location& loc = std::source_location::current()) const& -> E
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return Error();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapErrFail(loc);

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

#endif

    /// Returns the contained value, consuming the `Result`, or a default value.
    ///
    /// ## Example
    /// ```cpp
    /// auto res1 = violet::Ok<int, String>(42);
    /// int val1 = std::move(res1).UnwrapOr(0); // val1 == 42
    ///
    /// Result<int, String> res2 = violet::Err<String>("hello world");
    /// int val2 = std::move(res2).UnwrapOr(0); // val2 == 0
    /// ```
    ///
    /// @param def The default value to return if the `Optional` is empty.
    /// @return The contained value or the default value.
    constexpr auto UnwrapOr(T&& def) && -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }
        else
        {
            return VIOLET_MOVE(def);
        }
    }

    /// Returns the contained value, consuming the `Result`, or a default value.
    ///
    /// ## Example
    /// ```cpp
    /// auto res1 = violet::Ok<int, String>(42);
    /// int val1 = std::move(res1).UnwrapOr(0); // val1 == 42
    ///
    /// Result<int, String> res2 = violet::Err<String>("hello world");
    /// int val2 = std::move(res2).UnwrapOr(0); // val2 == 0
    /// ```
    ///
    /// @param def The default value to return if the `Optional` is empty.
    /// @return The contained value or the default value.
    constexpr auto UnwrapOr(T&& def) & -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }
        else
        {
            return VIOLET_MOVE(def);
        }
    }

    /// Returns the contained value, consuming the `Result`, or a default value.
    ///
    /// ## Example
    /// ```cpp
    /// auto res1 = violet::Ok<int, String>(42);
    /// int val1 = std::move(res1).UnwrapOr(0); // val1 == 42
    ///
    /// Result<int, String> res2 = violet::Err<String>("hello world");
    /// int val2 = std::move(res2).UnwrapOr(0); // val2 == 0
    /// ```
    ///
    /// @param def The default value to return if the `Optional` is empty.
    /// @return The contained value or the default value.
    constexpr auto UnwrapOr(T&& def) const& -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return Value();
        }
        else
        {
            return VIOLET_MOVE(def);
        }
    }

    /// Returns the contained value, consuming the `Result`, or computes it from a closure.
    ///
    /// ## Example
    /// ```cpp
    /// auto res1 = violet::Ok<int, String>(42);
    /// int val1 = std::move(res1).UnwrapOrElse([]() { return 0; }); // val1 == 42
    ///
    /// Result<int, String> res2 = violet::Err<String>("hello world");
    /// int val2 = std::move(res2).UnwrapOr([]() { return 0; }); // val2 == 0
    /// ```
    ///
    /// @param fun The closure to call to compute the default value.
    /// @return The contained value or the computed default value.
    template<typename Fun>
        requires callable<Fun> && std::convertible_to<std::invoke_result_t<Fun>, T>
    constexpr auto UnwrapOrElse(Fun&& fun) &&
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }
        else
        {
            return std::invoke(VIOLET_FWD(Fun, fun));
        }
    }

    /// Returns the contained value, consuming the `Result`, or computes it from a closure.
    ///
    /// ## Example
    /// ```cpp
    /// auto res1 = violet::Ok<int, String>(42);
    /// int val1 = std::move(res1).UnwrapOrElse([]() { return 0; }); // val1 == 42
    ///
    /// Result<int, String> res2 = violet::Err<String>("hello world");
    /// int val2 = std::move(res2).UnwrapOr([]() { return 0; }); // val2 == 0
    /// ```
    ///
    /// @param fun The closure to call to compute the default value.
    /// @return The contained value or the computed default value.
    template<typename Fun>
        requires callable<Fun> && std::convertible_to<std::invoke_result_t<Fun>, T>
    constexpr auto UnwrapOrElse(Fun&& fun) &
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }
        else
        {
            return std::invoke(VIOLET_FWD(Fun, fun));
        }
    }

    /// Returns the contained value, consuming the `Result`, or computes it from a closure.
    ///
    /// ## Example
    /// ```cpp
    /// auto res1 = violet::Ok<int, String>(42);
    /// int val1 = std::move(res1).UnwrapOrElse([]() { return 0; }); // val1 == 42
    ///
    /// Result<int, String> res2 = violet::Err<String>("hello world");
    /// int val2 = std::move(res2).UnwrapOr([]() { return 0; }); // val2 == 0
    /// ```
    ///
    /// @param fun The closure to call to compute the default value.
    /// @return The contained value or the computed default value.
    template<typename Fun>
        requires callable<Fun> && std::convertible_to<std::invoke_result_t<Fun>, T>
    constexpr auto UnwrapOrElse(Fun&& fun) const&
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return Value();
        }
        else
        {
            return std::invoke(VIOLET_FWD(Fun, fun));
        }
    }

    /// Returns the contained value, consuming the `Result`, without checking if it has a value.
    ///
    /// ## Safety
    /// This function is unsafe because it does not check if the `Result` has a value. If the `Result` is
    /// empty, this will result in undefined behavior.
    ///
    /// @return The contained value.
    constexpr auto UnwrapUnchecked(Unsafe) && noexcept -> T
    {
        return VIOLET_MOVE(this->n_storage.ok);
    }

    /// Returns the contained value, consuming the `Result`, without checking if it has a value.
    ///
    /// ## Safety
    /// This function is unsafe because it does not check if the `Result` has a value. If the `Result` is
    /// empty, this will result in undefined behavior.
    ///
    /// @return The contained value.
    constexpr auto UnwrapUnchecked(Unsafe) const&& noexcept -> T
    {
        return VIOLET_MOVE(this->n_storage.ok);
    }

    /// Returns the contained value, consuming the `Result`, without checking if it has a value.
    ///
    /// ## Safety
    /// This function is unsafe because it does not check if the `Result` has a value. If the `Result` is
    /// empty, this will result in undefined behavior.
    ///
    /// @return The contained value.
    constexpr auto UnwrapUnchecked(Unsafe) & noexcept -> T
    {
        return this->n_storage.ok;
    }

    /// Returns the contained value, consuming the `Result`, without checking if it has a value.
    ///
    /// ## Safety
    /// This function is unsafe because it does not check if the `Result` has a value. If the `Result` is
    /// empty, this will result in undefined behavior.
    ///
    /// @return The contained value.
    constexpr auto UnwrapUnchecked(Unsafe) const& noexcept -> T
    {
        return this->n_storage.ok;
    }

#ifndef VIOLET_HAS_EXCEPTIONS
    /// Returns the contained value, consuming the `Optional`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<int, String>("hello world");
    /// int val = std::move(res).Expect("value was not present"); // terminates
    /// ```
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Result` is empty, with a custom message.
    ///
    /// @param message The message to print on termination.
    /// @param loc The source location of the call, which is used for the termination message.
    /// @return The contained value.
    constexpr auto Expect(Str message, const std::source_location& loc = std::source_location::current()) && -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

        resultUnwrapFail(message.data(), loc);
    }

    /// Returns the contained value, consuming the `Optional`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<int, String>("hello world");
    /// int val = std::move(res).Expect("value was not present"); // terminates
    /// ```
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Result` is empty, with a custom message.
    ///
    /// @param message The message to print on termination.
    /// @param loc The source location of the call, which is used for the termination message.
    /// @return The contained value.
    constexpr auto Expect(Str message, const std::source_location& loc = std::source_location::current()) & -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

        resultUnwrapFail(message.data(), loc);
    }

    /// Returns the contained value, consuming the `Optional`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<int, String>("hello world");
    /// int val = std::move(res).Expect("value was not present"); // terminates
    /// ```
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Result` is empty, with a custom message.
    ///
    /// @param message The message to print on termination.
    /// @param loc The source location of the call, which is used for the termination message.
    /// @return The contained value.
    constexpr auto Expect(Str message, const std::source_location& loc = std::source_location::current()) const& -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

        resultUnwrapFail(message.data(), loc);
    }
#else
    /// Returns the contained value, consuming the `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<int, String>("hello world");
    /// int val = std::move(res).Expect("value was not present"); // throws
    /// ```
    ///
    /// @throws [std::logic_error] if the `Result` is empty, with a custom message.
    /// @param message The message to use for the exception.
    /// @return The contained value.
    ///
    constexpr auto Expect(Str message) && -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail(message.data());

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming the `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<int, String>("hello world");
    /// int val = std::move(res).Expect("value was not present"); // throws
    /// ```
    ///
    /// @throws [std::logic_error] if the `Result` is empty, with a custom message.
    /// @param message The message to use for the exception.
    /// @return The contained value.
    ///
    constexpr auto Expect(Str message) & -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(Value());
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail(message.data());

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    /// Returns the contained value, consuming the `Result`.
    ///
    /// ## Example
    /// ```cpp
    /// auto res = violet::Ok<int, String>("hello world");
    /// int val = std::move(res).Expect("value was not present"); // throws
    /// ```
    ///
    /// @throws [std::logic_error] if the `Result` is empty, with a custom message.
    /// @param message The message to use for the exception.
    /// @return The contained value.
    ///
    constexpr auto Expect(Str message) const& -> T
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return Value();
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        resultUnwrapFail(message.data());

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

#endif
    constexpr auto operator*() & noexcept -> T&
    {
        return this->UnwrapUnchecked(Unsafe("it is upon the callee"));
    }

    constexpr auto operator*() const& noexcept -> const T&
    {
        return this->UnwrapUnchecked(Unsafe("it is upon the callee"));
    }

    constexpr auto operator*() && noexcept -> T&&
    {
        return this->UnwrapUnchecked(Unsafe("it is upon the callee"));
    }

    constexpr auto operator*() const&& noexcept -> const T&&
    {
        return this->UnwrapUnchecked(Unsafe("it is upon the callee"));
    }

    constexpr auto operator->() noexcept -> T*
    {
        return std::addressof(this->n_storage.ok);
    }

    constexpr auto operator->() const noexcept -> const T*
    {
        return std::addressof(this->n_storage.ok);
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->Ok();
    }

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202302L) || __cplusplus >= 202302L
    constexpr VIOLET_EXPLICIT operator std::expected<T, E>() noexcept
    {
        if (this->Ok()) {
            return std::expected<T, E>(Value());
        }

        return std::unexpected<E>(Error());
    }

    constexpr VIOLET_EXPLICIT operator std::unexpected<E>() noexcept
    {
        assert(this->Err());
        return std::unexpected<E>(Error());
    }
#endif

    auto ToString() const noexcept -> String
    {
        if (this->Ok()) {
            if constexpr (Stringify<T>) {
                return violet::ToString(Value());
            }

#if VIOLET_USE_RTTI
            const auto& type = typeid(T);
            return std::format("ok variant: type `{}@{}`", util::DemangleCXXName(type.name()), type.hash_code());
#else
            return "error variant: ????";
#endif
        } else {
            if constexpr (Stringify<E>) {
                return violet::ToString(Value());
            }

#if VIOLET_USE_RTTI
            const auto& type = typeid(E);
            return std::format("error variant: type `{}@{}`", util::DemangleCXXName(type.name()), type.hash_code());
#else
            return "error variant: ????";
#endif
        }
    }

    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

private:
    mutable bool n_ok = false;
    union storage_t { // NOLINT(cppcoreguidelines-special-member-functions)
        T ok;
        violet::Err<E> err;

        storage_t() {}
        ~storage_t() {}
    } n_storage;

    void destroy()
    {
        if (this->n_ok) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                this->n_storage.ok.~T();
            }
        } else {
            if constexpr (!std::is_trivially_destructible_v<E>) {
                this->n_storage.err.~Err<E>();
            }
        }
    }
};

/// Specialization for `void`-returned types of a [`Result`].
template<typename E>
struct [[nodiscard("always check the error state before discard")]] VIOLET_API Result<void, E> final {
    using value_type = void;
    using error_type = E;

    constexpr Result() noexcept = default;
    constexpr VIOLET_IMPLICIT Result(const violet::Err<E>& err)
        : n_err(new violet::Err<E>(err))
    {
    }

    constexpr VIOLET_IMPLICIT Result(violet::Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<E>)
        : n_err(new violet::Err<E>(VIOLET_MOVE(err)))
    {
    }

    template<typename U = E>
        requires std::convertible_to<U, E>
    constexpr VIOLET_IMPLICIT Result(const U& err)
        : n_err(new violet::Err<U>(err))
    {
    }

    template<typename U = E>
        requires std::convertible_to<U, E>
    constexpr VIOLET_IMPLICIT Result(U&& err)
        : n_err(new violet::Err<U>(VIOLET_FWD(U, err)))
    {
    }

    ~Result()
    {
        delete this->n_err;
    }

    constexpr VIOLET_IMPLICIT Result(const Result& other)
        : n_err(other.n_err ? new violet::Err<E>(*other.n_err) : nullptr)
    {
    }

    constexpr auto operator=(const Result& other) noexcept -> Result&
    {
        if (this != &other) {
            delete this->n_err;
            this->n_err = other.n_err ? new violet::Err<E>(*other.n_err) : nullptr;
        }

        return *this;
    }

    constexpr VIOLET_IMPLICIT Result(Result&& other) noexcept
        : n_err(std::exchange(other.n_err, nullptr))
    {
    }

    constexpr auto operator=(Result&& other) noexcept -> Result&
    {
        if (this != &other) {
            delete this->n_err;
            this->n_err = std::exchange(other.n_err, nullptr);
        }

        return *this;
    }

    [[nodiscard]] constexpr auto Ok() const noexcept -> bool
    {
        return this->n_err == nullptr;
    }

    [[nodiscard]] constexpr auto Err() const noexcept -> bool
    {
        return !this->Ok();
    }

    [[nodiscard]] constexpr auto Error() const -> const E&
    {
        assert(Err());
        return this->n_err->Error();
    }

    [[nodiscard]] constexpr auto Error() -> E&
    {
        assert(Err());
        return this->n_err->Error();
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_err == nullptr;
    }

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202302L) || __cplusplus >= 202302L
    constexpr VIOLET_EXPLICIT operator std::expected<void, E>() const noexcept
    {
        if (this->Ok()) {
            return std::expected<void, E>();
        }

        return std::unexpected<E>(Error());
    }

    constexpr VIOLET_EXPLICIT operator std::unexpected<E>() const noexcept
    {
        assert(this->Err());
        return std::unexpected<E>(Error());
    }
#endif

    constexpr auto operator<<(std::ostream& os) noexcept -> std::ostream&
    {
        if (Ok()) {
            return os << "«result of 'void' type»";
        }

        if constexpr (Stringify<E>) {
            return os << violet::ToString(Error());
        }

#if VIOLET_USE_RTTI
        const auto& type = typeid(E);
        return os << "error variant type `" << util::DemangleCXXName(type.name()) << '@' << type.hash_code()
                  << "` not streamable»";
#else
        return os << "«type `E` not streamable»";
#endif
    }

private:
    violet::Err<E>* n_err = nullptr;
};

} // namespace violet

template<typename T, typename E>
struct std::formatter<violet::Result<T, E>>: public violet::StringifyFormatter<violet::Result<T, E>> {};
