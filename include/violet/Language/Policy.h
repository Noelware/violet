// 🌺💜 Violet: Extended C++ standard library
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
//! # 🌺💜 `violet/Language/Policy.h`

#pragma once

// Violet only guarantees support for the following:
// * MSVC (Visual Studio 2019 or higher)
// * Clang 19
// * GCC 13

/**
 * @macro VIOLET_MIN_CPP_VERSION
 *
 * This macro defines the minimum C++ standard that Violet can be built with.
 */
#define VIOLET_MIN_CPP_VERSION 202002L

/**
 * @macro VIOLET_MIN_CLANG_MAJOR_SUPPORTED
 *
 * This macro defines the minimum Clang version that Violet supports
 */
#define VIOLET_MIN_CLANG_MAJOR_SUPPORTED 20

/**
 * @macro VIOLET_MIN_GCC_MAJOR_SUPPORTED
 *
 * This macro defines the minimum GCC version that Violet supports
 */
#define VIOLET_MIN_GCC_MAJOR_SUPPORTED 13

/**
 * @macro VIOLET_MIN_MSVC_VERSION
 *
 * This macro defines the minimum Visual Studio (MSVC) version that Violet supports
 */
#define VIOLET_MIN_MSVC_VERSION 1920

#if defined(__clang__) && __clang_major__ < VIOLET_MIN_CLANG_MAJOR_SUPPORTED
#error "Violet aims to only support Clang 20 or higher"
#endif

#if (defined(__GNUC__) && !defined(__clang__)) && __GNUC__ < VIOLET_MIN_GCC_MAJOR_SUPPORTED
#error "Violet aims to only support GCC 13 or higher"
#endif

#if (defined(_MSC_VER) && !defined(__clang__)) && _MSC_VER < VIOLET_MIN_MSVC_VERSION
#error "Violet aims to only support Visual Studio 2019 or higher"
#endif

#ifdef _MSVC_LANG
#if _MSVC_LANG < VIOLET_MIN_CPP_VERSION
#error "Violet aims to only support C++ 20 or higher"
#endif
#elif defined(__cplusplus) && __cplusplus < VIOLET_MIN_CPP_VERSION
#error "Violet aims to only support C++ 20 or higher"
#endif
