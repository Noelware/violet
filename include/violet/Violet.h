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
/// @since 26.02
using Int8 = std::int8_t;

/// Newtype for [`std::int16_t`].
/// @since 26.02
using Int16 = std::int16_t;

/// Newtype for [`std::int32_t`].
/// @since 26.02
using Int32 = std::int32_t;

/// Newtype for [`std::int64_t`].
/// @since 26.02
using Int64 = std::int64_t;

/// Newtype for [`std::ptrdiff_t`].
/// @since 26.02
using Int = std::ptrdiff_t;

/// Newtype for [`std::uint8_t`].
/// @since 26.02
using UInt8 = std::uint8_t;

/// Newtype for [`std::uint16_t`].
/// @since 26.02
using UInt16 = std::uint16_t;

/// Newtype for [`std::uint32_t`].
/// @since 26.02
using UInt32 = std::uint32_t;

/// Newtype for [`std::uint64_t`].
/// @since 26.02
using UInt64 = std::uint64_t;

/// Newtype for [`std::size_t`].
/// @since 26.02
using UInt = std::size_t;

/// Newtype for [`std::string`].
/// @since 26.02
using String = std::string;

/// Newtype for [`std::string_view`].
/// @since 26.02
using Str = std::string_view;

/// Newtype for `const char*`
/// @since 26.02
using CStr = const char*;

/// Newtype for [`std::mutex`].
/// @since 26.02
using Mutex = std::mutex;

/// Newtype for [`std::any`].
/// @since 26.02
using Any = std::any;

/// Newtype for [`std::condition_variable`].
/// @since 26.02
using Condvar = std::condition_variable;

/// Newtype for [`std::unique_lock`].
/// @since 26.02
template<typename Mutex = violet::Mutex>
using UniqueLock = std::unique_lock<Mutex>;

/// Newtype for [`std::shared_ptr`].
/// @since 26.02
template<typename T>
using SharedPtr = std::shared_ptr<T>;

/// Newtype for [`std::unique_ptr`].
/// @since 26.02
template<typename T, typename Destructor = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, Destructor>;

/// Newtype for [`std::vector`].
/// @since 26.02
template<typename T>
using Vec = std::vector<T>;

/// Newtype for [`std::span`].
/// @since 26.02
template<typename T, UInt Extent = std::dynamic_extent>
using Span = std::span<T, Extent>;

/// Newtype for [`std::array`].
/// @since 26.02
template<typename T, UInt Size>
using Array = std::array<T, Size>;

/// Newtype for [`std::map`].
/// @since 26.02
template<typename K, typename V>
using Map = std::map<K, V>;

/// Newtype for [`std::unordered_map`].
/// @since 26.02
template<typename K, typename V>
using UnorderedMap = std::unordered_map<K, V>;

/// Newtype for [`std::pair`].
/// @since 26.02
template<typename T1, typename T2>
using Pair = std::pair<T1, T2>;

/// C++ concept to require type `T` to have a `ToString` instance member that can
/// stringify `T`.
///
/// @since 26.02
/// @note You can use the [`violet::ToString`] function to call any `Stringify`-able
/// types as well
template<typename T>
concept Stringify = requires(T ty) {
    { ty.ToString() } -> std::convertible_to<String>;
};

template<Stringify T>
NOELDOC_SINCE("26.02")
inline auto ToString(const T& val) -> String
{
    return val.ToString();
}

NOELDOC_SINCE("26.02")
inline auto ToString(const String& val) -> String
{
    return val;
}

NOELDOC_SINCE("26.02")
inline auto ToString(const Str& val) -> String
{
    return String(val);
}

NOELDOC_SINCE("26.02")
inline auto ToString(CStr val) -> String
{
    return val;
}

NOELDOC_SINCE("26.02")
inline auto ToString(bool val) -> String
{
    return val ? "true" : "false";
}

template<typename N>
    requires(std::is_arithmetic_v<N>)
NOELDOC_SINCE("26.02")
constexpr auto ToString(N num) -> String
{
    return std::format("{}", num);
}

template<Stringify T, Stringify U>
NOELDOC_SINCE("26.02")
constexpr auto ToString(const Pair<T, U>& pair) -> String
{
    return std::format("({}, {})", pair.first, pair.second);
}

NOELDOC_SINCE("26.02")
constexpr auto ToString(std::nullopt_t) -> String
{
    return "violet::Nothing";
}

NOELDOC_SINCE("26.02")
constexpr auto ToString(std::nullptr_t) -> String
{
    return "nullptr";
}

/// A marker type that determines a function is unsafe for a specific reason.
struct NOELDOC_SINCE("26.02") Unsafe final {
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Unsafe);
    constexpr VIOLET_EXPLICIT Unsafe(const char*) { }
};

/// A marker type that represents an infallible state, analogous to Rust's [`std::convert::Infallible`].
///
/// [`std::convert::Infallible`]: https://doc.rust-lang.org/1.91.1/std/convert/enum.Infallible.html
struct NOELDOC_SINCE("26.02") Infallible final {
    constexpr VIOLET_IMPLICIT Infallible() = default;
};

/// Explicitly destroys `value`, ending its lifetime immediately, analogous to Rust's [`std::mem::drop`].
///
/// The argument is taken **by value**, so the caller's object is moved (or copied, if it is not movable)
/// into the parameter, and that parameter is destroyed when `Drop` returns, running `T`'s destructor. This
/// is useful for releasing a resource before the end of the enclosing scope, e.g. closing file handles held
/// by an iterator before performing follow-up work that depends on them being closed:
///
/// ## Example
/// ```cpp
/// auto iter = VIOLET_TRY(filesystem::WalkDir(path));
/// for (auto entry: iter) {
///     // ...
/// }
///
/// // release the iterator's open directory handles before we mutate the tree below.
/// Drop(VIOLET_MOVE(iter));
/// ```
///
/// ## Remarks
/// To actually end a value's lifetime the argument must be moved in (or be a copyable temporary); passing an
/// lvalue you still own merely drops a copy. Calling `Drop` on a trivially destructible value (e.g. an `Int`)
/// has no observable effect.
///
/// @param value the value to take ownership of and destroy.
///
/// [`std::mem::drop`]: https://doc.rust-lang.org/1.95.0/std/mem/fn.drop.html
template<typename T>
NOELDOC_SINCE("26.07")
constexpr void Drop(T value) noexcept(std::is_nothrow_destructible_v<T>)
{
    static_cast<void>(value);
}

} // namespace violet
