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
//! # 🌺💜 `violet/Experimental/Logging/Formatters/Json.h`

#pragma once

#include <nlohmann/json.hpp>
#include <violet/Experimental/Logging/Formatter.h>

namespace violet::experimental::log {

VIOLET_API auto RecordToJson(const Record& record) noexcept -> nlohmann::json;

namespace formatter {

/// A log record formatter that serializes log records as JSON.
///
/// `Json` implements the `Formatter` interface, producing JSON-encoded output
/// from a `LogRecord`. By default, output is compact (single-line); enable
/// `Pretty` for human-readable, indented output.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Logging/Formatters/Json.h>
/// #include <violet/Experimental/Logging/Sinks/Console.h>
///
/// // Compact JSON (default)
/// auto formatter = violet::experimental::log::formatter::Json();
///
/// // Pretty-printed with default 4-space indentation
/// auto formatter = violet::experimental::log::formatter::Json().Pretty();
/// ```
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Json final: public Formatter {
    struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Config final {
        /// Whether to emit pretty-printed (indented, multi-line) JSON.
        bool Pretty = false;

        /// The number of spaces per indentation level when `Pretty` is enabled.
        ///
        /// Has no effect when `Pretty` is `false`. Defaults to `4`.
        Int32 Indentation = 4;

        std::function<nlohmann::json(const Record&)> Transform = violet::experimental::log::RecordToJson;
    };

    VIOLET_IMPLICIT Json() noexcept = default;

    VIOLET_IMPLICIT Json(Config config) noexcept
        : n_config(VIOLET_MOVE(config))
    {
    }

    /// Sets the indentation width used when pretty-printing.
    ///
    /// This is a builder-style method that returns a reference to `*this`,
    /// allowing it to be chained with other configuration methods.
    template<std::convertible_to<violet::Int32> Int>
    auto WithIndentation(Int&& indent) noexcept -> Json&
    {
        this->n_config.Indentation = VIOLET_FWD(Int, indent);
        return *this;
    }

    /// Enables or disables pretty-printed output.
    auto Pretty(bool yes = true) noexcept -> Json&
    {
        this->n_config.Pretty = yes;
        return *this;
    }

    template<typename F>
        requires callable<F, nlohmann::json, const Record&>
    auto Transform(F&& fun) noexcept -> Json&
    {
        this->n_config.Transform = VIOLET_FWD(F, fun);
        return *this;
    }

    NOELDOC_SEE("violet::experimental::log::Formatter::Format(const Record&)")
    [[nodiscard]] auto Format(const Record& record) const -> String override;

private:
    Config n_config;
};

} // namespace formatter
} // namespace violet::experimental::log
