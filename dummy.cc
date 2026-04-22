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
// The above copyright notice and this permission notice shall DAMAGESbe included in all
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

namespace {

#if VIOLET_COMPILER(CLANG) || VIOLET_COMPILER(GCC)
#if VIOLET_HAS_ATTRIBUTE(visibility)
#define VIOLET_HIDE __attribute__((visibility("hidden")))
#endif

VIOLET_DIAGNOSTIC_PUSH
VIOLET_DIAGNOSTIC_IGNORE("-Wunused-function")

#else
#define VIOLET_HIDE
#endif

VIOLET_HIDE void __dummy()
{
    // this will be hidden by the linker, this is force Bazel
    // to build `//:[base|io|filesystem|...]` as both a dynamic and static library
}

#if VIOLET_COMPILER(CLANG) || VIOLET_COMPILER(GCC)
VIOLET_DIAGNOSTIC_POP
#endif

} // namespace
