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
//! # 🌺💜 `violet/violet.h`
//! This header file contains important types and aliases to more readable
//! types from C++'s standard library.

#pragma once

#include "absl/numeric/int128.h"

#include <any>
#include <array>
#include <concepts>
#include <condition_variable>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <mutex>
#include <span>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Noelware::Violet {

template<typename Fun, typename... Args>
concept Callable = std::invocable<Fun, Args...>;

template<typename Fun, typename ShouldReturn, typename... Args>
concept CallableShouldReturn = std::convertible_to<std::invoke_result_t<Fun, Args...>, ShouldReturn>;

using int8 = std::int8_t; ///< Newtype for [`std::int8_t`].
using int16 = std::int16_t; ///< Newtype for [`std::int16_t`].
using int32 = std::int32_t; ///< Newtype for [`std::int32_t`].
using int64 = std::int64_t; ///< Newtype for [`std::int64_t`].
using int128 = absl::int128; ///< Newtype for [`absl::int128`].
using isize = std::ptrdiff_t; ///< Newtype for [`std::ptrdiff_t`].
using uint8 = std::uint8_t; ///< Newtype for [`std::uint8_t`].
using uint16 = std::uint16_t; ///< Newtype for [`std::uint16_t`].
using uint32 = std::uint32_t; ///< Newtype for [`std::uint32_t`].
using uint64 = std::uint64_t; ///< Newtype for [`std::uint64_t`].
using uint128 = absl::uint128; ///< Newtype for [`absl::uint128`].
using usize = std::size_t; ///< Newtype for [`std::size_t`].

using String = std::string; ///< Newtype for [`std::string`].
using Str = std::string_view; ///< Newtype for [`std::string_view`].
using CStr = const char*; ///< Newtype for `const char*`
using Mutex = std::mutex; ///< Newtype for [`std::mutex`].
using Any = std::any; ///< Newtype for [`std::any`].
using Condvar = std::condition_variable; ///< Newtype for [`std::condition_variable`].

template<typename Mutex = Noelware::Violet::Mutex>
using UniqueLock = std::unique_lock<Mutex>; ///< Newtype for [`std::unique_lock`].

template<typename T>
using SharedPtr = std::shared_ptr<T>; ///< Newtype for [`std::shared_ptr`].

template<typename T>
using Vec = std::vector<T>; ///< Newtype for [`std::vector`].

template<typename T, usize Extent = std::dynamic_extent>
using Span = std::span<T, Extent>; ///< Newtype for [`std::span`].

template<typename T, usize Size>
using Array = std::array<T, Size>; ///< Newtype for [`std::array`].

template<typename K, typename V>
using Map = std::map<K, V>; ///< Newtype for [`std::map`].

template<typename K, typename V>
using UnorderedMap = std::unordered_map<K, V>; ///< Newtype for [`std::unordered_map`].

template<typename T1, typename T2>
using Pair = std::pair<T1, T2>; ///< Newtype for [`std::pair`].

inline auto ToString() noexcept -> String
{
    return "";
}

template<typename T>
inline auto ToString(const T& val) -> String
{
    if constexpr (requires { val.ToString(); }) {
        return val.ToString();
    } else if constexpr (requires { Noelware::Violet::ToString(val); }) {
        return Noelware::Violet::ToString(val);
    } else {
        static_assert(sizeof(T) == 0, "`T` doesn't satisfy the `Stringify` concept");
    }
}

/// C++ concept to require type `T` to have a `ToString` instance member that can
/// stringify `T`.
///
/// NOTE: You can use the [`Noelware::Violet::ToString`] function to call any `Stringify`-able
/// types as well.
template<typename T>
concept Stringify = requires(T ty) {
    { ty.ToString() } -> std::convertible_to<std::string>;
} || requires(const T& value) {
    { Noelware::Violet::ToString(value) } -> std::convertible_to<std::string>;
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

template<std::integral T>
constexpr auto ToString(T val) -> String
{
    std::stringstream buf;
    buf << val;

    return buf.str();
}

constexpr auto ToString(int128 val) -> String
{
    std::stringstream buf;
    buf << val;

    return buf.str();
}

constexpr auto ToString(uint128 val) -> String
{
    std::stringstream buf;
    buf << val;

    return buf.str();
}

} // namespace Noelware::Violet

#define VIOLET_SHARED_PTR(TYPE, VALUE, ...) ::std::make_shared<TYPE>(VALUE, ##__VA_ARGS__)
#define VIOLET_UNIQUE_PTR(TYPE, VALUE, ...) ::std::make_unique<TYPE>(VALUE, ##__VA_ARGS__)
#define VIOLET_FWD(TYPE, VALUE) ::std::forward<TYPE>(VALUE)
#define VIOLET_MOVE(VALUE) ::std::move(VALUE)
#define VIOLET_ANY(TYPE, VALUE) ::std::make_any<TYPE>(VALUE)

#define VIOLET_TO_STRING(TYPE, NAME, BLOCK)                                                                            \
    namespace Noelware::Violet {                                                                                       \
        inline auto ToString(TYPE NAME) -> ::Noelware::Violet::String BLOCK                                            \
    }

#define VIOLET_IMPLICIT explicit(false)
#define VIOLET_EXPLICIT explicit(true)
