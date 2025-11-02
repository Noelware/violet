// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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
//! # 🌺💜 `violet/Violet.h`

#pragma once

#include "absl/container/btree_map.h"
#include "absl/container/btree_set.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/container/node_hash_map.h"
#include "absl/container/node_hash_set.h"
#include "absl/numeric/int128.h"
#include "violet/Language/Macros.h" // IWYU pragma: export
#include "violet/Language/Policy.h" // IWYU pragma: export

#include <any>
#include <condition_variable>
#include <cstddef>
#include <string>
#include <string_view>

namespace violet {

/// Type trait that checks whether type `T` is an instantiation of a given class template.
///
/// ## Example
/// ```cpp
/// #include <violet/Violet.h>
/// #include <optional>
/// #include <tuple>
///
/// static_assert(instanceof<std::optional, std::optional<violet::UInt32>>::value);
/// static_assert(!instanceof<std::optional, violet::UInt32>::value);
///
/// // Can also work for variadic templates!
/// template<typename... Args> struct User {};
/// static_assert(instanceof<User, User<violet::UInt32, double>>::value);
/// static_assert(!instanceof<User, std::tuple<violet::UInt32>>::value);
///
/// // Also works in `constexpr` contexts via `if constexpr`:
/// template<typename T>
/// void foo() {
///     if constexpr (instanceof<std::optional, T>::value) {
///         /* do something here...? */
///     }
/// }
/// ```
///
/// @note Only detects the primary template; partial specializations of `Template` must match exactly.
/// @note Can be used in `requires` clauses for SFINAE or concepts with [`violet::instanceof_v<Template, T>`].
template<template<class...> typename Template, typename T>
struct instanceof: std::false_type {};

template<template<class...> typename Template, typename... Args>
struct instanceof<Template, Template<Args...>>: std::true_type {};

/// Returns the value from the [`instanceof<Template, T>`] type trait.
template<template<class...> typename Template, typename T>
inline constexpr bool instanceof_v = instanceof<Template, T>::value;

template<typename Fun, typename... Args>
concept callable = std::invocable<Fun, Args...>;

template<typename Fun, typename Return, typename... Args>
concept callable_returns = std::convertible_to<std::invoke_result_t<Fun, Args...>, Return>;

/// Newtype for [`std::int8_t`].
using Int8 = std::int8_t;

/// Newtype for [`std::int16_t`].
using Int16 = std::int16_t;

/// Newtype for [`std::int32_t`].
using Int32 = std::int32_t;

/// Newtype for [`std::int64_t`].
using Int64 = std::int64_t;

/// Newtype for [`absl::int128`].
using Int128 = absl::int128;

/// Newtype for [`std::ptrdiff_t`].
using Int = std::ptrdiff_t;

/// Newtype for [`std::uint8_t`].
using UInt8 = std::uint8_t;

/// Newtype for [`std::uint16_t`].
using UInt16 = std::uint16_t;

/// Newtype for [`std::uint32_t`].
using UInt32 = std::uint32_t;

/// Newtype for [`std::uint64_t`].
using UInt64 = std::uint64_t;

/// Newtype for [`absl::uint128`].
using UInt128 = absl::uint128;

/// Newtype for [`std::size_t`].
using UInt = std::size_t;

/// Newtype for [`std::string`].
using String = std::string;

/// Newtype for [`std::string_view`].
using Str = std::string_view;

/// Newtype for `const char*`
using CStr = const char*;

/// Newtype for [`std::mutex`].
using Mutex = std::mutex;

/// Newtype for [`std::any`].
using Any = std::any;

/// Newtype for [`std::condition_variable`].
using Condvar = std::condition_variable;

/// Newtype for [`std::unique_lock`].
template<typename Mutex = violet::Mutex>
using UniqueLock = std::unique_lock<Mutex>;

/// Newtype for [`std::shared_ptr`].
template<typename T>
using SharedPtr = std::shared_ptr<T>;

/// Newtype for [`std::unique_ptr`].
template<typename T, typename Destructor = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, Destructor>;

/// Newtype for [`std::vector`].
template<typename T>
using Vec = std::vector<T>;

/// Newtype for [`std::span`].
template<typename T, UInt Extent = std::dynamic_extent>
using Span = std::span<T, Extent>;

/// Newtype for [`std::array`].
template<typename T, UInt Size>
using Array = std::array<T, Size>;

/// Newtype for [`std::map`].
template<typename K, typename V>
using Map = std::map<K, V>;

/// Newtype for [`std::unordered_map`].
template<typename K, typename V>
using UnorderedMap = std::unordered_map<K, V>;

/// Newtype for [`std::pair`].
template<typename T1, typename T2>
using Pair = std::pair<T1, T2>;

/// Newtype for [`absl::flat_hash_map`].
template<typename K, typename V, typename Hash = absl::DefaultHashContainerHash<K>,
    typename Eq = absl::DefaultHashContainerEq<K>, typename Alloc = std::allocator<Pair<const K, V>>>
using FlatHashMap = absl::flat_hash_map<K, V, Hash, Eq, Alloc>;

/// Newtype for [`absl::node_hash_map`].
template<typename K, typename V, typename Hash = absl::DefaultHashContainerHash<K>,
    typename Eq = absl::DefaultHashContainerEq<K>, typename Alloc = std::allocator<Pair<const K, V>>>
using NodeHashMap = absl::node_hash_map<K, V, Hash, Eq, Alloc>;

/// Newtype for [`absl::btree_map`].
template<typename K, typename V, typename Comparator = std::less<K>, typename Alloc = std::allocator<Pair<const K, V>>>
using BTreeMap = absl::btree_map<K, V, Comparator, Alloc>;

/// Newtype for [`absl::flat_hash_set`].
template<typename T, typename Hash = absl::DefaultHashContainerHash<T>, typename Eq = absl::DefaultHashContainerEq<T>,
    typename Alloc = std::allocator<T>>
using FlatHashSet = absl::flat_hash_set<T, Hash, Eq, Alloc>;

/// Newtype for [`absl::node_hash_set`].
template<typename T, typename Hash = absl::DefaultHashContainerHash<T>, typename Eq = absl::DefaultHashContainerEq<T>,
    typename Alloc = std::allocator<T>>
using NodeHashSet = absl::node_hash_set<T, Hash, Eq, Alloc>;

/// Newtype for [`absl::btree_set`].
template<typename T, typename Comparator = std::less<T>, typename Alloc = std::allocator<T>>
using BTreeSet = absl::btree_set<T, Comparator, Alloc>;

constexpr auto ToString() noexcept -> String
{
    return {};
}

template<typename T>
inline auto ToString(const T& val) -> String
{
    if constexpr (requires { val.ToString(); }) {
        return val.ToString();
    } else if constexpr (requires { violet::ToString(val); }) {
        return violet::ToString(val);
    } else {
        std::ostringstream buf;
        if constexpr (requires { buf << val; }) {
            buf << val;
            return buf.str();
        }

        static_assert(sizeof(T) == 0, "`T` doesn't satisfy the `Stringify` concept");
    }
}

/// C++ concept to require type `T` to have a `ToString` instance member that can
/// stringify `T`.
///
/// @note You can use the [`violet::ToString`] function to call any `Stringify`-able
/// types as well.
template<typename T>
concept Stringify = requires(T ty) {
    { ty.ToString() } -> std::convertible_to<std::string>;
} || requires(const T& value) {
    { violet::ToString(value) } -> std::convertible_to<std::string>;
};

inline auto ToString(const String& val) -> String
{
    return val;
}

inline auto ToString(const Str& val) -> String
{
    return String(val);
}

inline auto ToString(CStr val) -> String
{
    return val;
}

inline auto ToString(bool val) -> String
{
    return val ? "true" : "false";
}

constexpr auto ToString(Int128 val) -> String
{
    std::stringstream buf;
    buf << val;

    return buf.str();
}

constexpr auto ToString(UInt128 val) -> String
{
    std::stringstream buf;
    buf << val;

    return buf.str();
}

template<Stringify T, Stringify U>
constexpr auto ToString(const Pair<T, U>& pair) -> String
{
    return std::format("({}, {})", pair.first, pair.second);
}

struct Unsafe final {
    constexpr VIOLET_IMPLICIT Unsafe() = delete;
    constexpr VIOLET_EXPLICIT Unsafe(const char*) {}
};

} // namespace violet
