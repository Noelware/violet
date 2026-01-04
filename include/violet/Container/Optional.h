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

#include <memory>
#include <optional>
#include <type_traits>

namespace violet {

/* --=-- START :: internals --=--*/

#ifndef VIOLET_HAS_EXCEPTIONS
VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void optionalUnwrapFail(const std::source_location& loc);

VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void optionalUnwrapFail(CStr message, const std::source_location& loc);
#else
VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void optionalUnwrapFail();

VIOLET_COLD [[noreturn, deprecated("internal function -- do not use")]]
void optionalUnwrapFail(CStr message);
#endif

/* --=--  END :: internals  --=-- */

template<typename T>
struct Optional;

/// Represents an empty `Optional<T>` value.
inline constexpr static std::nullopt_t Nothing = std::nullopt;

// /// An iterator for a [`Optional`].
// template<typename T>
// struct Iter;

/// Creates a new `Optional<T>` that contains a value.
///
/// @tparam T The underlying type to construct.
/// @tparam Args The arguments to pass to `T`'s constructor.
/// @param args The arguments to pass to `T`'s constructor.
/// @return An `Optional<T>` with the value constructed.
template<typename T, typename... Args>
constexpr static auto Some(Args&&... args) -> Optional<T>
{
    return Optional<T>(std::in_place, VIOLET_FWD(Args, args)...);
}

/// Represents an optional value, which is a value that can either be something or nothing.
/// @tparam T The underlying type of this `Optional`.
template<typename T>
struct VIOLET_API Optional final {
    using value_type = T;

    /// Constructs a new `Optional<T>` that is empty.
    constexpr VIOLET_IMPLICIT Optional() noexcept = default;

    /// Constructs a new `Optional<T>` that is empty.
    constexpr VIOLET_IMPLICIT Optional(std::nullopt_t) noexcept {}

    /// Constructs a new `Optional<T>` with a value, constructed in-place.
    /// @tparam ...Args The argument types to forward to `T`'s constructor.
    /// @param ...args The arguments to forward to `T`'s constructor.
    template<typename... Args>
    constexpr VIOLET_EXPLICIT Optional(std::in_place_t, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>)
    {
        // This is safe since we are constructing a new value, so we don't need to
        // worry about the old value.
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        this->construct(VIOLET_FWD(Args, args)...);
    }

    template<typename U = T>
    constexpr VIOLET_IMPLICIT Optional(const U& other) noexcept(std::is_nothrow_constructible_v<T, const U&>)
        requires(
            // Disallow copying `U` if `U` is an another optional, let it forward to the `const Optional<U>&`
            // constructor
            !std::is_same_v<std::remove_cvref_t<U>, Optional> &&

            // Same as above but for `std::optional`
            !violet::instanceof_v<std::optional, std::remove_cvref_t<U>> &&

            // Ensures that our `T` can be constructible from `U`.
            std::constructible_from<T, const U&>)
    {
        // This is safe since we are constructing a new value, so we don't need to
        // worry about the old value.
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        this->construct(other);
    }

    template<typename U = T>
    constexpr VIOLET_IMPLICIT Optional(U&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        requires(
            // Disallow copying `U` if `U` is an another optional, let it forward to the `const Optional<U>&`
            // constructor
            !std::is_same_v<std::remove_cvref_t<U>, Optional> &&

            // Same as above but for `std::optional`
            !std::is_same_v<std::remove_cvref_t<U>, std::optional<U>> &&

            // Ensures that our `T` can be constructible from `U`.
            std::constructible_from<T, U &&>)
    {
        // This is safe since we are constructing a new value, so we don't need to
        // worry about the old value.
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        this->construct(VIOLET_FWD(U, other));
    }

    /// Constructs a new `Optional<T>` by copying another `Optional<T>`.
    /// @param other The other `Optional<T>` to copy from.
    constexpr VIOLET_IMPLICIT Optional(const Optional& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        if (other.HasValue()) {
            this->construct(*other.ptr());
        }
    }

    constexpr VIOLET_IMPLICIT Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        if (other.HasValue()) {
            this->construct(VIOLET_MOVE(*other.ptr()));
            other.reset();
        }
    }

    constexpr VIOLET_IMPLICIT Optional(const std::optional<T>& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        if (other.has_value()) {
            this->construct(*other);
        }
    }

    constexpr VIOLET_IMPLICIT Optional(
        std::optional<T>&& other) noexcept( // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        std::is_nothrow_move_constructible_v<T>)
    {
        if (other.has_value()) {
            // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
            this->construct(VIOLET_MOVE(*other));
            other.reset();
        }
    }

    constexpr ~Optional() noexcept(std::is_nothrow_destructible_v<T>)
    {
        reset();
    }

    /// Resets this `Optional` to an empty state.
    constexpr auto operator=(std::nullopt_t) noexcept -> Optional&
    {
        reset();
        return *this;
    }

    /// Assigns this `Optional` to the value of another `Optional`.
    constexpr auto operator=(const Optional& other) noexcept(
        std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) -> Optional&
        requires std::is_copy_assignable_v<T> || std::is_copy_constructible_v<T>
    {
        if (this != &other) {
            if (other.HasValue()) {
                if (this->HasValue()) {
                    *ptr() = *other.ptr();
                } else {
                    this->construct(*other.ptr());
                }
            } else if (this->HasValue()) {
                reset();
            }
        }

        return *this;
    }

    /// Moves the value of another `Optional` into this one.
    constexpr auto operator=(Optional&& other) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) -> Optional&
        requires std::is_move_assignable_v<T> || std::is_move_constructible_v<T>
    {
        if (this != &other) {
            if (other.HasValue()) {
                if (this->HasValue()) {
                    *ptr() = *other.ptr();
                } else {
                    this->construct(*other.ptr());
                }

                other.reset();
            } else if (this->HasValue()) {
                reset();
            }
        }

        return *this;
    }

    /// Assigns this `Optional` to a new value.
    template<typename U = T>
    constexpr auto operator=(const U& other) noexcept(std::is_nothrow_constructible_v<T, const U&>) -> Optional&
        requires(
            // Disallow copying `U` if `U` is an another optional, let it forward to the `const Optional<U>&`
            // constructor
            !std::is_same_v<std::remove_cvref_t<U>, Optional> &&

            // Same as above but for `std::optional`
            !violet::instanceof_v<std::optional, std::remove_cvref_t<U>> &&

            // Ensures that our `T` can be constructible from `U`.
            std::constructible_from<T, const U&>)
    {
        if (this->HasValue()) {
            *ptr() = other;
        } else {
            this->construct(other);
        }

        return *this;
    }

    /// Moves a new value into this `Optional`.
    template<typename U = T>
    constexpr auto operator=(U&& other) noexcept(std::is_nothrow_constructible_v<T, U&&>) -> Optional&
        requires(
            // Disallow copying `U` if `U` is an another optional, let it forward to the `const Optional<U>&`
            // constructor
            !std::is_same_v<std::remove_cvref_t<U>, Optional> &&

            // Same as above but for `std::optional`
            !violet::instanceof_v<std::optional, std::remove_cvref_t<U>> &&

            // Ensures that our `T` can be constructible from `U`.
            std::constructible_from<T, U &&>)
    {
        if (this->HasValue()) {
            *ptr() = VIOLET_FWD(U, other);
        } else {
            this->construct(VIOLET_FWD(U, other));
        }

        return *this;
    }

    /// Assigns this `Optional` to the value of a `std::optional`.
    constexpr auto operator=(std::optional<T>& other) noexcept(
        std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>) -> Optional&
        requires std::is_copy_assignable_v<T> || std::is_copy_constructible_v<T>
    {
        if (other.has_value()) {
            if (this->HasValue()) {
                *ptr() = *other;
            } else {
                this->construct(*other);
            }
        } else if (this->HasValue()) {
            reset();
        }

        return *this;
    }

    /// Moves the value of a `std::optional` into this one.
    constexpr auto operator=(std::optional<T>&& other) noexcept(
        std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) -> Optional&
        requires std::is_move_assignable_v<T> || std::is_move_constructible_v<T>
    {
        if (other.has_value()) {
            if (this->HasValue()) {
                *ptr() = VIOLET_MOVE(*other);
            } else {
                this->construct(VIOLET_MOVE(*other));
            }

            other.reset();
        } else if (this->HasValue()) {
            reset();
        }

        (void)VIOLET_MOVE(other);
        return *this;
    }

    /// Returns a pointer to the contained value.
    ///
    /// ## Remarks
    /// This will assert in debug builds if this optional is not empty.
    ///
    /// @return A pointer to the contained value.
    constexpr auto Value() noexcept -> T*
    {
        VIOLET_DEBUG_ASSERT(this->n_containsValue, "`Optional<T>` contains no value");
        return this->ptr();
    }

    /// Returns a pointer to the contained value.
    ///
    /// ## Remarks
    /// This will assert in debug builds if this optional is not empty.
    ///
    /// @return A pointer to the contained value.
    constexpr auto Value() const noexcept -> const T*
    {
        VIOLET_DEBUG_ASSERT(this->n_containsValue, "`Optional<T>` contains no value");
        return this->ptr();
    }

    /// Returns `true` if the `Optional` contains a value.
    constexpr auto HasValue() const noexcept -> bool
    {
        return this->n_containsValue;
    }

#ifndef VIOLET_HAS_EXCEPTIONS
    /// Returns the contained value, consuming the `Optional`.
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Optional` is empty. If you want to provide a
    /// default value, use `UnwrapOr` or `UnwrapOrElse`.
    ///
    /// ## Example
    /// ```cpp
    /// auto opt = violet::Some<int>(42);
    /// int val = std::move(opt).Unwrap(); // val == 42
    /// ```
    ///
    /// @param loc The source location of the call, which is used for the termination message.
    /// @return The contained value.
    ///
    constexpr auto Unwrap(const std::source_location& loc = std::source_location::current()) && -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(*ptr());
        }

        optionalUnwrapFail(loc);
    }
#else
    /// Returns the contained value, consuming the `Optional`.
    ///
    /// ## Example
    /// ```cpp
    /// auto opt = violet::Some<int>(42);
    /// int val = std::move(opt).Unwrap(); // val == 42
    /// ```
    ///
    /// @throws [std::logic_error] if this `Optional` is empty.
    /// @return The contained value.
    constexpr auto Unwrap() && -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(*ptr());
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        optionalUnwrapFail();

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }
#endif

    /// Returns the contained value, consuming the `Optional`, or a default value.
    ///
    /// ## Example
    /// ```cpp
    /// auto opt1 = violet::Some<int>(42);
    /// int val1 = std::move(opt1).UnwrapOr(0); // val1 == 42
    ///
    /// auto opt2 = violet::Optional<int>();
    /// int val2 = std::move(opt2).UnwrapOr(0); // val2 == 0
    /// ```
    ///
    /// @param def The default value to return if the `Optional` is empty.
    /// @return The contained value or the default value.
    constexpr auto UnwrapOr(T&& def) && -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(*ptr());
        }
        else
        {
            return VIOLET_MOVE(def);
        }
    }

    /// Returns the contained value, consuming the `Optional`, or computes it from a closure.
    /// ## Example
    /// ```cpp
    /// auto opt1 = violet::Some<int>(42);
    /// int val1 = std::move(opt1).UnwrapOrElse([]() { return 0; }); // val1 == 42
    ///
    /// auto opt2 = violet::Optional<int>();
    /// int val2 = std::move(opt2).UnwrapOrElse([]() { return 0; }); // val2 == 0
    /// ```
    ///
    /// @param fun The closure to call to compute the default value.
    /// @return The contained value or the computed default value.
    template<typename Fun>
        requires callable<Fun> && std::convertible_to<std::invoke_result_t<Fun>, T>
    constexpr auto UnwrapOrElse(Fun&& fun) &&
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(*ptr());
        }
        else
        {
            return std::invoke(VIOLET_FWD(Fun, fun));
        }
    }

    /// Returns the contained value, consuming the `Optional`, without checking if it has a value.
    ///
    /// ## Safety
    /// This function is unsafe because it does not check if the `Optional` has a value. If the `Optional` is
    /// empty, this will result in undefined behavior.
    ///
    /// @return The contained value.
    constexpr auto UnwrapUnchecked(Unsafe) && -> T
    {
        return VIOLET_MOVE(*ptr());
    }

#ifndef VIOLET_HAS_EXCEPTIONS
    /// Returns the contained value, consuming the `Optional`.
    ///
    /// ## Example
    /// ```cpp
    /// auto opt = violet::Optional<int>();
    /// int val = std::move(opt).Expect("value was not present"); // terminates
    /// ```
    ///
    /// ## Process Termination
    /// This function will terminate the process if the `Optional` is empty, with a custom message.
    ///
    /// @param message The message to print on termination.
    /// @param loc The source location of the call, which is used for the termination message.
    /// @return The contained value.
    constexpr auto Expect(Str message, const std::source_location& loc = std::source_location::current()) && -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(*ptr());
        }

        optionalUnwrapFail(message.data(), loc);
    }
#else
    /// Returns the contained value, consuming the `Optional`.
    ///
    /// ## Example
    /// ```cpp
    /// auto opt = violet::Optional<int>();
    /// int val = std::move(opt).Expect("value was not present"); // throws
    /// ```
    ///
    /// @throws [std::logic_error] if the `Optional` is empty, with a custom message.
    /// @param message The message to use for the exception.
    /// @return The contained value.
    ///
    constexpr auto Expect(Str message) && -> T
    {
        VIOLET_LIKELY_IF(this->HasValue())
        {
            return VIOLET_MOVE(*ptr());
        }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wdeprecated-declarations")
#endif

        optionalUnwrapFail(message.data());

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

#endif

    /// Calls a function with the contained value if it exists, and returns the result.
    template<typename Fun>
        requires callable<Fun, T&>
    constexpr auto AndThen(Fun&& fun) &
    {
        using U = std::invoke_result_t<Fun, T&>;
        if (HasValue()) {
            return Some<U>(std::invoke(VIOLET_FWD(Fun, fun), *ptr()));
        }

        return decltype(Optional<U>{})(Nothing);
    }

    /// Calls a function with the contained value if it exists, and returns the result.
    template<typename Fun>
        requires callable<Fun, const T&>
    constexpr auto AndThen(Fun&& fun) const&
    {
        using U = std::invoke_result_t<Fun, const T&>;
        if (HasValue()) {
            return Some<U>(std::invoke(VIOLET_FWD(Fun, fun), *ptr()));
        }

        return decltype(Optional<U>{})(Nothing);
    }

    /// Calls a function with the contained value if it exists, and returns the result.
    template<typename Fun>
        requires callable<Fun, T&&>
    constexpr auto AndThen(Fun&& fun) &&
    {
        using U = std::invoke_result_t<Fun, T&&>;
        if (HasValue()) {
            return Some<U>(std::invoke(VIOLET_FWD(Fun, fun), *ptr()));
        }

        return decltype(Optional<U>{})(Nothing);
    }

    /// Returns `other` if this `Optional` has a value, otherwise returns an empty `Optional`.
    template<typename U>
    constexpr auto And(const Optional<U>& other) const -> Optional<U>
    {
        return HasValue() ? other : Optional<U>();
    }

    /// Returns `true` if the `Optional` has a value and the predicate returns `true`.
    template<typename Fun>
        requires callable<Fun, const T&>
    constexpr auto HasValueAnd(Fun&& fun) const -> bool
    {
        return HasValue() && std::invoke(VIOLET_FWD(Fun, fun), *ptr());
    }

    /// Maps an `Optional<T>` to `Optional<U>` by applying a function to a contained value.
    template<typename Fun>
        requires callable<Fun, T&>
    constexpr auto Map(Fun&& fun) &
    {
        using U = std::invoke_result_t<Fun, T&>;
        if (HasValue()) {
            return Some<U>(std::invoke(VIOLET_FWD(Fun, fun), *ptr()));
        }

        return decltype(Optional<U>{})(Nothing);
    }

    /// Maps an `Optional<T>` to `Optional<U>` by applying a function to a contained value.
    template<typename Fun>
        requires callable<Fun, const T&>
    constexpr auto Map(Fun&& fun) const&
    {
        using U = std::invoke_result_t<Fun, const T&>;
        if (HasValue()) {
            return Some<U>(std::invoke(VIOLET_FWD(Fun, fun), *ptr()));
        }

        return decltype(Optional<U>{})(Nothing);
    }

    /// Maps an `Optional<T>` to `Optional<U>` by applying a function to a contained value.
    template<typename Fun>
        requires callable<Fun, T&&>
    constexpr auto Map(Fun&& fun) &&
    {
        using U = std::invoke_result_t<Fun, T&&>;
        if (HasValue()) {
            return Some<U>(std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(*ptr())));
        }

        return decltype(Optional<U>{})(Nothing);
    }

    /// Returns the provided default value if the `Optional` is empty, otherwise applies a function to the contained
    /// value and returns the result.
    template<typename U, typename Fun>
        requires callable<Fun, const T&> && std::convertible_to<std::invoke_result_t<Fun, const T&>, U>
    constexpr auto MapOr(U&& default_value, Fun&& fun) const& -> U
    {
        if (HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), *ptr());
        }

        return VIOLET_FWD(U, default_value);
    }

    /// Returns the provided default value if the `Optional` is empty, otherwise applies a function to the contained
    /// value and returns the result.
    template<typename U, typename Fun>
        requires callable<Fun, T&&> && std::convertible_to<std::invoke_result_t<Fun, T&&>, U>
    constexpr auto MapOr(U&& default_value, Fun&& fun) && -> U
    {
        if (HasValue()) {
            return std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(*ptr()));
        }

        return VIOLET_FWD(U, default_value);
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
        reset();
    }

    auto ToString() const noexcept -> String
    {
        if (!this->HasValue()) {
            return "«no value»";
        }

        if constexpr (Stringify<T>) {
            return violet::ToString(*Value());
        }

#if VIOLET_USE_RTTI
        const auto& type = typeid(T);
        return std::format("type `{}@{}`", util::DemangleCXXName(type.name()), type.hash_code());
#else
        return "????";
#endif
    }

    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

    constexpr auto operator->() noexcept -> T*
    {
        VIOLET_DEBUG_ASSERT(HasValue(), "cannot dereference invalid data");
        return ptr();
    }

    constexpr auto operator->() const noexcept -> const T*
    {
        VIOLET_DEBUG_ASSERT(HasValue(), "cannot access invalid data");
        return ptr();
    }

    constexpr auto operator*() noexcept -> T&
    {
        VIOLET_DEBUG_ASSERT(HasValue(), "cannot dereference invalid data");
        return *ptr();
    }

    constexpr auto operator*() const noexcept -> const T&
    {
        VIOLET_DEBUG_ASSERT(HasValue(), "cannot dereference invalid data");
        return *ptr();
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_containsValue;
    }

    constexpr VIOLET_EXPLICIT operator std::optional<T>() const noexcept
    {
        return this->n_containsValue ? std::optional<T>(*ptr()) : Nothing;
    }

    constexpr auto operator==(const std::nullopt_t&) const noexcept -> bool
    {
        return !this->HasValue();
    }

    constexpr auto operator!=(const std::nullopt_t&) const noexcept -> bool
    {
        return this->HasValue();
    }

    /// Checks if two `Optional`s are equal.
    /// @return `true` if they are equal, `false` otherwise.
    auto operator==(const Optional& other) const noexcept -> bool
    {
        if (this->n_containsValue != other.n_containsValue) {
            return false;
        }

        return *this->ptr() == *other.ptr();
    }

    /// Checks if two `Optional`s are not equal.
    /// @return `true` if they are not equal, `false` otherwise.
    auto operator!=(const Optional& other) const noexcept -> bool
    {
        return !(this == other);
    }

    /// Checks if an `Optional` and a `std::optional` are equal.
    /// @return `true` if they are equal, `false` otherwise.
    auto operator==(const std::optional<T>& other) const noexcept -> bool
    {
        if (this->n_containsValue != other.has_value()) {
            return false;
        }

        return *this->ptr() == *other;
    }

    /// Checks if an `Optional` and a `std::optional` are not equal.
    /// @return `true` if they are not equal, `false` otherwise.
    auto operator!=(const std::optional<T>& other) const noexcept -> bool
    {
        return !(this == other);
    }

    /// Checks if an `Optional` and a value are equal.
    /// @return `true` if the `Optional` has a value and it is equal to the other value, `false` otherwise.
    auto operator==(const T& other) const noexcept -> bool
        requires std::equality_comparable<T>
    {
        if (!this->n_containsValue) {
            return false;
        }

        return *this->ptr() == other;
    }

    /// Checks if an `Optional` and a value are not equal.
    /// @return `true` if the `Optional` does not have a value or it is not equal to the other value, `false` otherwise.
    auto operator!=(const T& value) const noexcept -> bool
        requires std::equality_comparable<T>
    {
        return !(this == value);
    }

private:
    mutable bool n_containsValue = false;
    alignas(T) UInt8 n_storage[sizeof(T)];

    /// Constructs a value in-place in the storage.
    /// @param ...args The arguments to forward to the `T`'s constructor.
    template<typename... Args>
    constexpr void construct(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        std::construct_at(ptr(), VIOLET_FWD(Args, args)...);
        this->n_containsValue = true;
    }

    constexpr auto ptr() noexcept -> T*
    {
        // Safety: This is usually correct in most cases.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return std::launder(reinterpret_cast<T*>(this->n_storage));
    }

    constexpr auto ptr() const noexcept -> const T*
    {
        // Safety: This is usually correct in most cases.
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return std::launder(reinterpret_cast<const T*>(this->n_storage));
    }

    /// @internal
    /// This will reset the storage contained in this `Optional` by dropping
    /// the contents then de-allocating the memory as well.
    constexpr void reset() noexcept(std::is_nothrow_destructible_v<T>)
    {
        if (this->n_containsValue) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                std::destroy_at(this->ptr());
            }

            this->n_containsValue = false;
        }
    }
};

} // namespace violet

/**
 * @macro LET_IF_SOME(opt, val)
 *
 * C macro that resembles the `if let Some(val) = opt` expression from Rust.
 */
#define LET_IF_SOME(opt, val) if (auto val = opt)

VIOLET_FORMATTER_TEMPLATE(violet::Optional<T>, typename T);
