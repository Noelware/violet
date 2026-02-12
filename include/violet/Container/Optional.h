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

#include <violet/Support/Demangle.h>
#include <violet/Violet.h>

#include <optional>
#include <source_location>

namespace violet {

template<typename T>
struct Optional;

/// Marker for representing a empty `Optional<T>`.
constexpr static std::nullopt_t Nothing = std::nullopt;

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
struct is_optional: std::false_type {};

template<typename T>
struct is_optional<Optional<T>>: std::true_type {};

template<typename T>
struct is_optional<std::optional<T>>: std::true_type {};

template<typename T>
static constexpr bool is_optional_v = is_optional<T>::value;

/// Type trait to extract the inner type from an [`Optional`] or [`std::optional`].
///
/// The primary template (`T`) is intentionally left undefined. It is meant to be specialized
/// on arbitrary optional types, most used in the `violet/Iterator.h` header.
///
/// @tparam T The type from which to extract an inner value type.
template<class T>
struct optional_type;

/// Specialization of [`optional_type`] for [`violet::Optional`].
template<class U>
struct optional_type<Optional<U>> {
    /// Extracted inner type from [`violet::Optional`]
    using type = U;
};

/// Specialization of [`optional_type`] for [`std::optional`].
template<class U>
struct optional_type<std::optional<U>> {
    /// Extracted inner type from [`violet::Optional`]
    using type = U;
};

/// Convenience alias for accessing the extracted inner type.
///
/// It is the equivalent to `typename violet::optional_type<T>::type`.
///
/// @tparam T which optional wrapper whose inner type should be extracted.
template<class T>
using optional_type_t = typename optional_type<T>::type;

/// Constructs an [`Optional`] containing a value.
///
/// @tparam T value type to store
/// @tparam Args argument types that are forwarded to `T`'s constructor
/// @param args arguments that are forwarded to `T`'s constructor
template<typename T, typename... Args>
constexpr static auto Some(Args&&... args) -> Optional<T>
{
    return Optional<T>(std::in_place, VIOLET_FWD(Args, args)...);
}

// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)

/// A container type that may or may not contain a value, analogous to Rust's [`Option`] enumeration
/// and C++'s [`std::optional`] with functional-style utilities.
///
/// [`Option`]: https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html
///
/// ## Type Requirements
/// - Be an object type
/// - Not be a reference, use [`std::remove_cvref_t`] to remove const and volatile requirements.
/// - Be destructible, copyable, and movable.
///
/// ## Examples
/// ```cpp
/// #include <violet/Container/Optional.h>
///
/// using violet::Some;
/// using violet::Optional;
/// using violet::Nothing;
///
/// auto value = Some<int>(10);
/// auto doubled = value.Map([](int val) -> int { return val * 2; });
/// VIOLET_ASSERT(value.HasValue(), "doctest failed: no value was provided");
/// VIOLET_ASSERT(*value == 20, "doctest failed: didn't double up");
///
/// Optional<int> empty = Nothing;
/// VIOLET_ASSERT(empty.UnwrapOr(5) == 5, "doctest failed: wasn't `5`");
/// ```
///
/// ## Panic handling / Exceptions
/// Intentionally, since Violet supports both C++ exceptions and exception-free code, it'll depend
/// on if it is enabled or not.
///
/// If `-fno-exceptions` (Clang/GCC), `/...` (MSVC) was passed in, then Violet will send a panic message
/// in the process' standard error and fully abort the process via [`std::unreachable`] or with compiler-available
/// instrinstics if [`std::unreachable`] isn't available.
///
/// Otherwise, all `Unwrap` and `Expect` will throw a [`std::logic_error`] with the panic message.
///
/// ### Debug Assertions
/// This type also triggers debug assertions via the [`VIOLET_DEBUG_ASSERT`] macro. You can disable this in production
/// with the `-DNDEBUG` flag, which will remove unnecessary assertions.
template<typename T>
struct [[nodiscard("check its state before discarding")]] VIOLET_API Optional final {
    static_assert(std::is_object_v<T>, "`Optional<T>` requires `T` to be an object type");
    static_assert(!std::is_reference_v<T>, "`Optional<T>` cannot wrap a reference");
    static_assert(std::is_destructible_v<T>, "`Optional<T>` requires `T` to be destructible");
    static_assert(
        std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>, "Optional<T> must be movable or copyable");

    /// Contained value type of this [`Optional`].
    using value_type = T;

    /// Constructs a empty optional value.
    constexpr VIOLET_IMPLICIT Optional() noexcept {}

    /// Constructs a empty optional value from [`std::nullopt`] or [`violet::Nothing`].
    constexpr VIOLET_IMPLICIT Optional(std::nullopt_t) noexcept
        : Optional()
    {
    }

    /// Constructs a empty optional value from a copied [`std::nullopt`] or [`violet::Nothing`].
    constexpr VIOLET_IMPLICIT Optional(std::nullopt_t&) noexcept
        : Optional()
    {
    }

    /// Constructs an engaged `Optional<T>` in-place. The contained value is constructed directly from `args...`.
    ///
    /// ## Exception Safety
    /// Propagates exceptions from `T`'s constructor unless `noexcept`.
    ///
    /// @param args arguments forwarded to `T`'s constructor.
    template<typename... Args>
        requires(std::is_constructible_v<T, Args...>)
    constexpr VIOLET_EXPLICIT Optional(std::in_place_t, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>)
        : n_engaged(true)
    {
        ::new (&this->n_value) T(VIOLET_FWD(Args, args)...);
    }

    /// Constructs an engaged [`Optional`] from `other`.
    ///
    /// This type will participate in overload resolution only if:
    /// - `U` is not [`Optional`] or [`std::optional`]
    /// - `T` is constructible from `U`.
    template<typename U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, Optional<T>>
            && !violet::instanceof_v<std::optional, std::remove_cvref_t<U>> && std::constructible_from<T, const U&>)
    constexpr VIOLET_IMPLICIT Optional(const U& other) noexcept(std::is_nothrow_constructible_v<T, const U&>)
        : Optional(std::in_place, other)
    {
    }

    /// Constructs an engaged [`Optional`] by moving `other`.
    ///
    /// This type will participate in overload resolution only if:
    /// - `U` is not [`Optional`] or [`std::optional`]
    /// - `T` is constructible from `U`.
    template<typename U = T>
        requires(!std::is_same_v<std::remove_cvref_t<U>, Optional<T>>
            && !violet::instanceof_v<std::optional, std::remove_cvref_t<U>> && std::constructible_from<T, U &&>)
    constexpr VIOLET_IMPLICIT Optional(U&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : Optional(std::in_place, VIOLET_FWD(U, other))
    {
    }

    /// Constructs from an existing [`std::optional`].
    ///
    /// If `other` contains a value, constructs `T` from `*other`. Otherwise, creates a
    // disengaged `Optional`.
    template<typename U>
        requires(!std::is_same_v<std::remove_cvref_t<U>, Optional> && std::constructible_from<T, U>)
    constexpr VIOLET_IMPLICIT Optional(const std::optional<U>& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : n_engaged(other.has_value())
    {
        if (other.has_value()) {
            ::new (&this->n_value) T(*other);
            this->n_engaged = true;
        } else {
            this->n_engaged = false;
        }
    }

    /// Constructs from an existing [`std::optional`] by move.
    ///
    /// If engaged, moves the value and resets `other`.
    template<typename U>
        requires(!std::is_same_v<std::remove_cvref_t<U>, Optional> && std::constructible_from<T, U>)
    constexpr VIOLET_IMPLICIT Optional(
        std::optional<U>&& other // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        ) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_engaged(other.has_value())
    {
        if (other.has_value()) {
            ::new (&this->n_value) T(VIOLET_MOVE(*other));
            this->n_engaged = true;
        } else {
            this->n_engaged = false;
        }
    }

    /// Copy-constructs from another `Optional`.
    ///
    /// If `other` is engaged, the contained value is copy-constructed. Otherwise, the result is disengaged.
    constexpr VIOLET_IMPLICIT Optional(const Optional& other)
        : n_engaged(other.n_engaged)
    {
        if (other.n_engaged) {
            ::new (&this->n_value) T(other.n_value);
        }
    }

    /// Copy-assigns from another `Optional`.
    ///
    /// - If both are engaged, copy-assigns the contained value.
    /// - If only `other` is engaged, constructs a new value.
    /// - If `other` is disengaged, this becomes disengaged.
    constexpr auto operator=(const Optional& other) noexcept(
        std::is_nothrow_copy_assignable_v<T> || std::is_nothrow_copy_constructible_v<T>) -> Optional&
        requires(std::is_copy_assignable_v<T> || std::is_copy_constructible_v<T>)
    {
        if (this != &other) {
            if (other.n_engaged) {
                if (this->n_engaged) {
                    this->getValueRef() = other.getValueRef();
                } else {
                    ::new (&this->n_value) T(other.getValueRef());
                    this->n_engaged = true;
                }
            } else {
                this->destroy();
            }
        }

        return *this;
    }

    /// Move-constructs from another `Optional`.
    ///
    /// If engaged, move-constructs the contained value
    /// and disengages `other`.
    constexpr VIOLET_IMPLICIT Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_engaged(other.n_engaged)
    {
        if (other.n_engaged) {
            ::new (&this->n_value) T(VIOLET_MOVE(other.n_value));
            other.n_engaged = false;
        }
    }

    /// Move-assigns from another `Optional`.
    ///
    /// - If both are engaged, move-assigns.
    /// - If only `other` is engaged, constructs.
    /// - If `other` is disengaged, this becomes disengaged.
    ///
    /// Leaves `other` disengaged.
    constexpr auto operator=(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>) -> Optional&
    {
        if (this != &other) {
            if (other.n_engaged) {
                if (this->n_engaged) {
                    this->getValueRef() = VIOLET_MOVE(other.getValueRef());
                } else {
                    ::new (&this->n_value) T(VIOLET_MOVE(other.getValueRef()));
                    this->n_engaged = true;
                }

                other.destroy();
            } else if (this->HasValue()) {
                this->destroy();
            }
        }

        return *this;
    }

    constexpr ~Optional() noexcept(std::is_nothrow_destructible_v<T>)
    {
        this->destroy();
    }

    /// Disengages the `Optional`, destroying the contained value if present.
    constexpr auto operator=(std::nullopt_t) noexcept -> Optional&
    {
        this->destroy();
        return *this;
    }

    /// Disengages the `Optional`, destroying the contained value if present.
    constexpr auto operator=(std::nullopt_t&) noexcept -> Optional&
    {
        this->destroy();
        return *this;
    }

    /// Assigns from `std::optional<T>`.
    ///
    /// Mirrors assignment semantics of `Optional<T>`.
    constexpr auto operator=(std::optional<T>& other) noexcept(
        std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) -> Optional&
        requires(std::is_copy_assignable_v<T> || std::is_copy_constructible_v<T>)
    {
        if (other.has_value()) {
            if (this->n_engaged) {
                *this->getValueRef() = *other;
            } else {
                ::new (&this->n_value) T(*other);
                this->n_engaged = true;
            }
        } else {
            this->destroy();
        }

        return *this;
    }

    /// Assigns from `std::optional<T>` by move.
    ///
    /// Resets `other` if engaged.
    constexpr auto operator=(std::optional<T>&& other) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) -> Optional&
        requires(std::is_move_assignable_v<T> || std::is_move_constructible_v<T>)
    {
        if (other.has_value()) {
            if (this->n_engaged) {
                *this->getValueRef() = *other;
            } else {
                ::new (&this->n_value) T(*other);
                this->n_engaged = true;
            }

            other.reset();
        } else {
            this->destroy();
        }

        (void)VIOLET_MOVE(other);
        return *this;
    }

    /// Assigns a new value to the `Optional`.
    ///
    /// If engaged, assigns to the contained value. Otherwise, constructs a new value in-place.
    constexpr auto operator=(const T& value) noexcept(
        std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) -> Optional&
    {
        if (this->n_engaged) {
            this->getValueRef() = value;
        } else {
            ::new (&this->n_value) T(value);
            this->n_engaged = true;
        }
        return *this;
    }

    /// Assigns a new value to the `Optional` by move.
    ///
    /// If engaged, move-assigns. Otherwise, constructs a new value in-place.
    constexpr auto operator=(T&& value) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) -> Optional&
    {
        if (this->n_engaged) {
            this->getValueRef() = VIOLET_MOVE(value);
        } else {
            ::new (&this->n_value) T(VIOLET_MOVE(value));
            this->n_engaged = true;
        }
        return *this;
    }

    /// Returns `true` if a value is present.
    constexpr auto HasValue() const noexcept -> bool
    {
        return this->n_engaged;
    }

    /// Returns a reference to the contained value.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if a value is present.
    constexpr auto Value() & noexcept -> T&
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "`Optional<T>` represents nothing");
        return this->getValueRef();
    }

    /// Returns a reference to the contained value.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if a value is present.
    constexpr auto Value() const& noexcept -> const T&
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "`Optional<T>` represents nothing");
        return this->getValueRef();
    }

    /// Returns a reference to the contained value.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if a value is present.
    constexpr auto Value() && noexcept -> T&&
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "`Optional<T>` represents nothing");
        return VIOLET_MOVE(this->getValueRef());
    }

    /// Returns a reference to the contained value.
    ///
    /// ## Panics
    /// This will provide a debug assertion to check if a value is present.
    constexpr auto Value() const&& noexcept -> const T&&
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "`Optional<T>` represents nothing");
        return VIOLET_MOVE(this->getValueRef());
    }

    /// Forefully retrieve a value from this container or panics if no value was present.
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
    constexpr auto Unwrap(std::source_location loc = std::source_location::current()) &
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return this->getValueRef();
        }

        panicUnexpectly("tried to unwrap nothing", loc);
    }

    /// Forefully retrieve a value from this container or panics if no value was present.
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
    constexpr auto Unwrap(std::source_location loc = std::source_location::current()) &&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(this->getValueRef());
        }

        panicUnexpectly("tried to unwrap nothing", loc);
    }

    /// Forefully retrieve a value from this container or panics if no value was present.
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
    constexpr auto Unwrap(std::source_location loc = std::source_location::current()) const&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return this->getValueRef();
        }

        panicUnexpectly("tried to unwrap nothing", loc);
    }

    /// Forefully retrieve a value from this container or panics if no value was present.
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
    constexpr auto Unwrap(std::source_location loc = std::source_location::current()) const&&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(this->getValueRef());
        }

        panicUnexpectly("tried to unwrap nothing", loc);
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
    constexpr auto Except(Str message, std::source_location loc = std::source_location::current()) &
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return this->getValueRef();
        }

        panicUnexpectly(message, loc);
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
    constexpr auto Except(Str message, std::source_location loc = std::source_location::current()) &&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(this->getValueRef());
        }

        panicUnexpectly(message, loc);
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
    constexpr auto Except(Str message, std::source_location loc = std::source_location::current()) const&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return this->getValueRef();
        }

        panicUnexpectly(message, loc);
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
    constexpr auto Except(Str message, std::source_location loc = std::source_location::current()) const&&
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
        -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(this->getValueRef());
        }

        panicUnexpectly(message, loc);
    }

    /// Returns the contained value if present, otherwise returns `defaultValue`.
    constexpr auto UnwrapOr(T&& defaultValue) & noexcept -> T
    {
        return this->HasValue() ? this->getValueRef() : VIOLET_MOVE(defaultValue);
    }

    /// Returns the contained value if present, otherwise returns `defaultValue`.
    constexpr auto UnwrapOr(T&& defaultValue) && noexcept -> T
    {
        return this->HasValue() ? VIOLET_MOVE(this->getValueRef()) : VIOLET_MOVE(defaultValue);
    }

    /// Returns the contained value if present, otherwise returns `defaultValue`.
    constexpr auto UnwrapOr(T&& defaultValue) const& noexcept -> T
    {
        return this->HasValue() ? this->getValueRef() : VIOLET_MOVE(defaultValue);
    }

    /// Returns the contained value if present, otherwise returns `defaultValue`.
    constexpr auto UnwrapOr(T&& defaultValue) const&& noexcept -> T
    {
        return this->HasValue() ? VIOLET_MOVE(this->getValueRef()) : VIOLET_MOVE(defaultValue);
    }

    /// Returns the contained value without checking engagement.
    ///
    /// # Safety
    /// Undefined behavior if no value is present.
    constexpr auto UnwrapUnchecked(Unsafe) & noexcept -> T&
    {
        return this->getValueRef();
    }

    /// Returns the contained value without checking engagement.
    ///
    /// # Safety
    /// Undefined behavior if no value is present.
    constexpr auto UnwrapUnchecked(Unsafe) const& noexcept -> const T&
    {
        return this->getValueRef();
    }

    /// Returns the contained value without checking engagement.
    ///
    /// # Safety
    /// Undefined behavior if no value is present.
    constexpr auto UnwrapUnchecked(Unsafe) && noexcept -> T&&
    {
        return VIOLET_MOVE(this->getValueRef());
    }

    /// Returns the contained value without checking engagement.
    ///
    /// # Safety
    /// Undefined behavior if no value is present.
    constexpr auto UnwrapUnchecked(Unsafe) const&& noexcept -> const T&&
    {
        return VIOLET_MOVE(this->getValueRef());
    }

    /// Applies `fun` to the contained value if present.
    ///
    /// Equivalent to Rust's [`Option::map`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map)
    ///
    /// @returns `Some(fun(value))` if this container is engaged, `Nothing` otherwise.
    template<typename Fun>
        requires(callable<Fun, T&>)
    constexpr auto Map(Fun&& fun) & -> Optional<std::invoke_result_t<Fun, T&>>
    {
        using Ret = std::invoke_result_t<Fun, T&>;
        if (this->HasValue()) {
            return Some<Ret>(std::invoke(VIOLET_FWD(Fun, fun), Value()));
        }

        return decltype(Optional<Ret>{})(Nothing);
    }

    /// Applies `fun` to the contained value if present.
    ///
    /// Equivalent to Rust's [`Option::map`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map)
    ///
    /// @returns `Some(fun(value))` if this container is engaged, `Nothing` otherwise.
    template<typename Fun>
        requires(callable<Fun, const T&>)
    constexpr auto Map(Fun&& fun) const& noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const T&>())))
        -> Optional<std::invoke_result_t<Fun, const T&>>
    {
        using Ret = std::invoke_result_t<Fun, const T&>;
        if (this->HasValue()) {
            return Some<Ret>(std::invoke(VIOLET_FWD(Fun, fun), Value()));
        }

        return decltype(Optional<Ret>{})(Nothing);
    }

    /// Applies `fun` to the contained value if present.
    ///
    /// Equivalent to Rust's [`Option::map`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map)
    ///
    /// @returns `Some(fun(value))` if this container is engaged, `Nothing` otherwise.
    template<typename Fun>
        requires(callable<Fun, T &&>)
    constexpr auto Map(Fun&& fun) && noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<T&&>())))
        -> Optional<std::invoke_result_t<Fun, T&&>>
    {
        using Ret = std::invoke_result_t<Fun, T&&>;
        if (this->HasValue()) {
            return Some<Ret>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(Value())));
        }

        return decltype(Optional<Ret>{})(Nothing);
    }

    /// Applies `fun` to the contained value if present.
    ///
    /// Equivalent to Rust's [`Option::map`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map)
    ///
    /// @returns `Some(fun(value))` if this container is engaged, `Nothing` otherwise.
    template<typename Fun>
        requires(callable<Fun, const T &&>)
    constexpr auto Map(Fun&& fun) const&& noexcept(noexcept(
        std::invoke(VIOLET_FWD(Fun, fun), std::declval<const T&&>()))) -> Optional<std::invoke_result_t<Fun, const T&&>>
    {
        using Ret = std::invoke_result_t<Fun, const T&&>;
        if (this->HasValue()) {
            return Some<Ret>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(Value())));
        }

        return decltype(Optional<Ret>{})(Nothing);
    }

    /// Returns `true` if a value is present and `fun(value)` returns `true`.
    ///
    /// Equivalent to Rust's
    /// [`Option::is_some_and`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.is_some_and)
    template<typename Fun>
        requires(callable<Fun, T&> && callable_returns<Fun, bool, T&>)
    constexpr auto HasValueAnd(Fun&& fun) & noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<T&>())))
        -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), Value());
    }

    /// Returns `true` if a value is present and `fun(value)` returns `true`.
    ///
    /// Equivalent to Rust's
    /// [`Option::is_some_and`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.is_some_and)
    template<typename Fun>
        requires(callable<Fun, const T&> && callable_returns<Fun, bool, const T&>)
    constexpr auto HasValueAnd(Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const T&>()))) -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), Value());
    }

    /// Returns `true` if a value is present and `fun(value)` returns `true`.
    ///
    /// Equivalent to Rust's
    /// [`Option::is_some_and`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.is_some_and)
    template<typename Fun>
        requires(callable<Fun, T &&> && callable_returns<Fun, bool, T &&>)
    constexpr auto HasValueAnd(Fun&& fun) && noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<T&&>())))
        -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), Value());
    }

    /// Returns `true` if a value is present and `fun(value)` returns `true`.
    ///
    /// Equivalent to Rust's
    /// [`Option::is_some_and`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.is_some_and)
    template<typename Fun>
        requires(callable<Fun, const T &&> && callable_returns<Fun, bool, const T &&>)
    constexpr auto HasValueAnd(Fun&& fun) const&& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const T&&>()))) -> bool
    {
        return this->HasValue() && std::invoke(VIOLET_FWD(Fun, fun), Value());
    }

    /// Applies `fun` to the contained value if present, otherwise returns `defaultValue`.
    ///
    /// Equivalent to Rust's
    /// [`Option::map_or`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map_or)
    template<typename U, typename Fun>
        requires(callable<Fun, T&> && std::convertible_to<std::invoke_result_t<Fun, T&>, U>)
    constexpr auto MapOr(U&& defaultValue, Fun&& fun) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<T&>()))) -> U
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), Value());
        }

        return VIOLET_FWD(U, defaultValue);
    }

    /// Applies `fun` to the contained value if present, otherwise returns `defaultValue`.
    ///
    /// Equivalent to Rust's
    /// [`Option::map_or`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map_or)
    template<typename U, typename Fun>
        requires(callable<Fun, const T&> && std::convertible_to<std::invoke_result_t<Fun, const T&>, U>)
    constexpr auto MapOr(U&& defaultValue, Fun&& fun) const& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const T&>()))) -> U
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), Value());
        }

        return VIOLET_FWD(U, defaultValue);
    }

    /// Applies `fun` to the contained value if present, otherwise returns `defaultValue`.
    ///
    /// Equivalent to Rust's
    /// [`Option::map_or`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map_or)
    template<typename U, typename Fun>
        requires(callable<Fun, T &&> && std::convertible_to<std::invoke_result_t<Fun, T &&>, U>)
    constexpr auto MapOr(U&& defaultValue, Fun&& fun) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<T&&>()))) -> U
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(Value()));
        }

        return VIOLET_FWD(U, defaultValue);
    }

    /// Applies `fun` to the contained value if present, otherwise returns `defaultValue`.
    ///
    /// Equivalent to Rust's
    /// [`Option::map_or`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.map_or)
    template<typename U, typename Fun>
        requires(callable<Fun, const T &&> && std::convertible_to<std::invoke_result_t<Fun, const T &&>, U>)
    constexpr auto MapOr(U&& defaultValue, Fun&& fun) const&& noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<const T&&>()))) -> U
    {
        if (this->HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(Value()));
        }

        return VIOLET_FWD(U, defaultValue);
    }

    /// Takes the contained value, leaving this `Optional` disengaged.
    ///
    /// Equivalent to Rust's
    /// [`Option::take`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.take).
    constexpr auto Take() noexcept -> Optional<T>
    {
        if (!this->HasValue()) {
            return {};
        }

        Optional tmp = Some<T>(VIOLET_MOVE(Value()));
        this->Reset();

        return tmp;
    }

    /// Replaces the contained value with a newly constructed one.
    ///
    /// Destroys the previous value if present, returns a reference to the new value.
    ///
    /// Equivalent to Rust's
    /// [`Option::replace`](https://doc.rust-lang.org/1.93.0/std/option/enum.Option.html#method.replace).
    template<typename... Args>
    constexpr auto Replace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) -> T&
    {
        if (this->HasValue()) {
            this->Reset();
        }

        ::new (&this->n_value) T(VIOLET_FWD(Args, args)...);
        this->n_engaged = true;

        return Value();
    }

    /// Destroys the contained value and disengages the `Optional`.
    auto Reset() noexcept(std::is_nothrow_destructible_v<T>)
    {
        this->destroy();
    }

    constexpr auto operator->() noexcept -> T*
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return std::addressof(this->getValueRef());
    }

    constexpr auto operator->() const noexcept -> const T*
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return std::addressof(this->getValueRef());
    }

    constexpr auto operator*() & noexcept -> T&
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return this->getValueRef();
    }

    constexpr auto operator*() const& noexcept -> const T&
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return this->getValueRef();
    }

    constexpr auto operator*() && noexcept -> T&&
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return VIOLET_MOVE(this->getValueRef());
    }

    constexpr auto operator*() const&& noexcept -> const T&&
    {
        VIOLET_DEBUG_ASSERT(this->HasValue(), "cannot dereference nothing");
        return VIOLET_MOVE(this->getValueRef());
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->HasValue();
    }

    constexpr VIOLET_EXPLICIT operator std::optional<T>() const noexcept
    {
        return this->HasValue() ? std::optional<T>(Value()) : Nothing;
    }

    constexpr auto operator==(const std::nullopt_t&) const noexcept -> bool
    {
        return !this->HasValue();
    }

    constexpr auto operator!=(const std::nullopt_t&) const noexcept -> bool
    {
        return this->HasValue();
    }

    constexpr auto operator==(const Optional& other) const noexcept -> bool
        requires(requires { this->Value() == other.Value(); })
    {
        if (this->HasValue() != other.HasValue()) {
            return false;
        }

        return this->Value() == other.Value();
    }

    constexpr auto operator!=(const Optional& other) const noexcept -> bool
        requires(requires { this->Value() == other.Value(); })
    {
        return !(*this == other);
    }

    constexpr auto operator==(const std::optional<T>& other) const noexcept -> bool
        requires(requires { this->Value() == *other; })
    {
        if (this->HasValue() != other.has_value()) {
            return false;
        }

        return this->Value() == *other;
    }

    constexpr auto operator!=(const std::optional<T>& other) const noexcept -> bool
        requires(requires { this->Value() == *other; })
    {
        return !(*this == other);
    }

    constexpr auto operator==(const T& other) const noexcept -> bool
        requires(requires { this->Value() == other; })
    {
        return this->HasValue() && this->Value() == other;
    }

    constexpr auto operator!=(const T& other) const noexcept -> bool
        requires(requires { this->Value() == other; })
    {
        return !(*this == other);
    }

    auto ToString() const noexcept -> String
    {
        if (!this->HasValue()) {
            return "«no value»";
        }

        return violet::ToString(Value());
    }

    friend auto operator<<(std::ostream& os, const Optional<T>& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    mutable bool n_engaged = false;
    union { // NOLINT(cppcoreguidelines-special-member-functions)
        T n_value;
    };

    constexpr void destroy() noexcept(std::is_nothrow_destructible_v<T>)
    {
        if (this->n_engaged) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                this->n_value.~T();
            }

            this->n_engaged = false;
        }
    }

    constexpr auto getValueRef() & noexcept -> T&
    {
        return *std::launder(&this->n_value);
    }

    constexpr auto getValueRef() const& noexcept -> const T&
    {
        return *std::launder(&this->n_value);
    }

    constexpr auto getValueRef() && noexcept -> T&
    {
        return VIOLET_MOVE(*std::launder(&this->n_value));
    }

    constexpr auto getValueRef() const&& noexcept -> const T&&
    {
        return VIOLET_MOVE(*std::launder(&this->n_value));
    }

    [[noreturn]] VIOLET_COLD static void panicUnexpectly(Str message, [[maybe_unused]] std::source_location loc)
#ifndef VIOLET_HAS_EXCEPTIONS
        noexcept
#endif
    {
#ifdef VIOLET_HAS_EXCEPTIONS
        throw std::logic_error(std::format("[violet::Optional][{}:{}:{} ({})]: panic: {}", loc.file_name(), loc.line(),
            loc.column(), util::DemangleCXXName(loc.function_name()), message));
#else
#if VIOLET_REQUIRE_STL(202302L)
        std::println(std::cerr, "[violet::Optional][{}:{}:{} ({})]: panic: {}", loc.file_name(), loc.line(),
            loc.column(), util::DemangleCXXName(loc.function_name()), message);
#else
        std::cerr << "[violet::Optional][" << loc.file_name() << ':' << loc.line() << ':' << loc.column() << " ("
                  << util::DemangleCXXName(loc.function_name()) << ")] panic: " << message;
#endif

        VIOLET_UNREACHABLE();
#endif
    }
};

// NOLINTEND(cppcoreguidelines-pro-type-union-access)

} // namespace violet

VIOLET_FORMATTER_TEMPLATE(violet::Optional<T>, typename T);
