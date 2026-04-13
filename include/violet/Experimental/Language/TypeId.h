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
//! # 🌺💜 `violet/Experimental/Language/TypeId.h`

#pragma once

#include <violet/Language/Macros.h>
#include <violet/Language/Policy.h>

#include <cstddef>
#include <format>
#include <string>
#include <string_view>

namespace violet::experimental {

/// A lightweight, RTTI-free type identification token.
///
/// A **TypeId** is an opaque value that uniquely identifies a C++ type within
/// a single dynamically shared object. Two [`TypeId`] values compare equal if and only
/// they were produced by [`TypeId::Of<T>()`] for the same type `T`.
///
/// Type IDs are typically cheap to copy, store, and compare; it is pointer-sized with
/// no associated allocation or destructor.
///
/// ## Limitations
/// ### No cross-DSO identity guaranteed
/// **TypeId** doesn't provide stable identity across DSO boundaries.
///
/// ### No serialization stability
/// **TypeId** values are *runtime pointer addresses*. They must not be persisted or transmitted
/// over IPC.
struct VIOLET_API TypeId final {
    /// Returns the unique [`TypeId`] for the type `T`.
    ///
    /// ## Example
    /// ```cpp
    /// #include <violet/Experimental/Language/TypeId.h>
    ///
    /// struct Foo {};
    /// struct Bar {};
    ///
    /// static_assert(violet::experimental::TypeId::Of<Foo>() == violet::experimental::TypeId::Of<Foo>());
    /// static_assert(violet::experimental::TypeId::Of<Foo>() != violet::experimental::TypeId::Of<Bar>());
    /// ```
    ///
    /// ## Rules
    /// - **cv-qualifiers** and reference-ness are part of the type identity; to strip them,
    ///   use [`std::remove_cvref_t`] explicitly.
    ///   - [`TypeId::Of<T>()`] and [`TypeId::Of<const T>()`] are **distinct**.
    ///   - [`TypeId::Of<T>()`] and [`TypeId::Of<T&>()`] are **distinct**.
    ///
    /// - If [`TypeId::Of<T>()`] is instantiated in both a host binary and a `dlopen`-loaded context,
    ///   the two calls may return different **TypeId** values for the same `T`.
    template<typename T>
    constexpr static auto Of() -> TypeId
    {
        constexpr static char kTypeId = 0;
        return TypeId(&kTypeId, getTypeNameOf<T>());
    }

    /// Returns a hash-suitable representation of this token.
    /// The returned value has no meaning beyond use as a hash key.
    [[nodiscard]] constexpr auto HashCode() const noexcept -> size_t
    {
        return reinterpret_cast<size_t>(this->n_id);
    }

    /// Returns a human-readable name for the type, derived from compiler extensions.
    ///
    /// ## Warning
    /// The format of the result of calling this function is **compiler-dependent and *NOT STABLE*** across
    /// compiler versions. It is intended for diagnostics and logging output only.
    [[nodiscard]] constexpr auto Name() const noexcept -> std::string_view
    {
        return this->n_name;
    }

    [[nodiscard]] auto ToString() const noexcept -> std::string
    {
        return std::format("TypeId(0x{:x})", this->HashCode());
    }

    friend auto operator<<(std::ostream& os, const TypeId& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

    constexpr auto operator==(TypeId other) const noexcept -> bool
    {
        return this->n_id == other.n_id;
    }

    constexpr auto operator!=(TypeId other) const noexcept -> bool
    {
        return this->n_id != other.n_id;
    }

private:
    constexpr VIOLET_EXPLICIT TypeId(const void* ptr, std::string_view name) noexcept
        : n_id(ptr)
        , n_name(name)
    {
    }

    const void* n_id;
    std::string_view n_name;

    template<typename T>
    constexpr static auto getTypeNameOf() noexcept -> std::string_view
    {
#if defined(VIOLET_GCC) || defined(VIOLET_CLANG)
        constexpr std::string_view sv = __PRETTY_FUNCTION__;
        constexpr auto start = sv.rfind('=') + 2;
        constexpr auto end = sv.rfind(']');
        return sv.substr(start, end - start);
#elif defined(VIOLET_MSVC)
        constexpr std::string_view sv = __FUNCSIG__;
        constexpr auto start = sv.rfind("getTypeNameOf<") + 14; // len("getTypeNameOf<")
        constexpr auto end = sv.rfind('>');
        return sv.substr(start, end - start);
#else
        return "(unknown)";
#endif
    }
};

} // namespace violet::experimental

template<>
struct VIOLET_API std::hash<violet::experimental::TypeId> final {
    auto operator()(violet::experimental::TypeId id) const noexcept -> size_t
    {
        return id.HashCode();
    }
};
