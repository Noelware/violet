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

namespace violet {

/// Describes how to reference-count a handle of type `T`.
/// Specialize this for each *self*-reference-counting handle you want to manage with [`RefCnt`].
///
/// ### `RefCntTraits<T>::BumpRef(T)`
/// **Retain**: increment the handle's reference count.
///
/// ### `RefCntTraits<T>::DecRef(T)`
/// **Release**: decrement the count, freeing the resource at zero; must be a no-op on `Default()`.
///
/// ### `RefCntTraits<T>::Valid(T)`
/// whether the handle refers to a live resource (not the sentinel).
///
/// ### `RefCntTraits<T>::Default()`
/// The sentinel, default value.
template<typename T>
struct VIOLET_API NOELDOC_SINCE("26.07") RefCntTraits;

/// Satisfied when `RefCntTraits<T>` provides the `BumpRef`, `DecRef`, `Valid`, and `Default` operations
/// `RefCnt` needs to manage a `T`.
template<typename T>
concept ValidRefCntTraits = requires(T handle) {
    { RefCntTraits<T>::BumpRef(handle) } -> std::same_as<void>;
    { RefCntTraits<T>::DecRef(handle) } -> std::same_as<void>;
    { RefCntTraits<T>::Valid(handle) } -> std::same_as<bool>;
    { RefCntTraits<T>::Default() } -> std::same_as<T>;
};

/// An owning, externally reference-counted handle to a resource of type `T`.
///
/// `RefCnt` is the value-semantics analogoue of Rust's `Rc`/`Arc` for handles that carry their own reference count
/// (the count lives in the resource, not in `RefCnt`). Copy will retain; destruction will release, freeing the resource
/// when the last `RefCnt` referring to it goes away.
///
/// @tparam T the handle type.
/// @tparam Traits the reference-counting policy; defaults to `RefCntTraits<T>`.
template<typename T, typename Traits = RefCntTraits<T>>
struct VIOLET_API NOELDOC_SINCE("26.07") RefCnt {
    static_assert(ValidRefCntTraits<T>, "`T` doesn't specialize `RefCntTraits`");
    static_assert(noexcept(Traits::BumpRef(std::declval<T>())),
        "`BumpRef(T)` should be marked as `noexcept` as throwing exceptions is ill-formed");
    static_assert(noexcept(Traits::DecRef(std::declval<T>())),
        "`DecRef(T)` should be marked as `noexcept` as throwing exceptions is ill-formed");
    static_assert(noexcept(Traits::Valid(std::declval<T>())),
        "`Valid(T)` should be marked as `noexcept` as throwing exceptions is ill-formed");

    /// Constructs a `RefCnt` that owns nothing (holds `Traits::Default()`).
    VIOLET_IMPLICIT RefCnt() noexcept(noexcept(Traits::Default()))
        : Object(Traits::Default())
    {
    }

    /// Adopts `handle`, taking over a reference the caller already holds (no extra retain). Use `Retain` to
    /// instead acquire a new reference to a handle you don't own.
    VIOLET_EXPLICIT RefCnt(T handle)
        : Object(handle)
    {
    }

    /// Retains the shared handle.
    VIOLET_IMPLICIT RefCnt(const RefCnt& other)
        : Object(other.Object)
    {
        Traits::BumpRef(this->Object);
    }

    /// Retains `other`'s handle, then releases the current one.
    auto operator=(const RefCnt& other) -> RefCnt&
    {
        Traits::BumpRef(other.Object);
        Traits::DecRef(this->Object);

        this->Object = other.Object;
        return *this;
    }

    /// Transfers ownership from `other`, leaving it owning nothing.
    VIOLET_IMPLICIT RefCnt(RefCnt&& other) noexcept
        : Object(std::exchange(other.Object, Traits::Default()))
    {
    }

    /// Releases the current handle and transfers ownership from `other`, leaving it owning nothing.
    auto operator=(RefCnt&& other) noexcept -> RefCnt&
    {
        if (this != &other) {
            Traits::DecRef(this->Object);
            this->Object = std::exchange(other.Object, Traits::Default());
        }

        return *this;
    }

    /// Releases this reference, freeing the resource if it was the last.
    ~RefCnt()
    {
        Traits::DecRef(this->Object);
    }

    /// Acquires a new reference to `handle` and returns an owner for it (retain and adopt).
    ///
    /// Use this when you have a handle you do *not* already own a reference to; the plain constructor adopts
    /// an existing reference instead.
    template<typename U = T>
        requires(std::convertible_to<U, T>)
    static auto Retain(U handle) -> RefCnt
    {
        Traits::BumpRef(handle);
        return RefCnt(handle);
    }

    /// Returns the underlying handle without retaining it. The result is *borrowed*, do not release it; it
    /// stays owned by this `RefCnt` (and any copies).
    auto Get() const -> T
    {
        return this->Object;
    }

    /// Returns whether this owns a live handle
    [[nodiscard]] auto Valid() const -> bool
    {
        return Traits::Valid(this->Object);
    }

    VIOLET_EXPLICIT operator bool() const
    {
        return this->Valid();
    }

protected:
    T Object;
};

} // namespace violet
