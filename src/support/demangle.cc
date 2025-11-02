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

#include <violet/Support/Demangle.h>
#include <violet/Violet.h>

#if VIOLET_HAS_INCLUDE(<cxxabi.h>)
#    include <cxxabi.h>
#    define VIOLET_HAS_CXXABI_HDR 1
#else
#    define VIOLET_HAS_CXXABI_HDR 0
#endif

using violet::CStr;
using violet::Int32;
using violet::UniquePtr;

auto violet::util::DemangleCXXName(CStr name) -> String
{
#ifndef VIOLET_HAS_CXXABI_HDR
    return name;
#else
    Int32 status = -1;
    UniquePtr<char, void (*)(void*)> result(abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free);

    return status == 0 ? result.get() : name;
#endif
}
