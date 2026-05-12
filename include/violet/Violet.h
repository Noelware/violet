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
//! # 🌺💜 `violet/Violet.h`

#pragma once

#include <violet/Language/Assert.h> // IWYU pragma: export
#include <violet/Language/Macros.h> // IWYU pragma: export
#include <violet/Language/Panic.h> // IWYU pragma: export
#include <violet/Language/Policy.h> // IWYU pragma: export
#include <violet/Traits.h> // IWYU pragma: export

#include <any>
#include <condition_variable>
#include <cstddef>
#include <format>
#include <map>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace violet {

/// Newtype for [`std::int8_t`].
using Int8 = std::int8_t;

/// Newtype for [`std::int16_t`].
using Int16 = std::int16_t;

/// Newtype for [`std::int32_t`].
using Int32 = std::int32_t;

/// Newtype for [`std::int64_t`].
using Int64 = std::int64_t;

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

/// C++ concept to require type `T` to have a `ToString` instance member that can
/// stringify `T`.
///
/// @note You can use the [`violet::ToString`] function to call any `Stringify`-able
/// types as well.
template<typename T>
concept Stringify = requires(T ty) {
    { ty.ToString() } -> std::convertible_to<String>;
};

template<Stringify T>
inline auto ToString(const T& val) -> String
{
    return val.ToString();
}

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

template<typename N>
    requires(std::is_arithmetic_v<N>)
constexpr auto ToString(N num) -> String
{
    return std::format("{}", num);
}

template<Stringify T, Stringify U>
constexpr auto ToString(const Pair<T, U>& pair) -> String
{
    return std::format("({}, {})", pair.first, pair.second);
}

constexpr auto ToString(std::nullopt_t) -> String
{
    return "violet::Nothing";
}

constexpr auto ToString(std::nullptr_t) -> String
{
    return "nullptr";
}

/// A marker type that determines a function is unsafe for a specific reason.
struct Unsafe final {
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Unsafe);
    constexpr VIOLET_EXPLICIT Unsafe(const char*) { }
};

/// A marker type that represents an infallible state, analogous to Rust's [`std::convert::Infallible`].
///
/// [`std::convert::Infallible`]: https://doc.rust-lang.org/1.91.1/std/convert/enum.Infallible.html
struct Infallible final {
    constexpr VIOLET_IMPLICIT Infallible() = default;
};

} // namespace violet
