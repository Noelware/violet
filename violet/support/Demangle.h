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
//! # 🌺💜 `violet/support/Demangle.h`
//! A utility C++ header to demangle a C++ type name.

#pragma once

#include "violet/violet.h"

// For Clang and GCC, we can check if we have the `cxxabi.h` header.
#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
#    ifdef __has_include
#        if __has_include(<cxxabi.h>)
#            define VIOLET_COMPILER_SUPPORTS_CXXABI_HDR
#        endif
#    endif
#endif

#ifdef VIOLET_COMPILER_SUPPORTS_CXXABI_HDR
#    include <cxxabi.h>
#endif

namespace Noelware::Violet::Utility {

/// Utility function to demangle a C++ type by its name, usually from `typeid(T).name()`.
///
/// ## Remarks
/// - This will use `abi::__cxa_demangle` on compilers that support the `cxxabi.h` header (Clang, GCC)
///
/// @param name The mangled C++ type to demangle
/// @return the demangled name, or the name itself if unsupported
inline auto DemangleCXXName(CStr name) -> String
{
#ifdef VIOLET_COMPILER_SUPPORTS_CXXABI_HDR
    int32 status = -1;
    UniquePtr<char, void (*)(void*)> res(abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free);

    return status == 0 ? res.get() : name;
#else
    return name;
#endif
}

} // namespace Noelware::Violet::Utility
