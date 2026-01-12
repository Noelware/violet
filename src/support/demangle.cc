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

#include <violet/Language/Macros.h>
#include <violet/Support/Demangle.h>

#if VIOLET_HAS_INCLUDE(<cxxabi.h>)
#include <cxxabi.h>
#include <memory>

#define VIOLET_HAS_CXXABI_HDR
#endif

#ifdef VIOLET_APPLE_MACOS
// on macOS, `std::free` doesn't exist so we use `::free` instead
#define __free ::free
#else
#define __free ::std::free
#endif

auto violet::util::DemangleCXXName(const char* name) -> std::string
{
#ifndef VIOLET_HAS_CXXABI_HDR
    return name;
#else
    int status = -1;
    std::unique_ptr<char, void (*)(void*)> result(abi::__cxa_demangle(name, nullptr, nullptr, &status), __free);

    return status == 0 ? result.get() : name;
#endif
}
