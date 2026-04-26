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
//! * Meson: ...?

#pragma once

#ifndef VIOLET_VERSION
#error "missing `VIOLET_VERSION` define"
#endif

#if !defined(VIOLET_HAS_ATTRIBUTE) && defined(__has_attribute)
#define VIOLET_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define VIOLET_HAS_ATTRIBUTE(x) 0
#endif

#if !defined(VIOLET_HAS_CPP_ATTRIBUTE) && defined(__has_cpp_attribute)
#define VIOLET_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define VIOLET_HAS_CPP_ATTRIBUTE(x) 0
#endif

#if !defined(VIOLET_HAS_INCLUDE) && defined(__has_include)
#define VIOLET_HAS_INCLUDE(x) __has_include(x)
#else
#define VIOLET_HAS_INCLUDE(x) 0
#endif

#if !defined(VIOLET_HAS_FEATURE) && defined(__has_feature)
#define VIOLET_HAS_FEATURE(x) __has_feature(x)
#else
#define VIOLET_HAS_FEATURE(x) 0
#endif

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
#if defined(VIOLET_LINUX) && !defined(VIOLET_PLATFORM_LINUX)
#warning "`VIOLET_LINUX` is a deprecated define, use `VIOLET_PLATFORM_LINUX` instead"
#define VIOLET_PLATFORM_LINUX
#endif // defined(VIOLET_LINUX) && !defined(VIOLET_PLATFORM_LINUX)

#ifndef VIOLET_PLATFORM_LINUX
#if defined(__linux__) && !defined(__ANDROID__)
#define VIOLET_PLATFORM_LINUX
#endif
#endif // !defined(VIOLET_PLATFORM_LINUX)

#if defined(VIOLET_APPLE_MACOS) && !defined(VIOLET_PLATFORM_APPLE_MACOS)
#warning "`VIOLET_APPLE_MACOS` is a deprecated define, use `VIOLET_PLATFORM_APPLE_MACOS` instead"
#define VIOLET_PLATFORM_APPLE_MACOS
#endif

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
#if !defined(VIOLET_PLATFORM_APPLE_MACOS) && VIOLET_HAS_INCLUDE(<TargetConditionals.h>)
#include <TargetConditionals.h>
#if TARGET_OS_MAC && TARGET_OS_OSX
#define VIOLET_PLATFORM_APPLE_MACOS
#endif
#endif // !defined(VIOLET_PLATFORM_APPLE_MACOS) && VIOLET_HAS_INCLUDE(<TargetConditionals.h>)

#if defined(VIOLET_WINDOWS) && !defined(VIOLET_PLATFORM_WINDOWS)
#warning "`VIOLET_WINDOWS` is a deprecated define, use `VIOLET_PLATFORM_WINDOWS` instead"
#define VIOLET_PLATFORM_WINDOWS
#endif

#if !defined(VIOLET_PLATFORM_WINDOWS) && defined(_WIN32)
#define VIOLET_PLATFORM_WINDOWS
#endif // !defined(VIOLET_PLATFORM_WINDOWS) && defined(_WIN32)

#if defined(VIOLET_X86_64) && !defined(VIOLET_ARCH_X86_64)
#warning "`VIOLET_X86_64` is a deprecated define, use `VIOLET_ARCH_X86_64` instead"
#define VIOLET_ARCH_X86_64
#endif

#if !defined(VIOLET_ARCH_X86_64)                                                                                       \
    && (defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(__x86_64) || defined(_M_AMD64))
#define VIOLET_ARCH_VIOLET_ARCH_X86_64
#endif // !defined(VIOLET_ARCH_X86_64) && (defined(__x86_64__) || defined(__amd64__) || defined(__amd64) ||
       // defined(__x86_64) || defined(_M_AMD64))

#if defined(VIOLET_AARCH64) && !defined(VIOLET_ARCH_AARCH64)
#warning "`VIOLET_AARCH64` is a deprecated define, use `VIOLET_ARCH_AARCH64` instead"
#define VIOLET_ARCH_AARCH64
#endif

#if !defined(VIOLET_ARCH_AARCH64) && (defined(__aarch64__) || defined(_M_ARM64))
#define VIOLET_ARCH_AARCH64
#endif // !defined(VIOLET_ARCH_AARCH64) && (defined(__aarch64__) || defined(_M_ARM64))

#if defined(VIOLET_CLANG) && !defined(VIOLET_COMPILER_CLANG)
#warning "`VIOLET_CLANG` is a deprecated define, use `VIOLET_COMPILER_CLANG` instead"
#define VIOLET_COMPILER_CLANG
#endif

#if !defined(VIOLET_COMPILER_CLANG) && (defined(__clang__) && !defined(_MSC_VER))
#define VIOLET_COMPILER_CLANG
#endif // !defined(VIOLET_COMPILER_CLANG) && (defined(__clang__) && !defined(_MSC_VER))

#if !defined(VIOLET_COMPILER_CLANG_CL) && (defined(__clang__) && defined(_MSC_VER))
#define VIOLET_COMPILER_CLANG_CL
#endif // !defined(VIOLET_COMPILER_CLANG_CL) && (defined(__clang__) && defined(_MSC_VER))

#if defined(VIOLET_GCC) && !defined(VIOLET_COMPILER_GCC)
#warning "`VIOLET_GCC` is a deprecated define, use `VIOLET_COMPILER_GCC` instead"
#define VIOLET_COMPILER_GCC
#endif

#if !defined(VIOLET_GCC) && (defined(__GNUC__) && !defined(__clang__))
#define VIOLET_COMPILER_GCC
#endif // !defined(VIOLET_GCC) && (defined(__GNUC__) && !defined(__clang__))

#if defined(VIOLET_MSVC) && !defined(VIOLET_COMPILER_MSVC)
#warning "`VIOLET_MSVC` is a deprecated define, use `VIOLET_COMPILER_MSVC` instead"
#define VIOLET_COMPILER_MSVC
#endif

#if !defined(VIOLET_MSVC) && (defined(_MSC_VER) && !defined(__clang__))
#define VIOLET_COMPILER_MSVC
#endif // !defined(VIOLET_MSVC) && (defined(_MSC_VER) && !defined(__clang__))

#if !defined(VIOLET_BUILDSYSTEM_BAZEL) && !defined(VIOLET_BUILDSYSTEM_CMAKE) && !defined(VIOLET_BUILDSYSTEM_MESON)     \
    && !defined(VIOLET_BUILDSYSTEM_GN)
#warning                                                                                                               \
    "Neither `VIOLET_BUILDSYSTEM_{BAZEL|CMAKE|MESON|GN}` are not set, this is a foreign buildsystem we don't have full support on!"
#define VIOLET_BUILDSYSTEM_FOREIGN
#endif

#define VIOLET_PLATFORM(platform) defined(VIOLET_PLATFORM_##platform)
#define VIOLET_ARCH(arch) defined(VIOLET_ARCH_##arch)
#define VIOLET_COMPILER(compiler) defined(VIOLET_COMPILER_##compiler)
#define VIOLET_BUILDSYSTEM(system) defined(VIOLET_BUILDSYSTEM_##system)

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

#ifndef VIOLET_MSAN
#if VIOLET_COMPILER(CLANG) && VIOLET_HAS_FEATURE(memory_sanitizer)
#define VIOLET_MSAN
#endif
#endif

#ifndef VIOLET_ASAN
#if VIOLET_HAS_FEATURE(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#define VIOLET_ASAN
#endif
#endif

#ifndef VIOLET_TSAN
#if VIOLET_HAS_FEATURE(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#define VIOLET_TSAN
#endif
#endif

#ifndef VIOLET_UBSAN
#if VIOLET_HAS_FEATURE(undefined_behavior_sanitizer)
#define VIOLET_UBSAN
#endif
#endif

#ifdef VIOLET_USE_RTTI
#error "`VIOLET_USE_RTTI` cannot be directly set"
#elif VIOLET_HAS_FEATURE(cxx_rtti)
#define VIOLET_USE_RTTI 1
#elif defined(__GNUC__) && defined(__GXX_RTTI)
#define VIOLET_USE_RTTI 1
#elif defined(_MSC_VER) && defined(_CPPRTTI)
#define VIOLET_USE_RTTI 1
#else
#define VIOLET_USE_RTTI 0
#endif

#define VIOLET_FWD(TYPE, VALUE) ::std::forward<TYPE>(VALUE)
#define VIOLET_MOVE(VALUE) ::std::move(VALUE)

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

#define VIOLET_IMPLICIT
#define VIOLET_EXPLICIT explicit

#if VIOLET_HAS_CPP_ATTRIBUTE(likely)
#define VIOLET_LIKELY [[likely]]
#else
#define VIOLET_LIKELY
#endif

#if VIOLET_HAS_CPP_ATTRIBUTE(unlikely)
#define VIOLET_UNLIKELY [[unlikely]]
#else
#define VIOLET_UNLIKELY
#endif

// clang-format off
#define VIOLET_LIKELY_IF(expr) if (expr) VIOLET_LIKELY
#define VIOLET_UNLIKELY_IF(expr) if (expr) VIOLET_UNLIKELY
// clang-format on

#if VIOLET_HAS_CPP_ATTRIBUTE(gnu::cold)
#define VIOLET_COLD [[gnu::cold]]
#elif VIOLET_WINDOWS
#define VIOLET_COLD __declspec(noinline)
#else
#define VIOLET_COLD
#endif

#if VIOLET_HAS_CPP_ATTRIBUTE(gnu::hot)
#define VIOLET_HOT [[gnu::hot]]
#else
#define VIOLET_HOT
#endif

#if VIOLET_HAS_CPP_ATTRIBUTE(clang::lifetimebound)
#define VIOLET_LIFETIMEBOUND [[clang::lifetimebound]]
#elif VIOLET_HAS_CPP_ATTRIBUTE(msvc::lifetimebound)
#define VIOLET_LIFETIMEBOUND [[msvc::lifetimebound]]
#else
#define VIOLET_LIFETIMEBOUND
#endif

#if VIOLET_HAS_CPP_ATTRIBUTE(gnu::pure)
#define VIOLET_PURE [[gnu::pure]]
#elif VIOLET_HAS_ATTRIBUTE(pure)
#define VIOLET_PURE __attribute__((pure))
#else
#define VIOLET_PURE
#endif

#if VIOLET_HAS_CPP_ATTRIBUTE(clang::noescape)
#define VIOLET_NOESCAPE [[clang::noescape]]
#else
#define VIOLET_NOESCAPE
#endif

#if VIOLET_HAS_CPP_ATTRIBUTE(clang::reinitializes)
#define VIOLET_REINITIALIZES_MEMORY [[clang::reinitializes]]
#else
#define VIOLET_REINITIALIZES_MEMORY
#endif

#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(__cpp_exceptions)
#define VIOLET_HAS_EXCEPTIONS
#endif

#define __stringify_helper(x) #x
#define __stringify(x) __stringify_helper(x)

#if VIOLET_COMPILER(CLANG)
#define VIOLET_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#define VIOLET_DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#define VIOLET_DIAGNOSTIC_IGNORE(x) _Pragma(__stringify(clang diagnostic ignored x))
#elif VIOLET_COMPILER(GCC)
#define VIOLET_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define VIOLET_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#define VIOLET_DIAGNOSTIC_IGNORE(x) _Pragma(__stringify(GCC diagnostic ignored x))
#elif VIOLET_COMPILER(MSVC)
#define VIOLET_DIAGNOSTIC_PUSH __pragma(warning(push))
#define VIOLET_DIAGNOSTIC_POP __pragma(warning(pop))
#define VIOLET_DIAGNOSTIC_IGNORE(x) __pragma(warning(disable : x))
#else
#define VIOLET_DIAGNOSTIC_PUSH
#define VIOLET_DIAGNOSTIC_POP
#define VIOLET_DIAGNOSTIC_IGNORE(x)
#endif

#define VIOLET_DISALLOW_CONSTRUCTOR(Type) VIOLET_IMPLICIT Type() noexcept = delete;
#define VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Type) constexpr VIOLET_IMPLICIT Type() noexcept = delete;
#define VIOLET_DISALLOW_COPY(Type)                                                                                     \
    VIOLET_IMPLICIT Type(const Type&) noexcept = delete;                                                               \
    auto operator=(const Type&) noexcept -> Type& = delete;

#define VIOLET_DISALLOW_MOVE(Type)                                                                                     \
    VIOLET_IMPLICIT Type(Type&&) noexcept = delete;                                                                    \
    auto operator=(Type&&) noexcept -> Type& = delete;

#define VIOLET_DISALLOW_COPY_AND_MOVE(Type)                                                                            \
    VIOLET_DISALLOW_COPY(Type)                                                                                         \
    VIOLET_DISALLOW_MOVE(Type)

#define VIOLET_DISALLOW_CONSTEXPR_COPY(Type)                                                                           \
    constexpr VIOLET_IMPLICIT Type(const Type&) noexcept = delete;                                                     \
    constexpr auto operator=(const Type&) noexcept -> Type& = delete;

#define VIOLET_DISALLOW_CONSTEXPR_MOVE(Type)                                                                           \
    constexpr VIOLET_IMPLICIT Type(Type&&) noexcept = delete;                                                          \
    constexpr auto operator=(Type&&) noexcept -> Type& = delete;

#define VIOLET_DISALLOW_CONSTEXPR_COPY_AND_MOVE(Type)                                                                  \
    VIOLET_DISALLOW_CONSTEXPR_COPY(Type)                                                                               \
    VIOLET_DISALLOW_CONSTEXPR_MOVE(Type)

#define VIOLET_IMPLICIT_COPY(Type)                                                                                     \
    VIOLET_IMPLICIT Type(const Type&) noexcept = default;                                                              \
    auto operator=(const Type&) noexcept -> Type& = default;

#define VIOLET_IMPLICIT_MOVE(Type)                                                                                     \
    VIOLET_IMPLICIT Type(Type&&) noexcept = default;                                                                   \
    auto operator=(Type&&) noexcept -> Type& = default;

#define VIOLET_IMPLICIT_COPY_AND_MOVE(Type)                                                                            \
    VIOLET_IMPLICIT_COPY(Type)                                                                                         \
    VIOLET_IMPLICIT_MOVE(Type)

#define VIOLET_IMPLICIT_CONSTEXPR_COPY(Type)                                                                           \
    constexpr VIOLET_IMPLICIT Type(const Type&) noexcept = default;                                                    \
    constexpr auto operator=(const Type&) noexcept -> Type& = default;

#define VIOLET_IMPLICIT_CONSTEXPR_MOVE(Type)                                                                           \
    constexpr VIOLET_IMPLICIT Type(Type&&) noexcept = default;                                                         \
    constexpr auto operator=(Type&&) noexcept -> Type& = default;

#define VIOLET_IMPLICIT_CONSTEXPR_COPY_AND_MOVE(Type)                                                                  \
    VIOLET_IMPLICIT_CONSTEXPR_COPY(Type)                                                                               \
    VIOLET_IMPLICIT_CONSTEXPR_MOVE(Type)

#define VIOLET_REQUIRE_STL(ver) (defined(_MSVC_LANG) && _MSVC_LANG >= ver) || __cplusplus >= ver

#ifdef VIOLET_IS_LITTLE_ENDIAN
#error "Do not pre-define `VIOLET_IS_LITTLE_ENDIAN`"
#endif

#ifdef VIOLET_IS_BIG_ENDIAN
#error "Do not pre-define `VIOLET_IS_BIG_ENDIAN`"
#endif

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define VIOLET_IS_LITTLE_ENDIAN 1
#elif defined(VIOLET_WINDOWS)
// Windows is always little-endian
#define VIOLET_IS_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define VIOLET_IS_BIG_ENDIAN 1
#else
#error "Cannot determine endianness"
#endif

#define __violet_concat_inner__(x, y) x##y
#define VIOLET_CONCAT(x, y) __violet_concat_inner__(x, y)
#define VIOLET_UNIQUE_NAME(prefix) VIOLET_CONCAT(prefix, __COUNTER__)

#if VIOLET_HAS_ATTRIBUTE(deprecated)
#define VIOLET_DEPRECATED(since) [[deprecated("since " #since)]]
#define VIOLET_DEPRECATED_BECAUSE(since, message) [[deprecated("since " #since ": " message)]]
#else
#define VIOLET_DEPRECATED(since)
#define VIOLET_DEPRECATED_BECAUSE(since, message)
#endif

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

#if __cpp_if_consteval >= 202106L
#define VIOLET_IF_CONSTEVAL consteval
#define VIOLET_IF_NOT_CONSTEVAL not consteval
#else
#define VIOLET_IF_CONSTEVAL (std::is_constant_evaluated())
#define VIOLET_IF_NOT_CONSTEVAL (!std::is_constant_evaluated())
#endif
