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
//! # 🌺💜 `violet/Experimental/Own.h`
//! Thread-safe, ref-counted smart pointer with Rust's [`Arc`]\<T\> semantics and modeled
//! by C++'s [`std::shared_ptr`].
//!
//! [`Arc`]: https://doc.rust-lang.org/1.95.0/std/sync/struct.Arc.html
//!
//! [`violet::experimental::sync::Own`]\<T\> is a thread-safe, intrusively ref-counted smart
//! pointer that provides shared ownership of a heap-allocated object of type `T`. Invoking
//! the copy constrructor will increment the reference count. The inner count is dropped (destroyed)
//! when the last owner ("unique owner") pointing to it is destroyed or reset via [`Own::Reset`].
//!
//! [`violet::experimental::sync::Weak`]\<T\> is a non-owning companion that holds a weak reference
//! to the same controlled block. Weak references do not keep the managed object alive, but they do
//! keep the control block alive so that [`Weak::Upgrade`] can atomically check whether the object
//! is still alive and, if so, promote the weak reference back into a strong [`Own`]ed object.
//!
//! ## Design
//! This also concludes why we decided to make our own smart pointers rather than stick with [`std::shared_ptr`].
//!
//! * Unlike C++'s [`std::shared_ptr`], [`Own`] propagates `const` through its accessors, which matches the
//!   semantics of Rust's `&Arc<T>` -> `&T`.
//! * Destruction of the managed object doesn't require a virtual destructor of `T`. The concrete destructor
//!   is captured when the shared object at construction time through a type-erased function pointer, which
//!   allows `Own<Base>::New<Derived>(...)` to correctly destroy `Derived` through a `Base`-typed handle.
//! * Construction of derived classes from base classes is more ergonomic.
//!
//! ## Thread Safety
//! The reference counters are atomic and safe to mutate from multiple threads. The managed object itself is **not**
//! synchronized, concurrent mutation requires external synchronization like [`violet::experimental::Synchronized<T>`]
//! or pairing with `violet::experimental::Mutex`, which depending on the context, can be [`std::mutex`] or
//! [`absl::Mutex`].
//!
//! ## Examples
//! ### Basic Usage
//! ```cpp
//! #include <violet/Experimental/Own.h>
//!
//! using namespace violet::experimental;
//!
//! Own<violet::Int32> ptr(42);
//! assert(*ptr == 42);
//! assert(ptr.StrongRefs() == 1);
//!
//! auto ptr2 = ptr;
//! assert(ptr.StrongRefs() == 2);
//! ```
//!
//! ### Polymorphic Construction
//! ```cpp
//! #include <violet/Experimental/Own.h>
//!
//! using namespace violet::experimental;
//!
//! struct Animal {
//!     violet::Int32 Legs;
//! };
//!
//! struct Cat final: public Animal {
//!     violet::String Name;
//!
//!     VIOLET_IMPLICIT Cat(violet::Str name, violet::Int32 legs) : Animal(legs), Name(name) {}
//! };
//!
//! Own<Animal> cat(std::in_place_type<Cat>, "Mochi", 4);
//! ```

#pragma once

#include <violet/Container/Optional.h>

#include <atomic>
#include <memory>

namespace violet::experimental {
namespace detail {
    /// Base control block shared by all allocation strategies.
    ///
    /// Contains the atomic reference counts and type-erased function pointers
    /// for destruction and deallocation. Lifted out of `Own<T>` so the control
    /// block has a single type independent of `T`, allowing upcasts like
    /// `Own<Derived> -> Own<Base>` to share the same block pointer.
    struct ablock {
        std::atomic<UInt> Strong = 0;
        std::atomic<UInt> Weak = 0;

        void (*Destruct)(void* me, void* data) = nullptr;
        void (*Deallocate)(void* blk) = nullptr;
    };

    /// Control block for the raw-pointer-with-deleter construction path.
    ///
    /// Stores a type-erased deleter. Stateless deleters (e.g. [`std::default_delete`])
    /// consume zero additional space via `[[no_unique_address]]` if supported.
    template<typename Delete>
    struct dblock final: public ablock {
        VIOLET_NO_UNIQUE_ADDRESS Delete Deleter;

        template<typename D>
        VIOLET_EXPLICIT dblock(D&& deleter)
            : Deleter(VIOLET_FWD(D, deleter))
        {
        }
    };

    /// Control block for the fused single-allocation construction path.
    ///
    /// Stores the allocator and provides inline storage for an object of type
    /// `U`. Stateless allocators consume zero additional space.
    template<typename U, typename Alloc = std::allocator<U>>
    struct block final: public ablock {
        VIOLET_NO_UNIQUE_ADDRESS Alloc Allocator;
        alignas(U) Array<UInt8, sizeof(U)> Storage;

        auto Data() -> U*
        {
            return std::launder(reinterpret_cast<U*>(this->Storage.data()));
        }

        auto Data() const -> const U*
        {
            return std::launder(reinterpret_cast<const U*>(this->Storage.data()));
        }
    };
} // namespace detail

template<typename T>
struct Weak;

template<typename T>
struct Own;

/// Detects whether `T` is a specialization of [`Own`].
///
/// Follows the convention of the standard `is_*` traits: the primary template
/// derives from [`std::false_type`], and the partial specialization for `Own<U>`
/// derives from [`std::true_type`]. Prefer the [`is_owned_v`] variable template in
/// most call sites.
template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") is_owned final: std::false_type { };

template<typename T>
struct is_owned<Own<T>>: std::true_type { };

/// `true` if `T` is a specialization of [`Own`], `false` otherwise.
template<typename T>
NOELDOC_EXPERIMENTAL_SINCE("26.06.05")
constexpr static inline bool is_owned_v = is_owned<T>::value;

/// Extracts the managed type from an [`Own`] specialization.
///
/// `owned_type<Own<U>>::type` is `U`. The primary template is left incomplete and
/// is only specialized for [`Own`], so naming `owned_type<T>::type` for any other
/// `T` is ill-formed; useful as a hard constraint. Prefer the [`owned_type_t`]
/// alias.
template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") owned_type;

template<typename T>
struct owned_type<Own<T>> final {
    using type = T;
};

/// The managed type of an [`Own`] specialization, i.e. `owned_type_t<Own<U>>` is `U`.
///
/// @since 26.06.05
template<typename T>
using owned_type_t = typename owned_type<T>::type;

/// Detects whether `T` is a specialization of [`Weak`].
///
/// The [`Weak`] counterpart to [`is_owned`]. Prefer the [`is_weak_v`] variable
/// template in most call sites.
template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("26.07.03") is_weak final: std::false_type { };

template<typename T>
struct is_weak<Weak<T>>: std::true_type { };

/// `true` if `T` is a specialization of [`Weak`], `false` otherwise.
template<typename T>
NOELDOC_EXPERIMENTAL_SINCE("26.07.03")
constexpr static inline bool is_weak_v = is_weak<T>::value;

/// Extracts the referenced type from a [`Weak`] specialization.
///
/// `weak_type<Weak<U>>::type` is `U`. The [`Weak`] counterpart to [`owned_type`];
/// the primary template is incomplete, so naming `::type` for a non-[`Weak`] type
/// is ill-formed. Prefer the [`weak_type_t`] alias.
template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("26.07.03") weak_type;

template<typename T>
struct weak_type<Weak<T>> final {
    using type = T;
};

/// The referenced type of a [`Weak`] specialization, i.e. `weak_type_t<Weak<U>>` is `U`.
///
/// @since 26.07.03
template<typename T>
using weak_type_t = typename weak_type<T>::type;

/// A stateless deleter that does nothing when invoked.
///
/// Pass this as the deleter to [`Own`]'s raw-pointer constructor when the handle
/// should participate in shared ownership without ever freeing the pointee. For
/// example when the managed object has automatic or static storage duration, or
/// is owned elsewhere:
///
/// ```cpp
/// int y = 32;
/// Own<int> ref(&y, NoOpDeleter());   // shares `&y`, never deletes it
/// ```
struct NOELDOC_EXPERIMENTAL_SINCE("26.07.03") NoOpDeleter final {
    /// Constructs a [`NoOpDeleter`]. Stateless, so this is trivial.
    constexpr VIOLET_IMPLICIT NoOpDeleter() = default;

    /// Does nothing. The pointee is intentionally left untouched.
    template<typename T>
    constexpr void operator()(T*) const noexcept
    {
        static_assert(!std::is_function_v<T>, "`NoOpDeleter` cannot be instantiated for function types");
        static_assert(sizeof(T) >= 0 && !std::is_void_v<T>, "cannot delete an incomplete type");
    }
};

/// A thread-safe, reference-counted smart pointer with shared ownership.
///
/// View the [module documentation](#) for more information.
template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("26.06.05") Own final {
    using value_type = T;

    ~Own()
    {
        this->Release();
    }

    /// Constructs a dummy, null [`Own`] that owns nothing.
    VIOLET_IMPLICIT Own() noexcept = default;

    /// Constructs a dummy, null [`Own`] that owns nothing.
    VIOLET_IMPLICIT Own(std::nullptr_t) noexcept { }

    /// Takes ownership of a raw pointer with a custom deleter (defaults to [`std::default_delete`]).
    ///
    /// The deleter is type-erased and stores in a separate contrrol block. When the last [`Own`] is destroyed,
    /// `deleter(data)` is invoked.
    ///
    /// ## Example
    /// ```cpp
    /// // with default deleter (calls `delete ptr;`)
    /// Own<Foo> ptr(new Foo());
    ///
    /// // with a custom deleter
    /// Own<FILE> file(::fopen("x.txt", "r"), [](FILE* file) -> void { ::fclose(file); });
    ///
    /// // Non-owning
    /// violet::Int32 x = 42;
    /// Own<violet::Int32> ref(&x, [](violet::Int32*) -> void {})
    /// ```
    ///
    /// @param data    raw pointer that this [`Own`]ed structure will own.
    /// @param deleter custom deleter to properly destruct the data.
    template<typename U = T, typename Deleter = std::default_delete<U>>
        requires(std::convertible_to<U*, T*>)
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
    VIOLET_IMPLICIT Own(U* data, Deleter&& deleter = { })
        : n_data(data)
    {
        auto* blk = new detail::dblock<Deleter>(VIOLET_FWD(Deleter, deleter));
        blk->Strong.store(1, std::memory_order_relaxed);
        blk->Weak.store(0, std::memory_order_relaxed);
        blk->Destruct = [](void* me, void* data) -> void {
            auto* self = static_cast<detail::dblock<Deleter>*>(me);
            if (data != nullptr) {
                self->Deleter(static_cast<U*>(data));
            }
        };

        blk->Deallocate = [](void* me) -> void {
            auto* self = static_cast<detail::dblock<Deleter>*>(me);
            delete self;
        };

        this->n_blk = blk;
    }

    /// Shares ownership with `owner` but exposes `ptr`.
    ///
    /// The new handle joins `owner`'s control block (incrementing the strong
    /// count), keeping the managed object alive, while [`Get`] returns `ptr`. `ptr`
    /// must address a subobject whose lifetime is bound by `owner`. No independent
    /// lifetime is tracked for it. A null `owner` yields a null handle regardless
    /// of `ptr`.
    template<typename U>
    NOELDOC_EXPERIMENTAL_SINCE("26.07.03")
    VIOLET_EXPLICIT Own(const Own<U>& owner, T* ptr) noexcept
        : n_data(owner.n_blk != nullptr ? ptr : nullptr)
        , n_blk(owner.n_blk)
    {
        if (this->n_blk != nullptr) {
            this->n_blk->Strong.fetch_add(1, std::memory_order_relaxed);
        }
    }

    /// Assumes ownership from `owner` while exposing `ptr`.
    ///
    /// The aliasing analogue of the move constructor: rather than incrementing the
    /// strong count, it steals `owner`'s control block and leaves `owner` null. `ptr`
    /// must address a subobject whose lifetime is bound by the managed object. No
    /// independent lifetime is tracked for it. A null `owner` yields a null handle
    /// regardless of `ptr`. Saves the atomic increment/decrement pair that the
    /// copying overload incurs.
    template<typename U>
    NOELDOC_EXPERIMENTAL_SINCE("26.07.03")
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    VIOLET_EXPLICIT Own(Own<U>&& owner, T* ptr) noexcept
        : n_data(owner.n_blk != nullptr ? ptr : nullptr)
        , n_blk(std::exchange(owner.n_blk, nullptr))
    {
        owner.n_data = nullptr;
    }

    /// Constructs a new [`Own<T>`] from an [`Own<U>`] where `U*` is convertible
    /// to `T*`, incrementing the strong reference count.
    template<typename U>
        requires(std::convertible_to<U*, T*> && !std::same_as<U, T>)
    VIOLET_IMPLICIT Own(const Own<U>& other) noexcept
        : n_data(other.n_data)
        , n_blk(other.n_blk)
    {
        if (this->n_blk != nullptr) {
            this->n_blk->Strong.fetch_add(1, std::memory_order_relaxed);
        }
    }

    /// Upcasting copy-assigns from an [`Own<U>`] where `U*` is convertible to `T*`.
    ///
    /// Releases any object currently held by `*this`, then adopts `other`'s control
    /// block and increments the strong reference count.
    template<typename U>
        requires(std::convertible_to<U*, T*> && !std::same_as<U, T>)
    auto operator=(const Own<U>& other) noexcept -> Own&
    {
        this->Release();

        this->n_data = other.n_data;
        this->n_blk = other.n_blk;
        if (this->n_blk != nullptr) {
            this->n_blk->Strong.fetch_add(1, std::memory_order_relaxed);
        }

        return *this;
    }

    /// Move-constructs from an [`Own<U>`], transferring ownership without
    /// touching the reference count.
    template<typename U>
        requires(std::convertible_to<U*, T*> && !std::same_as<U, T>)
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    VIOLET_IMPLICIT Own(Own<U>&& other) noexcept
        : n_data(std::exchange(other.n_data, nullptr))
        , n_blk(std::exchange(other.n_blk, nullptr))
    {
    }

    /// Upcasting move-assigns from an [`Own<U>`], transferring ownership without
    /// touching the reference count. The previously held object, if any, is released.
    template<typename U>
        requires(std::convertible_to<U*, T*> && !std::same_as<U, T>)
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    auto operator=(Own<U>&& other) noexcept -> Own&
    {
        this->Release();

        this->n_data = std::exchange(other.n_data, nullptr);
        this->n_blk = std::exchange(other.n_blk, nullptr);

        return *this;
    }

    /// Copy constructor. Increments the strong reference count.
    ///
    /// ## Example
    /// ```cpp
    /// Own<violet::Int32> a = Own<violet::Int32>::New(42);
    /// auto b = a;
    /// assert(a.StrongRefs() == 2);
    /// ```
    VIOLET_IMPLICIT Own(const Own& other) noexcept
        : n_data(other.n_data)
        , n_blk(other.n_blk)
    {
        if (this->n_blk != nullptr) {
            this->n_blk->Strong.fetch_add(1, std::memory_order_relaxed);
        }
    }

    /// Copy-assignment. Releases the currently held object (if any), adopts `other`'s
    /// control block and increments the strong reference count. Self-assignment is a
    /// no-op.
    auto operator=(const Own& other) noexcept -> Own&
    {
        if (this != &other) {
            this->Release();

            this->n_data = other.n_data;
            this->n_blk = other.n_blk;
            if (this->n_blk != nullptr) {
                this->n_blk->Strong.fetch_add(1, std::memory_order_relaxed);
            }
        }

        return *this;
    }

    /// Move-constructs, leaving `other` null.
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    VIOLET_IMPLICIT Own(Own&& other) noexcept
        : n_data(std::exchange(other.n_data, nullptr))
        , n_blk(std::exchange(other.n_blk, nullptr))
    {
    }

    /// Move-assignment. Releases the currently held object (if any) and steals
    /// `other`'s control block, leaving `other` null. Self-assignment is a no-op.
    auto operator=(Own&& other) noexcept -> Own&
    {
        if (this != &other) {
            this->Release();

            this->n_data = std::exchange(other.n_data, nullptr);
            this->n_blk = std::exchange(other.n_blk, nullptr);
        }

        return *this;
    }

    /// Constructs `U` in-place with a fused allocation, using the default allocator.
    ///
    /// ## Examples
    /// ```cpp
    /// Own<Animal> pet(std::in_place_type<Cat>, "Moshi", 4);
    /// ```
    ///
    /// @param args arguments to supply when constructing `U`.
    template<typename U = T, typename Alloc = std::allocator<U>, typename... Args>
        requires(std::constructible_from<U, Args...> && std::convertible_to<U*, T*>)
    VIOLET_EXPLICIT Own(std::in_place_type_t<U>, Args&&... args)
        : Own(New<U>(VIOLET_FWD(Args, args)...))
    {
    }

    /// Constructs `U` in-place with a fused allocation, using a provided allocator.
    ///
    /// @param alloc the allocator to spply
    /// @param args arguments to supply when constructing `U`.
    template<typename U = T, typename Alloc = std::allocator<U>, typename... Args>
        requires(std::constructible_from<U, Args...> && std::convertible_to<U*, T*>)
    VIOLET_EXPLICIT Own(std::in_place_type_t<U>, Alloc alloc, Args&&... args)
        : Own(NewIn<U, Alloc>(VIOLET_MOVE(alloc), VIOLET_FWD(Args, args)...))
    {
    }

    /// Creates a new [`Own`] object with a fused single allocation containing both
    /// the control block and object, using the default allocator.
    ///
    /// This is the preferred construction path. The type parameter `U` allows
    /// constructing a derived type while returning `Own<Base>`:
    ///
    /// ```cpp
    /// Own<Animal> pet = Own<Animal>::New<Cat>("Moshi", 4);
    /// ```
    template<typename U = T, typename Alloc = std::allocator<U>, typename... Args>
        requires(std::constructible_from<U, Args...> && std::convertible_to<U*, T*>)
    static auto New(Args&&... args) -> Own
    {
        return Own::NewIn<U, Alloc>(Alloc{ }, VIOLET_FWD(Args, args)...);
    }

    /// Creates a new [`Own<T>`] with a fused single allocation, using a custom
    /// allocator.
    ///
    /// The allocator is rebound to the internal block type and stored alongside
    /// the object. Stateless allocators (e.g. [`std::allocator`]) consume zero
    /// additional space via `[[no_unique_address]]` if supported.
    template<typename U = T, typename Alloc = std::allocator<U>, typename... Args>
        requires(std::constructible_from<U, Args...> && std::convertible_to<U*, T*>)
    static auto NewIn(Alloc alloc, Args&&... args) -> Own
    {
        using blk = detail::block<U, Alloc>;
        using block_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<blk>;

        block_allocator block_alloc(alloc);
        blk* block = std::allocator_traits<block_allocator>::allocate(block_alloc, 1);
        std::construct_at(block);
        block->Strong.store(1, std::memory_order_relaxed);
        block->Weak.store(0, std::memory_order_relaxed);
        block->Allocator = alloc;

        if constexpr (!std::is_trivially_destructible_v<U>) {
            block->Destruct = [](void* /*me*/, void* data) -> void { static_cast<U*>(data)->~U(); };
        } else {
            block->Destruct = nullptr;
        }

        block->Deallocate = [](void* me) -> void {
            auto* self = static_cast<blk*>(me);
            Alloc owned_alloc(VIOLET_MOVE(self->Allocator));
            block_allocator owned_block_alloc(owned_alloc);

            std::destroy_at(self);
            std::allocator_traits<block_allocator>::deallocate(owned_block_alloc, self, 1);
        };

        std::allocator_traits<Alloc>::construct(alloc, block->Data(), VIOLET_FWD(Args, args)...);
        return Own(private_tag(), static_cast<T*>(block->Data()), block);
    }

    /// Returns the current strong reference count.
    ///
    /// The returned value is an approximation in multi-threaded contexts; by the
    /// time the caller inspects it, it may have already changed.
    [[nodiscard]] auto StrongRefs() const -> UInt
    {
        return this->n_blk != nullptr ? this->n_blk->Strong.load(std::memory_order_relaxed) : 0;
    }

    /// Returns the current weak reference count.
    [[nodiscard]] auto WeakRefs() const -> UInt
    {
        return this->n_blk != nullptr ? this->n_blk->Weak.load(std::memory_order_relaxed) : 0;
    }

    /// Returns `true` if this is the only strong reference.
    [[nodiscard]] auto Unique() const -> bool
    {
        return this->n_blk->Strong.load(std::memory_order_relaxed) == 1;
    }

    /// Returns a mutable pointer to the managed object, or `nullptr`.
    NOELDOC_CONSTEXPR_SINCE("26.07.03") constexpr auto Get() -> T*
    {
        return this->n_data;
    }

    /// Returns a immutable pointer to the managed object, or `nullptr`.
    NOELDOC_CONSTEXPR_SINCE("26.07.03") constexpr auto Get() const -> const T*
    {
        return this->n_data;
    }

#if VIOLET_FEATURE(RTTI) || VIOLET_FEATURE(NOELDOC)
    /// Shares ownership while attempting a checked `dynamic_cast` to `U`.
    ///
    /// On success the returned [`Own<U>`] joins this handle's control block
    /// (incrementing the strong count); this handle stays valid. Returns an empty
    /// [`Own`] if the `dynamic_cast` fails or this handle is empty. Requires `T` to
    /// be polymorphic.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto DynamicCast() & -> Own<U>
        requires std::is_polymorphic_v<T>
    {
        auto* casted = dynamic_cast<U*>(this->n_data);
        if (casted != nullptr) {
            return Own<U>(*this, casted);
        }

        return nullptr;
    }

    /// Shares ownership while attempting a checked `dynamic_cast` to `U`.
    ///
    /// The `const`-lvalue overload; identical in behavior to the non-`const` one.
    /// On success the returned [`Own<U>`] joins this handle's control block
    /// (incrementing the strong count); this handle stays valid. Returns an empty
    /// [`Own`] if the `dynamic_cast` fails or this handle is empty. Requires `T` to
    /// be polymorphic.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto DynamicCast() const& -> Own<U>
        requires std::is_polymorphic_v<T>
    {
        auto* casted = dynamic_cast<U*>(this->n_data);
        if (casted != nullptr) {
            return Own<U>(*this, casted);
        }

        return nullptr;
    }

    /// Steals ownership while attempting a checked `dynamic_cast` to `U`.
    ///
    /// **Always consumes this handle, leaving it empty**, regardless of whether the
    /// cast succeeds. On success the returned [`Own<U>`] takes over the control
    /// block without touching the strong count; on failure this handle is still
    /// released and an empty [`Own`] is returned. This keeps the moved-from state
    /// predictable rather than dependent on the runtime cast result. Requires `T`
    /// to be polymorphic.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto DynamicCast() && -> Own<U>
        requires std::is_polymorphic_v<T>
    {
        auto* casted = dynamic_cast<U*>(this->n_data);
        if (casted != nullptr) {
            return Own<U>(VIOLET_MOVE(*this), casted);
        }

        // The cast failed, but this is an rvalue overload: consume `*this`
        // unconditionally so the moved-from state never depends on the result.
        this->Reset();
        return nullptr;
    }
#endif

    /// Shares ownership while reinterpreting the managed pointer via `static_cast`.
    ///
    /// No RTTI required. The conversion must be statically valid (an up- or
    /// downcast within a known hierarchy); use [`DynamicCast`] when the
    /// relationship must be verified at runtime. Joins this handle's control block
    /// (incrementing the strong count); this handle stays valid. Yields an empty
    /// [`Own`] when this handle is empty.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto StaticCast() & -> Own<U>
    {
        return Own<U>(*this, static_cast<U*>(this->n_data));
    }

    /// Shares ownership while reinterpreting the managed pointer via `static_cast`.
    ///
    /// No RTTI required. The conversion must be statically valid (an up- or
    /// downcast within a known hierarchy); use [`DynamicCast`] when the
    /// relationship must be verified at runtime. Yields an empty [`Own`] when this
    /// handle is empty.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto StaticCast() const& -> Own<U>
    {
        return Own<U>(*this, static_cast<U*>(this->n_data));
    }

    /// Steals ownership while reinterpreting the managed pointer via `static_cast`.
    ///
    /// The moving counterpart of the lvalue overloads: transfers this handle's
    /// control block to the result without touching the strong count, leaving this
    /// handle empty. No RTTI required; the conversion must be statically valid (an
    /// up- or downcast within a known hierarchy). Yields an empty [`Own`] when this
    /// handle is empty.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto StaticCast() && -> Own<U>
    {
        return Own<U>(VIOLET_MOVE(*this), static_cast<U*>(this->n_data));
    }

    /// Shares ownership while adding or removing `const` via `const_cast`.
    ///
    /// The canonical way to convert between `Own<T>` and `Own<const T>` without
    /// allocating a fresh control block. Joins this handle's control block
    /// (incrementing the strong count); this handle stays valid. Yields an empty
    /// [`Own`] when this handle is empty.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto ConstCast() & -> Own<U>
    {
        return Own<U>(*this, const_cast<U*>(this->n_data));
    }

    /// Shares ownership while adding or removing `const` via `const_cast`.
    ///
    /// The canonical way to convert between `Own<T>` and `Own<const T>` without
    /// allocating a fresh control block. Yields an empty [`Own`] when this handle
    /// is empty.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto ConstCast() const& -> Own<U>
    {
        return Own<U>(*this, const_cast<U*>(this->n_data));
    }

    /// Steals ownership while adding or removing `const` via `const_cast`.
    ///
    /// The moving counterpart of the lvalue overloads: transfers the control block
    /// to the result without touching the strong count, leaving this handle empty.
    /// The allocation-free way to convert between `Own<T>` and `Own<const T>`.
    /// Yields an empty [`Own`] when this handle is empty.
    template<typename U>
    [[nodiscard]] NOELDOC_SINCE("26.07.03") auto ConstCast() && -> Own<U>
    {
        return Own<U>(VIOLET_MOVE(*this), const_cast<U*>(this->n_data));
    }

    /// Releases ownership and resets this handle to null.
    ///
    /// If this was the last strong reference, the managed object is destroyed.
    /// If there are also no remaining weak references, the control block is
    /// deallocated.
    void Reset()
    {
        this->Release();
        this->n_data = nullptr;
        this->n_blk = nullptr;
    }

    /// Replaces the managed object with a new raw pointer and deleter.
    ///
    /// Equivalent to `*this = Own(data, deleter)`.
    template<typename U = T, typename Deleter = std::default_delete<U>>
    void Reset(U* data, Deleter deleter = { })
    {
        *this = Own(data, VIOLET_MOVE(deleter));
    }

    /// Decrements the strong reference count. If it reaches zero, destroys the
    /// managed object and (if no weak references remain) deallocates the control
    /// block.
    void Release()
    {
        if (this->n_blk != nullptr) {
            if (this->n_blk->Strong.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                if (this->n_blk->Destruct != nullptr) {
                    if constexpr (std::is_const_v<T> || std::is_volatile_v<T>) {
                        // SAFETY: This is only used in `Own<const T>`/`Own<volatile T>`, which
                        // is produced most likely by `ConstCast` but the `Destruct` vtable function
                        // erases its argument to `void*`. The captured
                        // destructor was bound to the unqualified type, so stripping `const`
                        // here is sound.
                        this->n_blk->Destruct(
                            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                            this->n_blk, const_cast<void*>(static_cast<const volatile void*>(this->n_data)));
                    } else {
                        this->n_blk->Destruct(this->n_blk, this->n_data);
                    }
                }

                if (this->n_blk->Weak.load(std::memory_order_acquire) == 0) {
                    this->n_blk->Deallocate(this->n_blk);
                }
            }
        }
    }

    /// Returns a string representation of the managed object.
    [[nodiscard]] auto ToString() const -> String
        requires(Stringify<T>)
    {
        return violet::ToString(*this->Get());
    }

    /// Creates a [`Weak`] that references the same control block without
    /// preventing destruction of the managed object.
    auto Downgrade() -> Weak<T>;

    /// Streams the managed object to `os`, preferring [`Stringify`] if available
    /// and falling back to `T`'s own `operator<<` overload.
    friend auto operator<<(std::ostream& os, const Own& self) -> std::ostream
        requires(Stringify<T> || requires(std::ostream& os) { os << *self.Get(); })
    {
        if constexpr (Stringify<T>) {
            return os << self.ToString();
        } else if constexpr (requires { os << *self.Get(); }) {
            return os << *self.Get();
        } else {
            VIOLET_UNREACHABLE();
        }
    }

    NOELDOC_CONSTEXPR_SINCE("26.07.03") constexpr VIOLET_EXPLICIT operator bool() const
    {
        return this->n_data != nullptr;
    }

    NOELDOC_CONSTEXPR_SINCE("26.07.03") constexpr auto operator->() -> T*
    {
        return this->Get();
    }

    NOELDOC_CONSTEXPR_SINCE("26.07.03") constexpr auto operator->() const -> const T*
    {
        return this->Get();
    }

    NOELDOC_CONSTEXPR_SINCE("26.07.03") constexpr auto operator*() -> T&
    {
        return *this->n_data;
    }

    NOELDOC_CONSTEXPR_SINCE("26.07.03") constexpr auto operator*() const -> const T&
    {
        return *this->n_data;
    }

    NOELDOC_SINCE("26.07.03") constexpr auto operator==(std::nullptr_t) const -> bool
    {
        return this->n_data == nullptr;
    }

    NOELDOC_SINCE("26.07.03") constexpr auto operator!=(std::nullptr_t) const -> bool
    {
        return this->n_data != nullptr;
    }

    NOELDOC_SINCE("26.07.03") constexpr auto operator<=>(std::nullptr_t) const -> std::strong_ordering
    {
        constexpr auto cmp = std::compare_three_way{ };
        return cmp(this->Get(), static_cast<const T*>(nullptr));
    }

    template<typename U>
        requires(std::three_way_comparable_with<T, U>)
    NOELDOC_SINCE("26.07.03")
    constexpr auto operator<=>(const Own<U>& other) const -> std::compare_three_way_result_t<T, U>
    {
        // If either `*this` or `other` is null, then order by nulls first, let
        // pointer identity break the tie.
        if (this->Get() == nullptr || other.Get() == nullptr) {
            constexpr auto cmp = std::compare_three_way{ };
            return cmp(this->Get(), other.Get());
        }

        return *this->Get() <=> *other.Get();
    }

private:
    template<typename U>
    friend struct Own;
    friend struct Weak<T>;

    struct private_tag final { };

    VIOLET_EXPLICIT Own(private_tag, T* data, detail::ablock* blk)
        : n_data(data)
        , n_blk(blk)
    {
    }

    T* n_data = nullptr;
    detail::ablock* n_blk = nullptr;
};

/// A non-owning weak reference to a value managed by [`Own<T>`].
///
/// [`Weak<T>`] references the same control block as an [`Own<T>`] but does not
/// contribute to the strong reference count. The managed object may be destroyed
/// while [`Weak<T>`] handles still exist; the control block itself is kept alive
/// until both the strong **and** weak counts reach zero.
///
/// Use [`Upgrade`] to attempt promotion back to an [`Own<T>`]. The promotion
/// atomically succeeds only if the strong count is non-zero at the moment of
/// the compare-exchange.
///
/// ## Example
/// ```cpp
/// auto strong = Own<int>::New(99);
/// Weak<int> weak = strong.Downgrade();
///
/// if (auto upgraded = weak.Upgrade(); upgraded.HasValue()) {
///     assert(**upgraded == 99);
/// }
///
/// strong.Reset();
/// assert(!weak.Upgrade());
/// ```
template<typename T>
struct Weak final {
    /// Constructs a null [`Weak`] that references no control block.
    VIOLET_IMPLICIT Weak() noexcept = default;

    /// Decrements the weak reference count and deallocates the control block if both
    /// strong and weak counts have reached zero.
    ~Weak()
    {
        this->release();
    }

    /// Copy-constructs a [`Weak`], incrementing the weak reference count.
    VIOLET_IMPLICIT Weak(const Weak& other) noexcept
        : n_data(other.n_data)
        , n_blk(other.n_blk)
    {
        if (this->n_blk != nullptr) {
            this->n_blk->Weak.fetch_add(1, std::memory_order_relaxed);
        }
    }

    /// Copy-assignment. Releases the currently referenced control block (if any)
    /// and adopts `other`'s, incrementing the weak reference count.
    auto operator=(const Weak& other) noexcept -> Weak&
    {
        if (this != &other) {
            this->release();

            this->n_data = other.n_data;
            this->n_blk = other.n_blk;
            if (this->n_blk != nullptr) {
                this->n_blk->Weak.fetch_add(1, std::memory_order_relaxed);
            }
        }

        return *this;
    }

    /// Move-constructs a [`Weak`], leaving `other` null without touching the
    /// reference count.
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    VIOLET_IMPLICIT Weak(Weak&& other) noexcept
        : n_data(std::exchange(other.n_data, nullptr))
        , n_blk(std::exchange(other.n_blk, nullptr))
    {
    }

    /// Move-assignment. Releases the currently referenced control block (if any)
    /// and steals `other`'s, leaving `other` null.
    auto operator=(Weak&& other) noexcept -> Weak&
    {
        if (this != &other) {
            this->release();

            this->n_data = std::exchange(other.n_data, nullptr);
            this->n_blk = std::exchange(other.n_blk, nullptr);
        }

        return *this;
    }

    /// Returns the current strong reference count, or `0` if this [`Weak`] is null.
    [[nodiscard]] auto StrongRefs() const -> UInt
    {
        return this->n_blk != nullptr ? this->n_blk->Strong.load(std::memory_order_relaxed) : 0;
    }

    /// Returns the current weak reference count, or `0` if this [`Weak`] is null.
    [[nodiscard]] auto WeakRefs() const -> UInt
    {
        return this->n_blk != nullptr ? this->n_blk->Weak.load(std::memory_order_relaxed) : 0;
    }

    /// Attempts to promote this [`Weak`] back into a strong [`Own<T>`].
    ///
    /// Atomically succeeds only if the strong count is non-zero at the moment of the
    /// compare-exchange. Returns [`Nothing`] if the managed object has already been
    /// destroyed or this [`Weak`] is null.
    auto Upgrade() const -> Optional<Own<T>>
    {
        if (this->n_blk == nullptr) {
            return Nothing;
        }

        auto old = this->n_blk->Strong.load(std::memory_order_relaxed);
        while (old != 0) {
            if (this->n_blk->Strong.compare_exchange_weak(
                    old, old + 1, std::memory_order_acquire, std::memory_order_relaxed)) {
                return Own<T>(typename Own<T>::private_tag(), this->n_data, this->n_blk);
            }
        }

        return Nothing;
    }

private:
    friend struct Own<T>;

    VIOLET_EXPLICIT Weak(T* data, detail::ablock* blk) noexcept
        : n_data(data)
        , n_blk(blk)
    {
        if (this->n_blk != nullptr) {
            this->n_blk->Weak.fetch_add(1, std::memory_order_relaxed);
        }
    }

    T* n_data = nullptr;
    detail::ablock* n_blk = nullptr;

    void release()
    {
        if (this->n_blk == nullptr) {
            return;
        }

        if (this->n_blk->Weak.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            if (this->n_blk->Strong.load(std::memory_order_acquire) == 0) {
                this->n_blk->Deallocate(this->n_blk);
            }
        }
    }
};

template<typename T>
inline auto Own<T>::Downgrade() -> Weak<T>
{
    return Weak<T>(this->n_data, this->n_blk);
}

} // namespace violet::experimental
