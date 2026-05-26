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

#pragma once

#include <violet/Language/Macros.h>
#include <violet/Language/Policy.h>

#include <string>

namespace violet::util {

/// Utility function to demangle a C++ type by its name, usually from `typeid(T).name()`.
///
/// ## Remarks
/// - This will use `abi::__cxa_demangle` on compilers that support the `cxxabi.h` header (Clang, GCC)
///
/// @param name The mangled C++ type to demangle
/// @return the demangled name, or the name itself if unsupported
VIOLET_API NOELDOC_SINCE("26.02") auto DemangleCXXName(const char* name) -> std::string;

/// Returns the demangled type name of the current in-flight C++ exception.
///
/// Must be called within a `catch` block. On Itanium ABI platforms (GCC, Clang),
/// this will use [`abi::__cxa_current_exception_type()`] to retrieve the type
/// information of the thrown object directly.
///
/// [`abi::__cxa_current_exception_type()`]: https://libcxxabi.llvm.org/spec.html
///
/// ## Remarks
/// On Itanium ABI platforms, this works for any thrown type, including integrals, C strings, or user-defined
/// types that weren't derived from [`std::exception`].
///
/// On unsupported platforms or if it couldn't be demangled, this will return `"unknown"`.
VIOLET_API NOELDOC_SINCE("26.07") auto DemangleCXXException() -> std::string;

// VIOLET_API auto DemangleRust(const char* name) -> std::string;

} // namespace violet::util
