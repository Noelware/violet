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

#pragma once

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
struct NOELDOC_SINCE("26.02") instanceof: std::false_type { };

template<template<class...> typename Template, typename... Args>
struct NOELDOC_SINCE("26.04.01") instanceof<Template, Template<Args...>>: std::true_type { };

/// Returns the value from the [`instanceof<Template, Args...>`] type trait.
template<template<class...> typename Template, typename T>
NOELDOC_SINCE("26.02")
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
struct NOELDOC_SINCE("26.04.01") pack_element final {
    using type = typename pack_element<I - 1, Ts...>::type;
};

template<typename T, typename... Ts>
struct NOELDOC_SINCE("26.04.01") pack_element<0, T, Ts...> final {
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
///
/// @since 26.04.01
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
///
/// @since 26.04.01
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
struct NOELDOC_SINCE("26.04.01") pack_index;

template<typename T, typename... Ts>
struct NOELDOC_SINCE("26.04.01") pack_index<T, T, Ts...> final {
    constexpr static std::size_t value = 0;
};

template<typename T, typename U, typename... Ts>
struct NOELDOC_SINCE("26.04.01") pack_index<T, U, Ts...> {
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
NOELDOC_SINCE("26.04.01")
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
NOELDOC_SINCE("26.04.01")
inline constexpr bool pack_contains_v = (std::is_same_v<T, Ts> || ...);

/// @since 26.02
template<typename Fun, typename... Args>
concept callable = std::invocable<Fun, Args...>;

/// @since 26.02
template<typename Fun, typename Return, typename... Args>
concept callable_returns = std::convertible_to<std::invoke_result_t<Fun, Args...>, Return>;

/// @since 26.02
template<typename T, typename Item>
concept collectable = requires(T& ty, Item value) {
    { ty.insert(ty.end(), value) };
} || requires(T& cnt, Item value) {
    { cnt.push_back(value) };
};

/// @since 26.08
template<typename T, typename... Args>
concept constructible
    = std::constructible_from<T, Args...> && !(sizeof...(Args) == 1 && (std::same_as<std::decay_t<Args>, T> || ...));

/// Detects whenever `T` is a [`violet::SharedPtr`] instantiation.
///
/// ## Examples
/// ```cpp
/// static_assert(violet::is_shared_ptr<violet::SharedPtr<int>>::value);
/// static_assert(!violet::is_shared_ptr<int>::value);
/// static_assert(!violet::is_shared_ptr<violet::UniquePtr<int>>::value);
/// ```
template<typename T>
struct NOELDOC_SINCE("26.06.05") is_shared_ptr final: std::false_type { };

template<typename T>
struct NOELDOC_SINCE("26.06.05") is_shared_ptr<std::shared_ptr<T>>: std::true_type { };

/// Convenience variable template for [`is_shared_ptr`].
///
/// # Examples
///
/// ```cpp
/// static_assert(is_shared_ptr_v<violet::SharedPtr<int>>);
/// static_assert(!is_shared_ptr_v<int>);
/// ```
template<typename T>
NOELDOC_SINCE("26.06.05")
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
struct NOELDOC_SINCE("26.06.05") shared_ptr_type;

template<typename T>
struct NOELDOC_SINCE("26.06.05") shared_ptr_type<std::shared_ptr<T>> {
    using type = T;
};

/// Convenience alias for `shared_ptr_type<T>::type`.
///
/// @since 26.06.05
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
struct NOELDOC_SINCE("26.06.05") FunctionParams;

template<typename R, typename... Args>
struct NOELDOC_SINCE("26.06.05") FunctionParams<R(Args...)> final {
    /// The parameter types packed into a `std::tuple`.
    using types = std::tuple<Args...>;

    /// The number of parameters in the function signature.
    constexpr static std::size_t arity = sizeof...(Args);
};

/// Satisfied when `T` is a complete object type; one whose size is known.
///
/// This excludes `void`, function types, incomplete class types, and arrays of
/// unknown bound.
///
/// @since 26.09
template<typename T>
concept complete_object = requires { sizeof(T); };
static_assert(complete_object<int32_t>);
static_assert(!complete_object<void>);
static_assert(!complete_object<int32_t[]>);

/// Satisfied when `T` has a known alignment requirement.
///
/// Nearly coextensive with [`complete_object`], but permits arrays of unknown bound (`alignof(T[])`
/// is valid where `sizeof(T[])` is not).
///
/// ## Compatibility
/// On Clang/MSVC, function types with `alignof`/`sizeof` is ill-formed, per the standard, but GCC
/// accepts it as a GNU extension (which returns `1`, which makes it "alignable"). So, function types
/// are permitted on GCC only, Clang/MSVC are a hard error.
///
/// @since 26.09
#if VIOLET_COMPILER(GCC)
template<typename T>
concept alignable = requires { alignof(T); } && !std::is_function_v<T>;
#else
template<typename T>
concept alignable = requires { alignof(T); };
#endif

static_assert(alignable<int32_t>);
static_assert(alignable<int32_t[]>);
static_assert(!alignable<void>);
static_assert(!alignable<void()>);

namespace NOELDOC_HIDE traits_internal {

#if VIOLET_FEATURE(TRIVIAL_RELOCATION)
template<typename T>
constexpr inline bool detect_relocatable_v = std::is_trivially_relocatable_v<T>;
#else
template<typename T>
constexpr inline bool detect_relocatable_v = std::is_trivially_copyable_v<T>;
#endif

} // namespace NOELDOC_HIDE traits_internal

/// Customization point declaring whether `T` can be relocated safely by copying its bytes.
///
/// A type is *trivially relocatable* when moving it to fresh storage and destroying the original
/// is equivalent to copying its object representation to that storage; i.e. when a move-construct-then-destroy
/// pair can be replaced by a `memcpy`. Containers in violet use this to grow a buffer with a single
/// `memcpy` instead of an per-element move loop.
///
/// The primary template is answered conseravtively: it says **true** for types the compiler can prove
/// by either being copyable or with `std::is_trivially_relocatable_v` in C++26. Many types are
/// trivially relocatable without being *detectable* so; violet's `Own<T>`, `Unique<T>`, `Vec<T>`, etc
/// have non-trivial move constructors that nonetheless cancel out against their destructors. No language
/// facility can detect that, so those types must opt in explicitly via [`VIOLET_DECLARE_TRIVIALLY_RELOCATABLE`].
///
/// Being wrong in the permissive direction is not a missed optimization, but silent memory corruption, so the
/// default never guesses. A type is safe to opt in when **no pointer inside of the object points at the object itself,
/// and nothing outside holds a pointer to it**. Disqualifying patterns:
///
/// * self-referential SBO (`libstdc++` (GCC)'s `std::string` points as its own internal buffer for SBO)
/// * embedded sentinel nodes (`libstdc++` (GCC) and libc++ (LLVM) store a list/tree end node inside
///   the container; heap nodes point back at it.)
/// * type erasure over unknown types ([`std::function`], [`std::any`], [`violet::experimental::Any`]; the stored
///   object may itself be self-referential)
/// * registration with some external resource ([`std::mutex`] may have published its address to the kernel).
///
/// Note that this is a property of implementation rather than interface: `std::string` is trivially relocatable
/// on libc++ and MSVC STL but not on stdlibc++.
///
/// @tparam T type that is being queried. Must be a complete type; cv-qualification are stripped by
///           [`is_trivially_relocatable_v`] before reaching the template, so specialization are
///           written for unqualified CV types.
template<typename T>
struct NOELDOC_SINCE("26.09") trivially_relocatable: std::bool_constant<traits_internal::detect_relocatable_v<T>> { };

template<typename T, std::size_t N>
struct NOELDOC_SINCE("26.09") NOELDOC_SEE("violet::trivially_relocatable") trivially_relocatable<T[N]> final
    : trivially_relocatable<std::remove_cv_t<T>> { };

/// Whether `T` is trivially relocatable.
///
/// The interface to [`trivially_relocatable_v`]; strips CV qualification before querying, so
/// `const T` and `T` always agree and specializations need to be only written once.
///
/// @tparam T The type being queried; must be complete.
template<typename T>
NOELDOC_SINCE("26.09")
constexpr inline bool is_trivially_relocatable_v = trivially_relocatable<std::remove_cv_t<T>>::value;

} // namespace violet

/**
 * @macro VIOLET_DECLARE_TRIVIALLY_RELOCATABLE_UNSAFE
 * @since 26.09
 *
 * Permits polymorphic types to be opted into trivial relocation. Disabled by default.
 *
 * Relocating a polymorphic object copies its vptr along-side everything else. Because
 * virtual dispatch tables are static per-class object at a fixed address, the copied vptr
 * remains correct on every ABI that Violet targets (libstdc++, libc++, MSVC STL); so
 * in practice, this works. The C++ standard doesn't guarantee it, and the committee hasn't
 * settled whether it should: [`P2786` (*"Trivial Relocatability For C++26"*)][P2786] treats
 * unannotated polymorphic types as eligible for trivial relocation, while [`P1144` (*"Object relocation in terms of
 * move plus destroy"*)][P1144] argues that it is unsound and can segfault in supposedly well-defined code.
 * Disagreements is the sharpest around virtual bases, where an implementation may lay out *offset-to-virtual-base*
 * entries such that a byte copy to a different address is not an equivalent to a move.
 *
 * Defining this macro to `1` only removes the static assertion that rejects polymorphic types
 * at the declaration site, it doesn't make any type relocatable on its own; each type must still
 * opt-in individualy with [`VIOLET_DECLARE_TRIVIALLY_RELOCATABLE`].
 *
 * > [!WARNING]
 * > * Never valid for any types with virtual bases, regardless of this setting.
 * > * This is a TU-wide switch, so enabling it for one type disables the guardrail for every other type
 *     in the same translation unit. Prefer scoping it to the narrowest build target that needs it.
 *
 * [P2786]: https://wg21.link/p2786
 * [P1144]: https://wg21.link/p1144
 */
#ifndef VIOLET_DECLARE_TRIVIALLY_RELOCATABLE_UNSAFE
#define VIOLET_DECLARE_TRIVIALLY_RELOCATABLE_UNSAFE 0
#endif

/**
 * @macro VIOLET_DECLARE_TRIVIALLY_RELOCATABLE
 * @since 26.09
 */
#define VIOLET_DECLARE_TRIVIALLY_RELOCATABLE(...)                                                                      \
    template<>                                                                                                         \
    struct violet::trivially_relocatable<__VA_ARGS__> final: std::true_type {                                          \
        static_assert(std::is_move_constructible_v<__VA_ARGS__> && std::is_destructible_v<__VA_ARGS__>,                \
            "type must be movable and destructible to be relocatable");                                                \
                                                                                                                       \
        static_assert(!std::is_polymorphic_v<__VA_ARGS__> || VIOLET_DECLARE_TRIVIALLY_RELOCATABLE_UNSAFE,              \
            "relocating a polymorphic type is a footgun; define `VIOLET_DECLARE_TRIVIALLY_RELOCATABLE_UNSAFE` to "     \
            "opt-in deliberately (read the documentation for more information: (TODO: this))");                        \
    };
