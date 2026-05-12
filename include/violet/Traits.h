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
//! # 🌺💜 `violet/Traits.h`

#include <violet/Language/Macros.h> // IWYU pragma: export
#include <violet/Language/Policy.h> // IWYU pragma: export

#include <concepts>
#include <memory>
#include <type_traits>

namespace violet {

/// Type trait that checks whether type `T` is an instantiation of a given class template.
///
/// ## Example
/// ```cpp
/// #include <violet/Traits.h>
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

/// Returns the value from the [`instanceof<Template, Args...>`] type trait.
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
/// #include <violet/Traits.h>
///
/// using T = violet::pack_element<1, int, float, double>::type;
/// static_assert(std::is_same_v<T, float>);
/// ```
///
/// @note Prefer the [`violet::pack_element_t`] alias to avoid the trailing `::type`.
/// @note Index `I` must be less than `sizeof...(Ts)`; out-of-range access is a compile error.
template<std::size_t I, typename T, typename... Ts>
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
#if ((defined(__cpp_pack_indexing) >= 202311L) && !VIOLET_COMPILER(GCC)) || VIOLET_COMPILER(CLANG)
/// Convenience alias for using the pack indexing operator on newer compiler
/// versions that support it
///
/// ## Example
/// ```cpp
/// #include <violet/Traits.h>
///
/// static_assert(std::is_same_v<violet::pack_element_t<0, int, float, double>, int>);
/// static_assert(std::is_same_v<violet::pack_element_t<2, int, float, double>, double>);
/// ```
template<std::size_t I, typename... Ts>
using pack_element_t = Ts...[I];
#else
/// Convenience alias for [`violet::pack_element<I, Ts...>::type`].
///
/// ## Example
/// ```cpp
/// #include <violet/Traits.h>
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
/// #include <violet/Traits.h>
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
    constexpr static std::size_t value = 0;
};

template<typename T, typename U, typename... Ts>
struct pack_index<T, U, Ts...> {
    constexpr static std::size_t value = 1 + pack_index<T, Ts...>::value;
};

/// Convenience variable template for [`violet::pack_index<T, Ts...>::value`].
///
/// ## Example
/// ```cpp
/// #include <violet/Traits.h>
///
/// constexpr size_t idx = violet::pack_index_v<double, int, float, double>;
/// static_assert(idx == 2);
/// ```
template<typename T, typename... Ts>
constexpr inline std::size_t pack_index_v = pack_index<T, Ts...>::value;

/// Compile-time predicate that is `true` when type `T` is present in the
/// parameter pack `Ts...`, and `false` otherwise.
///
/// Implemented as a fold expression over [`std::is_same_v`], so it compiles in
/// O(N) template instantiations but requires no recursive helper struct.
///
/// ## Example
/// ```cpp
/// #include <violet/Traits.h>
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

/// Detects whenever `T` is a [`violet::SharedPtr`] instantiation.
///
/// ## Examples
/// ```cpp
/// static_assert(violet::is_shared_ptr<violet::SharedPtr<int>>::value);
/// static_assert(!violet::is_shared_ptr<int>::value);
/// static_assert(!violet::is_shared_ptr<violet::UniquePtr<int>>::value);
/// ```
template<typename T>
struct is_shared_ptr final: std::false_type { };

template<typename T>
struct is_shared_ptr<std::shared_ptr<T>>: std::true_type { };

/// Convenience variable template for [`is_shared_ptr`].
///
/// # Examples
///
/// ```cpp
/// static_assert(is_shared_ptr_v<violet::SharedPtr<int>>);
/// static_assert(!is_shared_ptr_v<int>);
/// ```
template<typename T>
constexpr static inline bool is_shared_ptr_v = is_shared_ptr<T>::value;

/// Extracts the element type `T` from a `violet::SharedPtr<T>`.
///
/// Only valid when instantiated with a `violet::SharedPtr<T>` specialization.
/// Using it with a non-`SharedPtr` type is a compile error.
///
/// # Examples
///
/// ```cpp
/// static_assert(std::same_as<shared_ptr_type_t<violet::SharedPtr<int>>, int>);
/// static_assert(std::same_as<shared_ptr_type_t<violet::SharedPtr<Config>>, Config>);
/// ```
template<typename T>
struct shared_ptr_type;

template<typename T>
struct shared_ptr_type<std::shared_ptr<T>> {
    using type = T;
};

/// Convenience alias for `shared_ptr_type<T>::type`.
template<typename T>
using shared_ptr_type_t = typename shared_ptr_type<T>::type;

/// Introspects the parameter types and arity of a function signature.
///
/// Given a function type `R(Args...)`, provides access to the parameter
/// types as a `std::tuple` and the parameter count as a compile-time
/// constant.
///
/// Only the `R(Args...)` specialization is defined; passing a non-function
/// type is a compile error.
template<typename F>
struct FunctionParams;

template<typename R, typename... Args>
struct FunctionParams<R(Args...)> final {
    /// The parameter types packed into a `std::tuple`.
    using types = std::tuple<Args...>;

    /// The number of parameters in the function signature.
    constexpr static std::size_t arity = sizeof...(Args);
};

} // namespace violet
