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

#include <string_view>

namespace violet {

/// Represents a location within a source file, capturing the file path, line number,
/// column number, and enclosing function name at the point of construction.
///
/// `SourceLocation` is designed to be used as a **default parameter** so that call sites
/// are captured automatically without any boilerplate:
///
/// ```cpp
/// void log(violet::Str msg, violet::SourceLocation loc = {}) {
///     violet::Println("[{}:{}] {}", loc.File, loc.Line, msg);
/// }
///
/// log("hello"); // loc is filled in with the caller's file, line, column, and function
/// ```
///
/// On GCC and Clang the `Function` field is populated with the pretty-printed signature
/// (via `__PRETTY_FUNCTION__`), which includes template parameters and namespaces.
struct SourceLocation final {
    /// Path to the source file, as reported by the compiler (typically `__builtin_FILE()`).
    std::string_view File;

    /// 1-based line number within [`File`] where this location was captured.
    uint32_t Line;

    /// 1-based column number within the line where this location was captured.
    uint32_t Column;

    /// Name of the enclosing function where this location was captured.
    ///
    /// On GCC and Clang this is the full pretty-printed signature from `__PRETTY_FUNCTION__`.
    std::string_view Function;

    /// Constructs a `SourceLocation` from the given source coordinates.
    ///
    /// All parameters have compiler-builtin defaults, so an empty brace-initialization
    /// `{}` (or a defaulted function parameter) captures the **caller's** location:
    ///
    /// ```cpp
    /// constexpr auto loc = violet::SourceLocation{};
    /// // loc.File     -> current file
    /// // loc.Line     -> current line
    /// // loc.Column   -> current column
    /// // loc.Function -> enclosing function signature
    /// ```
    ///
    /// # Parameters
    /// @param file source file path; defaults to `__builtin_FILE()` at the call site.
    /// @param line line number; defaults to `__builtin_LINE()` at the call site.
    /// @param column column number; defaults to `__builtin_COLUMN()` at the call site.
    // clang-format off
    constexpr SourceLocation(
        std::string_view file = __builtin_FILE(),
        uint32_t line = __builtin_LINE(),
        uint32_t column = __builtin_COLUMN(),

#if defined(VIOLET_GCC) || defined(VIOLET_CLANG)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wpredefined-identifier-outside-function")
        std::string_view func = __PRETTY_FUNCTION__
        VIOLET_DIAGNOSTIC_POP
#endif
        ) noexcept
        : File(file)
        , Line(line)
        , Column(column)

#if defined(VIOLET_GCC) || defined(VIOLET_CLANG)
        , Function(func)
#endif
    {
    }
    // clang-format on
};

} // namespace violet
