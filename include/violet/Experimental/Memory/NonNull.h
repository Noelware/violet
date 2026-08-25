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
//! # 🌺💜 `violet/Experimental/Memory/NonNull.h`
//! A non-nullable, non-owning wrapper of a pointer modeled after Rust's <code>[`std::ptr::NonNull`]\<T\></code>.
//! [`std::ptr::NonNull`]: https://doc.rust-lang.org/std/ptr/struct.NonNull.html
//!
//! <code>[`violet::experimental::ptr::NonNull`]\<T\></code> wraps a raw pointer that is guaranteed
//! to be never `nullptr`. It keeps raw-pointer semantics: trivially copyable, comparable, formattable
//! while ruling out the null case at construction time instead of at every use site.
//!
//! `NonNull` does **not** own the pointee; it is exactly as non-owning as a raw `T*`. Dereferencing
//! a `NonNull` built from a dangling pointer is still UB; the type only rules out the null case,
//! not lifetime.
//!
//! ## Example
//! ```cpp
//! #include <violet/Experimental/Memory/NonNull.h>
//!
//! using namespace violet::experimental::ptr;
//! using namespace violet;
//!
//! Int32 value = 42;
//! NonNull p(value); // convert a reference to `value` to NonNull
//! VIOLET_ASSERT(*p == value);
//!
//! Int32* raw = &value;
//! if (auto checked = NonNull<Int32>::New(raw)) {
//!     VIOLET_ASSERT(**checked == 42);
//! }
//! ```

#pragma once

#include <violet/Container/Optional.h>

#include <type_traits>

namespace violet::experimental::ptr {
namespace NOELDOC_HIDE ptr_internal {

template<typename T>
concept sentinel_constructible
    = std::is_object_v<T> && requires { std::bool_constant<(static_cast<void>(T{}), true)>{}; };

template<sentinel_constructible T>
constexpr inline T __dangling_object{};

static_assert(sentinel_constructible<Int32>);
static_assert(sentinel_constructible<Int32[3]>);

static_assert(!sentinel_constructible<void>);
static_assert(!sentinel_constructible<Int32&>);
static_assert(!sentinel_constructible<void()>);

} // namespace NOELDOC_HIDE ptr_internal

/// A non-null, non-owning wrapper around `T*`.
///
/// View the [module documentation](#) for more information.
template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("current") NonNull final {
    static_assert(!std::is_reference_v<T>, "`NonNull<T&>` is not a thing; use `NonNull<T>`");
    static_assert(!std::is_pointer_v<T>, "`NonNull<T*>` is illegal; use `NonNull<T>`");

    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(NonNull);
    constexpr ~NonNull() noexcept = default;

    /// Implicitly construct a [`NonNull`] from a reference. Since references
    /// are never not null, the conversion is infallible and requires no runtime checks.
    template<typename U>
        requires(!std::same_as<std::remove_cvref_t<U>, NonNull>)
    constexpr VIOLET_IMPLICIT NonNull(U& other) noexcept
        : n_value(std::addressof(other))
    {
    }

    /// Implicitly converts a `NonNull<U>` -> `NonNull<T>`.
    template<typename U>
        requires(!std::same_as<U, T> && std::same_as<std::remove_cv_t<U>, std::remove_cv_t<T>>
            && std::convertible_to<U*, T*>)
    constexpr VIOLET_IMPLICIT NonNull(NonNull<U> other) noexcept
        : n_value(other.Get())
    {
    }

    /// Constructs a [`NonNull`] from a pointer.
    ///
    /// This can return [`Nothing`] if `value` is `nullpte`, otherwise a [`NonNull`] wrapping it. Prefer
    /// this over [`NonNull::NewUnchecked`] whether `value`'s nullness isn't already well established.
    constexpr static auto New(T* value) noexcept -> Optional<NonNull>
    {
        if (value == nullptr) {
            return Nothing;
        }

        return NonNull(private_tag{}, value);
    }

    /// Constructs a [`NonNull`] from a pointer the caller has already proven is non-null,
    /// skipping the runtime check that [`NonNull::New`] performs.
    ///
    /// ## Safety
    /// `value` SHOULD NEVER be `nullptr`. Passing `nullptr` is UB.
    constexpr static auto NewUnchecked(Unsafe, T* value) noexcept -> NonNull
    {
        VIOLET_ASSUME(value != nullptr);
        return NonNull(private_tag{}, value);
    }

    /// Returns a well-aligned [`NonNull`] that is never meant to be dereferenced.
    ///
    /// This is useful as a placeholder value for a moved-from or not-yet-initialized `NonNull<T>`
    /// field, mirroring Rust's [`NonNull::dangling`]. When `T` conforms to [`sentinel_constructible`],
    /// the returned pointer addresses a shared static object rather than an arbitrary `alignof(T)` address;
    /// either way, dereferencing it is undefined behaviour.
    ///
    /// [`NonNull::dangling`]: https://doc.rust-lang.org/std/ptr/struct.NonNull.html#method.dangling
    constexpr static auto Dangling() noexcept -> NonNull
        requires(alignable<T>)
    {
        if constexpr (ptr_internal::sentinel_constructible<T>) {
            return NonNull(private_tag{}, const_cast<T*>(std::addressof(ptr_internal::__dangling_object<T>)));
        } else {
            return NonNull(private_tag{}, reinterpret_cast<T*>(alignof(T)));
        }
    }

    /// Returns the pointer. Always non-null.
    constexpr auto Get() noexcept -> T*
    {
        VIOLET_ASSUME(this->n_value != nullptr);
        return this->n_value;
    }

    /// Returns the pointer. Always non-null.
    constexpr auto Get() const noexcept -> T*
    {
        VIOLET_ASSUME(this->n_value != nullptr);
        return this->n_value;
    }

    /// Reinterprets this <code>[`NonNull`]<\T\></code> as <code>[`NonNull`]\<U\></code> via `reinterpret_cast`.
    ///
    /// ## Safety
    /// This does **not** verify layout compatibility between `T` and `U`; the caller is responsible for
    /// ensuring that casts are valid.
    template<typename U>
    auto Cast(Unsafe) const noexcept -> NonNull<U>
    {
        return NonNull<U>::NewUnchecked(Unsafe("it should be safe, hopefully..."), reinterpret_cast<U*>(this->n_value));
    }

    constexpr auto operator<=>(const NonNull& other) const = default;

    template<typename U = T>
        requires(!std::is_void_v<U> && std::same_as<U, T>)
    constexpr auto operator->() const noexcept -> U*
    {
        return this->Get();
    }

    template<typename U = T>
        requires(!std::is_void_v<U> && std::same_as<U, T>)
    constexpr auto operator*() const noexcept -> U&
    {
        VIOLET_ASSUME(this->n_value != nullptr);
        return *this->n_value;
    }

    constexpr VIOLET_EXPLICIT operator T*() noexcept
    {
        return this->Get();
    }

    template<typename U = T>
        requires(!std::is_void_v<U> && std::same_as<U, T>)
    constexpr VIOLET_EXPLICIT operator U&() noexcept
    {
        return *this->Get();
    }

    constexpr VIOLET_EXPLICIT operator const T*() const noexcept
    {
        return this->Get();
    }

    template<typename U = T>
        requires(!std::is_void_v<U> && std::same_as<U, T>)
    constexpr VIOLET_EXPLICIT operator const U&() const noexcept
    {
        return *this->Get();
    }

    [[nodiscard]] constexpr auto ToString() const noexcept -> String
    {
        return std::format("{:p}", static_cast<const void*>(this->Get()));
    }

    friend auto operator<<(std::ostream& os, const NonNull& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    struct private_tag final { };

    constexpr VIOLET_EXPLICIT NonNull(private_tag, T* value) noexcept
        : n_value(value)
    {
        VIOLET_ASSUME(this->n_value != nullptr);
    }

    T* n_value;
};

template<typename T>
NonNull(T&) -> NonNull<T>;

} // namespace violet::experimental::ptr

template<typename T>
struct std::formatter<violet::experimental::ptr::NonNull<T>>: public std::formatter<const void*> {
    auto format(const violet::experimental::ptr::NonNull<T>& self, auto& cx) const
    {
        return std::formatter<const void*>::format(static_cast<const void*>(self.Get()), cx);
    }
};

/// <code>[`NonNull`]<\T\></code> is trivially relocatable: it's a single pointer with no self-referential
/// or externally-observed address, so moving its bytes is equivalent to a move-construct
/// followed by a destroy.
template<typename T>
struct violet::trivially_relocatable<violet::experimental::ptr::NonNull<T>> final: std::true_type {
    // clang-format off
    static_assert(
        std::is_move_constructible_v<violet::experimental::ptr::NonNull<T>> &&
        std::is_destructible_v<violet::experimental::ptr::NonNull<T>>,
        "type must be movable and destructible to be relocatable"
    );
    // clang-format on
};
