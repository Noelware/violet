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

#ifndef VIOLET_FEATURE_BACKWARDS_STL_OPTIONAL
#define VIOLET_FEATURE_BACKWARDS_STL_OPTIONAL 1
#endif

#include <functional>
#include <utility>

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
#include <optional>
#endif

namespace violet {
namespace NOELDOC_HIDE optional_internal {

/// The `Optional` type produced by mapping over a value with a function returning `R`.
///
/// Lvalue references are kept, so mapping to a member reference produces `Optional<U&>`. Everything
/// else is stripped of references and cv-qualifiers so it can be stored by value.
template<typename R>
using map_result_t = std::conditional_t<std::is_lvalue_reference_v<R>, R, std::remove_cvref_t<R>>;

template<typename T>
struct storage {
    bool n_engaged = false;
    union {
        char Dummy;
        T Data;
    };

    constexpr VIOLET_IMPLICIT storage() noexcept
        : Dummy('\0')
    {
    }

    constexpr VIOLET_IMPLICIT storage(const storage&)
        requires std::is_trivially_copy_constructible_v<T>
    = default;

    constexpr VIOLET_IMPLICIT storage(const storage& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires(std::is_copy_constructible_v<T> && !std::is_trivially_copy_constructible_v<T>)
        : Dummy('\0')
    {
        if (other.Engaged()) {
            this->Construct(other.Data);
        }
    }

    constexpr auto operator=(const storage&) -> storage&
        requires(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T>
                    && std::is_trivially_destructible_v<T>)
    = default;

    constexpr auto operator=(const storage& other) noexcept(std::is_nothrow_copy_constructible_v<T>
        && (!std::is_copy_assignable_v<T> || std::is_nothrow_copy_assignable_v<T>)) -> storage&
        requires(std::is_copy_constructible_v<T>
            && !(std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T>
                && std::is_trivially_destructible_v<T>))
    {
        if (this == &other) {
            return *this;
        }

        if (!other.Engaged()) {
            this->Disengage();
            return *this;
        }

        if constexpr (std::is_copy_assignable_v<T>) {
            if (this->Engaged()) {
                this->Data = other.Data;
                return *this;
            }
        }

        this->Construct(other.Data);
        return *this;
    }

    constexpr VIOLET_IMPLICIT storage(storage&&)
        requires std::is_trivially_move_constructible_v<T>
    = default;

    constexpr VIOLET_IMPLICIT storage(storage&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires(std::is_move_constructible_v<T> && !std::is_trivially_move_constructible_v<T>)
        : Dummy('\0')
    {
        if (other.Engaged()) {
            this->Construct(VIOLET_MOVE(other.Data));
        }
    }

    constexpr auto operator=(storage&&) -> storage&
        requires(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>
                    && std::is_trivially_destructible_v<T>)
    = default;

    constexpr auto operator=(storage&& other) noexcept(std::is_nothrow_move_constructible_v<T>
        && (!std::is_move_assignable_v<T> || std::is_nothrow_move_assignable_v<T>)) -> storage&
        requires(std::is_move_constructible_v<T>
            && !(std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T>
                && std::is_trivially_destructible_v<T>))
    {
        if (this == &other) {
            return *this;
        }

        if (!other.Engaged()) {
            this->Disengage();
            return *this;
        }

        if constexpr (std::is_move_assignable_v<T>) {
            if (this->Engaged()) {
                this->Data = VIOLET_MOVE(other.Data);
                return *this;
            }
        }

        this->Construct(VIOLET_MOVE(other.Data));
        return *this;
    }

    constexpr ~storage()
        requires std::is_trivially_destructible_v<T>
    = default;

    constexpr ~storage()
    {
        this->Disengage();
    }

    [[nodiscard]]
    constexpr auto Engaged() const noexcept -> bool
    {
        return this->n_engaged;
    }

    template<typename... Args>
    constexpr void Construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        if (this->n_engaged) {
            this->Disengage();
        }

        std::construct_at(std::addressof(this->Data), VIOLET_FWD(Args, args)...);
        this->n_engaged = true;
    }

    constexpr void Disengage() noexcept
    {
        if (this->n_engaged) {
            std::destroy_at(std::addressof(this->Data));
            this->n_engaged = false;
        }
    }
};

template<typename T>
struct storage<T&> {
    T* Data = nullptr;

    constexpr VIOLET_IMPLICIT storage() noexcept = default;

    [[nodiscard]]
    constexpr auto Engaged() const noexcept -> bool
    {
        return this->Data != nullptr;
    }

    constexpr void Construct(T& value) noexcept
    {
        this->Data = std::addressof(value);
    }

    constexpr void Construct(std::remove_reference_t<T>&&) = delete;

    constexpr void Disengage() noexcept
    {
        this->Data = nullptr;
    }
};

} // namespace NOELDOC_HIDE optional_internal

template<typename T>
struct Optional;

/// Type for representing a empty `Optional<T>`. This is not useful for consumers, use [`Nothing`].
struct VIOLET_API NOELDOC_SINCE("26.11") nothing_t final {
    struct VIOLET_API NOELDOC_HIDE secret final { };

    constexpr VIOLET_EXPLICIT nothing_t(secret, secret) noexcept { }
};

/// Marker for representing a empty `Optional<T>`.
NOELDOC_SINCE("26.02")
static inline constexpr nothing_t Nothing{nothing_t::secret{}, nothing_t::secret{}};

/// Type trait that detects whether a type `T` is a `Optional<...>` or `std::optional<...>`.
///
/// ## Example
/// ```cpp
/// #include <violet/Container/Optional.h>
/// #include <type_traits>
///
/// static_assert(violet::is_optional_v<violet::Optional<int>>);
/// static_assert(violet::is_optional_v<std::optional<int>>);
/// static_assert(!violet::is_optional_v<int>);
/// ```
template<typename T>
struct NOELDOC_SINCE("26.02.03") is_optional: std::false_type { };

template<typename T>
struct NOELDOC_SINCE("26.02.03") is_optional<Optional<T>>: std::true_type { };

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
/// Specialization for [`is_optional`] for `std::optional`.
///
/// ## Remarks
/// As of 26.11, this is now guarded by the `BACKWARDS_STL_OPTIONAL` feature.
template<typename T>
struct NOELDOC_SINCE("26.02.03") is_optional<std::optional<T>>: std::true_type { };
#endif

template<typename T>
NOELDOC_SINCE("26.02.03")
[[maybe_unused]] static constexpr bool is_optional_v = is_optional<T>::value;

/// Type trait to extract the inner type from an [`Optional`] or [`std::optional`].
///
/// The primary template (`T`) is intentionally left undefined. It is meant to be specialized
/// on arbitrary optional types, most used in the `violet/Iterator.h` header.
///
/// @tparam T The type from which to extract an inner value type.
template<class T>
struct VIOLET_API NOELDOC_SINCE("26.03.05") optional_type;

/// Specialization of [`optional_type`] for [`violet::Optional`].
template<class U>
struct VIOLET_API NOELDOC_SINCE("26.03.05") optional_type<Optional<U>> {
    /// Extracted inner type from [`violet::Optional`]
    using type = U;
};

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
/// Specialization of [`optional_type`] for [`std::optional`].
///
/// ## Remarks
/// As of 26.11, this is now guarded by the `BACKWARDS_STL_OPTIONAL` feature.
template<class U>
struct VIOLET_API NOELDOC_SINCE("26.03.05") optional_type<std::optional<U>> {
    /// Extracted inner type from [`violet::Optional`]
    using type = U;
};
#endif

/// Convenience alias for accessing the extracted inner type.
///
/// It is the equivalent to `typename violet::optional_type<T>::type`.
///
/// @tparam T which optional wrapper whose inner type should be extracted.
/// @since 26.03.05
template<class T>
using optional_type_t = optional_type<T>::type;

/// Marks a value as present, so it can be converted into an engaged [`Optional`].
///
/// `Some<T>` is a small wrapper type with no other job than to say that "this value exists". It turns
/// into an [`Optional<T>`] (or [`std::optional<T>`] if the `BACKWARDS_STL_OPTIONAL` is enabled) wherever
/// one is expected, so code reads like Rust's [`Some(value)`].
///
/// [`Some(value)`]: https://doc.rust-lang.org/stable/std/option/enum.Option.html#variant.Some
///
/// ```cpp
/// auto value = Some(42);  // CTAD deduces this to `Some<Int32>`
/// Optional opt(Some(42)); // CTAD deduces this to `Optional<Int32>`
/// ```
///
/// `Some<T>` has no accessors of its own. Convert it to an [`Optional<T>`] to read the value or to use
/// combinators like `Map` and `UnwrapOr`.
///
/// ## Example
/// ```cpp
/// #include <violet/Container/Optional.h>
///
/// using violet::Optional;
/// using violet::Some;
///
/// auto findUser(int id) -> Optional<violet::String>
/// {
///     if (id == 1) {
///         return Some("Noel");
///     }
///
///     return violet::Nothing;
/// }
///
/// auto user = findUser(1);
/// VIOLET_ASSERT(user.HasValue(), "doctest failed: expected a user");
/// ```
///
/// @tparam T The type of the wrapped value.
template<typename T>
struct NOELDOC_SINCE("26.02") Some final {
    static_assert(std::is_object_v<T>, "`Some<T>` requires `T` to be a object type");
    static_assert(!std::is_void_v<T>, "`Some<void>` is ill-formed");
    static_assert(!std::is_array_v<T>, "`Some<T>` must wrap an array type");
    static_assert(std::is_destructible_v<T>, "`Some<T>` requires `T` to be destructible");
    static_assert(std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>,
        "`Some<T>` requires `T` to be movable or copyable");
    static_assert(!std::same_as<T, Some<T>>, "`Some<Some<T>>` is ill-formed");

    /// Default-initializes this [`Some<T>`] if `T` is default-initialized.
    ///
    /// ## Remarks
    /// ### Since `26.05.02`
    /// `Some<T>` was not allowed to be default-initialized.
    ///
    /// ### Since `26.11`
    /// `Some<T>` is now allowed to be default-initialized if `T` is default constructible.
    constexpr VIOLET_IMPLICIT Some() noexcept(std::is_nothrow_default_constructible_v<T>)
        requires std::default_initializable<T>
    = default;

    /// Converting from a mutable lvalue `Some<Other>` is deleted.
    ///
    /// Converting would need either a copy or a move of the source. A copy may be surprising to
    /// the caller, and a move would empty an object the caller still owns. Write
    /// `VIOLET_MOVE(other)` to convert explicitly instead.
    ///
    /// @tparam Other The source wrapper's value type. It must convert to `T`.
    template<typename Other>
        requires(!std::same_as<T, std::decay_t<Other>> && std::constructible_from<T, std::decay_t<Other>>)
    NOELDOC_SINCE("26.05.02")
    constexpr VIOLET_IMPLICIT Some(Some<std::decay_t<Other>>&) = delete;

    /// Converts a `Some<Other>` rvalue into a `Some<T>` by moving its value and constructing `T` from it.
    ///
    /// ## Example
    /// ```cpp
    /// Some<violet::String> name = Some("Noel");  // `Some<const char*>` -> `Some<String>`
    /// ```
    ///
    /// @tparam Other The source wrapper's value type. It must convert to `T`.
    /// @param other The wrapper to move from.
    template<typename Other>
        requires(!std::same_as<T, std::decay_t<Other>> && std::constructible_from<T, std::decay_t<Other>>)
    NOELDOC_SINCE("26.05.02")
    constexpr VIOLET_IMPLICIT Some(Some<Other>&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_value(VIOLET_MOVE(other).n_value)
    {
    }

    /// Wraps a copy of `value`.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// The copy constructor for `Some<T>` is now guarded by [`std::copyable<T>`].
    ///
    /// @param value The value to copy into the wrapper.
    NOELDOC_SINCE("26.05.02")
    constexpr VIOLET_EXPLICIT Some(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires std::copyable<T>
        : n_value(value)
    {
    }

    /// Wraps `value` by moving it into the wrapper.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// The copy constructor for `Some<T>` is now guarded by [`std::movable<T>`].
    ///
    /// @param value The value to move into the wrapper.
    NOELDOC_SINCE("26.05.02")
    constexpr VIOLET_EXPLICIT Some(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::movable<T>
        : n_value(VIOLET_MOVE(value))
    {
    }

    /// Constructs the wrapped `T` in place from `args`.
    ///
    /// This overload is excluded when the only argument is already a `T`, so the copy and move
    /// constructors handle that case.
    ///
    /// ## Example
    /// ```cpp
    /// auto pair = Some<std::pair<int, int>>(1, 2);
    /// ```
    ///
    /// @tparam Args The argument types forwarded to `T`'s constructor.
    /// @param args The arguments to forward.
    template<typename... Args>
        requires(std::constructible_from<T, Args...>
            && !(sizeof...(Args) == 1 && (std::same_as<std::decay_t<Args>, T> || ...)))
    NOELDOC_SINCE("26.05.02")
    constexpr VIOLET_EXPLICIT Some(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : n_value(VIOLET_FWD(Args, args)...)
    {
    }

    /// Converts into an engaged [`Optional<T>`] that holds a copy of the wrapped value.
    /// @return An `Optional<T>` holding a copy of the value.
    NOELDOC_SINCE("26.11")
    constexpr VIOLET_IMPLICIT operator Optional<T>() const& noexcept
    {
        return Optional<T>(std::in_place, this->n_value);
    }

    /// Converts into an engaged [`Optional<T>`] by moving the wrapped value into it.
    /// @return An `Optional<T>` holding the moved value.
    NOELDOC_SINCE("26.05.02")
    constexpr VIOLET_IMPLICIT operator Optional<T>() && noexcept
    {
        return Optional<T>(std::in_place, VIOLET_MOVE(this->n_value));
    }

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Converts into an engaged [`std::optional<T>`] that holds a copy of the wrapped value.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This conversion operator is now guarded by the `BACKWARDS_STL_OPTIONAL` feature.
    ///
    /// @return A `std::optional<T>` holding a copy of the value.
    NOELDOC_SINCE("26.11")
    constexpr VIOLET_IMPLICIT operator std::optional<T>() const& noexcept
    {
        return std::optional<T>(std::in_place, this->n_value);
    }

    /// Converts into an engaged [`std::optional<T>`] by moving the wrapped value into it.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This conversion operator is now guarded by the `BACKWARDS_STL_OPTIONAL` feature.
    ///
    /// @return A `std::optional<T>` holding the moved value.
    NOELDOC_SINCE("26.05.02")
    constexpr VIOLET_IMPLICIT operator std::optional<T>() && noexcept
    {
        return std::optional<T>(std::in_place, VIOLET_MOVE(this->n_value));
    }
#endif

private:
    template<typename U>
    friend struct violet::Optional;

    T n_value;
};

/// Deduction guide: wrapping a C array (such as a string literal) decays it to a pointer.
///
/// ## Example
/// ```cpp
/// auto s = Some("hello");  // Some<const char*>
/// ```
template<typename T, UInt N>
NOELDOC_SINCE("26.05.02")
Some(T (&)[N]) -> Some<const T*>;

/// Deduction guide: `Some(value)` deduces `T` with references and cv-qualifiers stripped.
template<typename T>
NOELDOC_SINCE("26.05.02")
Some(T&&) -> Some<std::remove_cvref_t<T>>;

/// A container type that may or may not contain a value, analogous to Rust's [`Option`] enumeration
/// and C++'s [`std::optional`] with functional-style utilities.
///
/// [`Option`]: https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html
///
/// `T` may be an object type or an lvalue reference to one. `Optional<U&>` stores a pointer
/// internally and is the same size as `U*`.
///
/// ## Type Requirements
/// - Be an object type, or an lvalue reference to an object type
/// - Be destructible
///
/// ## Example
/// ```cpp
/// #include <violet/Container/Optional.h>
///
/// using violet::Nothing;
/// using violet::Optional;
/// using violet::Some;
///
/// Optional<int> value = Some(10);
/// auto doubled = value.Map([](int val) -> int { return val * 2; });
/// VIOLET_ASSERT(doubled.HasValue(), "doctest failed: no value was provided");
/// VIOLET_ASSERT(*doubled == 20, "doctest failed: didn't double up");
///
/// Optional<int> empty = Nothing;
/// VIOLET_ASSERT(empty.UnwrapOr(5) == 5, "doctest failed: wasn't `5`");
/// ```
///
/// ## Remarks
/// ### C++ Exceptions
/// Violet supports both C++ exceptions and exception-free code, and `Optional<T>` follows suit. Accessors
/// that cannot produce a value (like `Unwrap()`) throw [`std::logic_error`] when exceptions are enabled,
/// and go through Violet's panic system otherwise.
///
/// ### Since `26.11`
/// `Optional<T&>` is now supported directly; [`std::reference_wrapper`] is no longer needed. An
/// `Optional<U&>` behaves like a nullable, rebindable pointer:
///
/// - Assigning to it rebinds the reference; it never assigns through to the referred-to object.
/// - `const Optional<U&>` still gives mutable access to the `U`, the same way `U* const` does.
/// - It can never be bound to a temporary; those constructors and assignments are deleted.
template<typename T>
struct [[nodiscard("check the state before discarding")]] NOELDOC_SINCE("26.02") Optional final
    : private optional_internal::storage<T> {
    /// The contained value type.
    ///
    /// For `Optional<U&>`, this is `U` (keeping any `const`), so `value_type&` is always the type of
    /// the contained or referred-to object.
    using value_type = std::remove_reference_t<T>;

    static_assert(!std::is_rvalue_reference_v<T>, "`Optional<T&&>` is ill-formed");
    static_assert(
        std::is_object_v<value_type>, "`Optional<T>` requires `T` to be an object type or an lvalue reference to one");
    static_assert(std::is_destructible_v<value_type>, "`Optional<T>` requires `T` to be destructible");

private:
    static constexpr bool kIsReference = std::is_lvalue_reference_v<T>;
    using unqualified_type = std::remove_cv_t<value_type>;

public:
    /// Constructs an empty `Optional`.
    constexpr VIOLET_IMPLICIT Optional() noexcept = default;

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Constructs an empty `Optional` from [`std::nullopt`].
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// [`violet::Nothing`] used to be an alias for [`std::nullopt`]; it no longer is. This overload is
    /// kept for backwards compatibility with [`std::optional`].
    constexpr VIOLET_IMPLICIT Optional(std::nullopt_t) noexcept
        : Optional()
    {
    }
#endif

    /// Constructs an empty `Optional` from [`violet::Nothing`].
    NOELDOC_SINCE("26.11")
    constexpr VIOLET_IMPLICIT Optional(nothing_t) noexcept
        : Optional()
    {
    }

    /// Constructs an engaged `Optional` whose value is built in place from `args`.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<std::pair<int, int>> pair(std::in_place, 1, 2);
    /// VIOLET_ASSERT0(pair->first == 1);
    /// ```
    ///
    /// @param args arguments forwarded to `T`'s constructor
    template<typename... Args>
        requires(!kIsReference && std::is_constructible_v<T, Args...>)
    constexpr VIOLET_EXPLICIT Optional(std::in_place_t, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>)
    {
        this->Construct(VIOLET_FWD(Args, args)...);
    }

    /// Constructs an `Optional<U&>` that refers to `value`.
    /// This mirrors the in-place constructor of `Optional<T>` so generic code can use `std::in_place`
    /// for both.
    ///
    /// @param value the object to refer to; it must outlive every use of this `Optional`
    NOELDOC_SINCE("26.11")
    constexpr VIOLET_EXPLICIT Optional(std::in_place_t, value_type& value) noexcept
        requires kIsReference
    {
        this->Construct(value);
    }

    NOELDOC_SINCE("26.11")
    constexpr Optional(std::in_place_t, value_type&&)
        requires kIsReference
    = delete;

    /// Constructs an engaged `Optional` from `value`, copying or moving it depending on its value category.
    ///
    /// Participates in overload resolution only if:
    /// - `T` is not a reference
    /// - `U` is not [`Optional`], [`Some`], [`Nothing`], or `std::in_place` (nor [`std::optional`] or
    ///   [`std::nullopt`] if the `BACKWARDS_STL_OPTIONAL` feature is enabled)
    /// - `T` is constructible from `U&&`
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This used to take `const U&`, which always copied and couldn't hold move-only types.
    ///
    /// @param value the value to copy or move in
    template<typename U = T>
    // clang-format off
    requires(
        !kIsReference &&
        !instanceof_v<Optional, std::remove_cvref_t<U>> &&
        !instanceof_v<Some, std::remove_cvref_t<U>> &&
        !std::same_as<std::remove_cvref_t<U>, nothing_t> &&
        !std::same_as<std::remove_cvref_t<U>, std::in_place_t> &&
        std::constructible_from<T, U&&>
        #if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
        && !instanceof_v<std::optional, std::remove_cvref_t<U>>
        && !std::same_as<std::remove_cvref_t<U>, std::nullopt_t>
        #endif
    )
    // clang-format on
    constexpr VIOLET_IMPLICIT Optional(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : Optional(std::in_place, VIOLET_FWD(U, value))
    {
    }

    /// Constructs an `Optional<U&>` that refers to `value`.
    ///
    /// ## Example
    /// ```cpp
    /// int x = 1;
    /// violet::Optional<int&> ref = x;
    ///
    /// ref.Value() = 2;
    /// VIOLET_ASSERT0(x == 2);
    /// ```
    ///
    /// @param value the object to refer to; it must outlive every use of this `Optional`
    NOELDOC_SINCE("26.11")
    constexpr VIOLET_IMPLICIT Optional(value_type& value) noexcept
        requires kIsReference
    {
        this->Construct(value);
    }

    NOELDOC_SINCE("26.11")
    constexpr Optional(value_type&&)
        requires kIsReference
    = delete;

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Constructs from a [`std::optional`] by copy.
    ///
    /// If `other` holds a value, `T` is constructed from it; otherwise this `Optional` is empty.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This is now guarded by the `BACKWARDS_STL_OPTIONAL` feature, and is unavailable for `Optional<U&>`.
    ///
    /// @param other the `std::optional` to copy from
    template<typename U>
        requires(
            !kIsReference && !instanceof_v<Optional, std::remove_cvref_t<U>> && std::constructible_from<T, const U&>)
    constexpr VIOLET_IMPLICIT Optional(const std::optional<U>& other) noexcept(
        std::is_nothrow_constructible_v<T, const U&>)
    {
        if (other.has_value()) {
            this->Construct(*other);
        }
    }

    /// Constructs from a [`std::optional`] by move.
    ///
    /// If `other` holds a value, `T` is move-constructed from it; otherwise this `Optional` is empty.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This is now guarded by the `BACKWARDS_STL_OPTIONAL` feature, and is unavailable for `Optional<U&>`.
    ///
    /// @param other the `std::optional` to move from
    template<typename U>
        requires(!kIsReference && !instanceof_v<Optional, std::remove_cvref_t<U>> && std::constructible_from<T, U &&>)
    constexpr VIOLET_IMPLICIT Optional(std::optional<U>&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>)
    {
        if (other.has_value()) {
            this->Construct(*VIOLET_MOVE(other));
        }
    }

    /// Assigns from a [`std::optional`] by copy, following the same rules as copy-assigning an `Optional`.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This is now guarded by the `BACKWARDS_STL_OPTIONAL` feature, and is unavailable for `Optional<U&>`.
    ///
    /// @param other the `std::optional` to copy from
    template<typename U>
        requires(!kIsReference && std::same_as<U, T> && std::is_copy_constructible_v<T>)
    constexpr auto operator=(const std::optional<U>& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        -> Optional&
    {
        if (other.has_value()) {
            this->Construct(*other);
        } else {
            this->Disengage();
        }

        return *this;
    }

    /// Assigns from a [`std::optional`] by move, following the same rules as move-assigning an `Optional`.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This is now guarded by the `BACKWARDS_STL_OPTIONAL` feature, and is unavailable for `Optional<U&>`.
    ///
    /// @param other the `std::optional` to move from
    template<typename U>
        requires(!kIsReference && std::same_as<U, T> && std::is_move_constructible_v<T>)
    constexpr auto operator=(std::optional<U>&& other) noexcept(std::is_nothrow_move_constructible_v<T>) -> Optional&
    {
        if (other.has_value()) {
            this->Construct(*VIOLET_MOVE(other));
        } else {
            this->Disengage();
        }

        return *this;
    }
#endif

    /// Constructs an engaged `Optional` from a copy of the value wrapped in `some`.
    /// @param some the wrapper to copy from
    template<typename U>
        requires(!kIsReference && std::convertible_to<const U&, T>)
    constexpr VIOLET_IMPLICIT Optional(const Some<U>& some) noexcept(std::is_nothrow_constructible_v<T, const U&>)
        : Optional(std::in_place, some.n_value)
    {
    }

    /// Constructs an engaged `Optional` by moving the value out of `some`.
    /// @param some the wrapper to move from
    template<typename U>
        requires(!kIsReference && std::convertible_to<U &&, T>)
    constexpr VIOLET_IMPLICIT Optional(Some<U>&& some) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : Optional(std::in_place, VIOLET_MOVE(some.n_value))
    {
    }

    /// Replaces the held value with a copy of the value wrapped in `other`.
    /// @param other the wrapper to copy from
    template<typename U>
        requires(!kIsReference && std::convertible_to<const U&, T>)
    NOELDOC_SINCE("26.11")
    constexpr auto operator=(const Some<U>& other) noexcept(std::is_nothrow_constructible_v<T, const U&>) -> Optional&
    {
        this->Construct(other.n_value);
        return *this;
    }

    /// Replaces the held value by moving the value out of `other`.
    /// @param other the wrapper to move from
    template<typename U>
        requires(!kIsReference && std::convertible_to<U &&, T>)
    NOELDOC_SINCE("26.11")
    constexpr auto operator=(Some<U>&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>) -> Optional&
    {
        this->Construct(VIOLET_MOVE(other.n_value));
        return *this;
    }

    /// Copy-constructs from another `Optional`.
    ///
    /// If `other` holds a value, it is copy-constructed into this `Optional`; otherwise this `Optional`
    /// is empty. Trivial when `T` is trivially copy-constructible. For `Optional<U&>`, this copies the
    /// reference, not the referred-to object.
    constexpr VIOLET_IMPLICIT Optional(const Optional&) = default;

    /// Move-constructs from another `Optional`.
    ///
    /// If `other` holds a value, it is move-constructed into this `Optional`. `other` keeps holding its
    /// (moved-from) value, matching [`std::optional`]; use [`Take`] to leave it empty instead.
    constexpr VIOLET_IMPLICIT Optional(Optional&&) = default;

    constexpr ~Optional() = default;

    /// Copy-assigns from another `Optional`.
    ///
    /// - If both hold a value, the value is copy-assigned.
    /// - If only `other` holds a value, it is copy-constructed into this `Optional`.
    /// - If `other` is empty, this `Optional` becomes empty.
    ///
    /// For `Optional<U&>`, this rebinds the reference.
    constexpr auto operator=(const Optional&) -> Optional& = default;

    /// Move-assigns from another `Optional`, following the same rules as copy assignment but moving.
    constexpr auto operator=(Optional&&) -> Optional& = default;

    /// Empties this `Optional`, destroying the held value if there is one.
    constexpr auto operator=(nothing_t) noexcept -> Optional&
    {
        this->Reset();
        return *this;
    }

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Empties this `Optional`, destroying the held value if there is one.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This is now guarded by the `BACKWARDS_STL_OPTIONAL` feature.
    ///
    /// @return `*this`
    constexpr auto operator=(std::nullopt_t) noexcept -> Optional&
    {
        this->Disengage();
        return *this;
    }
#endif

    /// Replaces the held value with a copy of `value`.
    /// @param value the value to copy in
    constexpr auto operator=(const value_type& value) noexcept(std::is_nothrow_copy_constructible_v<value_type>)
        -> Optional&
        requires(!kIsReference && std::is_copy_constructible_v<value_type>)
    {
        this->Construct(value);
        return *this;
    }

    /// Replaces the held value by moving `value` in.
    /// @param value the value to move in
    constexpr auto operator=(value_type&& value) noexcept(std::is_nothrow_move_constructible_v<value_type>) -> Optional&
        requires(!kIsReference && std::is_move_constructible_v<value_type>)
    {
        this->Construct(VIOLET_MOVE(value));
        return *this;
    }

    /// Rebinds this `Optional<U&>` to refer to `value`.
    ///
    /// The previously referred-to object is left untouched; this never assigns through the reference.
    ///
    /// ## Example
    /// ```cpp
    /// int a = 1;
    /// int b = 2;
    ///
    /// violet::Optional<int&> ref = a;
    /// ref = b;
    ///
    /// VIOLET_ASSERT0(a == 1);
    /// VIOLET_ASSERT0(&ref.Value() == &b);
    /// ```
    ///
    /// @param value the object to refer to; it must outlive every use of this `Optional`
    /// @return `*this`
    NOELDOC_SINCE("26.11")
    constexpr auto operator=(value_type& value) noexcept -> Optional&
        requires kIsReference
    {
        this->Construct(value);
        return *this;
    }

    NOELDOC_SINCE("26.11")
    constexpr auto operator=(value_type&&) -> Optional&
        requires kIsReference
    = delete;

    /// Returns `true` if this `Optional` holds a value.
    /// Equivalent to Rust's
    /// [`Option::is_some`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.is_some).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> some = violet::Some(1);
    /// violet::Optional<int> none = violet::Nothing;
    ///
    /// VIOLET_ASSERT0(some.HasValue());
    /// VIOLET_ASSERT0(!none.HasValue());
    /// ```
    [[nodiscard]]
    constexpr auto HasValue() const noexcept -> bool
    {
        return this->Engaged();
    }

    /// Returns `true` if this `Optional` holds a value. Equivalent to [`HasValue`].
    [[nodiscard]]
    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->HasValue();
    }

    /// Returns a reference to the held value.
    ///
    /// ## Panics
    /// Debug-asserts that a value is present. In release builds, calling this on an empty `Optional`
    /// is undefined behaviour; use [`Unwrap`] for a checked access.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// The debug assertion now reports the caller's location instead of `Optional<T>::Value()`'s.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> value = violet::Some(1);
    /// value.Value() += 1;
    ///
    /// VIOLET_ASSERT0(value.Value() == 2);
    /// ```
    ///
    /// @param loc call site reported by the debug assertion
    /// @return a reference to the held value
    [[nodiscard]]
    constexpr auto Value(SourceLocation loc = std::source_location::current()) & noexcept VIOLET_LIFETIMEBOUND
        -> value_type&
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT_LOC(this->HasValue(), "`Optional<T>` represents nothing", loc);
        return this->Data;
    }

    /// Overload of [`Value`] for `const` lvalues.
    /// @param loc call site reported by the debug assertion
    /// @return a `const` reference to the held value
    [[nodiscard]]
    constexpr auto Value(SourceLocation loc = std::source_location::current()) const& noexcept VIOLET_LIFETIMEBOUND
        -> const value_type&
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT_LOC(this->HasValue(), "`Optional<T>` represents nothing", loc);
        return this->Data;
    }

    /// Overload of [`Value`] for rvalues, allowing the held value to be moved out.
    /// @param loc call site reported by the debug assertion
    /// @return an rvalue reference to the held value
    [[nodiscard]]
    constexpr auto Value(SourceLocation loc = std::source_location::current()) && noexcept VIOLET_LIFETIMEBOUND
        -> value_type&&
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT_LOC(this->HasValue(), "`Optional<T>` represents nothing", loc);
        return VIOLET_MOVE(this->Data);
    }

    /// Overload of [`Value`] for `const` rvalues.
    /// @param loc call site reported by the debug assertion
    /// @return a `const` rvalue reference to the held value
    [[nodiscard]]
    constexpr auto Value(SourceLocation loc = std::source_location::current()) const&& noexcept VIOLET_LIFETIMEBOUND
        -> const value_type&&
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT_LOC(this->HasValue(), "`Optional<T>` represents nothing", loc);
        return VIOLET_MOVE(this->Data);
    }

    /// Returns the referred-to object.
    ///
    /// Constness of the `Optional` does not carry over to the object: a `const Optional<U&>` still
    /// gives `U&`, the same way a `U* const` still points at a mutable `U`. This is also safe to call
    /// on a temporary `Optional<U&>`, since the result refers to the original object and not into the
    /// `Optional`.
    ///
    /// ## Panics
    /// Debug-asserts that a value is present. In release builds, calling this on an empty `Optional`
    /// is undefined behaviour; use [`Unwrap`] for a checked access.
    ///
    /// ## Example
    /// ```cpp
    /// int x = 1;
    /// const violet::Optional<int&> ref = x;
    ///
    /// ref.Value() = 2;
    /// VIOLET_ASSERT0(x == 2);
    /// ```
    ///
    /// @param loc call site reported by the debug assertion
    /// @return the referred-to object
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Value(SourceLocation loc = std::source_location::current()) const& noexcept
        -> value_type&
        requires kIsReference
    {
        VIOLET_DEBUG_ASSERT_LOC(this->HasValue(), "`Optional<T&>` represents nothing", loc);
        return *this->Data;
    }

    /// Returns a reference to the held value. Equivalent to [`Value`].
    ///
    /// ## Panics
    /// Debug-asserts that a value is present.
    ///
    /// @return a reference to the held value
    [[nodiscard]]
    constexpr auto operator*() & noexcept VIOLET_LIFETIMEBOUND->value_type&
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return this->Data;
    }

    /// Overload of [`operator*`] for `const` lvalues.
    /// @return a `const` reference to the held value
    [[nodiscard]]
    constexpr auto operator*() const& noexcept VIOLET_LIFETIMEBOUND->const value_type&
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return this->Data;
    }

    /// Overload of [`operator*`] for rvalues, allowing the held value to be moved out.
    /// @return an rvalue reference to the held value
    [[nodiscard]]
    constexpr auto operator*() && noexcept VIOLET_LIFETIMEBOUND->value_type&&
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return VIOLET_MOVE(this->Data);
    }

    /// Overload of [`operator*`] for `const` rvalues.
    /// @return a `const` rvalue reference to the held value
    [[nodiscard]]
    constexpr auto operator*() const&& noexcept VIOLET_LIFETIMEBOUND->const value_type&&
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return VIOLET_MOVE(this->Data);
    }

    /// Returns the referred-to object. Equivalent to [`Value`].
    ///
    /// ## Panics
    /// Debug-asserts that a value is present.
    ///
    /// @return the referred-to object
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto operator*() const& noexcept -> value_type&
        requires kIsReference
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return *this->Data;
    }

    /// Returns a pointer to the held value, for member access.
    ///
    /// ## Panics
    /// Debug-asserts that a value is present.
    ///
    /// @return a pointer to the held value
    [[nodiscard]]
    constexpr auto operator->() noexcept -> value_type*
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return std::addressof(this->Data);
    }

    /// Overload of [`operator->`] for `const` instances.
    /// @return a pointer to the `const` held value
    [[nodiscard]]
    constexpr auto operator->() const noexcept -> const value_type*
        requires(!kIsReference)
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return std::addressof(this->Data);
    }

    /// Returns a pointer to the referred-to object, for member access.
    ///
    /// ## Panics
    /// Debug-asserts that a value is present.
    ///
    /// @return a pointer to the referred-to object
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto operator->() const noexcept -> value_type*
        requires kIsReference
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return this->Data;
    }

    /// Returns a copy of the held value, or panics if this `Optional` is empty.
    ///
    /// The engaged path is marked as the likely branch.
    /// Equivalent to Rust's
    /// [`Option::unwrap`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.unwrap).
    ///
    /// ## Panics
    /// Panics if this `Optional` is empty. With C++ exceptions enabled, this throws [`std::logic_error`]
    /// with the panic message; with them disabled, it'll passthrough Violet's panic system.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> value = violet::Some(42);
    /// VIOLET_ASSERT0(value.Unwrap() == 42);
    /// ```
    ///
    /// @param loc call site reported by the panic
    /// @return a copy of the held value
    [[nodiscard]]
    constexpr auto Unwrap(
        [[maybe_unused]] SourceLocation loc = std::source_location::current()) const& -> unqualified_type
        requires(!kIsReference && std::is_copy_constructible_v<value_type>)
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return this->Data;
        }

#if VIOLET_FEATURE(EXCEPTIONS)
        VIOLET_THROW(std::logic_error, "tried to unwrap nothing");
#else
        VIOLET_PANIC_USERLAND("tried to unwrap nothing", loc);
#endif
    }

    /// Overload of [`Unwrap`] for rvalues - moves the held value out instead of copying it.
    ///
    /// ## Panics
    /// Panics if this `Optional` is empty, or [`std::logic_error`] is thrown if exceptions
    /// are enabled.
    ///
    /// @param loc call site reported by the panic
    /// @return the held value, moved out
    [[nodiscard]]
    constexpr auto Unwrap([[maybe_unused]] SourceLocation loc = std::source_location::current()) && -> unqualified_type
        requires(!kIsReference && std::is_move_constructible_v<value_type>)
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(this->Data);
        }

#if VIOLET_FEATURE(EXCEPTIONS)
        VIOLET_THROW(std::logic_error, "tried to unwrap nothing");
#else
        VIOLET_PANIC_USERLAND("tried to unwrap nothing", loc);
#endif
    }

    /// Returns the referred-to object, or panics if this `Optional` is empty.
    /// Unlike `Optional<T>::Unwrap`, this never copies: the result is the original object.
    ///
    /// ## Panics
    /// Panics if this `Optional` is empty. With C++ exceptions enabled, this throws [`std::logic_error`]
    /// with the panic message; with them disabled, it'll passthrough Violet's panic system.
    ///
    /// ## Example
    /// ```cpp
    /// violet::String name = "Noel";
    /// violet::Optional<violet::String&> ref = name;
    ///
    /// VIOLET_ASSERT0(&ref.Unwrap() == &name);
    /// ```
    ///
    /// @param loc call site reported by the panic
    /// @return the referred-to object
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Unwrap(
        [[maybe_unused]] SourceLocation loc = std::source_location::current()) const& -> value_type&
        requires kIsReference
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return *this->Data;
        }

#if VIOLET_FEATURE(EXCEPTIONS)
        VIOLET_THROW(std::logic_error, "tried to unwrap nothing");
#else
        VIOLET_PANIC_USERLAND("tried to unwrap nothing", loc);
#endif
    }

    /// Returns a copy of the held value, or panics with `message` if this `Optional` is empty.
    ///
    /// Equivalent to Rust's
    /// [`Option::expect`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.expect).
    ///
    /// ## Panics
    /// Panics with `message` if this `Optional` is empty. With C++ exceptions enabled, this throws
    /// [`std::logic_error`]; with them disabled, it'll passthrough Violet's panic system.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> port = violet::Some(8080);
    /// VIOLET_ASSERT0(port.Expect("port should be configured") == 8080);
    /// ```
    ///
    /// @param message the panic message
    /// @param loc call site reported by the panic
    /// @return a copy of the held value
    [[nodiscard]]
    constexpr auto Expect(
        Str message, [[maybe_unused]] SourceLocation loc = std::source_location::current()) const& -> unqualified_type
        requires(!kIsReference && std::is_copy_constructible_v<value_type>)
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return this->Data;
        }

#if VIOLET_FEATURE(EXCEPTIONS)
        VIOLET_THROW(std::logic_error, String(message));
#else
        VIOLET_PANIC_USERLAND(message, loc);
#endif
    }

    /// Overload of [`Expect`] for rvalues - moves the held value out instead of copying it.
    ///
    /// ## Panics
    /// Panics with `message` if this `Optional` is empty or [`std::logic_error`] is thrown if exceptions
    /// are enabled.
    ///
    /// @param message the panic message
    /// @param loc call site reported by the panic
    /// @return the held value, moved out
    [[nodiscard]]
    constexpr auto Expect(
        Str message, [[maybe_unused]] SourceLocation loc = std::source_location::current()) && -> unqualified_type
        requires(!kIsReference && std::is_move_constructible_v<value_type>)
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(this->Data);
        }

#if VIOLET_FEATURE(EXCEPTIONS)
        VIOLET_THROW(std::logic_error, String(message));
#else
        VIOLET_PANIC_USERLAND(message, loc);
#endif
    }

    /// Returns the referred-to object, or panics with `message` if this `Optional` is empty.
    ///
    /// ## Panics
    /// Panics with `message` if this `Optional` is empty. With C++ exceptions enabled, this throws
    /// [`std::logic_error`]; with them disabled, it'll passthrough Violet's panic system.
    ///
    /// @param message the panic message
    /// @param loc call site reported by the panic
    /// @return the referred-to object
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Expect(
        Str message, [[maybe_unused]] SourceLocation loc = std::source_location::current()) const& -> value_type&
        requires kIsReference
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return *this->Data;
        }

#if VIOLET_FEATURE(EXCEPTIONS)
        VIOLET_THROW(std::logic_error, String(message));
#else
        VIOLET_PANIC_USERLAND(message, loc);
#endif
    }

    /// Returns a copy of the held value, or `fallback` if this `Optional` is empty.
    /// Equivalent to Rust's
    /// [`Option::unwrap_or`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.unwrap_or).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> none = violet::Nothing;
    /// VIOLET_ASSERT0(none.UnwrapOr(5) == 5);
    /// ```
    ///
    /// @param fallback the value returned when this `Optional` is empty
    /// @return the held value or `fallback`
    [[nodiscard]]
    constexpr auto UnwrapOr(unqualified_type fallback) const& noexcept(std::is_nothrow_copy_constructible_v<value_type>
        && std::is_nothrow_move_constructible_v<unqualified_type>) -> unqualified_type
        requires(!kIsReference && std::is_copy_constructible_v<value_type>)
    {
        return this->HasValue() ? unqualified_type(this->Data) : VIOLET_MOVE(fallback);
    }

    /// Overload of [`UnwrapOr`] for rvalues - moves the held value out instead of copying it.
    /// @param fallback the value returned when this `Optional` is empty
    [[nodiscard]]
    constexpr auto UnwrapOr(unqualified_type fallback) && noexcept(
        std::is_nothrow_move_constructible_v<unqualified_type>) -> unqualified_type
        requires(!kIsReference && std::is_move_constructible_v<value_type>)
    {
        return this->HasValue() ? unqualified_type(VIOLET_MOVE(this->Data)) : VIOLET_MOVE(fallback);
    }

    /// Returns a copy of the referred-to object, or `fallback` if this `Optional` is empty.
    ///
    /// This returns a copy rather than a reference so a temporary `fallback` can't dangle. It is only
    /// available when `U` is copyable; otherwise, the reference-returning overload is used.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<const int&> none = violet::Nothing;
    /// VIOLET_ASSERT0(none.UnwrapOr(5) == 5);
    /// ```
    ///
    /// @param fallback the value returned when this `Optional` is empty
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto UnwrapOr(unqualified_type fallback) const& noexcept(
        std::is_nothrow_copy_constructible_v<value_type> && std::is_nothrow_move_constructible_v<unqualified_type>)
        -> unqualified_type
        requires(kIsReference && std::is_copy_constructible_v<value_type>)
    {
        return this->HasValue() ? unqualified_type(*this->Data) : VIOLET_MOVE(fallback);
    }

    /// Returns the referred-to object, or `fallback` if this `Optional` is empty.
    /// Used when `U` is not copyable, so no copy can be returned. `fallback` must be an lvalue; passing
    /// a temporary is deleted, since the returned reference would dangle.
    ///
    /// ## Example
    /// ```cpp
    /// std::unique_ptr<int> backup = std::make_unique<int>(1);
    /// violet::Optional<std::unique_ptr<int>&> none = violet::Nothing;
    ///
    /// VIOLET_ASSERT0(&none.UnwrapOr(backup) == &backup);
    /// ```
    ///
    /// @param fallback the object returned when this `Optional` is empty
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto UnwrapOr(value_type& fallback VIOLET_LIFETIMEBOUND) const& noexcept
        -> value_type&
        requires(kIsReference && !std::is_copy_constructible_v<value_type>)
    {
        return this->HasValue() ? *this->Data : fallback;
    }

    /// Passing a temporary `fallback` to a non-copyable `Optional<U&>` is deleted, as the returned
    /// reference would dangle.
    NOELDOC_SINCE("26.11")
    constexpr auto UnwrapOr(value_type&&) const& -> value_type&
        requires(kIsReference && !std::is_copy_constructible_v<value_type>)
    = delete;

    /// Returns a copy of the held value, or a value-initialized `T` if this `Optional` is empty.
    ///
    /// Equivalent to Rust's
    /// [`Option::unwrap_or_default`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.unwrap_or_default).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> none = violet::Nothing;
    /// VIOLET_ASSERT0(none.UnwrapOrDefault() == 0);
    /// ```
    [[nodiscard]]
    constexpr auto UnwrapOrDefault() const& noexcept(std::is_nothrow_copy_constructible_v<value_type>
        && std::is_nothrow_default_constructible_v<unqualified_type>) -> unqualified_type
        requires(!kIsReference
            && std::is_default_constructible_v<unqualified_type> && std::is_copy_constructible_v<value_type>)
    {
        return this->HasValue() ? unqualified_type(this->Data) : unqualified_type{};
    }

    /// Overload of [`UnwrapOrDefault`] for rvalues - moves the held value out instead of copying it.
    [[nodiscard]]
    constexpr auto UnwrapOrDefault() && noexcept(std::is_nothrow_move_constructible_v<value_type>
        && std::is_nothrow_default_constructible_v<unqualified_type>) -> unqualified_type
        requires(!kIsReference
            && std::is_default_constructible_v<unqualified_type> && std::is_move_constructible_v<value_type>)
    {
        return this->HasValue() ? unqualified_type(VIOLET_MOVE(this->Data)) : unqualified_type{};
    }

    /// Returns a copy of the referred-to object, or a value-initialized `U` if this `Optional` is empty.
    /// The default is a temporary, so there is nothing a reference could point at; this always returns
    /// by value.
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto UnwrapOrDefault() const& noexcept(
        std::is_nothrow_copy_constructible_v<value_type> && std::is_nothrow_default_constructible_v<unqualified_type>)
        -> unqualified_type
        requires(kIsReference
            && std::is_default_constructible_v<unqualified_type> && std::is_copy_constructible_v<value_type>)
    {
        return this->HasValue() ? unqualified_type(*this->Data) : unqualified_type{};
    }

    /// Returns a copy of the held value, or the result of calling `fun` if this `Optional` is empty.
    ///
    /// Unlike [`UnwrapOr`], the fallback is only computed when it's needed.
    ///
    /// Equivalent to Rust's
    /// [`Option::unwrap_or_else`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.unwrap_or_else).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<violet::String> name = violet::Nothing;
    /// VIOLET_ASSERT0(name.UnwrapOrElse([] { return violet::String("anonymous"); }) == "anonymous");
    /// ```
    ///
    /// @param fun called with no arguments to produce the fallback
    /// @return the held value or `fun()`
    template<typename Fun>
        requires(!kIsReference && std::is_copy_constructible_v<value_type> && callable_returns<Fun, unqualified_type>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto UnwrapOrElse(Fun&& fun) const& -> unqualified_type
    {
        if (this->HasValue()) {
            return this->Data;
        }

        return std::invoke(VIOLET_FWD(Fun, fun));
    }

    /// Overload of [`UnwrapOrElse`] for rvalues - moves the held value out instead of copying it.
    /// @param fun called with no arguments to produce the fallback
    template<typename Fun>
        requires(!kIsReference && std::is_move_constructible_v<value_type> && callable_returns<Fun, unqualified_type>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto UnwrapOrElse(Fun&& fun) && -> unqualified_type
    {
        if (this->HasValue()) {
            return VIOLET_MOVE(this->Data);
        }

        return std::invoke(VIOLET_FWD(Fun, fun));
    }

    /// Returns a copy of the referred-to object, or the result of calling `fun` if this `Optional` is empty.
    ///
    /// Chosen when `fun` returns a value. Returning a reference here would dangle from `fun`'s
    /// temporary result, so this always copies; `U` must be copyable.
    ///
    /// @param fun called with no arguments to produce the fallback
    /// @return a copy of the referred-to object or `fun()`
    template<typename Fun>
        requires(kIsReference && callable<Fun> && !std::is_lvalue_reference_v<std::invoke_result_t<Fun>>
            && std::is_copy_constructible_v<value_type> && callable_returns<Fun, unqualified_type>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto UnwrapOrElse(Fun&& fun) const& -> unqualified_type
    {
        if (this->HasValue()) {
            return *this->Data;
        }

        return std::invoke(VIOLET_FWD(Fun, fun));
    }

    /// Returns the referred-to object, or the object `fun` returns a reference to if this `Optional`
    /// is empty.
    ///
    /// Chosen when `fun` returns an lvalue reference, so neither path copies. This also works when
    /// `U` isn't copyable.
    ///
    /// ## Example
    /// ```cpp
    /// violet::String fallback = "anonymous";
    /// violet::Optional<violet::String&> none = violet::Nothing;
    ///
    /// violet::String& name = none.UnwrapOrElse([&]() -> violet::String& { return fallback; });
    /// VIOLET_ASSERT0(&name == &fallback);
    /// ```
    ///
    /// @param fun called with no arguments; must return an lvalue reference that outlives the result
    /// @return the referred-to object or `fun()`
    template<typename Fun>
        requires(kIsReference && callable<Fun> && std::is_lvalue_reference_v<std::invoke_result_t<Fun>>
            && std::convertible_to<std::invoke_result_t<Fun>, value_type&>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto UnwrapOrElse(Fun&& fun) const& -> value_type&
    {
        if (this->HasValue()) {
            return *this->Data;
        }

        return std::invoke(VIOLET_FWD(Fun, fun));
    }

    /// Returns a reference to the held value without checking that one is present.
    ///
    /// ## Safety
    /// Undefined behaviour if this `Optional` is empty.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> value = violet::Some(1);
    /// if (value.HasValue()) {
    ///     VIOLET_ASSERT0(value.UnwrapUnchecked(violet::Unsafe("checked above")) == 1);
    /// }
    /// ```
    ///
    /// @return a reference to the held value
    [[nodiscard]]
    NOELDOC_SINCE("26.04") constexpr auto UnwrapUnchecked(Unsafe) & noexcept VIOLET_LIFETIMEBOUND->value_type&
        requires(!kIsReference)
    {
        return this->Data;
    }

    /// Overload of [`UnwrapUnchecked`] for `const` lvalues.
    ///
    /// ## Safety
    /// Undefined behaviour if this `Optional` is empty.
    [[nodiscard]]
    NOELDOC_SINCE("26.04") constexpr auto UnwrapUnchecked(
        Unsafe) const& noexcept VIOLET_LIFETIMEBOUND->const value_type&
        requires(!kIsReference)
    {
        return this->Data;
    }

    /// Overload of [`UnwrapUnchecked`] for rvalues, allowing the held value to be moved out.
    ///
    /// ## Safety
    /// Undefined behaviour if this `Optional` is empty.
    [[nodiscard]]
    NOELDOC_SINCE("26.04") constexpr auto UnwrapUnchecked(Unsafe) && noexcept VIOLET_LIFETIMEBOUND->value_type&&
        requires(!kIsReference)
    {
        return VIOLET_MOVE(this->Data);
    }

    /// Overload of [`UnwrapUnchecked`] for `const` rvalues.
    ///
    /// ## Safety
    /// Undefined behaviour if this `Optional` is empty.
    [[nodiscard]]
    NOELDOC_SINCE("26.04") constexpr auto UnwrapUnchecked(
        Unsafe) const&& noexcept VIOLET_LIFETIMEBOUND->const value_type&&
        requires(!kIsReference)
    {
        return VIOLET_MOVE(this->Data);
    }

    /// Returns the referred-to object without checking that one is present.
    ///
    /// ## Safety
    /// Undefined behaviour if this `Optional` is empty.
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto UnwrapUnchecked(Unsafe) const& noexcept -> value_type&
        requires kIsReference
    {
        return *this->Data;
    }

    /// Applies `fun` to the held value, if any, and wraps the result.
    /// Equivalent to Rust's [`Option::map`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map).
    ///
    /// If `fun` returns an lvalue reference, the result is an `Optional<U&>` referring to it; otherwise
    /// the result is stored by value.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> value = violet::Some(10);
    /// auto doubled = value.Map([](int x) -> int { return x * 2; });
    ///
    /// VIOLET_ASSERT0(doubled.Value() == 20);
    /// ```
    ///
    /// @param fun the function to apply
    template<typename Fun>
        requires(!kIsReference && callable<Fun, value_type&>)
    [[nodiscard]]
    constexpr auto Map(Fun&& fun) & noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&>())))
        -> Optional<optional_internal::map_result_t<std::invoke_result_t<Fun, value_type&>>>
    {
        using result_type = optional_internal::map_result_t<std::invoke_result_t<Fun, value_type&>>;
        if (this->HasValue()) {
            return Optional<result_type>(std::in_place, std::invoke(VIOLET_FWD(Fun, fun), this->Data));
        }

        return Nothing;
    }

    /// Overload of [`Map`] for `const` lvalues.
    /// @param fun the function to apply
    template<typename Fun>
        requires(!kIsReference && callable<Fun, const value_type&>)
    [[nodiscard]]
    constexpr auto Map(Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&>())))
        -> Optional<optional_internal::map_result_t<std::invoke_result_t<Fun, const value_type&>>>
    {
        using result_type = optional_internal::map_result_t<std::invoke_result_t<Fun, const value_type&>>;
        if (this->HasValue()) {
            return Optional<result_type>(std::in_place, std::invoke(VIOLET_FWD(Fun, fun), this->Data));
        }

        return Nothing;
    }

    /// Overload of [`Map`] for rvalues - `fun` receives the held value as an rvalue.
    ///
    /// The result is always stored by value, even if `fun` returns a reference, since that reference
    /// would most likely point into this dying `Optional`.
    ///
    /// @param fun the function to apply
    template<typename Fun>
        requires(!kIsReference && callable<Fun, value_type &&>)
    [[nodiscard]]
    constexpr auto Map(Fun&& fun) && noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&&>())))
        -> Optional<std::remove_cvref_t<std::invoke_result_t<Fun, value_type&&>>>
    {
        using result_type = std::remove_cvref_t<std::invoke_result_t<Fun, value_type&&>>;
        if (this->HasValue()) {
            return Optional<result_type>(std::in_place, std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->Data)));
        }

        return Nothing;
    }

    /// Overload of [`Map`] for `const` rvalues. The result is always stored by value.
    /// @param fun the function to apply
    template<typename Fun>
        requires(!kIsReference && callable<Fun, const value_type &&>)
    [[nodiscard]]
    constexpr auto Map(Fun&& fun) const&& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&&>())))
        -> Optional<std::remove_cvref_t<std::invoke_result_t<Fun, const value_type&&>>>
    {
        using result_type = std::remove_cvref_t<std::invoke_result_t<Fun, const value_type&&>>;
        if (this->HasValue()) {
            return Optional<result_type>(std::in_place, std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->Data)));
        }

        return Nothing;
    }

    /// Applies `fun` to the referred-to object, if any, and wraps the result.
    ///
    /// `fun` always receives `U&`, whatever the value category or constness of this `Optional`. If
    /// `fun` returns an lvalue reference, the result is an `Optional<V&>` referring to it.
    ///
    /// ## Example
    /// ```cpp
    /// struct User {
    ///     violet::String Name;
    /// };
    ///
    /// User user{.Name = "Noel"};
    /// violet::Optional<User&> ref = user;
    ///
    /// violet::Optional<violet::String&> name = ref.Map([](User& u) -> violet::String& { return u.Name; });
    /// VIOLET_ASSERT0(&name.Value() == &user.Name);
    /// ```
    ///
    /// @param fun the function to apply
    template<typename Fun>
        requires(kIsReference && callable<Fun, value_type&>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Map(Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&>())))
        -> Optional<optional_internal::map_result_t<std::invoke_result_t<Fun, value_type&>>>
    {
        using result_type = optional_internal::map_result_t<std::invoke_result_t<Fun, value_type&>>;
        if (this->HasValue()) {
            return Optional<result_type>(std::in_place, std::invoke(VIOLET_FWD(Fun, fun), *this->Data));
        }

        return Nothing;
    }

    /// Returns `true` if this `Optional` holds a value and `fun(value)` returns `true`.
    /// Equivalent to Rust's
    /// [`Option::is_some_and`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.is_some_and).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> value = violet::Some(4);
    /// VIOLET_ASSERT0(value.HasValueAnd([](int x) { return x % 2 == 0; }));
    /// ```
    ///
    /// @param fun the predicate
    /// @return `true` if a value is present and satisfies `fun`
    template<typename Fun>
        requires(!kIsReference && callable_returns<Fun, bool, value_type&>)
    [[nodiscard]]
    constexpr auto HasValueAnd(Fun&& fun) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&>()))) -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), this->Data);
    }

    /// Overload of [`HasValueAnd`] for `const` lvalues.
    /// @param fun the predicate
    /// @return `true` if a value is present and satisfies `fun`
    template<typename Fun>
        requires(!kIsReference && callable_returns<Fun, bool, const value_type&>)
    [[nodiscard]]
    constexpr auto HasValueAnd(Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&>()))) -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), this->Data);
    }

    /// Overload of [`HasValueAnd`] for rvalues - `fun` receives the held value as an rvalue.
    /// @param fun the predicate
    /// @return `true` if a value is present and satisfies `fun`
    template<typename Fun>
        requires(!kIsReference && callable_returns<Fun, bool, value_type &&>)
    [[nodiscard]]
    constexpr auto HasValueAnd(Fun&& fun) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&&>()))) -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->Data));
    }

    /// Overload of [`HasValueAnd`] for `const` rvalues.
    /// @param fun the predicate
    /// @return `true` if a value is present and satisfies `fun`
    template<typename Fun>
        requires(!kIsReference && callable_returns<Fun, bool, const value_type &&>)
    [[nodiscard]]
    constexpr auto HasValueAnd(Fun&& fun) const&& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const value_type&&>()))) -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->Data));
    }

    /// Returns `true` if this `Optional` holds a reference and `fun(object)` returns `true`.
    /// `fun` always receives `U&`, whatever the value category or constness of this `Optional`.
    ///
    /// @param fun the predicate
    /// @return `true` if a reference is present and the referred-to object satisfies `fun`
    template<typename Fun>
        requires(kIsReference && callable_returns<Fun, bool, value_type&>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto HasValueAnd(Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<value_type&>()))) -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), *this->Data);
    }

    /// Applies `fun` to the held value if there is one, otherwise returns `fallback`.
    /// Equivalent to Rust's
    /// [`Option::map_or`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map_or).
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// The result is now always returned by value (`std::remove_cvref_t<U>`). Previously, passing an
    /// lvalue `fallback` made the return type a reference, which couldn't bind to `fun`'s result.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<violet::String> name = violet::Nothing;
    /// auto length = name.MapOr(violet::UInt(0),[](const violet::String& s) { return s.size(); });
    ///
    /// VIOLET_ASSERT0(length == 0);
    /// ```
    ///
    /// @param fallback the value returned when this `Optional` is empty
    /// @param fun the function to apply
    template<typename U, typename Fun>
        requires(!kIsReference && callable<Fun, value_type&>
            && std::convertible_to<std::invoke_result_t<Fun, value_type&>, std::remove_cvref_t<U>>)
    [[nodiscard]]
    constexpr auto MapOr(U&& fallback, Fun&& fun) & -> std::remove_cvref_t<U>
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), this->Data);
        }

        return VIOLET_FWD(U, fallback);
    }

    /// Overload of [`MapOr`] for `const` lvalues.
    /// @param fallback the value returned when this `Optional` is empty
    /// @param fun the function to apply
    template<typename U, typename Fun>
        requires(!kIsReference && callable<Fun, const value_type&>
            && std::convertible_to<std::invoke_result_t<Fun, const value_type&>, std::remove_cvref_t<U>>)
    [[nodiscard]]
    constexpr auto MapOr(U&& fallback, Fun&& fun) const& -> std::remove_cvref_t<U>
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), this->Data);
        }

        return VIOLET_FWD(U, fallback);
    }

    /// Overload of [`MapOr`] for rvalues - `fun` receives the held value as an rvalue.
    /// @param fallback the value returned when this `Optional` is empty
    /// @param fun the function to apply
    template<typename U, typename Fun>
        requires(!kIsReference && callable<Fun, value_type &&>
            && std::convertible_to<std::invoke_result_t<Fun, value_type &&>, std::remove_cvref_t<U>>)
    [[nodiscard]]
    constexpr auto MapOr(U&& fallback, Fun&& fun) && -> std::remove_cvref_t<U>
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->Data));
        }

        return VIOLET_FWD(U, fallback);
    }

    /// Overload of [`MapOr`] for `const` rvalues.
    /// @param fallback the value returned when this `Optional` is empty
    /// @param fun the function to apply
    template<typename U, typename Fun>
        requires(!kIsReference && callable<Fun, const value_type &&>
            && std::convertible_to<std::invoke_result_t<Fun, const value_type &&>, std::remove_cvref_t<U>>)
    [[nodiscard]]
    constexpr auto MapOr(U&& fallback, Fun&& fun) const&& -> std::remove_cvref_t<U>
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(this->Data));
        }

        return VIOLET_FWD(U, fallback);
    }

    /// Applies `fun` to the referred-to object if there is one, otherwise returns `fallback`.
    /// `fun` always receives `U&`, whatever the value category or constness of this `Optional`.
    ///
    /// @param fallback the value returned when this `Optional` is empty
    /// @param fun the function to apply
    template<typename U, typename Fun>
        requires(kIsReference && callable<Fun, value_type&>
            && std::convertible_to<std::invoke_result_t<Fun, value_type&>, std::remove_cvref_t<U>>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto MapOr(U&& fallback, Fun&& fun) const& -> std::remove_cvref_t<U>
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), *this->Data);
        }

        return VIOLET_FWD(U, fallback);
    }

    /// Returns this `Optional` if it holds a value that satisfies `pred`, otherwise [`Nothing`].
    /// Equivalent to Rust's
    /// [`Option::filter`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.filter).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> value = violet::Some(4);
    ///
    /// VIOLET_ASSERT0(value.Filter([](const int& x) { return x % 2 == 0; }).HasValue());
    /// VIOLET_ASSERT0(!value.Filter([](const int& x) { return x > 10; }).HasValue());
    /// ```
    ///
    /// @param pred the predicate, called with the held value
    /// @return a copy of this `Optional` if `pred` returns `true`, `Nothing` otherwise
    template<typename Pred>
        requires(!kIsReference
            && std::is_copy_constructible_v<value_type> && callable_returns<Pred, bool, const value_type&>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Filter(Pred&& pred) const& -> Optional
    {
        if (this->HasValue() && std::invoke(VIOLET_FWD(Pred, pred), this->Data)) {
            return *this;
        }

        return Nothing;
    }

    /// Overload of [`Filter`] for rvalues - moves the held value into the result instead of copying it.
    /// @param pred the predicate, called with the held value
    /// @return this `Optional`, moved, if `pred` returns `true`, `Nothing` otherwise
    template<typename Pred>
        requires(!kIsReference
            && std::is_move_constructible_v<value_type> && callable_returns<Pred, bool, const value_type&>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Filter(Pred&& pred) && -> Optional
    {
        if (this->HasValue() && std::invoke(VIOLET_FWD(Pred, pred), std::as_const(this->Data))) {
            return VIOLET_MOVE(*this);
        }

        return Nothing;
    }

    /// Returns this `Optional<U&>` if it refers to an object that satisfies `pred`, otherwise [`Nothing`].
    /// `pred` receives `U&`, whatever the constness of this `Optional`.
    ///
    /// @param pred the predicate, called with the referred-to object
    /// @return an `Optional<U&>` referring to the same object if `pred` returns `true`, `Nothing` otherwise
    template<typename Pred>
        requires(kIsReference && callable_returns<Pred, bool, value_type&>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Filter(Pred&& pred) const& -> Optional
    {
        if (this->HasValue() && std::invoke(VIOLET_FWD(Pred, pred), *this->Data)) {
            return *this;
        }

        return Nothing;
    }

    /// Returns this `Optional` if it holds a value, otherwise `other`.
    /// `other` is always evaluated; use [`OrElse`] to compute it only when needed.
    ///
    /// Equivalent to Rust's [`Option::or`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.or).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> none = violet::Nothing;
    /// VIOLET_ASSERT0(none.Or(violet::Some(2)).Value() == 2);
    /// ```
    ///
    /// @param other the fallback `Optional`
    /// @return a copy of this `Optional` if it holds a value, `other` otherwise
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Or(Optional other) const& -> Optional
        requires(!kIsReference && std::is_copy_constructible_v<value_type>)
    {
        if (this->HasValue()) {
            return *this;
        }

        return other;
    }

    /// Overload of [`Or`] for rvalues - moves this `Optional` into the result instead of copying it.
    /// @param other the fallback `Optional`
    /// @return this `Optional`, moved, if it holds a value, `other` otherwise
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Or(Optional other) && -> Optional
        requires(!kIsReference && std::is_move_constructible_v<value_type>)
    {
        if (this->HasValue()) {
            return VIOLET_MOVE(*this);
        }

        return other;
    }

    /// Returns this `Optional<U&>` if it refers to an object, otherwise `other`.
    /// @param other the fallback `Optional<U&>`
    /// @return an `Optional<U&>` referring to this or `other`'s object
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Or(Optional other) const& noexcept -> Optional
        requires kIsReference
    {
        if (this->HasValue()) {
            return *this;
        }

        return other;
    }

    /// Returns this `Optional` if it holds a value, otherwise the result of calling `fun`.
    /// Equivalent to Rust's
    /// [`Option::or_else`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.or_else).
    ///
    /// ## Example
    /// ```cpp
    /// auto FromEnv() -> violet::Optional<violet::String>;
    /// auto FromConfig() -> violet::Optional<violet::String>;
    ///
    /// auto token = FromEnv().OrElse([] { return FromConfig(); });
    /// ```
    ///
    /// @param fun called with no arguments to produce the fallback `Optional`
    /// @return a copy of this `Optional` if it holds a value, `fun()` otherwise
    template<typename Fun>
        requires(!kIsReference && std::is_copy_constructible_v<value_type> && callable_returns<Fun, Optional>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto OrElse(Fun&& fun) const& -> Optional
    {
        if (this->HasValue()) {
            return *this;
        }

        return std::invoke(VIOLET_FWD(Fun, fun));
    }

    /// Overload of [`OrElse`] for rvalues - moves this `Optional` into the result instead of copying it.
    /// @param fun called with no arguments to produce the fallback `Optional`
    /// @return this `Optional`, moved, if it holds a value, `fun()` otherwise
    template<typename Fun>
        requires(!kIsReference && std::is_move_constructible_v<value_type> && callable_returns<Fun, Optional>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto OrElse(Fun&& fun) && -> Optional
    {
        if (this->HasValue()) {
            return VIOLET_MOVE(*this);
        }

        return std::invoke(VIOLET_FWD(Fun, fun));
    }

    /// Returns this `Optional<U&>` if it refers to an object, otherwise the result of calling `fun`.
    /// @param fun called with no arguments to produce the fallback `Optional<U&>`
    /// @return an `Optional<U&>` referring to this or `fun()`'s object
    template<typename Fun>
        requires(kIsReference && callable_returns<Fun, Optional>)
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto OrElse(Fun&& fun) const& -> Optional
    {
        if (this->HasValue()) {
            return *this;
        }

        return std::invoke(VIOLET_FWD(Fun, fun));
    }

    /// Returns whichever of this `Optional` and `other` holds a value, if exactly one does; otherwise
    /// [`Nothing`].
    /// Equivalent to Rust's [`Option::xor`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.xor).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> some = violet::Some(1);
    /// violet::Optional<int> none = violet::Nothing;
    ///
    /// VIOLET_ASSERT0(some.Xor(none).Value() == 1);
    /// VIOLET_ASSERT0(!some.Xor(violet::Some(2)).HasValue());
    /// VIOLET_ASSERT0(!none.Xor(violet::Nothing).HasValue());
    /// ```
    ///
    /// @param other the `Optional` to compare against
    /// @return the only engaged `Optional` of the two, or `Nothing`
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Xor(Optional other) const& -> Optional
        requires(!kIsReference && std::is_copy_constructible_v<value_type>)
    {
        if (this->HasValue() && !other.HasValue()) {
            return *this;
        }

        if (!this->HasValue() && other.HasValue()) {
            return other;
        }

        return Nothing;
    }

    /// Overload of [`Xor`] for rvalues - moves this `Optional` into the result instead of copying it.
    /// @param other the `Optional` to compare against
    /// @return the only engaged `Optional` of the two, or `Nothing`
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Xor(Optional other) && -> Optional
        requires(!kIsReference && std::is_move_constructible_v<value_type>)
    {
        if (this->HasValue() && !other.HasValue()) {
            return VIOLET_MOVE(*this);
        }

        if (!this->HasValue() && other.HasValue()) {
            return other;
        }

        return Nothing;
    }

    /// Returns whichever of this `Optional<U&>` and `other` refers to an object, if exactly one does;
    /// otherwise [`Nothing`].
    ///
    /// @param other the `Optional<U&>` to compare against
    /// @return the only engaged `Optional<U&>` of the two, or `Nothing`
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Xor(Optional other) const& noexcept -> Optional
        requires kIsReference
    {
        if (this->HasValue() && !other.HasValue()) {
            return *this;
        }

        if (!this->HasValue() && other.HasValue()) {
            return other;
        }

        return Nothing;
    }

    /// Takes the held value out, leaving this `Optional` empty.
    /// Equivalent to Rust's [`Option::take`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.take).
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// For `Optional<U&>`, this returns an `Optional<U&>` referring to the same object and leaves this
    /// one empty; the referred-to object is never copied or moved.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> value = violet::Some(1);
    /// auto taken = value.Take();
    ///
    /// VIOLET_ASSERT0(!value.HasValue());
    /// VIOLET_ASSERT0(taken.Value() == 1);
    /// ```
    ///
    /// @return the previous contents of this `Optional`
    [[nodiscard]]
    constexpr auto Take() noexcept(kIsReference || std::is_nothrow_move_constructible_v<value_type>) -> Optional
        requires(kIsReference || std::is_move_constructible_v<value_type>)
    {
        Optional taken(VIOLET_MOVE(*this));
        this->Disengage();

        return taken;
    }

    /// Destroys any held value and constructs a new one in place from `args`.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<violet::String> name = violet::Nothing;
    /// name.Replace("Noel");
    ///
    /// VIOLET_ASSERT0(name.Value() == "Noel");
    /// ```
    ///
    /// @param args arguments forwarded to `T`'s constructor
    /// @return a reference to the new value
    template<typename... Args>
        requires(!kIsReference && std::is_constructible_v<T, Args...>)
    VIOLET_REINITIALIZES_MEMORY constexpr auto Replace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) VIOLET_LIFETIMEBOUND -> value_type&
    {
        this->Construct(VIOLET_FWD(Args, args)...);
        return this->Data;
    }

    /// Rebinds this `Optional<U&>` to refer to `value`.
    /// The previously referred-to object is left untouched.
    ///
    /// @param value the object to refer to; it must outlive every use of this `Optional`
    /// @return the newly referred-to object
    NOELDOC_SINCE("26.11")
    VIOLET_REINITIALIZES_MEMORY constexpr auto Replace(value_type& value) noexcept -> value_type&
        requires kIsReference
    {
        this->Construct(value);
        return *this->Data;
    }

    NOELDOC_SINCE("26.11")
    constexpr auto Replace(value_type&&) -> value_type&
        requires kIsReference
    = delete;

    /// Takes the held value out, leaving this `Optional` empty, but only if `pred` returns `true` for it.
    /// `pred` receives a mutable reference, so it may modify the value before deciding.
    ///
    /// Equivalent to Rust's
    /// [`Option::take_if`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.take_if).
    ///
    /// ## Remarks
    /// For `Optional<U&>`, this takes the reference, not the referred-to object, like [`Take`].
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> value = violet::Some(4);
    ///
    /// auto odd = value.TakeIf([](int& x) { return x % 2 != 0; });
    /// VIOLET_ASSERT0(!odd.HasValue() && value.HasValue());
    ///
    /// auto even = value.TakeIf([](int& x) { return x % 2 == 0; });
    /// VIOLET_ASSERT0(even.Value() == 4 && !value.HasValue());
    /// ```
    ///
    /// @param pred the predicate, called with the held value
    /// @return the previous contents if `pred` returned `true`, `Nothing` otherwise
    template<typename Pred>
        requires(
            callable_returns<Pred, bool, value_type&> && (kIsReference || std::is_move_constructible_v<value_type>))
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto TakeIf(Pred&& pred) -> Optional
    {
        if (this->HasValue() && std::invoke(VIOLET_FWD(Pred, pred), this->Value())) {
            return this->Take();
        }

        return Nothing;
    }

    /// Returns a reference to the held value, inserting `value` first if this `Optional` is empty.
    /// `value` is always evaluated; it is discarded if a value is already present.
    ///
    /// Equivalent to Rust's
    /// [`Option::get_or_insert`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.get_or_insert).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<int> none = violet::Nothing;
    /// none.GetOrInsert(5) += 1;
    ///
    /// VIOLET_ASSERT0(none.Value() == 6);
    /// ```
    ///
    /// @param value the value to insert if this `Optional` is empty
    /// @return a reference to the held value
    NOELDOC_SINCE("26.11")
    constexpr auto GetOrInsert(unqualified_type value) & noexcept(
        std::is_nothrow_constructible_v<T, unqualified_type&&>) VIOLET_LIFETIMEBOUND -> value_type&
        requires(!kIsReference && std::is_constructible_v<T, unqualified_type &&>)
    {
        if (!this->HasValue()) {
            this->Construct(VIOLET_MOVE(value));
        }

        return this->Data;
    }

    /// Returns the referred-to object, binding this `Optional<U&>` to `value` first if it is empty.
    /// @param value the object to refer to if this `Optional` is empty; it must outlive every use of this `Optional`
    /// @return the referred-to object
    NOELDOC_SINCE("26.11")
    constexpr auto GetOrInsert(value_type& value) & noexcept -> value_type&
        requires kIsReference
    {
        if (!this->HasValue()) {
            this->Construct(value);
        }

        return *this->Data;
    }

    NOELDOC_SINCE("26.11")
    constexpr auto GetOrInsert(value_type&&) & -> value_type&
        requires kIsReference
    = delete;

    /// Returns a reference to the held value, inserting a value-initialized `T` first if this
    /// `Optional` is empty.
    ///
    /// Not available for `Optional<U&>`, since there is no object a default could refer to.
    ///
    /// Equivalent to Rust's
    /// [`Option::get_or_insert_default`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.get_or_insert_default).
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<violet::Vec<int>> items = violet::Nothing;
    /// items.GetOrInsertDefault().push_back(1);
    ///
    /// VIOLET_ASSERT0(items->size() == 1);
    /// ```
    ///
    /// @return a reference to the held value
    NOELDOC_SINCE("26.11")
    constexpr auto GetOrInsertDefault() & noexcept(std::is_nothrow_default_constructible_v<T>) VIOLET_LIFETIMEBOUND
        -> value_type&
        requires(!kIsReference && std::is_default_constructible_v<T>)
    {
        if (!this->HasValue()) {
            this->Construct();
        }

        return this->Data;
    }

    /// Empties this `Optional`, destroying the held value if there is one.
    VIOLET_REINITIALIZES_MEMORY
    constexpr void Reset() noexcept
    {
        this->Disengage();
    }

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Converts to a [`std::optional`] holding a copy of the value, if any.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This is now guarded by the `BACKWARDS_STL_OPTIONAL` feature, and is unavailable for
    /// `Optional<U&>`, since `std::optional<U&>` does not exist before C++26.
    ///
    /// @return a `std::optional` with the same contents
    template<typename U = T>
        requires(!kIsReference && std::same_as<U, T> && std::is_copy_constructible_v<T>)
    constexpr VIOLET_EXPLICIT operator std::optional<U>() const& noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        if (this->HasValue()) {
            return std::optional<T>(std::in_place, this->Data);
        }

        return std::nullopt;
    }

    /// Converts to a [`std::optional`] by moving the value out, if any.
    /// @return a `std::optional` with the same contents
    template<typename U = T>
        requires(!kIsReference && std::same_as<U, T> && std::is_move_constructible_v<T>)
    NOELDOC_SINCE("26.11")
    constexpr VIOLET_EXPLICIT operator std::optional<U>() && noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        if (this->HasValue()) {
            return std::optional<T>(std::in_place, VIOLET_MOVE(this->Data));
        }

        return std::nullopt;
    }
#endif

    /// Returns `true` if this `Optional` is empty.
    [[nodiscard]]
    constexpr auto operator==(nothing_t) const noexcept -> bool
    {
        return !this->HasValue();
    }

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Returns `true` if this `Optional` is empty.
    [[nodiscard]]
    constexpr auto operator==(std::nullopt_t) const noexcept -> bool
    {
        return !this->HasValue();
    }
#endif

    /// Compares two `Optional`s, they're equal if both are empty, or both hold values that compare equal.
    /// For `Optional<U&>`, this compares the referred-to objects, not their addresses.
    ///
    /// @param other the `Optional` to compare with
    /// @return `true` if both are empty or both values are equal
    [[nodiscard]]
    constexpr auto operator==(const Optional& other) const -> bool
        requires(std::equality_comparable<value_type>)
    {
        if (this->HasValue() != other.HasValue()) {
            return false;
        }

        return !this->HasValue() || this->Value() == other.Value();
    }

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Compares with a [`std::optional`]; equal if both are empty, or both hold values that compare equal.
    /// @param other the `std::optional` to compare with
    /// @return `true` if both are empty or both values are equal
    template<typename U>
        requires(!kIsReference && std::same_as<U, T> && std::equality_comparable<value_type>)
    [[nodiscard]]
    constexpr auto operator==(const std::optional<U>& other) const -> bool
    {
        if (this->HasValue() != other.has_value()) {
            return false;
        }

        return !this->HasValue() || this->Value() == *other;
    }
#endif

    /// Returns `true` if this `Optional` holds a value equal to the one wrapped in `other`.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// Also available for `Optional<U&>`, comparing the referred-to object with a `Some<U>`.
    ///
    /// @param other the wrapped value to compare with
    /// @return `true` if a value is present and equal to `other`'s
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto operator==(const Some<unqualified_type>& other) const -> bool
        requires(std::equality_comparable<value_type>)
    {
        return this->HasValue() && this->Value() == other.n_value;
    }

    /// Returns `true` if this `Optional` holds a value equal to `other`.
    ///
    /// `other` can be any type the held value compares equal with, like `std::optional`'s
    /// heterogeneous comparison.
    ///
    /// ## Remarks
    /// ### Since `26.11`
    /// This now accepts any type comparable with `T` instead of only `const T&`. Comparing with a
    /// value that converts to both `T` and `Optional<T>`, such as a string literal against
    /// `Optional<String>`, was ambiguous before. `Optional<U&>` can also be compared with `const`
    /// objects and temporaries.
    ///
    /// ## Example
    /// ```cpp
    /// violet::Optional<violet::String> name = violet::String("Noel");
    /// VIOLET_ASSERT0(name == "Noel");
    /// ```
    ///
    /// @tparam U the type of the value to compare with
    /// @param other the value to compare with
    /// @return `true` if a value is present and equal to `other`
    template<typename U>
    // clang-format off
    requires(
        !instanceof_v<Optional, std::remove_cvref_t<U>> &&
        !instanceof_v<Some, std::remove_cvref_t<U>> &&
        !std::same_as<std::remove_cvref_t<U>, nothing_t> &&
        #if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
        !instanceof_v<std::optional, std::remove_cvref_t<U>> &&
        !std::same_as<std::remove_cvref_t<U>, std::nullopt_t> &&
        #endif
        requires(const value_type& lhs, const U& rhs) {
            { lhs == rhs } -> std::convertible_to<bool>;
        }
    )
    // clang-format on
    [[nodiscard]]
    constexpr auto operator==(const U& other) const -> bool
    {
        return this->HasValue() && this->Value() == other;
    }

    /// Orders this `Optional` against [`Nothing`]; an empty `Optional` is equal to it, and an engaged
    /// one is greater.
    ///
    /// @return the ordering of `HasValue()` against `false`
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto operator<=>(nothing_t) const noexcept -> std::strong_ordering
    {
        return this->HasValue() <=> false;
    }

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Orders this `Optional` against [`std::nullopt`]; an empty `Optional` is equal to it, and an
    /// engaged one is greater.
    ///
    /// @return the ordering of `HasValue()` against `false`
    [[nodiscard]]
    NOELDOC_SINCE("26.05.08") constexpr auto operator<=>(std::nullopt_t) const noexcept -> std::strong_ordering
    {
        return this->HasValue() <=> false;
    }
#endif

    /// Orders two `Optional`s. An empty `Optional` is less than any engaged one; two engaged ones are
    /// ordered by their values.
    ///
    /// @param other the `Optional` to compare with
    /// @return the ordering of the two `Optional`s
    template<typename U = value_type>
        requires(std::three_way_comparable<U>)
    [[nodiscard]]
    NOELDOC_SINCE("26.05.08") constexpr auto operator<=>(const Optional& other) const
        -> std::compare_three_way_result_t<U>
    {
        if (this->HasValue() && other.HasValue()) {
            return this->Value() <=> other.Value();
        }

        return this->HasValue() <=> other.HasValue();
    }

    /// Orders two `Optional`s whose values have `<` but no `<=>`.
    /// @param other the `Optional` to compare with
    /// @return the ordering of the two `Optional`s
    [[nodiscard]]
    NOELDOC_SINCE("26.05.08") constexpr auto operator<=>(const Optional& other) const -> std::partial_ordering
        requires(!std::three_way_comparable<value_type> && std::totally_ordered<value_type>)
    {
        if (this->HasValue() && other.HasValue()) {
            if (this->Value() < other.Value()) {
                return std::partial_ordering::less;
            }

            if (other.Value() < this->Value()) {
                return std::partial_ordering::greater;
            }

            return std::partial_ordering::equivalent;
        }

        return this->HasValue() <=> other.HasValue();
    }

#if VIOLET_FEATURE(BACKWARDS_STL_OPTIONAL)
    /// Orders this `Optional` against a [`std::optional`], with the same rules as comparing two `Optional`s.
    /// @param other the `std::optional` to compare with
    /// @return the ordering of the two optionals
    template<typename U>
        requires(!kIsReference && std::same_as<U, T> && std::three_way_comparable<U>)
    [[nodiscard]]
    NOELDOC_SINCE("26.05.08") constexpr auto operator<=>(const std::optional<U>& other) const
        -> std::compare_three_way_result_t<U>
    {
        if (this->HasValue() && other.has_value()) {
            return this->Value() <=> *other;
        }

        return this->HasValue() <=> other.has_value();
    }
#endif

    /// Orders this `Optional` against a plain value. An empty `Optional` is less than any value.
    /// @param other the value to compare with
    /// @return the ordering of the held value against `other`
    template<typename U = value_type>
        requires(std::three_way_comparable<U>)
    [[nodiscard]]
    NOELDOC_SINCE("26.05.08") constexpr auto operator<=>(const value_type& other) const
        -> std::compare_three_way_result_t<U>
    {
        if (this->HasValue()) {
            return this->Value() <=> other;
        }

        return std::strong_ordering::less;
    }

    /// Returns a string representation of the held value, or `«no value»` if this `Optional` is empty.
    /// @return the formatted value
    [[nodiscard]]
    auto ToString() const noexcept -> String
    {
        if (!this->HasValue()) {
            return "«no value»";
        }

        return violet::ToString(this->Value());
    }

    /// Writes [`ToString`] to `os`.
    friend auto operator<<(std::ostream& os, const Optional& self) -> std::ostream&
    {
        return os << self.ToString();
    }
};

static_assert(sizeof(Optional<int&>) == sizeof(int*));
static_assert(std::is_trivially_copyable_v<Optional<int>>);
static_assert(std::is_trivially_copyable_v<Optional<int&>>);
static_assert(!std::is_constructible_v<Optional<const int&>, int>);

} // namespace violet

VIOLET_FORMATTER_TEMPLATE(violet::Optional<T>, typename T);

#define __violet_unwrap_impl__(decl, expr, tmp)                                                                        \
    auto&& tmp = (expr);                                                                                               \
    static_assert(::violet::instanceof_v<::violet::Optional, ::std::remove_cvref_t<decltype(tmp)>>,                    \
        "expression didn't return a violet::Optional");                                                                \
    if (!(tmp).HasValue()) {                                                                                           \
        return ::violet::Nothing;                                                                                      \
    }                                                                                                                  \
    decl = *VIOLET_FWD(decltype(tmp), tmp)

/**
 * @macro VIOLET_UNWRAP
 * @param decl the declaration to initialize, such as `const auto val` or `auto& ref`
 * @param expr an expression producing a [`violet::Optional`]
 * @since 26.11
 *
 * Declares `decl` from the value held by an [`Optional`], or returns [`Nothing`] from the enclosing
 * function if it is empty.
 *
 * This is the `Optional` counterpart to [`VIOLET_TRY`], in declaration form, so it works on every
 * compiler (no statement expressions involved). The enclosing function must return a type that
 * [`Nothing`] converts to, such as any `Optional<U>`.
 *
 * If `expr` is an lvalue, its value is copied into `decl`; if it is a temporary, the value is moved.
 * For `Optional<U&>`, `decl` binds to or copies the referred-to object, depending on how it is
 * declared.
 *
 * ## Remarks
 * This expands to several statements, so it cannot be used as the unbraced body of an `if`, `for`,
 * or `while`. It also uses `return`, so it can't be used in coroutines.
 *
 * ## Example
 * ```cpp
 * #include <violet/Container/Optional.h>
 *
 * auto FindUser(int id) -> violet::Optional<User>;
 *
 * auto FindUserName(int id) -> violet::Optional<violet::String>
 * {
 *     VIOLET_UNWRAP(const auto user, FindUser(id));
 *     return violet::Some(user.Name);
 * }
 * ```
 */
#define VIOLET_UNWRAP(decl, expr) __violet_unwrap_impl__(decl, expr, VIOLET_UNIQUE_NAME(__violet_unwrap_expr_))
