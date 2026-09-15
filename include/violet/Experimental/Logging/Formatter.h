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
//! # 🌺💜 `violet/Experimental/Logging/Formatter.h`

#pragma once

#include <violet/Violet.h>

namespace violet::experimental::log {

struct Record;

/// A log record formatter for sinks.
///
/// Formatter defines a interface for converting [`LogRecord`]s into a string suitable. Concrete
/// implementations decide the visual style, colourization, and structure of log messages.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Logging/Sinks/Console.h>
/// #include <violet/Experimental/Logging/Formatter.h>
/// #include <violet/Experimental/Logging/Logger.h>
/// #include <violet/Experimental/Logging/LogRecord.h>
///
/// struct Formatter final: public violet::experimental::log::Formatter {
///     auto Format(const violet::experimental::log::Record& record) const -> violet::String override { /* ... */ }
/// };
///
/// violet::experimental::log::Logger log(violet::experimental::log::LogLevel::Trace, "main");
/// log = log.AddSink<violet::experimental::log::sinks::Console>(Formatter());
///
/// log.Info("hello, world!");
/// ```
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Formatter {
    virtual ~Formatter() = default;

    /// Converts a log record into a string.
    /// @param record the record to format
    [[nodiscard]] virtual auto Format(const Record& record) const -> String = 0;
};

} // namespace violet::experimental::log
