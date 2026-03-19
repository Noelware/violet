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
//! # 🌺💜 `violet/Language/Assert.h`

#pragma once

#include <violet/Language/Macros.h>
#include <violet/Language/Policy.h>
#include <violet/Print.h>

#include <iostream>
#include <source_location>
#include <string>

namespace violet::internals {

inline void DoAssertion(bool condition, const char* conditionString, std::string message, std::ostream& os = std::cerr,
    std::source_location loc = std::source_location::current())
{
    VIOLET_UNLIKELY_IF(!condition)
    {
        Println(os, "[{}:{}:{}]: condition '{}' failed: {}", loc.file_name(), loc.line(), loc.column(), conditionString,
            message);

        std::abort();
    }
}

} // namespace violet::internals

#define VIOLET_ASSERT(cond, message) ::violet::internals::DoAssertion(cond, #cond, message)
#define VIOLET_ASSERT0(cond) ::violet::internals::DoAssertion(cond, #cond, "assertion failed")

#define VIOLET_TODO_WITH(msg) ::violet::internals::DoAssertion(false, "todo!(" msg ")", msg)
#define VIOLET_TODO() VIOLET_TODO_WITH("prototype not implemented")

#ifndef NDEBUG
#define VIOLET_DEBUG_ASSERT(expr, message) ::violet::internals::DoAssertion(expr, #expr, message)
#define VIOLET_DEBUG_ASSERT0(expr) ::violet::internals::DoAssertion(expr, #expr, "[debug] assertion failed")
#else
#define VIOLET_DEBUG_ASSERT(expr, message)
#endif
