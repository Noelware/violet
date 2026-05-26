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

NOELDOC_HIDE
inline void FailAssertion(const char* conditionString, std::string message, std::ostream& os = std::cerr,
    std::source_location loc = std::source_location::current())
{
    Println(os, "[{}:{}:{}]: condition '{}' failed: {}", loc.file_name(), loc.line(), loc.column(), conditionString,
        message);

    std::abort();
}

} // namespace violet::internals

/**
 * @macro VIOLET_ASSERT
 * @since 26.04.04
 * @param condition The expression to evaluate.
 * @param message A string literal or expression describing the failure.
 *
 * Asserts that @p condition is true. If the assertion fails, prints the
 * stringified condition, the provided message, and the source location to
 * `stderr`, then aborts. Active in all build configurations.
 *
 * @see VIOLET_ASSERT0
 * @see VIOLET_DEBUG_ASSERT
 */
#define VIOLET_ASSERT(condition, message)                                                                              \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            ::violet::internals::FailAssertion(#condition, message, ::std::cerr, ::std::source_location::current());   \
        }                                                                                                              \
    } while (false);

/**
 * @macro VIOLET_ASSERT0
 * @since 26.04.04
 * @param condition The expression to evaluate.
 *
 * Asserts that @p condition is true with a default failure message
 * (`"assertion failed"`). Active in all build configurations.
 *
 * @see VIOLET_ASSERT
 * @see VIOLET_DEBUG_ASSERT0
 */
#define VIOLET_ASSERT0(condition)                                                                                      \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            ::violet::internals::FailAssertion(                                                                        \
                #condition, "assertion failed", ::std::cerr, ::std::source_location::current());                       \
        }                                                                                                              \
    } while (false);

/**
 * @macro VIOLET_ASSERT_FMT
 * @since 26.04.04
 * @param condition The expression to evaluate.
 * @param fmt A `std::format`-compatible format string.
 * @param ... Arguments to the format string.
 *
 * Asserts that @p condition is true. On failure, formats the message via
 * `std::format` before passing it to the assertion handler. Active in all
 * build configurations.
 *
 * # Example
 *
 * ```cpp
 * VIOLET_ASSERT_FMT(index < size, "index {} out of bounds (size={})", index, size);
 * ```
 *
 * @see VIOLET_ASSERT
 * @see VIOLET_DEBUG_ASSERT_FMT
 */
#define VIOLET_ASSERT_FMT(condition, fmt, ...)                                                                         \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            ::violet::internals::FailAssertion(#condition, ::std::format(fmt __VA_OPT__(, ) __VA_ARGS__), ::std::cerr, \
                ::std::source_location::current());                                                                    \
        }                                                                                                              \
    } while (false);

/**
 * @macro VIOLET_TODO_WITH
 * @since 26.04.04
 * @param msg A string literal describing what is unimplemented.
 *
 * Unconditionally aborts with a `todo!()` message, analogous to Rust's
 * `todo!()` macro. Always active regardless of build configuration.
 *
 * @see VIOLET_TODO
 */
#define VIOLET_TODO_WITH(msg)                                                                                          \
    ::violet::internals::FailAssertion("todo!(" msg ")", msg, ::std::cerr, ::std::source_location::current())

/**
 * @macro VIOLET_TODO
 * @since 26.04.04
 *
 * Unconditionally aborts with the default message
 * `"prototype not implemented"`. Shorthand for
 * `VIOLET_TODO_WITH("prototype not implemented")`.
 *
 * @see VIOLET_TODO_WITH
 */
#define VIOLET_TODO() VIOLET_TODO_WITH("prototype not implemented")

/**
 * @macro VIOLET_DEBUG_ASSERT
 * @since 26.04.04
 * @param condition The expression to evaluate.
 * @param message A string literal or expression describing the failure.
 *
 * Debug-only variant of `VIOLET_ASSERT`. Evaluates the assertion in debug
 * builds (`NDEBUG` not defined) and expands to `((void)0)` in release builds.
 *
 * @see VIOLET_ASSERT
 */

/**
 * @macro VIOLET_DEBUG_ASSERT0
 * @since 26.04.04
 * @param condition The expression to evaluate.
 *
 * Debug-only variant of `VIOLET_ASSERT0`. Expands to `((void)0)` in release
 * builds.
 *
 * @see VIOLET_ASSERT0
 */

/**
 * @macro VIOLET_DEBUG_ASSERT_FMT
 * @since 26.04.04
 * @param condition The expression to evaluate.
 * @param fmt A `std::format`-compatible format string.
 * @param ... Arguments to the format string.
 *
 * Debug-only variant of `VIOLET_ASSERT_FMT`. Expands to `((void)0)` in
 * release builds.
 *
 * @see VIOLET_ASSERT_FMT
 */
#ifndef NDEBUG
#define VIOLET_DEBUG_ASSERT(condition, message)                                                                        \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            ::violet::internals::FailAssertion(#condition, message, ::std::cerr, ::std::source_location::current());   \
        }                                                                                                              \
    } while (false);

#define VIOLET_DEBUG_ASSERT0(condition)                                                                                \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            ::violet::internals::FailAssertion(                                                                        \
                #condition, "assertion failed", ::std::cerr, ::std::source_location::current());                       \
        }                                                                                                              \
    } while (false);

#define VIOLET_DEBUG_ASSERT_FMT(condition, fmt, ...)                                                                   \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            ::violet::internals::FailAssertion(#condition, ::std::format(fmt __VA_OPT__(, ) __VA_ARGS__), ::std::cerr, \
                ::std::source_location::current());                                                                    \
        }                                                                                                              \
    } while (false);
#else
#define VIOLET_DEBUG_ASSERT(condition, message) ((void)0)
#define VIOLET_DEBUG_ASSERT0(condition) ((void)0)
#define VIOLET_DEBUG_ASSERT_FMT(condition, fmt, ...) ((void)0)
#endif
