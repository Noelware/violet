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

#include <violet/Support/Demangle.h>

#ifndef VIOLET_FEATURE_ITANIUM_CXXABI
#if VIOLET_HAS_INCLUDE(<cxxabi.h>)
#include <cxxabi.h>

#define VIOLET_FEATURE_ITANIUM_CXXABI 1
#else
#define VIOLET_FEATURE_ITANIUM_CXXABI 0
#endif
#elif VIOLET_FEATURE(ITANIUM_CXXABI)
#include <cxxabi.h>
#endif

#if VIOLET_FEATURE(ITANIUM_CXXABI)
#include <cstdlib>
#include <typeinfo>
#endif

auto violet::util::DemangleCXXName(const char* name) -> std::string
{
#if VIOLET_FEATURE(ITANIUM_CXXABI)
    int status = -1;
    char* result = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    if (status == 0 && result != nullptr) {
        std::string final(result);
        ::free(result);

        return final;
    }
#endif

    return name;
}

auto violet::util::DemangleCXXException() -> std::string
{
#if VIOLET_FEATURE(ITANIUM_CXXABI)
    if (auto* ti = abi::__cxa_current_exception_type()) {
        return DemangleCXXName(ti->name());
    }
#endif

    return "{unknown C++ type}";
}
