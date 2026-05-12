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
//! # 🌺💜 `violet/Experimental/Language/Any.h`

#pragma once

#include <violet/Container/Optional.h>

#if !VIOLET_FEATURE(RTTI)
#include <violet/Experimental/Language/TypeId.h>
#else
#include <violet/Support/Demangle.h>

#include <typeindex>
#endif

#include <sstream>

namespace violet::experimental {

/// A type-erased virtual dispatch table for [`Any`]-like containers.
///
/// `AnyVTable` stores function pointers for converting to string, cloning,
/// and destructing a type-erased value. It is constructed at compile time
/// via [`For<T>()`], which generates the appropriate implementations based
/// on `T`'s capabilities.
///
/// ## Design
/// Rather than using inheritance and virtual methods, `AnyVTable` uses
/// a C-style vtable with function pointers operating on `void*`. This
/// avoids the overhead of virtual dispatch and allows the vtable itself
/// to be `constexpr`-constructed.
struct VIOLET_API AnyVTable {
    /// Converts the type-erased value at `ptr` to a string representation.
    ///
    /// The resolution order is:
    ///     1. `violet::ToString(*self)` if `violet::Stringify<T>` is satisfied.
    ///     2. `operator<<(std::ostream&, T)` if available.
    ///     3. RTTI-based demangled type name and hash (if RTTI is enabled).
    ///     4. A static fallback string.
    String (*ToString)(const void* ptr) = nullptr;

    /// Copy-constructs `T` from `src` into the storage at `dst` using placement new.
    ///
    /// Returns a pointer to the newly constructed object (which is `dst`).
    /// `nullptr` if `T` is not copy-constructible, callers must check before invoking.
    void* (*Clone)(const void* src, void* dst) = nullptr;

    /// Destroys the object at `ptr` via `std::destroy_at` and deallocates
    /// the memory with `::operator delete`.
    void (*Destruct)(void* ptr) = nullptr;

    /// Constructs an `AnyVTable` for the given type `T` at compile time.
    ///
    /// The generated vtable adapts to `T`'s capabilities:
    /// - `ToString` is always populated.
    /// - `Clone` is only populated if `T` is copy-constructible.
    /// - `Destruct` is always populated.
    ///
    /// ## Type Requirements
    /// - `T` must be destructible.
    /// - `T` does **not** need to be copy-constructible. If it isn't, `Clone` will be `nullptr`.
    ///
    /// @tparam T the concrete type to generate the vtable for.
    template<typename T>
    constexpr static auto For() -> AnyVTable
    {
        AnyVTable vtable;

        vtable.ToString = [](const void* ptr) -> String {
            const auto* self = static_cast<const T*>(ptr);
            VIOLET_DEBUG_ASSERT(self != nullptr, "object didn't outlive `AnyVTable`");

            if constexpr (violet::Stringify<T>) {
                return violet::ToString(*self);
            }

            if constexpr (requires(std::ostream& os) { os << *self; }) {
                std::ostringstream os;
                os << *self;

                return os.str();
            }

#if VIOLET_FEATURE(RTTI)
            const auto& type = typeid(T);
            return std::format("<type {}@{}>", util::DemangleCXXName(type.name()), type.hash_code());
#else
            return "cannot get stringified representation";
#endif
        };

        if constexpr (std::is_copy_constructible_v<T>) {
            vtable.Clone = [](const void* src, void* dst) -> void* {
                const auto* self = static_cast<const T*>(src);
                VIOLET_DEBUG_ASSERT(self != nullptr, "object didn't outlive `AnyVTable`");

                return new (dst) T(*self);
            };
        }

        vtable.Destruct = [](void* ptr) -> void {
            auto* self = static_cast<T*>(ptr);
            VIOLET_DEBUG_ASSERT(self != nullptr, "object didn't outlive `AnyVTable`");

            std::destroy_at(self);
            ::operator delete(self);
        };

        return vtable;
    }
};

/// A type-erased container that can hold any value.
///
/// `Any` provides a way to store arbitrary typed values behind a unified interface,
/// similar to [`std::any`] but with additional features like stringification with
/// [`operator<<`] or with the [`violet::Stringify`] concept, cloning via a virtual
/// table, and being non-RTTI friendly.
///
/// ## Downcasting
/// You can retrieve the stored value with [`Any::Downcast<T>()`], which will return [`violet::Nothing`]
/// if the requested type does not match the stored type.
///
/// ```cpp
/// #include <violet/Experimental/Any.h>
///
/// using violet::experimental::Any;
///
/// Any integral = Any::New<int>(42);
/// assert(integral.Downcast<int>().HasValue());
/// assert(!integral.Downcast<float>().HasValue());
/// ```
///
/// ## Copy/Move Semantics
/// [`violet::experimental::Any`] is both copyable and movable. Copies will perform a deep clone of the stored
/// object via the virtual table's `Clone' entry. Moving `Any` will transfer ownership, leaving
/// the source in a valid, but empty state.
struct VIOLET_API Any final {
    VIOLET_DISALLOW_CONSTRUCTOR(Any);
    ~Any();

    VIOLET_IMPLICIT Any(const Any& other) noexcept;
    auto operator=(const Any& other) noexcept -> Any&;

    VIOLET_IMPLICIT Any(Any&& other) noexcept;
    auto operator=(Any&& other) noexcept -> Any&;

    template<typename T>
        requires(!std::same_as<std::decay_t<T>, Any>)
    VIOLET_IMPLICIT Any(T&& value) // NOLINT(cppcoreguidelines-pro-type-member-init)
#if VIOLET_FEATURE(RTTI)
        : n_type(typeid(std::decay_t<T>))
#else
        : n_type(TypeId::Of<std::decay_t<T>>())
#endif
    {
        using U = std::decay_t<T>;

        constexpr UInt size = sizeof(U);
        constexpr static auto vtable = AnyVTable::For<U>();

        void* self = ::operator new(size);
        std::construct_at(static_cast<T*>(self), VIOLET_FWD(T, value));

        this->n_self = self;
        this->n_size = size;
        this->n_vtable = vtable;
    }

    template<typename T, typename... Args>
        requires(std::constructible_from<T, Args...>)
    VIOLET_EXPLICIT Any(std::in_place_type_t<T>, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
#if VIOLET_FEATURE(RTTI)
        : n_type(typeid(T))
#else
        : n_type(TypeId::Of<T>())
#endif
    {
        constexpr UInt size = sizeof(T);
        constexpr static auto vtable = AnyVTable::For<T>();

        void* self = ::operator new(size);
        std::construct_at(static_cast<T*>(self), VIOLET_FWD(Args, args)...);

        this->n_self = self;
        this->n_size = size;
        this->n_vtable = vtable;
    }

    /// Construct a new [`Any`] object holding a value of `T`.
    ///
    /// ## Examples
    /// ```cpp
    /// #include <violet/Experimental/Any.h>
    ///
    /// using violet::experimental::Any;
    ///
    /// auto i = Any::New<int>(42);
    /// auto s = Any::New<violet::String>(5, 'x'); // "xxxxx"
    /// ```
    ///
    /// @tparam T value type
    /// @tparam Args forwarded arguments for `T`'s constructor.
    /// @param args arguments for `T`'s constructor.
    template<typename T, typename... Args>
    static auto New(Args&&... args) -> Any
    {
        return Any(std::in_place_type<T>, VIOLET_FWD(Args, args)...);
    }

    /// Attempts to downcast the stored value to type `T`.
    ///
    /// @tparam T expected type of the stored value to downcast from
    /// @returns a copy of the stored value if `T` was matched, [`violet::Nothing`] otherwise.
    template<typename T>
    auto Downcast() const noexcept -> Optional<T>
    {
#if VIOLET_FEATURE(RTTI)
        if (this->n_type != typeid(T)) {
#else
        if (this->n_type != TypeId::Of<T>()) {
#endif

            return Nothing;
        }

        return this->DowncastUnchecked<T>(Unsafe("presumed \"unsafe\" checks were checked already"));
    }

    /// Unsafely downcasts the stored value of type `T` regardless if it is of type `T`.
    /// @tparam T expected type of the stored value to downcast from
    /// @returns a copy of the stored value that casts to `T`.
    template<typename T>
    auto DowncastUnchecked(Unsafe) const noexcept -> T
    {
        return *static_cast<const T*>(this->n_self);
    }

#if VIOLET_FEATURE(RTTI)
    /// Returns the demangled name of the stored type.
    [[nodiscard]] auto TypeName() const noexcept -> String;
#endif

    /// Returns a string representation of the stored value.
    [[nodiscard]] auto ToString() const noexcept -> violet::String;
    friend auto operator<<(std::ostream& os, const Any& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

private:
#if VIOLET_FEATURE(RTTI)
    using type_id = std::type_index;
#else
    using type_id = TypeId;
#endif

    VIOLET_EXPLICIT Any(void* self, UInt size, type_id type, AnyVTable vtable)
        : n_self(self)
        , n_vtable(vtable)
        , n_size(size)
        , n_type(type)
    {
    }

    void* n_self;
    AnyVTable n_vtable;
    UInt n_size;
    type_id n_type;

    void destructObject();
};

} // namespace violet::experimental
