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
#include <violet/Language/Policy.h> // IWYU pragma: export

#include <any>
#include <condition_variable>
#include <cstddef>
#include <format>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

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
struct instanceof: std::false_type { };

template<template<class...> typename Template, typename... Args>
struct instanceof<Template, Template<Args...>>: std::true_type { };

/// Returns the value from the [`instanceof<Template, T>`] type trait.
template<template<class...> typename Template, typename T>
inline constexpr bool instanceof_v = instanceof<Template, T>::value;

/// A compile-time type trait that retrieves the type at the zero-based index `I`
/// in the template parameter pack `Ts...`.
///
/// Recursively peels the head of the pack until `I` reaches zero, at which point
/// the current head type is selected.
///
/// ## Example
/// ```cpp
/// #include <violet/Violet.h>
///
/// using T = violet::pack_element<1, int, float, double>::type;
/// static_assert(std::is_same_v<T, float>);
/// ```
///
/// @note Prefer the [`violet::pack_element_t`] alias to avoid the trailing `::type`.
/// @note Index `I` must be less than `sizeof...(Ts)`; out-of-range access is a compile error.
template<size_t I, typename T, typename... Ts>
struct pack_element final {
    using type = typename pack_element<I - 1, Ts...>::type;
};

template<typename T, typename... Ts>
struct pack_element<0, T, Ts...> final {
    using type = T;
};

// As 18/04/26, GCC doesn't implement mangle pack indexing, so for now, we only
// enable using the pack indexing operator on Clang or if a compiler (that isn't
// GCC) has `__cpp_pack_indexing`
#if (defined(__cpp_pack_indexing) && !defined(VIOLET_GCC)) || defined(VIOLET_CLANG)
/// Convenience alias for using the pack indexing operator on newer compiler
/// versions that support it
///
/// ## Example
/// ```cpp
/// #include <violet/Violet.h>
///
/// static_assert(std::is_same_v<violet::pack_element_t<0, int, float, double>, int>);
/// static_assert(std::is_same_v<violet::pack_element_t<2, int, float, double>, double>);
/// ```
template<size_t I, typename... Ts>
using pack_element_t = Ts...[I];
#else
/// Convenience alias for [`violet::pack_element<I, Ts...>::type`].
///
/// ## Example
/// ```cpp
/// #include <violet/Violet.h>
///
/// static_assert(std::is_same_v<violet::pack_element_t<0, int, float, double>, int>);
/// static_assert(std::is_same_v<violet::pack_element_t<2, int, float, double>, double>);
/// ```
template<size_t I, typename... Ts>
using pack_element_t = typename pack_element<I, Ts...>::type;
#endif

/// A compile-time type trait that finds the zero-based index of type `T` in the
/// parameter pack `Ts...`.
///
/// Scans the pack from left to right, counting how far it must recurse before
/// reaching a head type that is the same as `T`. The result is stored in
/// `pack_index<T, Ts...>::value`.
///
/// ## Example
/// ```cpp
/// #include <violet/Violet.h>
///
/// static_assert(violet::pack_index<float, int, float, double>::value == 1);
/// static_assert(violet::pack_index<int,   int, float, double>::value == 0);
/// ```
///
/// @note Prefer the [`violet::pack_index_v`] variable template for brevity.
/// @note If `T` does not appear in `Ts...`, instantiation produces an incomplete
///       type and the program is ill-formed.
template<typename T, typename... Ts>
struct pack_index;

template<typename T, typename... Ts>
struct pack_index<T, T, Ts...> final {
    constexpr static size_t value = 0;
};

template<typename T, typename U, typename... Ts>
struct pack_index<T, U, Ts...> {
    constexpr static size_t value = 1 + pack_index<T, Ts...>::value;
};

/// Convenience variable template for [`violet::pack_index<T, Ts...>::value`].
///
/// ## Example
/// ```cpp
/// #include <violet/Violet.h>
///
/// constexpr size_t idx = violet::pack_index_v<double, int, float, double>;
/// static_assert(idx == 2);
/// ```
template<typename T, typename... Ts>
constexpr inline size_t pack_index_v = pack_index<T, Ts...>::value;

/// Compile-time predicate that is `true` when type `T` is present in the
/// parameter pack `Ts...`, and `false` otherwise.
///
/// Implemented as a fold expression over [`std::is_same_v`], so it compiles in
/// O(N) template instantiations but requires no recursive helper struct.
///
/// ## Example
/// ```cpp
/// #include <violet/Violet.h>
///
/// static_assert( violet::pack_contains_v<float, int, float, double>);
/// static_assert(!violet::pack_contains_v<char,  int, float, double>);
/// ```
template<typename T, typename... Ts>
inline constexpr bool pack_contains_v = (std::is_same_v<T, Ts> || ...);

template<typename Fun, typename... Args>
concept callable = std::invocable<Fun, Args...>;

template<typename Fun, typename Return, typename... Args>
concept callable_returns = std::convertible_to<std::invoke_result_t<Fun, Args...>, Return>;

template<typename T, typename Item>
concept collectable = requires(T& ty, Item value) {
    { ty.insert(ty.end(), value) };
} || requires(T& cnt, Item value) {
    { cnt.push_back(value) };
};

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

/// A non-copyable, non-movable scope guard that invokes a callable when it goes
/// out of scope, analgous to the `defer` keyword in Go or the [`scopeguard`] Rust
/// crate.
///
/// [`scopeguard`]: https://docs.rs/scopeguard/latest/scopeguard/
///
/// **Defer** is non-copyable and non-movable, it must remain in the scope
/// it was created in. If you need to share a deferrable action across scopes,
/// wrap the owning scope around a `violet::SharedPtr<violet::Defer>`.
///
/// You can construct a **Defer** like you would normally or using the [`VIOLET_DEFER`] macro,
/// which provides a unique name instead of naming it yourself, if you're weird like that.
///
/// ```cpp
/// #include <violet/Violet.h>
///
/// {
///     // 1. Normal Usage
///     violet::Defer _guard([&] -> void {
///         violet::Println("Hello, world!");
///     });
///
///     // 2. Using the `VIOLET_DEFER` macro
///     VIOLET_DEFER({
///         violet::Println("Hello, world 2: electric boogaloo");
///     });
/// }
/// ```
///
/// When multiple guards are in the same scope, they're destroyed in **reverse declaration order**,
/// matching C++ destructor semantics. In the example above, `_defer_block_0` (the generated name
/// when using [`VIOLET_DEFER`]) will be called first then `_guard` will be called last.
template<typename Fun>
    requires(callable<Fun> && callable_returns<Fun, void>)
struct Defer final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Defer);
    VIOLET_DISALLOW_CONSTRUCTOR(Defer);

    /// Constructs a new `Defer` object with a given callable, which will be invoked
    /// when the object is destroyed.
    VIOLET_EXPLICIT Defer(Fun&& fun) noexcept
        : n_fun(VIOLET_MOVE(fun))
    {
    }

    ~Defer() noexcept(noexcept(std::invoke(std::declval<Fun>())))
    {
        std::invoke(this->n_fun);
    }

private:
    Fun n_fun;
};

template<typename Fun>
Defer(Fun&&) -> Defer<std::decay_t<Fun>>;

/// A non-copyable, non-movable scope guard like [`violet::Defer`] that can be cancelled
/// before it fires.
///
/// When [`CancellableDefer::Cancel()`] is called, the store callable ***WILL NOT*** be invoked
/// when the guard goes out of scope. This is useful when a deferred cleanup action should only run
/// on failure paths. For example, in Noelware's [Eous](https://eous.noelware.cloud) Project reconcillation loop,
/// we want to create a network with a given driver and rollback if the transaction didn't execute successfully:
///
/// ```cpp
/// // Create the network on the driver side.
/// VIOLET_TRY_VOID(driver->Create(network));
///
/// violet::CancellableDefer rollback([&] -> void {
///     (void)driver->Destroy(network);
/// });
///
/// // Commit the new network into the RocksDB storage
/// VIOLET_TRY_VOID(store->Commit(network));
///
/// // At this point, when we go out of scope and `rollback.Cancel()` was set,
/// // then the callback is never executed.
/// rollback.Cancel();
/// ```
///
/// ## Remarks
/// Once [`CancellableDefer::Cancel()`] has been called, it cannot be undone. If you need
/// re-arming semantics, implement it yourself.
///
/// ## Thread Safety
/// **CancellableDefer** is safe to call from any thread if the `ThreadSafety` template parameter
/// is a truthy value.
template<typename Fun, bool ThreadSafety = true>
struct CancellableDefer final {
    VIOLET_DISALLOW_COPY_AND_MOVE(CancellableDefer);
    VIOLET_DISALLOW_CONSTRUCTOR(CancellableDefer);

    /// Creates a single-threaded, local [`violet::CancellableDefer`] object.
    ///
    /// This is sightly cheaper by default, use this if you know that the guard
    /// will be never cancelled from another thread.
    static auto Local(Fun&& fun) noexcept -> CancellableDefer<std::decay_t<Fun>, false>
    {
        return CancellableDefer<Fun, false>(VIOLET_MOVE(fun));
    }

    VIOLET_IMPLICIT CancellableDefer(Fun&& fun) noexcept
        : n_fun(VIOLET_MOVE(fun))
    {
    }

    ~CancellableDefer() noexcept(noexcept(std::invoke(std::declval<Fun>())))
    {
        if (!this->Cancelled()) {
            std::invoke(this->n_fun);
        }
    }

    /// Cancels the deferrable action. Cannot be undone.
    void Cancel() noexcept
    {
        if constexpr (ThreadSafety) {
            this->n_cancelled.store(true, std::memory_order_release);
        } else {
            this->n_cancelled = true;
        }
    }

    /// Returns **true** if this deferrable action was cancelled.
    [[nodiscard]] auto Cancelled() const noexcept -> bool
    {
        if constexpr (ThreadSafety) {
            return this->n_cancelled.load(std::memory_order_acquire);
        } else {
            return this->n_cancelled;
        }
    }

private:
    Fun n_fun;
    std::conditional_t<ThreadSafety, std::atomic<bool>, bool> n_cancelled = false;
};

template<typename Fun, bool ThreadSafety = true>
CancellableDefer(Fun&&) -> CancellableDefer<std::decay_t<Fun>, ThreadSafety>;

} // namespace violet

/**
 * @macro VIOLET_DEFER
 * C-style macro that defines a new `violet::Defer` object in a scope without figuring
 * out a name for it, just write it easily:
 *
 * ```cpp
 * void h() {
 *    VIOLET_DEFER({ violet::Println("hello, world!"); });
 * }
 *
 * h();
 * ```
 */
#define VIOLET_DEFER(fn) ::violet::Defer VIOLET_UNIQUE_NAME(_defer_block_)([&]()->void fn);

#define VIOLET_CANCELLABLE_DEFER(fn)                                                                                   \
    auto VIOLET_UNIQUE_NAME(_cancellable_defer_block_) = ::violet::CancellableDefer([&]()->void fn);
