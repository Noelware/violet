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
//! # 🌺💜 `violet/Language/Macros.h`
//! This header contains important C macros that are useful for the development
//! of the `Noelware.Violet` C++ libraries. They also provide default implementations
//! if foreign build systems (i.e, not Bazel, CMake, or Meson) forget or do not add
//! C macros for `VIOLET_{LINUX|APPLE_MACOS|WINDOWS|X86_64|AARCH64|CLANG|GCC|MSVC}`
//! while the supported build systems Noelware maintains provide them:
//!
//! * Bazel: `buildsystem/bazel/cc.bzl`
//! * CMake: `buildsystem/cmake/CXX.cmake`
//! * Meson: `buildsystem/meson/defines/meson.build`

#pragma once

#ifndef VIOLET_VERSION
#error "missing `VIOLET_VERSION` define"
#endif

/**
 * @macro VIOLET_HAS_ATTRIBUTE
 * @param x The GNU/Clang attribute to test for.
 * @since 26.02
 *
 * Checks whether the compiler supports the given `__attribute__((x))`.
 * Wraps `__has_attribute` when available, otherwise evaluates to `0`.
 */
#if !defined(VIOLET_HAS_ATTRIBUTE) && defined(__has_attribute)
#define VIOLET_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define VIOLET_HAS_ATTRIBUTE(x) 0
#endif

/**
 * @macro VIOLET_HAS_CPP_ATTRIBUTE
 * @param x The C++ attribute to test for (e.g. `clang::lifetimebound`).
 * @since 26.02
 *
 * Checks whether the compiler supports the given `[[x]]` attribute.
 * Wraps `__has_cpp_attribute` when available, otherwise evaluates to `0`.
 */
#if !defined(VIOLET_HAS_CPP_ATTRIBUTE) && defined(__has_cpp_attribute)
#define VIOLET_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define VIOLET_HAS_CPP_ATTRIBUTE(x) 0
#endif

/**
 * @macro VIOLET_HAS_INCLUDE
 * @param x The header path to test for (e.g. `<optional>`).
 * @since 26.02
 *
 * Checks whether the given header is available for inclusion.
 * Wraps `__has_include` when available, otherwise evaluates to `0`.
 */
#if !defined(VIOLET_HAS_INCLUDE) && defined(__has_include)
#define VIOLET_HAS_INCLUDE(x) __has_include(x)
#else
#define VIOLET_HAS_INCLUDE(x) 0
#endif

/**
 * @macro VIOLET_HAS_FEATURE
 * @param x The compiler feature to test for (e.g. `cxx_rtti`, `address_sanitizer`).
 * @since 26.02
 *
 * Checks whether the compiler supports the given feature.
 * Wraps `__has_feature` when available, otherwise evaluates to `0`.
 */
#if !defined(VIOLET_HAS_FEATURE) && defined(__has_feature)
#define VIOLET_HAS_FEATURE(x) __has_feature(x)
#else
#define VIOLET_HAS_FEATURE(x) 0
#endif

/**
 * @macro VIOLET_HAS_BUILTIN
 * @param x The builtin name without the `__builtin_` prefix.
 * @since 26.02
 *
 * Checks whether the compiler provides `__builtin_##x`.
 * Wraps `__has_builtin` when available, otherwise evaluates to `0`.
 */
#if !defined(VIOLET_HAS_BUILTIN) && defined(__has_builtin)
#define VIOLET_HAS_BUILTIN(x) __has_builtin(__builtin_##x)
#else
#define VIOLET_HAS_BUILTIN(x) 0
#endif

// We need to fill in the following defines if they weren't defined before
// from foreign build systems that we don't support or they were stubbed out.
//
// * VIOLET_PLATFORM_{LINUX,APPLE_MACOS,WINDOWS}
// * VIOLET_ARCH_{X86_64,AARCH64}
// * VIOLET_COMPILER_{CLANG|MSVC|GCC}
// * VIOLET_BUILDSYSTEM_{BAZEL|CMAKE|MESON|GN} - new as of 26.05.05

/**
 * @macro VIOLET_PLATFORM_LINUX
 * @since 26.02
 *
 * Evaluates to `1` when targeting Linux (excluding Android), `0` otherwise.
 * May be pre-defined by the build system; auto-detected from `__linux__` if not.
 *
 * > [!IMPORTANT]
 * > As of 26.06, this was renamed to `VIOLET_PLATFORM_LINUX` from its original value: `VIOLET_LINUX` from a
 * > pre 26.06 release.
 */
#if defined(__linux__) && !defined(__ANDROID__)
#ifndef VIOLET_PLATFORM_LINUX
#define VIOLET_PLATFORM_LINUX 1
#endif
#else
#define VIOLET_PLATFORM_LINUX 0
#endif // !defined(VIOLET_PLATFORM_LINUX)

// If we ever have plans on adding iOS/tvOS/watchOS/visionOS support for this library,
// we'll consider adding it under the `VIOLET_APPLE_{IOS,TVOS,WATCHOS,VISIONOS}` umbrella
// defines, but for now, we only ever support macOS. This graph is for whenever we plan
// on adding it.
//
// (originally from this Stackoverflow post ~> https://stackoverflow.com/a/49560690):
//
// +--------------------------------------------------------------------------------------+
// |                                    TARGET_OS_MAC                                     |
// | +-----+ +------------------------------------------------------------+ +-----------+ |
// | |     | |                  TARGET_OS_IPHONE                          | |           | |
// | |     | | +-----------------+ +----+ +-------+ +--------+ +--------+ | |           | |
// | |     | | |       IOS       | |    | |       | |        | |        | | |           | |
// | | OSX | | | +-------------+ | | TV | | WATCH | | BRIDGE | | VISION | | | DRIVERKIT | |
// | |     | | | | MACCATALYST | | |    | |       | |        | |        | | |           | |
// | |     | | | +-------------+ | |    | |       | |        | |        | | |           | |
// | |     | | +-----------------+ +----+ +-------+ +--------+ +--------+ | |           | |
// | +-----+ +------------------------------------------------------------+ +-----------+ |
// +--------------------------------------------------------------------------------------+

/**
 * @macro VIOLET_PLATFORM_APPLE_MACOS
 * @since 26.02
 *
 * Evaluates to `1` when targeting macOS (via `TargetConditionals.h`), `0` otherwise.
 * May be pre-defined by the build system. Other Apple platforms (iOS, tvOS, watchOS,
 * visionOS) are not currently supported.
 *
 * > [!IMPORTANT]
 * > As of 26.06, this was renamed to `VIOLET_PLATFORM_APPLE_MACOS` from its original value: `VIOLET_APPLE_MACOS`
 * > from a pre 26.06 release.
 */
#if VIOLET_HAS_INCLUDE(<TargetConditionals.h>)
#include <TargetConditionals.h>

#if TARGET_OS_MAC && TARGET_OS_OSX
#ifndef VIOLET_PLATFORM_APPLE_MACOS
#define VIOLET_PLATFORM_APPLE_MACOS 1
#endif
#endif
#else
#define VIOLET_PLATFORM_APPLE_MACOS 0
#endif // !defined(VIOLET_PLATFORM_APPLE_MACOS) && VIOLET_HAS_INCLUDE(<TargetConditionals.h>)

/**
 * @macro VIOLET_PLATFORM_WINDOWS
 * @since 26.02
 *
 * Evaluates to `1` when targeting Windows, `0` otherwise.
 * May be pre-defined by the build system; auto-detected from `_WIN32` if not.
 *
 * > [!IMPORTANT]
 * > As of 26.06, this was renamed to `VIOLET_PLATFORM_WINDOWS` from its original value: `VIOLET_WINDOWS` from a
 * > pre 26.06 release.
 */
#if defined(_WIN32)
#ifndef VIOLET_PLATFORM_WINDOWS
#define VIOLET_PLATFORM_WINDOWS 1
#endif
#else
#define VIOLET_PLATFORM_WINDOWS 0
#endif // !defined(VIOLET_PLATFORM_WINDOWS) && defined(_WIN32)

/**
 * @macro VIOLET_ARCH_X86_64
 * @since 26.02
 *
 * Evaluates to `1` when compiling for x86-64 (AMD64), `0` otherwise.
 * May be pre-defined by the build system; auto-detected from compiler-provided
 * architecture macros if not.
 *
 * > [!IMPORTANT]
 * > As of 26.06, this was renamed to `VIOLET_ARCH_X86_64` from its original value: `VIOLET_X86_64` from a
 * > pre 26.06 release.
 */
#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(__x86_64) || defined(_M_AMD64)
#ifndef VIOLET_ARCH_X86_64
#define VIOLET_ARCH_X86_64 1
#endif
#else
#define VIOLET_ARCH_X86_64 0
#endif

/**
 * @macro VIOLET_ARCH_AARCH64
 * @since 26.02
 *
 * Evaluates to `1` when compiling for AArch64 (ARM64), `0` otherwise.
 * May be pre-defined by the build system; auto-detected from `__aarch64__` or
 * `_M_ARM64` if not.
 *
 * > [!IMPORTANT]
 * > As of 26.06, this was renamed to `VIOLET_ARCH_AARCH64` from its original value: `VIOLET_AARCH64` from a
 * > pre 26.06 release.
 */
#if (defined(__aarch64__) || defined(_M_ARM64))
#ifndef VIOLET_ARCH_AARCH64
#define VIOLET_ARCH_AARCH64 1
#endif
#else
#define VIOLET_ARCH_AARCH64 0
#endif

/**
 * @macro VIOLET_COMPILER_CLANG
 * @since 26.02
 *
 * Evaluates to `1` when compiling with Clang (not clang-cl), `0` otherwise.
 *
 * > [!IMPORTANT]
 * > As of 26.06, this was renamed to `VIOLET_COMPILER_CLANG` from its original value: `VIOLET_CLANG` from a
 * > pre 26.06 release.
 */
#if (defined(__clang__) && !defined(_MSC_VER))
#ifndef VIOLET_COMPILER_CLANG
#define VIOLET_COMPILER_CLANG 1
#endif
#else
#define VIOLET_COMPILER_CLANG 0
#endif

/**
 * @macro VIOLET_COMPILER_CLANG_CL
 * @since 26.06
 *
 * Evaluates to `1` when compiling with clang-cl (Clang in MSVC-compatible mode),
 * `0` otherwise.
 */
#if (defined(__clang__) && defined(_MSC_VER))
#ifndef VIOLET_COMPILER_CLANG_CL
#define VIOLET_COMPILER_CLANG_CL 1
#endif
#else
#define VIOLET_COMPILER_CLANG_CL 0
#endif

/**
 * @macro VIOLET_COMPILER_GCC
 * @since 26.02
 *
 * Evaluates to `1` when compiling with GCC (excluding Clang, which also defines
 * `__GNUC__`), `0` otherwise.
 *
 * > [!IMPORTANT]
 * > As of 26.06, this was renamed to `VIOLET_COMPILER_GCC` from its original value: `VIOLET_GCC` from a
 * > pre 26.06 release.
 */
#if (defined(__GNUC__) && !defined(__clang__))
#ifndef VIOLET_COMPILER_GCC
#define VIOLET_COMPILER_GCC 1
#endif
#else
#define VIOLET_COMPILER_GCC 0
#endif

/**
 * @macro VIOLET_COMPILER_MSVC
 * @since 26.02
 *
 * Evaluates to `1` when compiling with MSVC (excluding clang-cl), `0` otherwise.
 *
 * > [!IMPORTANT]
 * > As of 26.06, this was renamed to `VIOLET_PLATFORM_MSVC` from its original value: `VIOLET_MSVC` from a
 * > pre 26.06 release.
 */
#if (defined(_MSC_VER) && !defined(__clang__))
#ifndef VIOLET_COMPILER_MSVC
#define VIOLET_COMPILER_MSVC 1
#endif
#else
#define VIOLET_COMPILER_MSVC 0
#endif

/**
 * @macro VIOLET_BUILDSYSTEM_BAZEL
 * @since 26.06.05
 *
 * `1` if we are building from the Bazel build-system. `0` otherwise.
 */

/**
 * @macro VIOLET_BUILDSYSTEM_CMAKE
 * @since 26.06.05
 *
 * `1` if we are building from the CMake build-system. `0` otherwise.
 */

/**
 * @macro VIOLET_BUILDSYSTEM_MESON
 * @since 26.06.05
 *
 * * `1` if we are building from the Meson build-system. `0` otherwise.
 */

/**
 * @macro VIOLET_BUILDSYSTEM_GN
 * @since 26.06.05
 *
 * `1` if we are building from the GN ("Generate Ninja") build-system. `0` otherwise.
 */

/**
 * @macro VIOLET_BUILDSYSTEM_FOREIGN
 * @since 26.06.05
 *
 * Defined to `1` when none of the supported build systems (`BAZEL`, `CMAKE`, `MESON`,
 * `GN`) have been identified. The supported build system macros are expected to be set
 * by the build system itself (e.g. via `-D`).
 */
#if !defined(VIOLET_BUILDSYSTEM_BAZEL) && !defined(VIOLET_BUILDSYSTEM_CMAKE) && !defined(VIOLET_BUILDSYSTEM_MESON)     \
    && !defined(VIOLET_BUILDSYSTEM_GN)
#warning                                                                                                               \
    "Neither `VIOLET_BUILDSYSTEM_{BAZEL|CMAKE|MESON|GN}` are not set, this is a foreign buildsystem we don't have full support on!"
#define VIOLET_BUILDSYSTEM_FOREIGN 1
#endif

/**
 * @macro VIOLET_FEATURE_NOELDOC
 * @since 26.06.05
 *
 * Evaluates to `1` when building under the noeldoc documentation tool (detected
 * via `__NOELDOC__` or `NOELDOC`), `0` otherwise. Can be used to conditionally
 * expose or hide declarations for documentation purposes.
 */
#if defined(__NOELDOC__) || defined(NOELDOC)
#ifndef VIOLET_FEATURE_NOELDOC
#define VIOLET_FEATURE_NOELDOC 1
#endif
#else
#define VIOLET_FEATURE_NOELDOC 0
#endif

/**
 * @macro VIOLET_PLATFORM
 * @param platform The platform suffix (e.g. `LINUX`, `APPLE_MACOS`, `WINDOWS`).
 * @since 26.06
 *
 * Convenience accessor that expands to `VIOLET_PLATFORM_##platform`.
 * Intended for use in `#if` directives: `#if VIOLET_PLATFORM(LINUX)`.
 */
#define VIOLET_PLATFORM(platform) VIOLET_PLATFORM_##platform

/**
 * @macro VIOLET_ARCH
 * @param arch The architecture suffix (e.g. `X86_64`, `AARCH64`).
 * @since 26.06
 *
 * Convenience accessor that expands to `VIOLET_ARCH_##arch`.
 */
#define VIOLET_ARCH(arch) VIOLET_ARCH_##arch

/**
 * @macro VIOLET_COMPILER
 * @param compiler The compiler suffix (e.g. `CLANG`, `GCC`, `MSVC`, `CLANG_CL`).
 * @since 26.06
 *
 * Convenience accessor that expands to `VIOLET_COMPILER_##compiler`.
 */
#define VIOLET_COMPILER(compiler) VIOLET_COMPILER_##compiler

/**
 * @macro VIOLET_BUILDSYSTEM
 * @param system The build system suffix (e.g. `BAZEL`, `CMAKE`, `MESON`, `GN`, `FOREIGN`).
 * @since 26.06
 *
 * Convenience accessor that expands to `VIOLET_BUILDSYSTEM_##system`.
 */
#define VIOLET_BUILDSYSTEM(system) VIOLET_BUILDSYSTEM_##system

/**
 * @macro VIOLET_SANITIZER
 * @param sanitizer The sanitizer suffix (e.g. `MEMORY`, `ADDRESS`, `THREAD`, `UNDEFINED`).
 * @since 26.06
 *
 * Convenience accessor that expands to `VIOLET_SANITIZER_##sanitizer`.
 */
#define VIOLET_SANITIZER(sanitizer) VIOLET_SANITIZER_##sanitizer

/**
 * @macro VIOLET_FEATURE
 * @param feature The feature suffix (e.g. `NOELDOC`, `RTTI`, `EXCEPTIONS`).
 * @since 26.06
 *
 * Convenience accessor that expands to `VIOLET_FEATURE_##feature`.
 */
#define VIOLET_FEATURE(feature) VIOLET_FEATURE_##feature

/**
 * @macro VIOLET_API_EXPORT
 * @since 26.05.04
 *
 * Symbol visibility annotation for exporting symbols from a shared library.
 * Uses `__declspec(dllexport)` on MSVC/clang-cl and
 * `__attribute__((visibility("default")))` on Clang/GCC.
 */

/**
 * @macro VIOLET_API_IMPORT
 * @since 26.05.04
 *
 * Symbol visibility annotation for importing symbols from a shared library.
 * Uses `__declspec(dllimport)` on MSVC/clang-cl and
 * `__attribute__((visibility("default")))` on Clang/GCC.
 */

/**
 * @macro VIOLET_LOCAL
 * @since 26.05.04
 *
 * Symbol visibility annotation for hiding symbols from shared library export.
 * Uses `__attribute__((visibility("hidden")))` on Clang/GCC; no-op on MSVC.
 * Cleared to a no-op when `VIOLET_BUILDING` is defined (static/internal builds).
 */

/**
 * @macro VIOLET_API
 * @since 26.02
 *
 * The primary symbol visibility annotation for Violet's public API surface.
 * Resolves to `VIOLET_API_EXPORT` when building the shared library
 * (`VIOLET_BUILD_SHARED`), `VIOLET_API_IMPORT` when consuming it
 * (`VIOLET_SHARED`), and empty otherwise (static linking or internal builds).
 */
#ifndef VIOLET_API
#if VIOLET_COMPILER(MSVC) || VIOLET_COMPILER(CLANG_CL)
#define VIOLET_API_EXPORT __declspec(dllexport)
#define VIOLET_API_IMPORT __declspec(dllimport)
#define VIOLET_LOCAL
#elif VIOLET_COMPILER(CLANG) || VIOLET_COMPILER(GCC)
#define VIOLET_API_EXPORT __attribute__((visibility("default")))
#define VIOLET_API_IMPORT __attribute__((visibility("default")))
#define VIOLET_LOCAL __attribute__((visibility("hidden")))
#else
#define VIOLET_API_EXPORT
#define VIOLET_API_IMPORT
#define VIOLET_LOCAL
#endif
#endif // !defined(VIOLET_API)

#ifdef VIOLET_BUILDING
#define VIOLET_API
#undef VIOLET_LOCAL
#define VIOLET_LOCAL
#elif defined(VIOLET_BUILD_SHARED)
#define VIOLET_API VIOLET_API_EXPORT
#elif defined(VIOLET_SHARED)
#define VIOLET_API VIOLET_API_IMPORT
#else
#define VIOLET_API
#undef VIOLET_LOCAL
#define VIOLET_LOCAL
#endif // defined(VIOLET_BUILDING)

/**
 * @macro VIOLET_SANITIZER_MEMORY
 * @since 26.02
 *
 * Defined when MemorySanitizer is active. Auto-detected via
 * `__has_feature(memory_sanitizer)` on Clang. Must not be pre-defined by the user.
 *
 * > [!IMPORTANT]
 * > As of 26.07, this was renamed to `VIOLET_SANITIZER_MEMORY` from its original value: `VIOLET_MSAN` from a
 * > pre 26.07 release.
 */
#if defined(VIOLET_SANITIZER_MEMORY)
#error "Do not define `VIOLET_SANITIZER_MEMORY` on your own"
#else
#if VIOLET_COMPILER(CLANG) && VIOLET_HAS_FEATURE(memory_sanitizer)
#define VIOLET_SANITIZER_MEMORY 1
#else
#define VIOLET_SANITIZER_MEMORY 0
#endif
#endif

/**
 * @macro VIOLET_SANITIZER_ADDRESS
 * @since 26.02
 *
 * Defined when AddressSanitizer is active. Auto-detected via
 * `__has_feature(address_sanitizer)` or `__SANITIZE_ADDRESS__`.
 * Must not be pre-defined by the user.
 *
 * > [!IMPORTANT]
 * > As of 26.07, this was renamed to `VIOLET_SANITIZER_ADDRESS` from its original value: `VIOLET_ASAN` from a
 * > pre 26.07 release.
 */
#if defined(VIOLET_SANITIZER_ADDRESS)
#error "Do not define `VIOLET_SANITIZER_ADDRESS` on your own"
#else
#if VIOLET_HAS_FEATURE(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define VIOLET_SANITIZER_ADDRESS 1
#else
#define VIOLET_SANITIZER_ADDRESS 0
#endif
#endif

/**
 * @macro VIOLET_SANITIZER_THREAD
 * @since 26.02
 *
 * Defined when ThreadSanitizer is active. Auto-detected via
 * `__has_feature(thread_sanitizer)` or `__SANITIZE_THREAD__`.
 * Must not be pre-defined by the user.
 *
 * > [!IMPORTANT]
 * > As of 26.07, this was renamed to `VIOLET_SANITIZER_THREAD` from its original value: `VIOLET_TSAN` from a
 * > pre 26.07 release.
 */
#if defined(VIOLET_SANITIZER_THREAD)
#error "Do not define `VIOLET_SANITIZER_THREAD` on your own"
#else
#if VIOLET_HAS_FEATURE(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#define VIOLET_SANITIZER_THREAD 1
#else
#define VIOLET_SANITIZER_THREAD 0
#endif
#endif

/**
 * @macro VIOLET_SANITIZER_UNDEFINED
 * @since 26.02
 *
 * Defined when UndefinedBehaviorSanitizer is active. Auto-detected via
 * `__has_feature(undefined_behavior_sanitizer)`. Must not be pre-defined by the user.
 *
 * > [!IMPORTANT]
 * > As of 26.07, this was renamed to `VIOLET_SANITIZER_UNDEFINED` from its original value: `VIOLET_UBSAN` from a
 * > pre 26.07 release.
 */
#if defined(VIOLET_SANITIZER_UNDEFINED)
#error "Do not define `VIOLET_SANITIZER_UNDEFINED` on your own"
#else
#if VIOLET_HAS_FEATURE(undefined_behavior_sanitizer)
#define VIOLET_SANITIZER_UNDEFINED 1
#else
#define VIOLET_SANITIZER_UNDEFINED 0
#endif
#endif

/**
 * @macro VIOLET_FEATURE_RTTI
 * @since 26.02
 *
 * Evaluates to `1` when RTTI is enabled, `0` otherwise. This is the canonical
 * way to check for RTTI support, usable via `VIOLET_FEATURE(RTTI)`.
 * Must not be pre-defined by the user.
 *
 * > [!IMPORTANT]
 * > As of 26.07, this was renamed to `VIOLET_FEATURE_RTTI` from its original value: `VIOLET_USE_RTTI` from a
 * > pre 26.07 release.
 */
#ifdef VIOLET_FEATURE_RTTI
#error "do not define `VIOLET_FEATURE_RTTI` yourself"
#elif VIOLET_HAS_FEATURE(cxx_rtti)
#define VIOLET_FEATURE_RTTI 1
#elif (VIOLET_COMPILER(GCC) || VIOLET_COMPILER(CLANG)) && defined(__GXX_RTTI)
#define VIOLET_FEATURE_RTTI 1
#elif VIOLET_COMPILER(MSVC) && defined(_CPPRTTI)
#define VIOLET_FEATURE_RTTI 1
#else
#define VIOLET_FEATURE_RTTI 0
#endif

/**
 * @macro VIOLET_FEATURE_EXCEPTIONS
 * @since 26.02
 *
 * Evaluates to `1` when C++ exceptions are enabled, `0` otherwise.
 * Auto-detected from `_CPPUNWIND`, `__EXCEPTIONS`, or `__cpp_exceptions`.
 * Must not be pre-defined by the user.
 *
 * > [!IMPORTANT]
 * > As of 26.07, this was renamed to `VIOLET_FEATURE_EXCEPTIONS` from its original value: `VIOLET_HAS_EXCEPTIONS` from
 * a > pre 26.07 release.
 */
#ifdef VIOLET_FEATURE_EXCEPTIONS
#error "do not define `VIOLET_FEATURE_EXCEPTIONS` yourself"
#elif defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(__cpp_exceptions)
#define VIOLET_FEATURE_EXCEPTIONS 1
#else
#define VIOLET_FEATURE_EXCEPTIONS 0
#endif

/**
 * @macro VIOLET_FWD
 * @since 26.02
 * @param TYPE The type to forward as.
 * @param VALUE The value to forward.
 *
 * Shorthand for `std::forward<TYPE>(VALUE)`.
 */
#define VIOLET_FWD(TYPE, VALUE) ::std::forward<TYPE>(VALUE)

/**
 * @macro VIOLET_MOVE
 * @since 26.02
 * @param VALUE The value to move from.
 *
 * Shorthand for `std::move(VALUE)`.
 */
#define VIOLET_MOVE(VALUE) ::std::move(VALUE)

/**
 * @macro VIOLET_TO_STRING
 * @since 26.02
 * @param TYPE The type to make string-convertible.
 * @param NAME The parameter name used in the body for the value reference.
 * @param BLOCK The function body returning a `violet::String`.
 *
 * Defines a `violet::ToString(const TYPE&)` free function and a corresponding
 * `std::formatter` specialization so the type can be used with `std::format`.
 *
 * ## Example
 * ```cpp
 * VIOLET_TO_STRING(MyEnum, value, {
 *     switch (value) {
 *         case MyEnum::A: return "A"_s;
 *         case MyEnum::B: return "B"_s;
 *     }
 * })
 * ```
 */
#define VIOLET_TO_STRING(TYPE, NAME, BLOCK)                                                                            \
    namespace violet {                                                                                                 \
    inline auto ToString(const TYPE& NAME) -> ::violet::String BLOCK                                                   \
    }                                                                                                                  \
                                                                                                                       \
    template<>                                                                                                         \
    struct std::formatter<std::remove_cvref_t<TYPE>> final: public std::formatter<std::string> {                       \
        constexpr formatter() = default;                                                                               \
        template<class FC>                                                                                             \
        auto format(const TYPE& value, FC& cx) const                                                                   \
        {                                                                                                              \
            return ::std::formatter<std::string>::format(::violet::ToString(value), cx);                               \
        }                                                                                                              \
    };

/**
 * @macro VIOLET_FORMATTER
 * @since 26.02
 * @param TYPE The type to specialize `std::formatter` for.
 *
 * Generates a `std::formatter` specialization that delegates to
 * `value.ToString()`. The type must have a `ToString() const` member
 * returning something convertible to `std::string`.
 */
#define VIOLET_FORMATTER(TYPE)                                                                                         \
    template<>                                                                                                         \
    struct std::formatter<TYPE> final: public std::formatter<std::string> {                                            \
        constexpr formatter() = default;                                                                               \
        template<class FC>                                                                                             \
        auto format(const TYPE& value, FC& cx) const                                                                   \
        {                                                                                                              \
            return ::std::formatter<std::string>::format(value.ToString(), cx);                                        \
        }                                                                                                              \
    };

/**
 * @macro VIOLET_FORMATTER_TEMPLATE
 * @since 26.02
 * @param TYPE The (possibly dependent) type to specialize `std::formatter` for.
 * @param ... The template parameter list (e.g. `class T, class U`).
 *
 * Like `VIOLET_FORMATTER`, but for class templates. The variadic arguments are
 * forwarded as the template parameter declaration.
 *
 * # Example
 *
 * ```cpp
 * VIOLET_FORMATTER_TEMPLATE(MyContainer<T>, class T)
 * ```
 */
#define VIOLET_FORMATTER_TEMPLATE(TYPE, ...)                                                                           \
    template<__VA_ARGS__>                                                                                              \
    struct std::formatter<TYPE> final: public std::formatter<std::string> {                                            \
        constexpr formatter() = default;                                                                               \
        template<class FC>                                                                                             \
        auto format(const TYPE& value, FC& cx) const                                                                   \
        {                                                                                                              \
            return ::std::formatter<std::string>::format(value.ToString(), cx);                                        \
        }                                                                                                              \
    };

/**
 * @macro VIOLET_IMPLICIT
 * @since 26.02
 *
 * Semantic annotation marking a constructor or conversion as intentionally
 * implicit. Expands to nothing; exists purely for readability and grep-ability.
 */
#define VIOLET_IMPLICIT

/**
 * @macro VIOLET_EXPLICIT
 * @since 26.02
 *
 * Semantic annotation equivalent to `explicit`. Provided for symmetry with
 * `VIOLET_IMPLICIT`.
 */
#define VIOLET_EXPLICIT explicit

/**
 * @macro VIOLET_LIKELY
 * @since 26.02
 *
 * Expands to `[[likely]]` when supported, empty otherwise.
 * Intended for use on branch targets to hint the optimizer.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(likely)
#define VIOLET_LIKELY [[likely]]
#else
#define VIOLET_LIKELY
#endif

/**
 * @macro VIOLET_UNLIKELY
 * @since 26.02
 *
 * Expands to `[[unlikely]]` when supported, empty otherwise.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(unlikely)
#define VIOLET_UNLIKELY [[unlikely]]
#else
#define VIOLET_UNLIKELY
#endif

/**
 * @macro VIOLET_LIKELY_IF
 * @since 26.02
 * @param expr The condition expression.
 *
 * Combines `if (expr)` with `[[likely]]` on the taken branch.
 */
// clang-format off
#define VIOLET_LIKELY_IF(expr) if (expr) VIOLET_LIKELY

/**
 * @macro VIOLET_UNLIKELY_IF
 * @since 26.02
 * @param expr The condition expression.
 *
 * Combines `if (expr)` with `[[unlikely]]` on the taken branch.
 */
#define VIOLET_UNLIKELY_IF(expr) if (expr) VIOLET_UNLIKELY
// clang-format on

/**
 * @macro VIOLET_COLD
 * @since 26.02
 *
 * Marks a function as cold (rarely executed). Expands to `[[gnu::cold]]` on
 * Clang/GCC, `__declspec(noinline)` on MSVC, or empty if unsupported.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(gnu::cold)
#define VIOLET_COLD [[gnu::cold]]
#elif VIOLET_PLATFORM(WINDOWS)
#define VIOLET_COLD __declspec(noinline)
#else
#define VIOLET_COLD
#endif

/**
 * @macro VIOLET_HOT
 * @since 26.02
 *
 * Marks a function as hot (frequently executed). Expands to `[[gnu::hot]]`
 * when supported, empty otherwise.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(gnu::hot)
#define VIOLET_HOT [[gnu::hot]]
#else
#define VIOLET_HOT
#endif

/**
 * @macro VIOLET_LIFETIMEBOUND
 * @since 26.05
 *
 * Annotates a parameter or return value as lifetime-bound to the receiver,
 * enabling compiler diagnostics for dangling references. Supports
 * `[[clang::lifetimebound]]`, `[[msvc::lifetimebound]]`, and the
 * vendor-neutral `[[lifetimebound]]`.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(clang::lifetimebound)
#define VIOLET_LIFETIMEBOUND [[clang::lifetimebound]]
#elif VIOLET_HAS_CPP_ATTRIBUTE(msvc::lifetimebound)
#define VIOLET_LIFETIMEBOUND [[msvc::lifetimebound]]
#elif VIOLET_HAS_CPP_ATTRIBUTE(lifetimebound)
#define VIOLET_LIFETIMEBOUND [[lifetimebound]]
#else
#define VIOLET_LIFETIMEBOUND
#endif

/**
 * @macro VIOLET_PURE
 * @since 26.05
 *
 * Marks a function as pure (result depends only on arguments and global state,
 * no side effects other than the return value). Expands to `[[gnu::pure]]` or
 * `__attribute__((pure))` when supported.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(gnu::pure)
#define VIOLET_PURE [[gnu::pure]]
#elif VIOLET_HAS_ATTRIBUTE(pure)
#define VIOLET_PURE __attribute__((pure))
#else
#define VIOLET_PURE
#endif

/**
 * @macro VIOLET_NOESCAPE
 * @since 26.05
 *
 * Annotates a pointer/reference parameter as not escaping the callee.
 * Expands to `[[clang::noescape]]` when supported.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(clang::noescape)
#define VIOLET_NOESCAPE [[clang::noescape]]
#else
#define VIOLET_NOESCAPE
#endif

/**
 * @macro VIOLET_REINITIALIZES_MEMORY
 * @since 26.05
 *
 * Annotates a method as reinitializing the object, suppressing
 * use-after-move warnings from Clang's consumed-analysis.
 * Expands to `[[clang::reinitializes]]` when supported.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(clang::reinitializes)
#define VIOLET_REINITIALIZES_MEMORY [[clang::reinitializes]]
#else
#define VIOLET_REINITIALIZES_MEMORY
#endif

#define __violet_stringify_helper__(x) #x
#define __violet_stringify__(x) __violet_stringify_helper__(x)

/**
 * @macro VIOLET_DIAGNOSTIC_PUSH
 * @since 26.02
 *
 * Pushes the current compiler diagnostic state onto the stack.
 * Pair with `VIOLET_DIAGNOSTIC_POP`.
 */

/**
 * @macro VIOLET_DIAGNOSTIC_POP
 * @since 26.02
 *
 * Pops the most recently pushed diagnostic state.
 */

/**
 * @macro VIOLET_DIAGNOSTIC_IGNORE
 * @since 26.02
 * @param x The warning flag to suppress (e.g. `"-Wunused-variable"`).
 *
 * Suppresses the specified warning within a `VIOLET_DIAGNOSTIC_PUSH` /
 * `VIOLET_DIAGNOSTIC_POP` region. On MSVC, pass the warning number instead.
 */
#if VIOLET_COMPILER(CLANG)
#define VIOLET_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#define VIOLET_DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#define VIOLET_DIAGNOSTIC_IGNORE(x) _Pragma(__violet_stringify__(clang diagnostic ignored x))
#elif VIOLET_COMPILER(GCC)
#define VIOLET_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define VIOLET_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#define VIOLET_DIAGNOSTIC_IGNORE(x) _Pragma(__violet_stringify__(GCC diagnostic ignored x))
#elif VIOLET_COMPILER(MSVC)
#define VIOLET_DIAGNOSTIC_PUSH __pragma(warning(push))
#define VIOLET_DIAGNOSTIC_POP __pragma(warning(pop))
#define VIOLET_DIAGNOSTIC_IGNORE(x) __pragma(warning(disable : x))
#else
#define VIOLET_DIAGNOSTIC_PUSH
#define VIOLET_DIAGNOSTIC_POP
#define VIOLET_DIAGNOSTIC_IGNORE(x)
#endif

/**
 * @macro VIOLET_DISALLOW_CONSTRUCTOR
 * @since 26.02
 * @param Type The class name.
 *
 * Deletes the default constructor.
 */
#define VIOLET_DISALLOW_CONSTRUCTOR(Type) VIOLET_IMPLICIT Type() noexcept = delete;

/**
 * @macro VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR
 * @since 26.02
 * @param Type The class name.
 *
 * Deletes the default constructor with `constexpr` qualification.
 */
#define VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Type) constexpr VIOLET_IMPLICIT Type() noexcept = delete;

/**
 * @macro VIOLET_DISALLOW_COPY
 * @since 26.02
 * @param Type The class name.
 *
 * Deletes the copy constructor and copy assignment operator.
 */
#define VIOLET_DISALLOW_COPY(Type)                                                                                     \
    VIOLET_IMPLICIT Type(const Type&) noexcept = delete;                                                               \
    auto operator=(const Type&) noexcept -> Type& = delete;

/**
 * @macro VIOLET_DISALLOW_MOVE
 * @since 26.02
 * @param Type The class name.
 *
 * Deletes the move constructor and move assignment operator.
 */
#define VIOLET_DISALLOW_MOVE(Type)                                                                                     \
    VIOLET_IMPLICIT Type(Type&&) noexcept = delete;                                                                    \
    auto operator=(Type&&) noexcept -> Type& = delete;

/**
 * @macro VIOLET_DISALLOW_COPY_AND_MOVE
 * @since 26.02
 * @param Type The class name.
 *
 * Deletes copy and move constructors and assignment operators.
 */
#define VIOLET_DISALLOW_COPY_AND_MOVE(Type)                                                                            \
    VIOLET_DISALLOW_COPY(Type)                                                                                         \
    VIOLET_DISALLOW_MOVE(Type)

/**
 * @macro VIOLET_DISALLOW_CONSTEXPR_COPY
 * @since 26.02
 * @param Type The class name.
 *
 * Deletes the copy constructor and copy assignment operator with `constexpr`
 * qualification.
 */
#define VIOLET_DISALLOW_CONSTEXPR_COPY(Type)                                                                           \
    constexpr VIOLET_IMPLICIT Type(const Type&) noexcept = delete;                                                     \
    constexpr auto operator=(const Type&) noexcept -> Type& = delete;

/**
 * @macro VIOLET_DISALLOW_CONSTEXPR_MOVE
 * @since 26.02
 * @param Type The class name.
 *
 * Deletes the move constructor and move assignment operator with `constexpr`
 * qualification.
 */
#define VIOLET_DISALLOW_CONSTEXPR_MOVE(Type)                                                                           \
    constexpr VIOLET_IMPLICIT Type(Type&&) noexcept = delete;                                                          \
    constexpr auto operator=(Type&&) noexcept -> Type& = delete;

/**
 * @macro VIOLET_DISALLOW_CONSTEXPR_COPY_AND_MOVE
 * @since 26.02
 * @param Type The class name.
 *
 * Deletes copy and move constructors and assignment operators with `constexpr`
 * qualification.
 */
#define VIOLET_DISALLOW_CONSTEXPR_COPY_AND_MOVE(Type)                                                                  \
    VIOLET_DISALLOW_CONSTEXPR_COPY(Type)                                                                               \
    VIOLET_DISALLOW_CONSTEXPR_MOVE(Type)

/**
 * @macro VIOLET_IMPLICIT_COPY
 * @since 26.02
 * @param Type The class name.
 *
 * Defaults the copy constructor and copy assignment operator.
 */
#define VIOLET_IMPLICIT_COPY(Type)                                                                                     \
    VIOLET_IMPLICIT Type(const Type&) noexcept = default;                                                              \
    auto operator=(const Type&) noexcept -> Type& = default;

/**
 * @macro VIOLET_IMPLICIT_MOVE
 * @since 26.02
 * @param Type The class name.
 *
 * Defaults the move constructor and move assignment operator.
 */
#define VIOLET_IMPLICIT_MOVE(Type)                                                                                     \
    VIOLET_IMPLICIT Type(Type&&) noexcept = default;                                                                   \
    auto operator=(Type&&) noexcept -> Type& = default;

/**
 * @macro VIOLET_IMPLICIT_COPY_AND_MOVE
 * @since 26.02
 * @param Type The class name.
 *
 * Defaults copy and move constructors and assignment operators.
 */
#define VIOLET_IMPLICIT_COPY_AND_MOVE(Type)                                                                            \
    VIOLET_IMPLICIT_COPY(Type)                                                                                         \
    VIOLET_IMPLICIT_MOVE(Type)

/**
 * @macro VIOLET_IMPLICIT_CONSTEXPR_COPY
 * @since 26.02
 * @param Type The class name.
 *
 * Defaults the copy constructor and copy assignment operator with `constexpr`
 * qualification.
 */
#define VIOLET_IMPLICIT_CONSTEXPR_COPY(Type)                                                                           \
    constexpr VIOLET_IMPLICIT Type(const Type&) noexcept = default;                                                    \
    constexpr auto operator=(const Type&) noexcept -> Type& = default;

/**
 * @macro VIOLET_IMPLICIT_CONSTEXPR_MOVE
 * @since 26.02
 * @param Type The class name.
 *
 * Defaults the move constructor and move assignment operator with `constexpr`
 * qualification.
 */
#define VIOLET_IMPLICIT_CONSTEXPR_MOVE(Type)                                                                           \
    constexpr VIOLET_IMPLICIT Type(Type&&) noexcept = default;                                                         \
    constexpr auto operator=(Type&&) noexcept -> Type& = default;

/**
 * @macro VIOLET_IMPLICIT_CONSTEXPR_COPY_AND_MOVE
 * @since 26.02
 * @param Type The class name.
 *
 * Defaults copy and move constructors and assignment operators with `constexpr`
 * qualification.
 */
#define VIOLET_IMPLICIT_CONSTEXPR_COPY_AND_MOVE(Type)                                                                  \
    VIOLET_IMPLICIT_CONSTEXPR_COPY(Type)                                                                               \
    VIOLET_IMPLICIT_CONSTEXPR_MOVE(Type)

/**
 * @macro VIOLET_CPLUSPLUS
 * @since 26.06
 *
 * The C++ standard version number. Uses `_MSVC_LANG` on MSVC (since MSVC's
 * `__cplusplus` is unreliable without `/Zc:__cplusplus`), and `__cplusplus`
 * everywhere else.
 */
#ifdef _MSVC_LANG
#define VIOLET_CPLUSPLUS _MSVC_LANG
#else
#define VIOLET_CPLUSPLUS __cplusplus
#endif

/**
 * @macro VIOLET_REQUIRE_STL
 * @since 26.02
 * @param ver The C++ standard version number (e.g. `202002L` for C++20).
 *
 * Evaluates to `true` if the current C++ standard is at least @p ver.
 */
#define VIOLET_REQUIRE_STL(ver) (VIOLET_CPLUSPLUS >= ver)

/**
 * @macro VIOLET_PLATFORM_LITTLE_ENDIAN
 * @since 26.02.03
 *
 * Defined to `1` on little-endian targets. Auto-detected from `__BYTE_ORDER__`
 * or assumed on Windows. Must not be pre-defined by the user.
 *
 * > [!IMPORTANT]
 * > As of 26.07, this was renamed to `VIOLET_PLATFORM_LITTLE_ENDIAN` from its original value: `VIOLET_IS_LITTLE_ENDIAN`
 * > from a pre 26.07 release.
 */

/**
 * @macro VIOLET_PLATFORM_BIG_ENDIAN
 * @since 26.02.03
 *
 * Defined to `1` on big-endian targets. Auto-detected from `__BYTE_ORDER__`.
 * Must not be pre-defined by the user.
 *
 * > [!IMPORTANT]
 * > As of 26.07, this was renamed to `VIOLET_PLATFORM_BIG_ENDIAN` from its original value: `VIOLET_IS_BIG_ENDIAN`
 * > from a pre 26.07 release.
 */
#ifdef VIOLET_PLATFORM_LITTLE_ENDIAN
#error "Do not pre-define `VIOLET_PLATFORM_LITTLE_ENDIAN`"
#endif

#ifdef VIOLET_PLATFORM_BIG_ENDIAN
#error "Do not pre-define `VIOLET_PLATFORM_LITTLE_ENDIAN`"
#endif

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define VIOLET_PLATFORM_LITTLE_ENDIAN 1
#elif VIOLET_PLATFORM(WINDOWS)
#define VIOLET_PLATFORM_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define VIOLET_PLATFORM_BIG_ENDIAN 1
#else
#error "Cannot determine endianness"
#endif

/**
 * @macro VIOLET_CONCAT
 * c
 * @param x The first token.
 * @param y The second token.
 *
 * Concatenates two preprocessor tokens after macro expansion.
 */
#define __violet_concat_inner__(x, y) x##y
#define VIOLET_CONCAT(x, y) __violet_concat_inner__(x, y)

/**
 * @macro VIOLET_UNIQUE_NAME
 * @since 26.03.05
 * @param prefix The name prefix.
 *
 * Generates a unique identifier by appending `__COUNTER__` to @p prefix.
 * Useful for creating non-colliding local variable names in macros.
 */
#define VIOLET_UNIQUE_NAME(prefix) VIOLET_CONCAT(prefix, __COUNTER__)

/**
 * @macro VIOLET_DEPRECATED
 * @since 26.03.08
 * @param since The version since which the symbol is deprecated.
 *
 * Marks a declaration as deprecated with a version tag.
 * Expands to `[[deprecated("since <version>")]]` when supported.
 */

/**
 * @macro VIOLET_DEPRECATED_BECAUSE
 * @since 26.03.08
 * @param since The version since which the symbol is deprecated.
 * @param message A human-readable reason for the deprecation.
 *
 * Marks a declaration as deprecated with a version tag and explanatory message.
 */
#if VIOLET_HAS_ATTRIBUTE(deprecated)
#define VIOLET_DEPRECATED(since) [[deprecated("since " #since)]]
#define VIOLET_DEPRECATED_BECAUSE(since, message) [[deprecated("since " #since ": " message)]]
#else
#define VIOLET_DEPRECATED(since)
#define VIOLET_DEPRECATED_BECAUSE(since, message)
#endif

/**
 * @macro VIOLET_UNREACHABLE
 * @since 26.04.04
 *
 * Marks a code path as unreachable. Uses `std::unreachable()` (C++23),
 * `__assume(false)` (MSVC), or `__builtin_unreachable()` (GCC/Clang).
 * Falls back to an infinite loop if none are available.
 */
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#define VIOLET_UNREACHABLE() ::std::unreachable()
#elif VIOLET_COMPILER(MSVC)
#define VIOLET_UNREACHABLE() __assume(false)
#elif VIOLET_COMPILER(GCC) || VIOLET_COMPILER(CLANG)
#define VIOLET_UNREACHABLE() __builtin_unreachable()
#else
#define VIOLET_UNREACHABLE()                                                                                           \
    do {                                                                                                               \
        for (;;)                                                                                                       \
            ;                                                                                                          \
    } while (false)
#endif

/**
 * @macro VIOLET_IF_CONSTEVAL
 * @since 26.05.05
 *
 * Expands to `consteval` (C++23) or `(std::is_constant_evaluated())` (C++20)
 * for use in `if VIOLET_IF_CONSTEVAL { ... }` patterns to branch on
 * constant evaluation context.
 */

/**
 * @macro VIOLET_IF_NOT_CONSTEVAL
 * @since 26.05.05
 *
 * The negated form of `VIOLET_IF_CONSTEVAL`. Expands to `not consteval`
 * (C++23) or `(!std::is_constant_evaluated())` (C++20).
 */
#if __cpp_if_consteval >= 202106L
#define VIOLET_IF_CONSTEVAL consteval
#define VIOLET_IF_NOT_CONSTEVAL not consteval
#else
#define VIOLET_IF_CONSTEVAL (std::is_constant_evaluated())
#define VIOLET_IF_NOT_CONSTEVAL (!std::is_constant_evaluated())
#endif

/**
 * @macro VIOLET_NO_UNIQUE_ADDRESS
 * @since 26.06.05
 *
 * Expands to `[[no_unique_address]]` (or the MSVC equivalent) when supported.
 * Allows empty class members to share storage with other members, enabling
 * EBO (Empty Base Optimization) for composition.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(msvc::no_unique_address)
#define VIOLET_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif VIOLET_HAS_CPP_ATTRIBUTE(no_unique_address)
#define VIOLET_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define VIOLET_NO_UNIQUE_ADDRESS
#endif

/**
 * @macro VIOLET_ANNOTATE
 * @since 26.06.05
 * @param ... Annotation arguments passed to `[[clang::annotate(...)]]`.
 *
 * Attaches a Clang annotation attribute to a declaration. No-op when the
 * `annotate` attribute is not supported.
 */
#if VIOLET_HAS_CPP_ATTRIBUTE(clang::annotate)
#define VIOLET_ANNOTATE(...) [[clang::annotate(__VA_ARGS__)]]
#define __noeldoc_annotate__(...) VIOLET_ANNOTATE("noeldoc(" __VA_ARGS__ ")")
#elif VIOLET_HAS_ATTRIBUTE(annotate)
#define VIOLET_ANNOTATE(...) attribute((annotate(__VA_ARGS__)))
#define __noeldoc_annotate__(...) VIOLET_ANNOTATE("noeldoc(" __VA_ARGS__ ")")
#else
#define VIOLET_ANNOTATE(...)
#define __noeldoc_annotate__(...)
#endif

/**
 * @macro NOELDOC_EXPERIMENTAL_SINCE
 * @since 26.07
 * @param ver The version string when the symbol became experimental.
 *
 * Annotates a declaration as experimental since the given version.
 * Consumed by the noeldoc documentation tool.
 */
#define NOELDOC_EXPERIMENTAL_SINCE(ver) __noeldoc_annotate__("experimental(since:" ver ")")

/**
 * @macro NOELDOC_STABLIZED_SINCE
 * @since 26.07
 * @param ver The version string when the symbol was stabilized.
 *
 * Annotates a declaration as stabilized since the given version.
 * Consumed by the noeldoc documentation tool.
 */
#define NOELDOC_STABLIZED_SINCE(ver) __noeldoc_annotate__("stablized(since:" ver ")")

/**
 * @macro NOELDOC_SINCE
 * @since 26.06.05
 * @param ver The version string when the symbol was introduced.
 *
 * Annotates a declaration with the version it was first available.
 */
#define NOELDOC_SINCE(ver) __noeldoc_annotate__("since:" ver)

/**
 * @macro NOELDOC_SEE
 * @since 26.06.05
 * @param ref A reference to a related symbol.
 *
 * Adds a cross-reference ("see also") annotation for the noeldoc tool.
 */
#define NOELDOC_SEE(ref) __noeldoc_annotate__("see:" #ref)

/**
 * @macro NOELDOC_HIDE
 * @since 26.06.05
 *
 * Hides the annotated declaration from generated documentation.
 */
#define NOELDOC_HIDE __noeldoc_annotate__("hide")
