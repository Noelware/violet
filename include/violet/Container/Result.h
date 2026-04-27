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

#include <violet/Print.h>
#include <violet/SourceLocation.h>
#include <violet/Violet.h>

#if VIOLET_REQUIRE_STL(202302L)
#include <expected>
#endif

#include <type_traits>
#include <utility>

namespace violet::detail {

#if VIOLET_REQUIRE_STL(202302L)
/// Returns **true** if `T` is an instance of `std::expected<T, E>`.
template<typename T>
constexpr inline bool is_std_expected_v = instanceof_v<std::expected, T>;

static_assert(is_std_expected_v<std::expected<String, Int32>>);
static_assert(!is_std_expected_v<bool>);
#else
/// Always returns **false** as `std::expected` is available in C++23 and higher
template<typename T, typename E>
constexpr inline bool is_std_expected_v = false;
#endif

} // namespace violet::detail

namespace violet {

template<typename T, typename E>
struct Result;

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
struct VIOLET_API is_result: std::false_type { };

template<typename T, typename E>
struct VIOLET_API is_result<Result<T, E>>: std::true_type { };

#if VIOLET_REQUIRE_STL(202302L)
template<typename T, typename E>
struct VIOLET_API is_result<std::expected<T, E>>: std::true_type { };
#endif

template<typename T>
static constexpr bool is_result_v = is_result<T>::value;

/// A type-trait to extract the inner value and error types from an [`Result`] type.
///
/// The primary template is intentionally left undefined. It is meant to be specialized
/// on arbitrary result types, mainly used in the `VIOLET_TRY_VOID` macro.
///
/// @tparam T The type from which to extract an inner value and error type.
template<typename T>
struct VIOLET_API result_type;

/// Specialization of [`result_type`] for [`violet::Result`].
template<typename U, typename E>
struct VIOLET_API result_type<Result<U, E>> final {
    using value_type = U;
    using error_type = E;
};

#if VIOLET_REQUIRE_STL(202302L)
/// Specialization of [`result_type`] for [`std::expected`].
template<typename U, typename E>
struct VIOLET_API result_type<std::expected<U, E>> final {
    using value_type = U;
    using error_type = E;
};
#endif

/// Convenience alias for accessing the extracted inner value type.
///
/// It is the equivalent to `typename violet::result_type<T>::value_type`.
///
/// @tparam T which optional wrapper whose inner type should be extracted.
template<typename T>
using result_value_type_t = typename result_type<T>::value_type;

/// Convenience alias for accessing the extracted inner error type.
///
/// It is the equivalent to `typename violet::result_type<T>::error_type`.
///
/// @tparam T which optional wrapper whose inner type should be extracted.
template<typename T>
using result_error_type_t = typename result_type<T>::error_type;

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

    template<typename Other>
        requires(!std::same_as<E, std::decay_t<Other>> && std::constructible_from<E, std::decay_t<Other>>)
    constexpr VIOLET_IMPLICIT Err(Err<std::decay_t<E>>& other) = delete;

    /// Implicitly convert a [`Err<Other>`](violet::Err) into an [`Err<E>`](violet::Err) when `E`
    /// is constructible from `Other`.
    ///
    /// This is meant to be the C++ equivalent of Rust's [`std::convert::From`] or [`std::convert::Into`] blanket
    /// implementations for error propagation, enabling the [`VIOLET_TRY`]/[`VIOLET_TRY_VOID`] macros to propagate
    /// concrete error types (like [`violet::io::Error`]) into a type-erased container (e.g., [`violet::anyhow::Error`])
    /// without explicit conversion at each callsite.
    ///
    /// [`std::convert::From`]: https://doc.rust-lang.org/1.94.1/std/convert/trait.From.html
    /// [`std::convert::Into`]: https://doc.rust-lang.org/1.94.1/std/convert/trait.Into.html
    template<typename Other>
        requires(!std::same_as<E, std::decay_t<Other>> && std::constructible_from<E, std::decay_t<Other>>)
    constexpr VIOLET_IMPLICIT Err(Err<Other>&& other) noexcept(std::is_nothrow_move_constructible_v<E>)
        : n_value(E(VIOLET_MOVE(other).Error()))
    {
    }

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

#if VIOLET_REQUIRE_STL(202302L)
    template<typename Other>
        requires(!std::same_as<E, std::decay_t<Other>> && std::constructible_from<E, std::decay_t<Other>>)
    constexpr VIOLET_IMPLICIT Err(std::unexpected<Other>&& other) noexcept(std::is_nothrow_move_constructible_v<E>)
        : n_value(E(VIOLET_MOVE(other).error()))
    {
    }
#endif

    /// [**lvalue**] Access the contained error.
    constexpr auto Error() & noexcept VIOLET_LIFETIMEBOUND -> E&
    {
        return this->n_value;
    }

    /// [**rvalue**] Access the contained error.
    constexpr auto Error() && noexcept VIOLET_LIFETIMEBOUND -> E&&
    {
        return VIOLET_MOVE(this->n_value);
    }

    /// [const **lvalue**] Access the contained error.
    constexpr auto Error() const& noexcept VIOLET_LIFETIMEBOUND -> const E&
    {
        return this->n_value;
    }

    /// [const **rvalue**] Access the contained error.
    constexpr auto Error() const&& noexcept VIOLET_LIFETIMEBOUND -> const E&&
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

    friend auto operator<<(std::ostream& os, const Err<E>& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    E n_value;
};

/// A tagged, explicit ok variant for [`violet::Result`].
template<typename T>
struct VIOLET_API Ok final {
    static_assert(std::is_object_v<T>, "`Ok<T>` requires `T` to be a object type");
    static_assert(!std::is_void_v<T>, "`Ok<void>` is ill-formed");
    static_assert(!std::is_array_v<T>, "`Ok<T>` must wrap an array type");
    static_assert(std::is_destructible_v<T>, "`Ok<T>` requires `T` to be destructible");
    static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>,
        "`Ok<T>` requires `T` to be movable or copyable");
    static_assert(!std::same_as<T, Ok<T>>, "`Ok<Ok<T>>` is ill-formed");
    static_assert(!std::same_as<T, std::in_place_t>, "`Ok<T>` must not wrap `std::in_place_t`");

    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Ok);

    template<typename Other>
        requires(!std::same_as<T, std::decay_t<Other>> && std::constructible_from<T, std::decay_t<Other>>)
    constexpr VIOLET_IMPLICIT Ok(Ok<std::decay_t<Other>>&) = delete;

    template<typename Other>
        requires(!std::same_as<T, std::decay_t<Other>> && std::constructible_from<T, std::decay_t<Other>>)
    constexpr VIOLET_IMPLICIT Ok(Ok<Other>&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_value(T(VIOLET_MOVE(other).Value()))
    {
    }

    constexpr VIOLET_EXPLICIT Ok(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : n_value(value)
    {
    }

    constexpr VIOLET_EXPLICIT Ok(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_value(VIOLET_MOVE(value))
    {
    }

    template<typename... Args>
        requires(std::constructible_from<T, Args...>
            && !(sizeof...(Args) == 1 && (std::same_as<std::decay_t<Args>, T> || ...)))
    constexpr VIOLET_EXPLICIT Ok(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : n_value(VIOLET_FWD(Args, args)...)
    {
    }

    constexpr auto Value() & noexcept VIOLET_LIFETIMEBOUND -> T&
    {
        return this->n_value;
    }

    constexpr auto Value() && noexcept VIOLET_LIFETIMEBOUND -> T&&
    {
        return VIOLET_MOVE(this->n_value);
    }

    constexpr auto Value() const& noexcept VIOLET_LIFETIMEBOUND -> const T&
    {
        return this->n_value;
    }

    constexpr auto Value() const&& noexcept VIOLET_LIFETIMEBOUND -> const T&&
    {
        return this->n_value;
    }

    constexpr auto operator==(const Ok& other) const noexcept -> bool
        requires(requires { this->Value() == other.Value(); })
    {
        return this->Value() == other.Value();
    }

    constexpr auto operator!=(const Ok& other) const noexcept -> bool
        requires(requires { this->Value() == other.Value(); })
    {
        return this->Value() != other.Value();
    }

    template<typename E>
    constexpr VIOLET_EXPLICIT operator violet::Result<T, E>() const noexcept
    {
        return Result<T, E>(std::in_place_index<0L>, this->Value());
    }

#if VIOLET_REQUIRE_STL(202302L)
    template<typename E>
    constexpr VIOLET_EXPLICIT operator std::expected<T, E>() const noexcept
    {
        return std::expected<T, E>(this->Value());
    }
#endif

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return violet::ToString(this->Value());
    }

    friend auto operator<<(std::ostream& os, const Ok& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    T n_value;
};

template<typename T, std::size_t N>
Ok(T (&)[N]) -> Ok<const T*>;

template<typename T>
Ok(const T&) -> Ok<std::decay_t<T>>;

template<typename T>
Ok(T&&) -> Ok<std::decay_t<T>>;

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

    using value_type = std::conditional_t<instanceof_v<std::reference_wrapper, T>,
        std::remove_reference_t<std::unwrap_reference_t<T>>, T>;

    using error_type = E;

    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Result);

    /// Constructs an `Ok` variant by copying `ok`.
    template<typename U = T>
        requires(!std::same_as<std::remove_cvref_t<U>, Result<T, E>> && !std::same_as<std::remove_cvref_t<U>, Ok<T>>
            && !std::same_as<std::remove_cvref_t<U>, Err<E>> && std::constructible_from<T, const U&>)
    constexpr VIOLET_IMPLICIT Result(const U& ok) noexcept(std::is_nothrow_constructible_v<T, const U&>)
        : n_ok(true)
    {
        std::construct_at(std::addressof(this->n_storage.Value), ok);
    }

    /// Constructs an `Ok` variant by moving `ok`.
    template<typename U = T>
        requires(!std::same_as<std::remove_cvref_t<U>, Result<T, E>> && !std::same_as<std::remove_cvref_t<U>, Ok<T>>
            && !std::same_as<std::remove_cvref_t<U>, Err<E>> && std::constructible_from<T, U &&>)
    constexpr VIOLET_IMPLICIT Result(U&& ok) noexcept(
        std::is_nothrow_constructible_v<T, U&&> && std::is_nothrow_move_constructible_v<U>)
        : n_ok(true)
    {
        std::construct_at(std::addressof(this->n_storage.Value), VIOLET_FWD(U, ok));
    }

    template<typename U>
        requires(std::is_convertible_v<const U&, T>)
    constexpr VIOLET_IMPLICIT Result(const Ok<U>& ok)
        : Result(ok.Value())
    {
    }

    template<typename U>
        requires(std::is_convertible_v<U &&, T>)
    constexpr VIOLET_IMPLICIT Result(Ok<U>&& ok)
        : Result(VIOLET_MOVE(ok).Value())
    {
    }

    /// Constructs the `Ok` variant in-place.
    ///
    /// Equivalent to `Ok(T(args...))`.
    ///
    /// # Constraints
    /// `T` must be constructible from `Args...`.
    template<typename... Args>
        requires(std::is_constructible_v<T, Args...>)
    constexpr VIOLET_EXPLICIT Result(std::in_place_index_t<0L>, Args&&... args)
        : Result(T(VIOLET_FWD(Args, args)...))
    {
    }

    /// Constructs the `Err` variant in-place.
    ///
    /// Equivalent to `Result(Err(args...))`.
    ///
    /// # Constraints
    /// `E` must be constructible from `Args...`.
    template<typename... Args>
        requires(std::is_constructible_v<E, Args...>)
    constexpr VIOLET_EXPLICIT Result(std::in_place_index_t<1L>, Args&&... args)
        : Result(violet::Err<E>(VIOLET_FWD(Args, args)...))
    {
    }

    /// Constructs the `Err` variant from `violet::Err<E>`.
    constexpr VIOLET_IMPLICIT Result(const violet::Err<E>& err)
    {
        std::construct_at(std::addressof(this->n_storage.Error), err);
    }

    /// Constructs the `Err` variant by move.
    constexpr VIOLET_IMPLICIT Result(violet::Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        std::construct_at(std::addressof(this->n_storage.Error), VIOLET_MOVE(err));
    }

    /// Constructs the `Err` variant from a convertible error type.
    template<typename F>
        requires(!std::same_as<std::remove_cvref_t<F>, E> && std::convertible_to<const F&, E>)
    constexpr VIOLET_IMPLICIT Result(const violet::Err<F>& err)
    {
        std::construct_at(std::addressof(this->n_storage.Error), violet::Err<E>(E(err.Error())));
    }

    template<typename F>
        requires(!std::same_as<std::remove_cvref_t<F>, E> && std::convertible_to<F &&, E>)
    constexpr VIOLET_IMPLICIT Result(violet::Err<F>&& err) noexcept(std::is_nothrow_constructible_v<E, F&&>)
    {
        std::construct_at(std::addressof(this->n_storage.Error), violet::Err<E>(E(VIOLET_MOVE(err).Error())));
    }

    constexpr ~Result()
    {
        this->destroy();
    }

    /// Copy-constructs from another `Result`. Preserves the active variant.
    constexpr VIOLET_IMPLICIT Result(const Result& other)
        : n_ok(other.n_ok)
    {
        if (this->n_ok) {
            std::construct_at(std::addressof(this->n_storage.Value), other.n_storage.Value);
        } else {
            std::construct_at(std::addressof(this->n_storage.Error), other.n_storage.Error);
        }
    }

    constexpr auto operator=(const Result& other) -> Result&
    {
        if (this != &other) {
            this->destroy();
            this->n_ok = other.n_ok;

            if (this->n_ok) {
                std::construct_at(std::addressof(this->n_storage.Value), other.n_storage.Value);
            } else {
                std::construct_at(std::addressof(this->n_storage.Error), other.n_storage.Error);
            }
        }

        return *this;
    }

    /// Move-constructs from another `Result`. Preserves the variant and transfers ownership.
    constexpr VIOLET_IMPLICIT Result(Result&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>)
        : n_ok(other.n_ok)
    {
        if (this->n_ok) {
            std::construct_at(std::addressof(this->n_storage.Value), VIOLET_MOVE(other.n_storage.Value));
        } else {
            std::construct_at(std::addressof(this->n_storage.Error), VIOLET_MOVE(other.n_storage.Error));
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
                    std::construct_at(std::addressof(this->getValueRef()), VIOLET_MOVE(other.n_storage.Value));
                } else {
                    std::construct_at(std::addressof(this->n_storage.Error), VIOLET_MOVE(other.n_storage.Error));
                }
            }
        }

        return *this;
    }

#if VIOLET_REQUIRE_STL(202302L)
    template<typename U, typename E2>
        requires(!std::same_as<std::remove_cvref_t<U>, Result> && std::constructible_from<T, U>
            && std::constructible_from<E, E2>)
    constexpr VIOLET_IMPLICIT Result(const std::expected<U, E2>& other)
    {
        if (other.has_value()) {
            std::construct_at(&this->n_storage.Value, other.value());
            this->n_ok = true;
        } else {
            std::construct_at(&this->n_storage.Error, violet::Err<E2>(other.error()));
            this->n_ok = false;
        }
    }

    template<typename U, typename E2>
        requires(!std::same_as<std::remove_cvref_t<U>, Result> && std::constructible_from<T, U>
            && std::constructible_from<E, E2>)
    constexpr VIOLET_IMPLICIT Result(std::expected<U, E2>&& other)
    {
        if (other.has_value()) {
            std::construct_at(&this->n_storage.Value, VIOLET_MOVE(other).value());
            this->n_ok = true;
        } else {
            std::construct_at(&this->n_storage.Error, violet::Err<E2>(VIOLET_MOVE(other).error()));
            this->n_ok = false;
        }
    }

    constexpr auto operator=(std::expected<T, E>& other) -> Result&
    {
        this->destroy();
        if (other.has_value()) {
            std::construct_at(&this->n_storage.Value, other.value());
            this->n_ok = true;
        } else {
            std::construct_at(&this->n_storage.Error, violet::Err<E>(other.error()));
            this->n_ok = false;
        }

        return *this;
    }

    constexpr auto operator=(std::expected<T, E>&& other) -> Result&
    {
        this->destroy();
        if (other.has_value()) {
            std::construct_at(&this->n_storage.Value, VIOLET_MOVE(other).value());
            this->n_ok = true;
        } else {
            std::construct_at(&this->n_storage.Error, violet::Err<E>(VIOLET_MOVE(other).error()));
            this->n_ok = false;
        }

        return *this;
    }
#endif

    /// Returns `true` if this is the `Ok` variant.
    [[nodiscard]] constexpr auto Ok() const noexcept -> bool
    {
        return this->n_ok;
    }

    /// Returns `true` if this is the `Ok` variant.
    [[nodiscard]] constexpr auto Ok() noexcept -> bool
    {
        return this->n_ok;
    }

    /// Returns `true` if this is the `Err` variant.
    [[nodiscard]] constexpr auto Err() const noexcept -> bool
    {
        return !this->Ok();
    }

    /// Returns `true` if this is the `Err` variant.
    [[nodiscard]] constexpr auto Err() noexcept -> bool
    {
        return !this->Ok();
    }

    /// Returns a reference to the contained value.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if this result is the `Ok` variant.
    [[nodiscard]] constexpr auto Value() & noexcept VIOLET_LIFETIMEBOUND -> value_type&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "`Result<T, E>` invariant reached");
        return this->getValueRef();
    }

    /// Returns a reference to the contained value.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if this result is the `Ok` variant.
    [[nodiscard]] constexpr auto Value() const& noexcept VIOLET_LIFETIMEBOUND -> const value_type&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "`Result<T, E>` invariant reached");
        return this->getValueRef();
    }

    /// Returns a reference to the contained value.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if this result is the `Ok` variant.
    [[nodiscard]] constexpr auto Value() && noexcept VIOLET_LIFETIMEBOUND -> value_type&&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "`Result<T, E>` invariant reached");
        return VIOLET_MOVE(this->getValueRef());
    }

    /// Returns a reference to the contained value.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if this result is the `Ok` variant.
    [[nodiscard]] constexpr auto Value() const&& noexcept VIOLET_LIFETIMEBOUND -> const value_type&&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "`Result<T, E>` invariant reached");
        return VIOLET_MOVE(this->getValueRef());
    }

    /// Returns a reference to the contained error.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if this result is the `Err` variant.
    [[nodiscard]] constexpr auto Error() & noexcept VIOLET_LIFETIMEBOUND -> error_type&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` invariant reached");
        return this->getErrorRef();
    }

    /// Returns a reference to the contained error.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if this result is the `Err` variant.
    [[nodiscard]] constexpr auto Error() const& noexcept VIOLET_LIFETIMEBOUND -> const error_type&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` invariant reached");
        return this->getErrorRef();
    }

    /// Returns a reference to the contained error.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if this result is the `Err` variant.
    [[nodiscard]] constexpr auto Error() const&& noexcept VIOLET_LIFETIMEBOUND -> const error_type&&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` invariant reached");
        return VIOLET_MOVE(this->getErrorRef());
    }

    /// Returns a reference to the contained error.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if this result is the `Err` variant.
    [[nodiscard]] constexpr auto Error() && noexcept VIOLET_LIFETIMEBOUND -> error_type&&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<T, E>` invariant reached");
        return VIOLET_MOVE(this->getErrorRef());
    }

    /// Converts `Result<Result<U, E>, E>` into `Result<U, E>`.
    ///
    /// Equivalent to Rust's
    /// [`Result::transpose`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.transpose).
    template<typename U = value_type, typename E2 = error_type>
        requires(nested_result<Result<U, E2>, E2> && std::convertible_to<value_type, U>
            && std::convertible_to<E2, error_type>)
    [[nodiscard]] constexpr auto Transpose() && -> Result<result_value_type_t<U>, E2>
    {
        if (this->Ok()) {
            return violet::Ok<result_value_type_t<U>>(VIOLET_MOVE(this->getValueRef()).Value());
        }

        return violet::Err<E2>(VIOLET_MOVE(this->Error()));
    }

    /// Applies `fun` to the contained `Ok` value and returns the result. If this is `Err`, propagates the error
    /// unchanged.
    ///
    /// Equivalent to Rust's
    /// [`Result::and_then`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.and_then).
    template<typename Fun>
        requires(callable<Fun, value_type &&>)
    [[nodiscard]] constexpr auto AndThen(Fun&& fun) && -> Result<std::invoke_result_t<Fun, T>, E>
    {
        using Ret = std::invoke_result_t<Fun, T>;
        if (this->Ok()) {
            return violet::Ok<Ret>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(Value())));
        }

        return violet::Err<E>(VIOLET_MOVE(Error()));
    }

    /// Applies `fun` to the contained `Ok` value and returns the result. If this is `Err`, propagates the error
    /// unchanged.
    ///
    /// Equivalent to Rust's
    /// [`Result::and_then`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.and_then).
    template<typename Fun>
        requires(callable<Fun, const value_type &&>)
    [[nodiscard]] constexpr auto AndThen(Fun&& fun) const&& -> Result<std::invoke_result_t<Fun, T>, E>
    {
        using Ret = std::invoke_result_t<Fun, T>;
        if (this->Ok()) {
            return violet::Ok<Ret>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(Value())));
        }

        return violet::Err<E>(VIOLET_MOVE(Error()));
    }

    /// Invokes `fun` with the contained `Ok` value (if present) and returns `*this` unchanged.
    template<typename Fun>
        requires(callable<Fun, const value_type&> && callable_returns<Fun, void, const value_type&>)
    [[nodiscard]] constexpr auto Inspect(Fun&& fun) noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<T>()))) -> Result&
    {
        if (this->Ok()) {
            std::invoke(VIOLET_FWD(Fun, fun), Value());
        }

        return *this;
    }

    /// Invokes `fun` with the contained `Err` value (if present) and returns `*this` unchanged.
    template<typename Fun>
        requires(callable<Fun> && callable_returns<Fun, void, const E&>)
    [[nodiscard]] constexpr auto InspectErr(Fun&& fun) noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<E>()))) -> Result&
    {
        if (this->Ok()) {
            std::invoke(VIOLET_FWD(Fun, fun), Error());
        }

        return *this;
    }

    /// Returns `true` if `Ok` and predicate returns `true`.
    template<typename Pred>
        requires(callable<Pred, const value_type&> && callable_returns<Pred, bool, const value_type&>)
    [[nodiscard]] constexpr auto OkAnd(Pred&& pred) const
        noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<const value_type&>()))) -> bool
    {
        return this->Ok() && std::invoke(VIOLET_FWD(Pred, pred), Value());
    }

    /// Returns `true` if `Err` and predicate returns `true`.
    template<typename Pred>
        requires(callable<Pred, const error_type&> && callable_returns<Pred, bool, const error_type&>)
    [[nodiscard]] constexpr auto ErrAnd(Pred&& pred) const
        noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<const error_type&>()))) -> bool
    {
        return this->Err() && std::invoke(VIOLET_FWD(Pred, pred), Error());
    }

    /// Applies `fun` to the contained value if present.
    ///
    /// Equivalent to Rust's [`Result::map`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map)
    ///
    /// @returns `Ok(fun(value))` if this container is engaged, `*this` otherwise.
    template<typename Fun>
        requires(callable<Fun, value_type&>)
    [[nodiscard]] constexpr auto Map(Fun&& fun) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&>())))
        -> Result<std::invoke_result_t<Fun, value_type&>, E>
    {
        using return_type = std::invoke_result_t<Fun, value_type&>;
        if (this->Ok()) {
            return violet::Ok<return_type>(std::invoke(VIOLET_FWD(Fun, fun), this->getValueRef()));
        }

        return violet::Err<error_type>(this->getErrorRef());
    }

    /// Applies `fun` to the contained value if present.
    ///
    /// Equivalent to Rust's [`Result::map`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map)
    ///
    /// @returns `Ok(fun(value))` if this container is engaged, `*this` otherwise.
    template<typename Fun>
        requires(callable<Fun, const value_type&>)
    [[nodiscard]] constexpr auto Map(Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&>())))
        -> Result<std::invoke_result_t<Fun, const value_type&>, E>
    {
        using return_type = std::invoke_result_t<Fun, const value_type&>;
        if (this->Ok()) {
            return violet::Ok<return_type>(std::invoke(VIOLET_FWD(Fun, fun), this->getValueRef()));
        }

        return violet::Err<error_type>(this->getErrorRef());
    }

    /// Applies `fun` to the contained value if present.
    ///
    /// Equivalent to Rust's [`Result::map`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map)
    ///
    /// @returns `Ok(fun(value))` if this container is engaged, `*this` otherwise.
    template<typename Fun>
        requires(callable<Fun, value_type &&>)
    [[nodiscard]] constexpr auto Map(Fun&& fun) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&&>())))
        -> Result<std::invoke_result_t<Fun, value_type&&>, E>
    {
        using return_type = std::invoke_result_t<Fun, value_type&&>;
        if (this->Ok()) {
            return violet::Ok<return_type>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->getValueRef())));
        }

        return violet::Err<error_type>(VIOLET_MOVE(this->getErrorRef()));
    }

    /// Applies `fun` to the contained value if present.
    ///
    /// Equivalent to Rust's [`Result::map`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map)
    ///
    /// @returns `Ok(fun(value))` if this container is engaged, `*this` otherwise.
    template<typename Fun>
        requires(callable<Fun, const value_type &&>)
    [[nodiscard]] constexpr auto Map(Fun&& fun) const&& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&&>())))
        -> Result<std::invoke_result_t<Fun, const value_type&&>, E>
    {
        using return_type = std::invoke_result_t<Fun, const value_type&&>;
        if (this->Ok()) {
            return violet::Ok<return_type>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->getValueRef())));
        }

        return violet::Err<error_type>(VIOLET_MOVE(this->getErrorRef()));
    }

    /// Applies `fun` to the contained error if present.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_err`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_err)
    ///
    /// @returns `Err(fun(error))` if this container is not engaged, `*this` otherwise.
    template<typename Fun>
        requires(callable<Fun, error_type&>)
    [[nodiscard]] constexpr auto MapErr(Fun&& fun) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<error_type&>())))
        -> Result<value_type, std::invoke_result_t<Fun, error_type&>>
    {
        using return_type = std::invoke_result_t<Fun, error_type&>;
        if (this->Err()) {
            return violet::Err<return_type>(std::invoke(VIOLET_FWD(Fun, fun), this->getErrorRef()));
        }

        return violet::Ok<value_type>(this->getValueRef());
    }

    /// Applies `fun` to the contained error if present.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_err`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_err)
    ///
    /// @returns `Err(fun(error))` if this container is not engaged, `*this` otherwise.
    template<typename Fun>
        requires(callable<Fun, const error_type&>)
    [[nodiscard]] constexpr auto MapErr(Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const error_type&>())))
        -> Result<value_type, std::invoke_result_t<Fun, const error_type&>>
    {
        using return_type = std::invoke_result_t<Fun, error_type&>;
        if (this->Err()) {
            return violet::Err<return_type>(std::invoke(VIOLET_FWD(Fun, fun), this->getErrorRef()));
        }

        return violet::Ok<value_type>(this->getValueRef());
    }

    /// Applies `fun` to the contained error if present.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_err`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_err)
    ///
    /// @returns `Err(fun(error))` if this container is not engaged, `*this` otherwise.
    template<typename Fun>
        requires(callable<Fun, error_type &&>)
    [[nodiscard]] constexpr auto MapErr(Fun&& fun) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<error_type&&>())))
        -> Result<value_type, std::invoke_result_t<Fun, error_type&&>>
    {
        using return_type = std::invoke_result_t<Fun, error_type&&>;
        if (this->Err()) {
            return violet::Err<return_type>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->getErrorRef())));
        }

        return violet::Ok<value_type>(VIOLET_MOVE(this->getValueRef()));
    }

    /// Applies `fun` to the contained error if present.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_err`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_err)
    ///
    /// @returns `Err(fun(error))` if this container is not engaged, `*this` otherwise.
    template<typename Fun>
        requires(callable<Fun, const error_type &&>)
    [[nodiscard]] constexpr auto MapErr(Fun&& fun) const&& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const error_type&&>())))
        -> Result<value_type, std::invoke_result_t<Fun, const error_type&&>>
    {
        using return_type = std::invoke_result_t<Fun, const error_type&&>;
        if (this->Err()) {
            return violet::Err<return_type>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->getErrorRef())));
        }

        return violet::Ok<value_type>(VIOLET_MOVE(this->getValueRef()));
    }

    /// Applies `fun` to the contained value if present, otherwise returns `defaultValue`.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_or`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or)
    template<typename U, typename Fun>
        requires(callable<Fun, value_type&> && std::convertible_to<std::invoke_result_t<Fun, value_type&>, U>)
    constexpr auto MapOr(U&& defaultValue, Fun&& fun) & -> U
    {
        if (this->Ok()) {
            return std::invoke(VIOLET_FWD(Fun, fun), this->getValueRef());
        }

        return VIOLET_FWD(U, defaultValue);
    }

    /// Applies `fun` to the contained value if present, otherwise returns `defaultValue`.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_or`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or)
    template<typename U, typename Fun>
        requires(
            callable<Fun, const value_type&> && std::convertible_to<std::invoke_result_t<Fun, const value_type&>, U>)
    constexpr auto MapOr(U&& defaultValue, Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&>()))) -> U
    {
        if (this->Ok()) {
            return std::invoke(VIOLET_FWD(Fun, fun), this->getValueRef());
        }

        return VIOLET_FWD(U, defaultValue);
    }

    /// Applies `fun` to the contained value if present, otherwise returns `defaultValue`.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_or`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or)
    template<typename U, typename Fun>
        requires(callable<Fun, value_type &&> && std::convertible_to<std::invoke_result_t<Fun, value_type &&>, U>)
    constexpr auto MapOr(U&& defaultValue, Fun&& fun) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&&>()))) -> U
    {
        if (this->Ok()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->getValueRef()));
        }

        return VIOLET_FWD(U, defaultValue);
    }

    /// Applies `fun` to the contained value if present, otherwise returns `defaultValue`.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_or`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or)
    template<typename U, typename Fun>
        requires(callable<Fun, const value_type &&>
            && std::convertible_to<std::invoke_result_t<Fun, const value_type &&>, U>)
    constexpr auto MapOr(U&& defaultValue, Fun&& fun) const&& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&&>()))) -> U
    {
        if (this->Ok()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->getValueRef()));
        }

        return VIOLET_FWD(U, defaultValue);
    }

    /// Applies `fun` to the contained value if present, otherwise calls `defaultValue`
    /// with the error that is contained instead.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_or_else`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or_else)
    template<typename U, typename Default, typename Fun>
        requires(callable<Fun, value_type&> && callable<Default, error_type&> && callable_returns<Fun, U, value_type&>
            && callable_returns<Default, U, error_type&>)
    constexpr auto MapOrElse(Default&& defaultValue, Fun&& fun) & -> U
    {
        if (this->Ok()) {
            return std::invoke(VIOLET_FWD(Fun, fun), this->getValueRef());
        }

        return std::invoke(VIOLET_FWD(Default, defaultValue), this->getErrorRef());
    }

    /// Applies `fun` to the contained value if present, otherwise calls `defaultValue`
    /// with the error that is contained instead.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_or_else`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or_else)
    template<typename U, typename Default, typename Fun>
        requires(callable<Fun, const value_type&> && callable<Default, const error_type&>
            && callable_returns<Fun, U, const value_type&> && callable_returns<Default, U, const error_type&>)
    constexpr auto MapOrElse(Default&& defaultValue, Fun&& fun) const& -> U
    {
        if (this->Ok()) {
            return std::invoke(VIOLET_FWD(Fun, fun), this->getValueRef());
        }

        return std::invoke(VIOLET_FWD(Default, defaultValue), this->getErrorRef());
    }

    /// Applies `fun` to the contained value if present, otherwise calls `defaultValue`
    /// with the error that is contained instead.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_or_else`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or_else)
    template<typename U, typename Default, typename Fun>
        requires(callable<Fun, value_type &&> && callable<Default, error_type &&>
            && callable_returns<Fun, U, value_type &&> && callable_returns<Default, U, error_type &&>)
    constexpr auto MapOrElse(Default&& defaultValue, Fun&& fun) && -> U
    {
        if (this->Ok()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->getValueRef()));
        }

        return std::invoke(VIOLET_FWD(Default, defaultValue), VIOLET_MOVE(this->getErrorRef()));
    }

    /// Applies `fun` to the contained value if present, otherwise calls `defaultValue`
    /// with the error that is contained instead.
    ///
    /// Equivalent to Rust's
    /// [`Result::map_or_else`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or_else)
    template<typename U, typename Default, typename Fun>
        requires(callable<Fun, const value_type &&> && callable<Default, const error_type &&>
            && callable_returns<Fun, U, const value_type &&> && callable_returns<Default, U, const error_type &&>)
    constexpr auto MapOrElse(Default&& defaultValue, Fun&& fun) const&& -> U
    {
        if (this->Ok()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->getValueRef()));
        }

        return std::invoke(VIOLET_FWD(Default, defaultValue), VIOLET_MOVE(this->getErrorRef()));
    }

    /// Applies `fun` to the contained value if present, otherwise returns a default
    /// constructible instance of `U`.
    ///
    /// Equivalent to Rust's
    /// [`Option::map_or_default`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or_default)
    template<typename U, typename Fun>
        requires(violet::callable<Fun, value_type&> && std::is_default_constructible_v<U>
            && std::convertible_to<std::invoke_result_t<Fun, value_type&>, U>)
    constexpr auto MapOrDefault(Fun&& fun) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&>()))) -> U
    {
        return this->MapOr(U{ }, VIOLET_FWD(Fun, fun));
    }

    /// Applies `fun` to the contained value if present, otherwise returns a default
    /// constructible instance of `U`.
    ///
    /// Equivalent to Rust's
    /// [`Option::map_or_default`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or_default)
    template<typename U, typename Fun>
        requires(violet::callable<Fun, const value_type&> && std::is_default_constructible_v<U>
            && std::convertible_to<std::invoke_result_t<Fun, const value_type&>, U>)
    constexpr auto MapOrDefault(Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&>()))) -> U
    {
        return this->MapOr(U{ }, VIOLET_FWD(Fun, fun));
    }

    /// Applies `fun` to the contained value if present, otherwise returns a default
    /// constructible instance of `U`.
    ///
    /// Equivalent to Rust's
    /// [`Option::map_or_default`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or_default)
    template<typename U, typename Fun>
        requires(violet::callable<Fun, value_type &&> && std::is_default_constructible_v<U>
            && std::convertible_to<std::invoke_result_t<Fun, value_type &&>, U>)
    constexpr auto MapOrDefault(Fun&& fun) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&&>()))) -> U
    {
        return this->MapOr(U{ }, VIOLET_FWD(Fun, fun));
    }

    /// Applies `fun` to the contained value if present, otherwise returns a default
    /// constructible instance of `U`.
    ///
    /// Equivalent to Rust's
    /// [`Option::map_or_default`](https://doc.rust-lang.org/1.93.0/std/result/enum.Result.html#method.map_or_default)
    template<typename U, typename Fun>
        requires(violet::callable<Fun, const value_type &&> && std::is_default_constructible_v<U>
            && std::convertible_to<std::invoke_result_t<Fun, const value_type &&>, U>)
    constexpr auto MapOrDefault(Fun&& fun) const&& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&&>()))) -> U
    {
        return this->MapOr(U{ }, VIOLET_FWD(Fun, fun));
    }

    /// Forefully retrieve the `Ok` variant's value or panics if no value was present.
    ///
    /// This will also enforce a fast, hot path if this container is in its `Ok` variant.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto Unwrap(violet::SourceLocation loc = std::source_location::current()) & -> value_type
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return this->getValueRef();
        }

        VIOLET_PANIC_USERLAND("tried to `Unwrap()` a error variant", loc);
    }

    /// Forefully retrieve the `Ok` variant's value or panics if no value was present.
    ///
    /// This will also enforce a fast, hot path if this container is in its `Ok` variant.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto Unwrap(violet::SourceLocation loc = std::source_location::current()) && -> value_type
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->getValueRef());
        }

        VIOLET_PANIC_USERLAND("tried to `Unwrap()` a error variant", loc);
    }

    /// Forefully retrieve the `Ok` variant's value or panics if no value was present.
    ///
    /// This will also enforce a fast, hot path if this container is in its `Ok` variant.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto Unwrap(violet::SourceLocation loc = std::source_location::current()) const& -> value_type
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return this->getValueRef();
        }

        VIOLET_PANIC_USERLAND("tried to `Unwrap()` a error variant", loc);
    }

    /// Forefully retrieve the `Ok` variant's value or panics if no value was present.
    ///
    /// This will also enforce a fast, hot path if this container is in its `Ok` variant.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto Unwrap(violet::SourceLocation loc = std::source_location::current()) const&& -> value_type
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->getValueRef());
        }

        VIOLET_PANIC_USERLAND("tried to `Unwrap()` a error variant", loc);
    }

    /// Forefully retrieve this container's `Err` variant or panics if no error was present.
    ///
    /// This will also enforce a fast, hot path if this container is in the `Err` variant.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto UnwrapErr(violet::SourceLocation loc = std::source_location::current()) & -> error_type
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return this->getErrorRef();
        }

        VIOLET_PANIC_USERLAND("tried to `Unwrap()` a ok variant", loc);
    }

    /// Forefully retrieve this container's `Err` variant or panics if no error was present.
    ///
    /// This will also enforce a fast, hot path if this container is in the `Err` variant.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto UnwrapErr(violet::SourceLocation loc = std::source_location::current()) && -> error_type
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return VIOLET_MOVE(this->getErrorRef());
        }

        VIOLET_PANIC_USERLAND("tried to `Unwrap()` a ok variant", loc);
    }

    /// Forefully retrieve this container's `Err` variant or panics if no error was present.
    ///
    /// This will also enforce a fast, hot path if this container is in the `Err` variant.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto UnwrapErr(violet::SourceLocation loc = std::source_location::current()) const& -> error_type
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return this->getErrorRef();
        }

        VIOLET_PANIC_USERLAND("tried to `Unwrap()` a ok variant", loc);
    }

    /// Forefully retrieve this container's `Err` variant or panics if no error was present.
    ///
    /// This will also enforce a fast, hot path if this container is in the `Err` variant.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto UnwrapErr(violet::SourceLocation loc = std::source_location::current()) const&& -> error_type
    {
        VIOLET_LIKELY_IF(this->Err())
        {
            return VIOLET_MOVE(this->getErrorRef());
        }

        VIOLET_PANIC_USERLAND("tried to `Unwrap()` a ok variant", loc);
    }

    /// Returns the contained value or panics with `message` if no value is present.
    ///
    /// This will also enforce a fast, hot path if this container is engaged.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto Except(Str message, violet::SourceLocation loc = std::source_location::current()) & -> value_type
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return this->getValueRef();
        }

        VIOLET_PANIC_USERLAND(message, loc);
    }

    /// Returns the contained value or panics with `message` if no value is present.
    ///
    /// This will also enforce a fast, hot path if this container is engaged.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto Except(Str message, violet::SourceLocation loc = std::source_location::current()) && -> value_type
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->getValueRef());
        }

        VIOLET_PANIC_USERLAND(message, loc);
    }

    /// Returns the contained value or panics with `message` if no value is present.
    ///
    /// This will also enforce a fast, hot path if this container is engaged.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto Except(
        Str message, violet::SourceLocation loc = std::source_location::current()) const& -> value_type
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return this->getValueRef();
        }

        VIOLET_PANIC_USERLAND(message, loc);
    }

    /// Returns the contained value or panics with `message` if no value is present.
    ///
    /// This will also enforce a fast, hot path if this container is engaged.
    ///
    /// ## Panics
    /// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
    /// on if it is enabled or not.
    ///
    /// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
    /// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
    /// instrinstics if [`std::unreachable`] isn't available.
    ///
    /// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
    constexpr auto Except(
        Str message, violet::SourceLocation loc = std::source_location::current()) const&& -> value_type
    {
        VIOLET_LIKELY_IF(this->Ok())
        {
            return VIOLET_MOVE(this->getValueRef());
        }

        VIOLET_PANIC_USERLAND(message, loc);
    }

    /// Returns the contained value if present, otherwise returns `defaultValue`.
    [[nodiscard]] constexpr auto UnwrapOr(T&& defaultValue) & noexcept -> value_type
    {
        return this->Ok() ? this->getValueRef() : VIOLET_MOVE(defaultValue);
    }

    /// Returns the contained value if present, otherwise returns `defaultValue`.
    [[nodiscard]] constexpr auto UnwrapOr(T&& defaultValue) && noexcept -> value_type
    {
        return this->Ok() ? VIOLET_MOVE(this->getValueRef()) : VIOLET_MOVE(defaultValue);
    }

    /// Returns the contained value if present, otherwise returns `defaultValue`.
    [[nodiscard]] constexpr auto UnwrapOr(T&& defaultValue) const& noexcept -> value_type
    {
        return this->Ok() ? this->getValueRef() : VIOLET_MOVE(defaultValue);
    }

    /// Returns the contained value if present, otherwise returns `defaultValue`.
    [[nodiscard]] constexpr auto UnwrapOr(T&& defaultValue) const&& noexcept -> value_type
    {
        return this->Ok() ? VIOLET_MOVE(this->getValueRef()) : VIOLET_MOVE(defaultValue);
    }

    /// Returns the contained value if it present, otherwise a default constructed `T` is used.
    constexpr auto UnwrapOrDefault() & noexcept -> value_type
        requires(std::is_default_constructible_v<T>)
    {
        return this->Ok() ? this->getValueRef() : T{ };
    }

    /// Returns the contained value if it present, otherwise a default constructed `T` is used.
    constexpr auto UnwrapOrDefault() const& noexcept -> value_type
        requires(std::is_default_constructible_v<T>)
    {
        return this->Ok() ? this->getValueRef() : T{ };
    }

    /// Returns the contained value if it present, otherwise a default constructed `T` is used.
    constexpr auto UnwrapOrDefault() && noexcept -> value_type
        requires(std::is_default_constructible_v<T>)
    {
        return this->Ok() ? VIOLET_MOVE(this->getValueRef()) : T{ };
    }

    /// Returns the contained value if it present, otherwise a default constructed `T` is used.
    constexpr auto UnwrapOrDefault() const&& noexcept -> value_type
        requires(std::is_default_constructible_v<T>)
    {
        return this->Ok() ? VIOLET_MOVE(this->getValueRef()) : T{ };
    }

    /// Returns the contained value without checking its state.
    ///
    /// # Safety
    /// Undefined behavior if no value is present.
    [[nodiscard]] constexpr auto UnwrapUnchecked(Unsafe) & noexcept VIOLET_LIFETIMEBOUND -> value_type&
    {
        return this->getValueRef();
    }

    /// Returns the contained value without checking its state.
    ///
    /// # Safety
    /// Undefined behavior if no value is present.
    [[nodiscard]] constexpr auto UnwrapUnchecked(Unsafe) const& noexcept VIOLET_LIFETIMEBOUND -> const value_type&
    {
        return this->getValueRef();
    }

    /// Returns the contained value without checking its state.
    ///
    /// # Safety
    /// Undefined behavior if no value is present.
    [[nodiscard]] constexpr auto UnwrapUnchecked(Unsafe) && noexcept VIOLET_LIFETIMEBOUND -> value_type&&
    {
        return VIOLET_MOVE(this->getValueRef());
    }

    /// Returns the contained value without checking its state.
    ///
    /// # Safety
    /// Undefined behavior if no value is present.
    [[nodiscard]] constexpr auto UnwrapUnchecked(Unsafe) const&& noexcept VIOLET_LIFETIMEBOUND -> const value_type&&
    {
        return VIOLET_MOVE(this->getValueRef());
    }

    /// Returns the contained error without checking its state.
    ///
    /// # Safety
    /// Undefined behavior if no error is present.
    [[nodiscard]] constexpr auto UnwrapErrUnchecked(Unsafe) & noexcept VIOLET_LIFETIMEBOUND -> error_type&
    {
        return this->getErrorRef();
    }

    /// Returns the contained error without checking its state.
    ///
    /// # Safety
    /// Undefined behavior if no error is present.
    [[nodiscard]] constexpr auto UnwrapErrUnchecked(Unsafe) const& noexcept VIOLET_LIFETIMEBOUND -> const error_type&
    {
        return this->getErrorRef();
    }

    /// Returns the contained error without checking its state.
    ///
    /// # Safety
    /// Undefined behavior if no error is present.
    [[nodiscard]] constexpr auto UnwrapErrUnchecked(Unsafe) && noexcept VIOLET_LIFETIMEBOUND -> error_type&&
    {
        return VIOLET_MOVE(this->getErrorRef());
    }

    /// Returns the contained error without checking its state.
    ///
    /// # Safety
    /// Undefined behavior if no error is present.
    [[nodiscard]] constexpr auto UnwrapErrUnchecked(Unsafe) const&& noexcept VIOLET_LIFETIMEBOUND -> const error_type&&
    {
        return VIOLET_MOVE(this->getErrorRef());
    }

    constexpr auto operator*() & VIOLET_LIFETIMEBOUND->value_type&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Dereferencing a Result in Err state");
        return this->getValueRef();
    }

    constexpr auto operator*() && VIOLET_LIFETIMEBOUND->value_type&&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Dereferencing a Result in Err state");
        return VIOLET_MOVE(this->getValueRef());
    }

    constexpr auto operator*() const & VIOLET_LIFETIMEBOUND->const value_type&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Dereferencing a Result in Err state");
        return this->getValueRef();
    }

    constexpr auto operator*() const && VIOLET_LIFETIMEBOUND->const value_type&&
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Dereferencing a Result in Err state");
        return VIOLET_MOVE(this->getValueRef());
    }

    constexpr auto operator->() -> value_type*
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Accessing a Result in Err state");
        return std::addressof(this->getValueRef());
    }

    constexpr auto operator->() const -> const value_type*
    {
        VIOLET_DEBUG_ASSERT(this->Ok(), "Accessing a Result in Err state");
        return std::addressof(this->getValueRef());
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

    friend auto operator<<(std::ostream& os, const Result<T, E>& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    bool n_ok = false;

    // NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
    union storage_t {
        T Value;
        violet::Err<E> Error;

        constexpr storage_t() noexcept { }
        constexpr ~storage_t() { }
    } n_storage;

    constexpr void destroy() noexcept(std::is_nothrow_destructible_v<T> && std::is_nothrow_destructible_v<E>)
    {
        if (this->n_ok) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                this->n_storage.Value.~T();
            }

            this->n_ok = false;
        } else {
            if constexpr (!std::is_trivially_destructible_v<E>) {
                this->n_storage.Error.~Err<E>();
            }
        }
    }

    constexpr auto getValueRef() & noexcept -> value_type&
    {
        if VIOLET_IF_CONSTEVAL {
            if constexpr (instanceof_v<std::reference_wrapper, T>) {
                return this->n_storage.Value.get();
            } else {
                return this->n_storage.Value;
            }
        } else {
            if constexpr (instanceof_v<std::reference_wrapper, T>) {
                return std::launder(reinterpret_cast<T*>(&this->n_storage.Value))->get();
            }

            return *std::launder(reinterpret_cast<T*>(&this->n_storage.Value));
        }
    }

    constexpr auto getValueRef() const& noexcept -> const value_type&
    {
        if VIOLET_IF_CONSTEVAL {
            if constexpr (instanceof_v<std::reference_wrapper, T>) {
                return this->n_storage.Value.get();
            } else {
                return this->n_storage.Value;
            }
        } else {
            if constexpr (instanceof_v<std::reference_wrapper, T>) {
                return std::launder(reinterpret_cast<const T*>(&this->n_storage.Value))->get();
            }

            return *std::launder(reinterpret_cast<const T*>(&this->n_storage.Value));
        }
    }

    constexpr auto getValueRef() && noexcept -> value_type&&
    {
        if VIOLET_IF_CONSTEVAL {
            if constexpr (instanceof_v<std::reference_wrapper, T>) {
                return VIOLET_MOVE(this->n_storage.Value).get();
            } else {
                return VIOLET_MOVE(this->n_storage.Value);
            }
        } else {
            if constexpr (instanceof_v<std::reference_wrapper, T>) {
                return VIOLET_MOVE(std::launder(reinterpret_cast<T*>(&this->n_storage.Value)))->get();
            }

            return VIOLET_MOVE(*std::launder(reinterpret_cast<T*>(&this->n_storage.Value)));
        }
    }

    constexpr auto getValueRef() const&& noexcept -> const value_type&&
    {
        if VIOLET_IF_CONSTEVAL {
            if constexpr (instanceof_v<std::reference_wrapper, T>) {
                return VIOLET_MOVE(this->n_storage.Value).get();
            } else {
                return VIOLET_MOVE(this->n_storage.Value);
            }
        } else {
            if constexpr (instanceof_v<std::reference_wrapper, T>) {
                return VIOLET_MOVE(std::launder(reinterpret_cast<const T*>(&this->n_storage.Value)))->get();
            }

            return VIOLET_MOVE(*std::launder(reinterpret_cast<const T*>(&this->n_storage.Value)));
        }
    }

    constexpr auto getErrorRef() & noexcept -> error_type&
    {
        if VIOLET_IF_CONSTEVAL {
            return this->n_storage.Error.Error();
        } else {
            return std::launder(std::addressof(this->n_storage.Error))->Error();
        }
    }

    constexpr auto getErrorRef() const& noexcept -> const error_type&
    {
        if VIOLET_IF_CONSTEVAL {
            return this->n_storage.Error.Error();
        } else {
            return std::launder(std::addressof(this->n_storage.Error))->Error();
        }
    }

    constexpr auto getErrorRef() && noexcept -> error_type&&
    {
        if VIOLET_IF_CONSTEVAL {
            return VIOLET_MOVE(this->n_storage.Error.Error());
        } else {
            return std::launder(std::addressof(this->n_storage.Error))->Error();
        }
    }

    constexpr auto getErrorRef() const&& noexcept -> const error_type&&
    {
        if VIOLET_IF_CONSTEVAL {
            return VIOLET_MOVE(this->n_storage.Error.Error());
        } else {
            return std::launder(std::addressof(this->n_storage.Error))->Error();
        }
    }
};

/// Specialization of `Result` for `void` success type.
///
/// Represents:
/// - success with no value
/// - failure with error `E`
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

    /// Constructs a successful `Result<void, E>`.
    constexpr VIOLET_IMPLICIT Result() noexcept = default;

    /// Constructs the `Err` variant.
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

    constexpr ~Result()
    {
        if (this->n_value != nullptr) {
            delete this->n_value;
            this->n_value = nullptr;
        }
    }

    constexpr VIOLET_IMPLICIT Result(const Result& other)
    {
        if (other.n_value != nullptr) {
            this->n_value = new violet::Err<E>(*other.n_value);
        }
    }

    constexpr auto operator=(const Result& other) noexcept -> Result&
    {
        if (this != &other) {
            if (other.n_value != nullptr) {
                this->n_value = new violet::Err<E>(*other.n_value);
            }
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
            if (this->n_value != nullptr) {
                delete this->n_value;
                this->n_value = nullptr;
            }

            this->n_value = std::exchange(other.n_value, nullptr);
        }

        return *this;
    }

#if VIOLET_REQUIRE_STL(202302L)
    template<typename E2>
        requires(!std::same_as<std::remove_cvref_t<E2>, Result<void, E2>> && std::constructible_from<E, E2>)
    constexpr VIOLET_IMPLICIT Result(const std::expected<void, E2>& other)
    {
        if (!other.has_value()) {
            this->n_value = new violet::Err<E2>(other.error());
        }
    }

    template<typename E2>
        requires(!std::same_as<std::remove_cvref_t<E2>, Result<void, E2>> && std::constructible_from<E, E2>)
    constexpr VIOLET_IMPLICIT Result(std::expected<void, E2>&& other)
    {
        if (!other.has_value()) {
            this->n_value = violet::Err<E2>(VIOLET_MOVE(other).error());
        }
    }

    constexpr auto operator=(std::expected<void, E>& other) -> Result&
    {
        if (!other.has_value()) {
            this->n_value = new violet::Err<E>(other.error());
        }

        return *this;
    }

    constexpr auto operator=(std::expected<void, E>&& other) -> Result&
    {
        if (this->n_value != nullptr) {
            delete this->n_value;
            this->n_value = nullptr;
        }

        if (!other.has_value()) {
            this->n_value = new violet::Err<E>(VIOLET_MOVE(other).error());
        }

        return *this;
    }
#endif

    /// Returns `true` if success.
    [[nodiscard]] constexpr auto Ok() const noexcept -> bool
    {
        return this->n_value == nullptr;
    }

    /// Returns `true` if failure.
    [[nodiscard]] constexpr auto Err() const noexcept -> bool
    {
        return this->n_value != nullptr;
    }

    constexpr auto Error() & noexcept VIOLET_LIFETIMEBOUND -> E&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<void, E>` invariant reached");
        return this->n_value->Error();
    }

    constexpr auto Error() const& noexcept VIOLET_LIFETIMEBOUND -> const E&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<void, E>` invariant reached");
        return this->n_value->Error();
    }

    constexpr auto Error() && noexcept VIOLET_LIFETIMEBOUND -> E&&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<void, E>` invariant reached");
        return VIOLET_MOVE(this->n_value->Error());
    }

    constexpr auto Error() const&& noexcept -> const E&&
    {
        VIOLET_DEBUG_ASSERT(this->Err(), "`Result<void, E>` invariant reached");
        return VIOLET_MOVE(this->n_value->Error());
    }

    template<typename Fun>
        requires(callable<Fun>)
    constexpr auto AndThen(Fun&& fun) && -> Result<std::invoke_result_t<Fun>, E>
    {
        using Ret = std::invoke_result_t<Fun>;
        if (this->Ok()) {
            return violet::Ok<Ret>(std::invoke(VIOLET_FWD(Fun, fun)));
        }

        return violet::Err<E>(VIOLET_MOVE(Error()));
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
        requires(callable<Pred, const E&> && callable_returns<Pred, bool, const E&>)
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

    friend auto operator<<(std::ostream& os, const Result<void, E>& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    violet::Err<E>* n_value = nullptr;
};

} // namespace violet

VIOLET_FORMATTER_TEMPLATE(violet::Err<E>, typename E);
VIOLET_FORMATTER_TEMPLATE(violet::Ok<T>, typename T);

template<typename T, typename E>
struct std::formatter<violet::Result<T, E>> final: public std::formatter<std::string> {
    constexpr formatter() = default;

    template<class FC>
    auto format(const violet::Result<T, E>& value, FC& cx) const
    {
        return ::std::formatter<std::string>::format(value.ToString(), cx);
    }
};

#if VIOLET_COMPILER(GCC) || VIOLET_COMPILER(CLANG)
#define __violet_try_impl__(expr, variable)                                                                            \
    ({                                                                                                                 \
        auto variable = (expr);                                                                                        \
        static_assert(::violet::is_result_v<decltype(variable)>, "expression didn't return a violet::Result");         \
        if ((variable).Err()) {                                                                                        \
            return ::violet::Err(VIOLET_MOVE((variable).Error()));                                                     \
        }                                                                                                              \
        VIOLET_MOVE(variable.Value());                                                                                 \
    })

#define VIOLET_TRY(expr) __violet_try_impl__(expr, VIOLET_UNIQUE_NAME(__violet_try_expr_))
#endif

#define __violet_try_void_impl__(expr, variable)                                                                       \
    do {                                                                                                               \
        auto variable = (expr);                                                                                        \
        static_assert(::violet::is_result_v<decltype(variable)>, "expression didn't return a violet::Result");         \
        static_assert(                                                                                                 \
            ::std::is_void_v<::violet::result_value_type_t<decltype(variable)>>, "result value type must be `void`");  \
                                                                                                                       \
        if ((variable).Err()) {                                                                                        \
            return ::violet::Err(VIOLET_MOVE((variable).Error()));                                                     \
        }                                                                                                              \
    } while (false);

#define VIOLET_TRY_VOID(expr) __violet_try_void_impl__(expr, VIOLET_UNIQUE_NAME(__violet_try_void_expr_))
