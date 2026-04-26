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

#include <cstdint>
#include <format>
#include <ostream>
#include <source_location>
#include <string_view>

namespace violet {

#if VIOLET_COMPILER(GCC) || VIOLET_COMPILER(CLANG)
#define __violet_pretty_function__ __PRETTY_FUNCTION__
#else
#define __violet_pretty_function__ ""
#endif

#if (VIOLET_COMPILER(CLANG) || VIOLET_COMPILER(CLANG_CL) || VIOLET_COMPILER(MSVC)) && VIOLET_HAS_BUILTIN(COLUMN)
#define __violet_column__ __builtin_COLUMN()
#else
#define __violet_column__ 0
#endif

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
struct VIOLET_API SourceLocation final {
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

    /// Constructs a [`violet::SourceLocation`] from a [`std::source_location`] implicitly
    constexpr SourceLocation(std::source_location loc) noexcept
        : SourceLocation(SourceLocation::FromStd(loc))
    {
    }

    constexpr auto operator=(std::source_location& loc) noexcept -> SourceLocation&
    {
        *this = SourceLocation::FromStd(loc);
        return *this;
    }

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
    /// @param column column number; defaults to `__builtin_COLUMN()` at the call site or `0` if not supported
    /// @param func the function name; defaults to `__PRETTY_FUNCTION__` on GCC/Clang or `""` if not supported
#if VIOLET_COMPILER(GCC) || VIOLET_COMPILER(CLANG)
    VIOLET_DIAGNOSTIC_PUSH
    VIOLET_DIAGNOSTIC_IGNORE("-Wpredefined-identifier-outside-function")
#endif
    constexpr SourceLocation(std::string_view file = __builtin_FILE(), uint32_t line = __builtin_LINE(),
        uint32_t column = __violet_column__, std::string_view func = __violet_pretty_function__) noexcept
        : File(file)
        , Line(line)
        , Column(column)
        , Function(func)
    {
    }

#if VIOLET_COMPILER(GCC) || VIOLET_COMPILER(CLANG)
    VIOLET_DIAGNOSTIC_POP
#endif

    /// Converts a [`std::source_location`] object into [`violet::SourceLocation`].
    /// @param loc the source location that was captured
    constexpr static auto FromStd(std::source_location loc) noexcept -> SourceLocation
    {
        return { loc.file_name(), loc.line(), loc.column(), loc.function_name() };
    }

    [[nodiscard]] constexpr auto ToString() const noexcept -> std::string
    {
        return std::format("SourceLocation(.file={}, .line={}, .column={}, .function=\"{}\")", this->File, this->Line,
            this->Column, this->Function);
    }

    friend auto operator<<(std::ostream& os, const SourceLocation& self) noexcept -> std::ostream&
    {
        if (!self.File.empty()) {
            os << self.File;
            if (self.Line > 0) {
                os << ':' << self.Line;
            }

            if (self.Column > 0) {
                os << ':' << self.Column;
            }

            return os;
        }

        return os;
    }
};

} // namespace violet
