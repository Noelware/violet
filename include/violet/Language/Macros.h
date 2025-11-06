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
//! # 🌺💜 `violet/Language/Macros.h`
//! This header contains important C macros that are useful for the development
//! of the `Noelware.Violet` C++ libraries. They also provide default implementations
//! if foreign build systems (i.e, not Bazel, CMake, or Meson) forget or do not add
//! C macros for `VIOLET_{LINUX|APPLE_MACOS|WINDOWS|X86_64|AARCH64|CLANG|GCC|MSVC}`
//! while the supported build systems Noelware maintains provide them:
//!
//! * Bazel: `buildsystem/bazel/cc.bzl`
//! * CMake: ...?
//! * Meson: ...?

#pragma once

#if !defined(VIOLET_HAS_ATTRIBUTE) && defined(__has_attribute)
#    define VIOLET_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#    define VIOLET_HAS_ATTRIBUTE(x) 0
#endif

#if !defined(VIOLET_HAS_CPP_ATTRIBUTE) && defined(__has_cpp_attribute)
#    define VIOLET_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#    define VIOLET_HAS_CPP_ATTRIBUTE(x) 0
#endif

#if !defined(VIOLET_HAS_INCLUDE) && defined(__has_include)
#    define VIOLET_HAS_INCLUDE(x) __has_include(x)
#else
#    define VIOLET_HAS_INCLUDE(x) 0
#endif

#if !defined(VIOLET_HAS_FEATURE) && defined(__has_feature)
#    define VIOLET_HAS_FEATURE(x) __has_feature(x)
#else
#    define VIOLET_HAS_FEATURE(x) 0
#endif

// We need to fill in the following defines if they weren't defined before
// from foreign build systems that we don't support or they were stubbed out.
//
// * VIOLET_{LINUX,APPLE_MACOS,WINDOWS}
// * VIOLET_{X86_64,AARCH64}
// * VIOLET_{CLANG|MSVC|GCC}

#ifndef VIOLET_LINUX
#    if defined(__linux__) && !defined(__ANDROID__)
#        define VIOLET_LINUX
#    endif
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
#if !defined(VIOLET_APPLE_MACOS) && VIOLET_HAS_INCLUDE(<TargetConditionals.h>)
#    include <TargetConditionals.h>
#    if TARGET_OS_MAC && TARGET_OS_OSX
#        define VIOLET_APPLE_MACOS
#    endif
#endif

#if !defined(VIOLET_WINDOWS) && defined(_WIN32)
#    define VIOLET_WINDOWS
#endif

#if !defined(VIOLET_X86_64)                                                                                            \
    && (defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(__x86_64) || defined(_M_AMD64))
#    define VIOLET_X86_64
#endif

#if !defined(VIOLET_AARCH64) && (defined(__aarch64__) || defined(_M_ARM64))
#    define VIOLET_AARCH64
#endif

#if !defined(VIOLET_CLANG) && (defined(__clang__) && !defined(_MSC_VER))
#    define VIOLET_CLANG
#endif

#if !defined(VIOLET_GCC) && (defined(__GNUC__) && !defined(__clang__))
#    define VIOLET_GCC
#endif

#if !defined(VIOLET_MSVC) && (defined(_MSC_VER) && !defined(__clang__))
#    define VIOLET_MSVC
#endif

#if defined(VIOLET_CLANG) && VIOLET_HAS_FEATURE(memory_sanitizer)
#    define VIOLET_MSAN
#endif

#if VIOLET_HAS_FEATURE(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#    define VIOLET_ASAN
#endif

#if VIOLET_HAS_FEATURE(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#    define VIOLET_TSAN
#endif

#if VIOLET_HAS_FEATURE(undefined_behavior_sanitizer)
#    define VIOLET_UBSAN
#endif

#ifdef VIOLET_USE_RTTI
#    error "`VIOLET_USE_RTTI` cannot be directly set"
#elif VIOLET_HAS_FEATURE(cxx_rtti)
#    define VIOLET_USE_RTTI 1
#elif defined(__GNUC__) && defined(__GXX_RTTI)
#    define VIOLET_USE_RTTI 1
#elif defined(_MSC_VER) && defined(_CPPRTTI)
#    define VIOLET_USE_RTTI 1
#else
#    define VIOLET_USE_RTTI 0
#endif

#define VIOLET_FWD(TYPE, VALUE) ::std::forward<TYPE>(VALUE)
#define VIOLET_MOVE(VALUE) ::std::move(VALUE)

#define VIOLET_TO_STRING(TYPE, NAME, BLOCK)                                                                            \
    namespace violet {                                                                                                 \
    inline auto ToString(TYPE NAME) -> ::violet::String BLOCK                                                          \
    }

#define VIOLET_IMPLICIT
#define VIOLET_EXPLICIT explicit

#if VIOLET_WINDOWS
#    ifdef VIOLET_DLL_EXPORT
#        define VIOLET_API __declspec(dllexport)
#    else
#        define VIOLET_API __declspec(dllimport)
#    endif
#else
#    if VIOLET_HAS_ATTRIBUTE(visibility)
#        define VIOLET_API __attribute__((visibility("default")))
#    else
#        define VIOLET_API
#    endif
#endif

#ifndef NDEBUG
#    define VIOLET_DEBUG_ASSERT(expr) assert(expr)
#else
#    define VIOLET_DEBUG_ASSERT(expr)
#endif

#if VIOLET_HAS_CPP_ATTRIBUTE(likely)
#    define VIOLET_LIKELY [[likely]]
#else
#    define VIOLET_LIKELY
#endif

#if VIOLET_HAS_CPP_ATTRIBUTE(unlikely)
#    define VIOLET_UNLIKELY(expr) [[unlikely]]
#else
#    define VIOLET_UNLIKELY(expr)
#endif

// clang-format off
#define VIOLET_LIKELY_IF(expr) if (expr) VIOLET_LIKELY
#define VIOLET_UNLIKELY_IF(expr) if (expr) VIOLET_UNLIKELY
// clang-format on

#if VIOLET_HAS_CPP_ATTRIBUTE(gnu::cold)
#    define VIOLET_COLD [[gnu::cold]]
#elif VIOLET_WINDOWS
#    define VIOLET_COLD __declspec(noinline)
#else
#    define VIOLET_COLD
#endif

#if defined(_CPPUNWIND) || defined(__EXCEPTIONS) || defined(__cpp_exceptions)
#    define VIOLET_HAS_EXCEPTIONS
#endif

#define __stringify_helper(x) #x
#define __stringify(x) __stringify_helper(x)

#if defined(VIOLET_CLANG)
#    define VIOLET_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#    define VIOLET_DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#    define VIOLET_DIAGNOSTIC_IGNORE(x) _Pragma(__stringify(clang diagnostic ignored x))
#elif defined(VIOLET_GCC)
#    define VIOLET_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#    define VIOLET_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#    define VIOLET_DIAGNOSTIC_IGNORE(x) _Pragma(__stringify(GCC diagnostic ignored x))
#elif defined(VIOLET_MSVC)
#    define VIOLET_DIAGNOSTIC_PUSH __pragma(warning(push))
#    define VIOLET_DIAGNOSTIC_POP __pragma(warning(pop))
#    define VIOLET_DIAGNOSTIC_IGNORE(x) __pragma(warning(disable : x))
#else
#    define VIOLET_DIAGNOSTIC_PUSH
#    define VIOLET_DIAGNOSTIC_POP
#    define VIOLET_DIAGNOSTIC_IGNORE(x)
#endif
