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
//! # 🌺💜 `violet/Print.h`
//! Provides [`violet::Print`], [`violet::Println`], [`violet::PrintErr`], and
//! [`violet::PrintErrln`] — thin wrappers around [`std::format`] for writing
//! formatted text to an output stream.
//!
//! On C++23 or later (`__cpp_lib_print` / `VIOLET_REQUIRE_STL(202302L)`), the
//! stream overloads delegate to [`std::print`] / [`std::println`], which write
//! directly without constructing an intermediate [`std::string`]. On earlier
//! standards, a [`std::format`] string is written via `operator<<`.

#pragma once

#include <violet/Language/Macros.h>
#include <violet/Language/Policy.h>

#include <format>
#include <iostream>

#if VIOLET_REQUIRE_STL(202302L)
#include <print>
#endif

namespace violet {

/// Writes a formatted string to `stdout`.
///
/// Delegates to the [`violet::Print(std::ostream&, ...)`] overload with
/// `std::cout` as the target stream. Format syntax follows [`std::format`].
///
/// ## Example
/// ```cpp
/// #include <violet/Print.h>
///
/// violet::Print("hello, {}!\n", "world"); // hello, world!
/// ```
///
/// @param fmt  A compile-time-checked format string.
/// @param args Arguments referenced by the format string.
template<typename... Args>
VIOLET_API void Print(std::format_string<Args...> fmt, Args&&... args)
{
    return Print(std::cout, fmt, VIOLET_FWD(Args, args)...);
}

/// Writes a formatted string to `stream`.
///
/// On C++23 and later, delegates to [`std::print(stream, fmt, args...)`].
/// On earlier standards, falls back to `stream << std::format(fmt, args...)`.
///
/// ## Example
/// ```cpp
/// #include <violet/Print.h>
/// #include <fstream>
///
/// std::ofstream f("out.txt");
/// violet::Print(f, "value = {}", 42);
/// ```
///
/// @param stream The output stream to write to.
/// @param fmt    A compile-time-checked format string.
/// @param args   Arguments referenced by the format string.
template<typename... Args>
VIOLET_API void Print(std::ostream& stream, std::format_string<Args...> fmt, Args&&... args)
{
#if VIOLET_REQUIRE_STL(202302L)
    std::print(stream, fmt, VIOLET_FWD(Args, args)...);
#else
    stream << std::format(fmt, VIOLET_FWD(Args, args)...);
#endif
}

/// Writes a formatted string followed by a newline (`'\n'`) to `stdout`.
///
/// Delegates to the [`violet::Println(std::ostream&, ...)`] overload with
/// `std::cout` as the target stream.
///
/// ## Example
/// ```cpp
/// #include <violet/Print.h>
///
/// violet::Println("count: {}", 3); // count: 3\n
/// ```
///
/// @param fmt  A compile-time-checked format string.
/// @param args Arguments referenced by the format string.
template<typename... Args>
VIOLET_API void Println(std::format_string<Args...> fmt, Args&&... args)
{
    return Println(std::cout, fmt, VIOLET_FWD(Args, args)...);
}

/// Writes a formatted string followed by a newline (`'\n'`) to `stream`.
///
/// On C++23 and later, delegates to [`std::println(stream, fmt, args...)`].
/// On earlier standards, falls back to `stream << std::format(fmt, args...) << '\n'`.
///
/// ## Example
/// ```cpp
/// #include <violet/Print.h>
///
/// violet::Println(std::cout, "x = {}", 7); // x = 7\n
/// ```
///
/// @param stream The output stream to write to.
/// @param fmt    A compile-time-checked format string.
/// @param args   Arguments referenced by the format string.
template<typename... Args>
VIOLET_API void Println(std::ostream& stream, std::format_string<Args...> fmt, Args&&... args)
{
#if VIOLET_REQUIRE_STL(202302L)
    std::println(stream, fmt, VIOLET_FWD(Args, args)...);
#else
    stream << std::format(fmt, VIOLET_FWD(Args, args)...) << '\n';
#endif
}

/// Writes a formatted string to `stderr` (`std::cerr`).
///
/// Equivalent to calling `violet::Print(std::cerr, fmt, args...)`.
///
/// ## Example
/// ```cpp
/// #include <violet/Print.h>
///
/// violet::PrintErr("error: {}", "something went wrong");
/// ```
///
/// @param fmt  A compile-time-checked format string.
/// @param args Arguments referenced by the format string.
template<typename... Args>
VIOLET_API void PrintErr(std::format_string<Args...> fmt, Args&&... args)
{
    return Print(std::cerr, fmt, VIOLET_FWD(Args, args)...);
}

/// Writes a formatted string followed by a newline (`'\n'`) to `stderr` (`std::cerr`).
///
/// Equivalent to calling `violet::Println(std::cerr, fmt, args...)`.
///
/// ## Example
/// ```cpp
/// #include <violet/Print.h>
///
/// violet::PrintErrln("fatal: {} (code {})", "segfault", 11);
/// ```
///
/// @param fmt  A compile-time-checked format string.
/// @param args Arguments referenced by the format string.
template<typename... Args>
VIOLET_API void PrintErrln(std::format_string<Args...> fmt, Args&&... args)
{
    return Println(std::cerr, fmt, VIOLET_FWD(Args, args)...);
}

} // namespace violet
