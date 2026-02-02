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

#pragma once

#include <violet/IO/Descriptor.h>

#include <format>

#ifdef VIOLET_WINDOWS
#include <windows.h>
#endif

namespace violet {

#ifdef VIOLET_UNIX
#define STDOUT_HANDLE 1
#define STDERR_HANDLE 2
#elif defined(VIOLET_WINDOWS)
#define STDOUT_HANDLE GetStdHandle(STD_OUTPUT_HANDLE)
#define STDERR_HANDLE GetStdHandle(STD_ERROR_HANDLE)
#endif

/// Prints formatted output to the given `descriptor` using C++20's `format` module.
///
/// ## Notes
/// - On I/O errors when writing to the descriptor will be silently not available by default,
///   if `-DVIOLET_PRINT_ERROR_ON_IO_FAILURE` is provided to the compiler when compiling, then
///   I/O errors will be printed to the process' standard error (if it can).
///
/// @param descriptor the `io::FileDescriptor` to write into
/// @param fmt format string
/// @param args additional arguments
template<typename... Args>
inline void print(const io::FileDescriptor& descriptor, std::format_string<Args...> fmt, Args&&... args)
{
    if (!descriptor.Valid()) {
        return;
    }

    auto buf = std::format(fmt, VIOLET_FWD(Args, args)...);

#ifndef VIOLET_PRINT_ERROR_ON_IO_FAILURE
    (void)descriptor.Write(buf);
#else
    if (auto res = descriptor.Write(buf); res.Err()) {
        std::cerr << "[violet/print@" << __FILE__ << ':' << __LINE__
                  << "]: failed to write to standard error: " << violet::ToString(VIOLET_MOVE(res.Error()));
    }
#endif
}

/// Prints formatted output to the the process' standard input.
///
/// ## Notes
/// - On I/O errors when writing to the descriptor will be silently not available by default,
///   if `-DVIOLET_PRINT_ERROR_ON_IO_FAILURE` is provided to the compiler when compiling, then
///   I/O errors will be printed to the process' standard error (if it can).
///
/// @param fmt format string
/// @param args additional arguments
template<typename... Args>
inline void print(std::format_string<Args...> fmt, Args&&... args)
{
    violet::print(STDOUT_HANDLE, fmt, VIOLET_FWD(Args, args)...);
}

/// Prints formatted output with a newline to the given `descriptor` using C++20's `format` module.
///
/// ## Notes
/// - On I/O errors when writing to the descriptor will be silently not available by default,
///   if `-DVIOLET_PRINT_ERROR_ON_IO_FAILURE` is provided to the compiler when compiling, then
///   I/O errors will be printed to the process' standard error (if it can).
///
/// @param descriptor the `io::FileDescriptor` to write into
/// @param fmt format string
/// @param args additional arguments
template<typename... Args>
inline void println(const io::FileDescriptor& descriptor, std::format_string<Args...> fmt, Args&&... args)
{
    if (!descriptor.Valid()) {
        return;
    }

    String buf;
    buf.reserve(128);

    std::format_to(std::back_inserter(buf), fmt, VIOLET_FWD(Args, args)...);
#ifdef VIOLET_WINDOWS
    buf.push_back('\r');
#endif

#ifndef VIOLET_PRINT_ERROR_ON_IO_FAILURE
    (void)descriptor.Write({ reinterpret_cast<const UInt8*>(buf.data()), buf.size() });
#else
    if (auto res = descriptor.Write({ reinterpret_cast<const UInt8*>(buf.data()), buf.size() }); res.Err()) {
        std::cerr << "[violet/print@" << __FILE__ << ':' << __LINE__
                  << "]: failed to write to standard error: " << violet::ToString(VIOLET_MOVE(res.Error()));
    }
#endif
}

/// Prints formatted output with a newline to the process' standard input
///
/// ## Notes
/// - On I/O errors when writing to the descriptor will be silently not available by default,
///   if `-DVIOLET_PRINT_ERROR_ON_IO_FAILURE` is provided to the compiler when compiling, then
///   I/O errors will be printed to the process' standard error (if it can).
///
/// @param descriptor the `io::FileDescriptor` to write into
/// @param fmt format string
/// @param args additional arguments
template<typename... Args>
inline void println(std::format_string<Args...> fmt, Args&&... args)
{
    violet::println(STDOUT_HANDLE, fmt, VIOLET_FWD(Args, args)...);
}

/// Prints formatted output to the process' standard error
///
/// ## Notes
/// - On I/O errors when writing to the descriptor will be silently not available by default,
///   if `-DVIOLET_PRINT_ERROR_ON_IO_FAILURE` is provided to the compiler when compiling, then
///   I/O errors will be printed to the process' standard error (if it can).
///
/// @param descriptor the `io::FileDescriptor` to write into
/// @param fmt format string
/// @param args additional arguments
template<typename... Args>
inline void eprint(std::format_string<Args...> fmt, Args&&... args)
{
    violet::print(STDERR_HANDLE, fmt, VIOLET_FWD(Args, args)...);
}

/// Prints formatted output with a newline to the process' standard error
///
/// ## Notes
/// - On I/O errors when writing to the descriptor will be silently not available by default,
///   if `-DVIOLET_PRINT_ERROR_ON_IO_FAILURE` is provided to the compiler when compiling, then
///   I/O errors will be printed to the process' standard error (if it can).
///
/// @param descriptor the `io::FileDescriptor` to write into
/// @param fmt format string
/// @param args additional arguments
template<typename... Args>
inline void eprintln(std::format_string<Args...> fmt, Args&&... args)
{
    violet::println(STDERR_HANDLE, fmt, VIOLET_FWD(Args, args)...);
}

} // namespace violet
