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
//! # 🌺💜 `violet/Experimental/Unique.h`
//! A move-only, exclusive-ownership smart pointer modeled after [`std::unique_ptr`], without
//! the sharp edges of [`std::unique_ptr`].
//!
//! ## Design
//! * Unlike [`std::unique_ptr`], [`Unique`] propagates `const` through its accessors,
//!   matching [`Own`]'s semantics and Rust's `&Box<T>` -> `&T`.
//! * `Unique<T[]>` is rejected. Use `Own<T>`, `Vec<T>`, or `Slice<T>` for arrays.
//! * `Unique<Base>::New<Derived>` destroys through the concrete `Derived` type
//!   even when `Base` has no virtual destructor, mirroring [`Own::New`]'s semantics.
//! * Giving up ownership without destroying is **Leak**, not `release`/`Release`.
//! * [`Unique::operator<=>`] will order by the pointee value, not addresses.
//!
//! ## Example
//! ```cpp
//! #include <violet/Experimental/Unique.h>
//!
//! using namespace violet::experimental::ptr;
//!
//! Unique<Int32> ptr(new Int32{42});
//! VIOLET_ASSERT(*ptr == 42);
//!
//! if (auto leaked = ptr.Leak()) {
//!     VIOLET_ASSERT(leaked != nullptr);
//!     delete leaked->Get();
//! }
//! ```
//!
//! ## Polymorphic Construction
//! ```cpp
//! struct Animal {
//!     Int32 Legs;
//! };
//!
//! struct Cat final: public Animal {
//!     String Name;
//!
//!     VIOLET_IMPLICIT Cat(Str name, Int32 legs)
//!         : Animal(legs)
//!         , Name(name)
//!     {
//!     }
//! };
//!
//! Unique<Animal> pet = Unique<Animal>::New<Cat>("Amara", 4);
//! ```

#pragma once

#include <violet/Experimental/Memory/NonNull.h>
#include <violet/Experimental/NoOpDeleter.h> // IWYU pragma: export

namespace violet::experimental::ptr {

template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("current") DefaultDelete final {
    static_assert(!std::is_function_v<T>, "`DefaultDelete` cannot be instantiated for function types");

    constexpr VIOLET_IMPLICIT DefaultDelete() noexcept = default;

    template<typename U = T>
        requires std::convertible_to<U*, T*>
    constexpr VIOLET_IMPLICIT DefaultDelete(const DefaultDelete<U>&) noexcept
    {
    }

    constexpr void operator()(T* ptr) const noexcept
    {
        static_assert(sizeof(T) >= 0, "cannot delete an incomplete type");
        static_assert(!std::is_void_v<T>, "cannot delete an incomplete type");

        delete ptr;
    }
};

/// A move-only smart pointer providing exclusive ownership of a heap-allocated object of
/// type `T`. View the [module documentation](#) for more information.
template<typename T, typename Deleter = DefaultDelete<T>>
struct NOELDOC_EXPERIMENTAL_SINCE("current") Unique final {
    VIOLET_DISALLOW_COPY(Unique);

    static_assert(!std::is_array_v<T>, "`Unique<T>` is not supported; use `Own<T>` or `Vec<T>`");

    using value_type = T;

    constexpr VIOLET_IMPLICIT Unique() noexcept = default;
    constexpr VIOLET_IMPLICIT Unique(std::nullptr_t) noexcept { }

    ~Unique()
    {
        this->Reset();
    }

    /// Takes ownership of a raw pointer with a custom deleter (defaults to [`DefaultDelete`]).
    ///
    /// This behaves exactly like [`std::unique_ptr`]'s equivalent constructor: the deleter
    /// is invoked with `T*` so `T` will need a virtual destructor if `data` points to a
    /// derived object; prefer [`Unique::New`].
    ///
    /// @param data    raw pointer that this [`Unique`] will own.
    /// @param deleter custom deleter to properly delete `T`.
    template<typename U = T>
        requires std::convertible_to<U*, T*>
    VIOLET_EXPLICIT Unique(U* data, Deleter deleter = {}) noexcept
        : n_data(data)
        , n_deleter(VIOLET_MOVE(deleter))
        , n_destroy(data != nullptr ? &DestroyViaDeleter : nullptr)
    {
    }

    VIOLET_IMPLICIT Unique(Unique&& other) noexcept
        : n_data(std::exchange(other.n_data, nullptr))
        , n_deleter(VIOLET_MOVE(other.n_deleter))
        , n_destroy(std::exchange(other.n_destroy, nullptr))
    {
    }

    auto operator=(Unique&& other) noexcept -> Unique&
    {
        if (this != &other) {
            this->Reset();

            this->n_data = std::exchange(other.n_data, nullptr);
            this->n_deleter = VIOLET_MOVE(other.n_deleter);
            this->n_destroy = std::exchange(other.n_destroy, nullptr);
        }

        return *this;
    }

    /// Constructs `U` on the heap and returns `Unique<T>`.
    ///
    /// This method is particularly useful for constructing new `T` with forwarding arguments
    /// or for polymorphic construction to ensure that `U` is destroyed without requiring `T`
    /// to have a virtual destructor.
    ///
    /// @param args forwarded arguments to construct `U`.
    template<typename U = T, typename... Args>
        requires(std::constructible_from<U, Args...> && std::same_as<Deleter, DefaultDelete<T>>)
    static auto New(Args&&... args) -> Unique
    {
        static_assert(!std::is_abstract_v<T> || std::derived_from<U, T>, "ensure `U` is inherited from `T`");

        Unique result(new U(VIOLET_FWD(Args, args)...));
        result.n_destroy = &DestroyViaNew<U>;

        return result;
    }

    /// Returns a mutable pointer to the managed data.
    constexpr auto Get() noexcept -> T*
    {
        return this->n_data;
    }

    /// Returns a immutable pointer to the managed data.
    constexpr auto Get() const noexcept -> const T*
    {
        return this->n_data;
    }

    /// Returns **true** if this [`Unique`] holds the managed data.
    [[nodiscard]] constexpr auto Valid() const noexcept -> bool
    {
        return this->n_data != nullptr;
    }

    /// Disarm the destructor and hands a non-null pointer back to you.
    ///
    /// ## Panics
    /// This method will panic if `Leak` was called twice, [`Reset`] was called,
    /// or if the destructor for `Unique` already ran.
    auto Leak() noexcept -> NonNull<T>
    {
        VIOLET_ASSERT0(this->n_data != nullptr);

        this->n_destroy = nullptr;
        return NonNull<T>::NewUnchecked(Unsafe("checked via assertion"), std::exchange(this->n_data, nullptr));
    }

    /// Destroys the currently managed object (if any) and takes ownership of `data`.
    void Reset(T* data = nullptr) noexcept
    {
        if (this->n_data != nullptr) {
            this->destroy();
        }

        this->n_data = data;
        this->n_destroy = data != nullptr ? &DestroyViaDeleter : nullptr;
    }

    constexpr VIOLET_IMPLICIT operator bool() const noexcept
    {
        return this->Valid();
    }

    constexpr auto operator*() noexcept -> T&
    {
        return *this->Get();
    }

    constexpr auto operator*() const noexcept -> const T&
    {
        return *this->Get();
    }

    constexpr auto operator->() noexcept -> T*
    {
        return this->Get();
    }

    constexpr auto operator->() const noexcept -> const T*
    {
        return this->Get();
    }

    constexpr auto operator==(std::nullptr_t) const noexcept -> bool
    {
        return this->n_data == nullptr;
    }

    constexpr auto operator!=(std::nullptr_t) const noexcept -> bool
    {
        return this->Valid();
    }

    constexpr auto operator<=>(std::nullptr_t) const -> std::strong_ordering
    {
        constexpr auto cmp = std::compare_three_way{};
        return cmp(this->Get(), static_cast<const T*>(nullptr));
    }

    template<typename U, typename D2>
        requires std::three_way_comparable_with<T, U>
    constexpr auto operator<=>(const Unique<U, D2>& other) const -> std::compare_three_way_result_t<T, U>
    {
        if (this->Get() == nullptr || other.Get() == nullptr) {
            constexpr auto cmp = std::compare_three_way{};
            return cmp(this->Get(), other.Get());
        }

        return *this->Get() <=> *other.Get();
    }

private:
    using deleter_fn = void(void*, Deleter&);

    static void DestroyViaDeleter(void* ptr, Deleter& deleter)
    {
        std::invoke(deleter, static_cast<T*>(ptr));
    }

    template<typename U>
    static void DestroyViaNew(void* me, Deleter&)
    {
        delete static_cast<U*>(me);
    }

    void destroy()
    {
        VIOLET_ASSUME(this->n_data != nullptr);

        if (this->n_destroy != nullptr) {
            std::invoke(this->n_destroy, this->n_data, this->n_deleter);
        }
    }

    T* n_data = nullptr;
    VIOLET_NO_UNIQUE_ADDRESS Deleter n_deleter{};
    deleter_fn* n_destroy = nullptr;
};

} // namespace violet::experimental::ptr
