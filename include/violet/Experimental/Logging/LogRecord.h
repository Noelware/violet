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
//! # 🌺💜 `violet/Experimental/Logging/LogRecord.h`

#pragma once

#include <violet/Experimental/Collections/HashMap.h>
#include <violet/Experimental/OneOf.h>
#include <violet/Experimental/Time/Clock.h>
#include <violet/Experimental/Time/TimePoint.h>
#include <violet/SourceLocation.h>

namespace violet::experimental::log {

/// The severity level of a log record.
enum struct NOELDOC_EXPERIMENTAL_SINCE("current") LogLevel : violet::UInt8 {
    Trace, ///< fine-grained diagnostic information
    Debug, ///< debug-level messages that are useful during development
    Info, ///< informational message describing normal operations.
    Warning, ///< recoverable issues or unexpected situations
    Error, ///< errors that are prevented an operation from completing
    Fatal, ///< critical errors that typically precede process termination.
    Off ///< no logs should be emitted at all.
};

/// A dynamically, typed attribute value.
///
/// AttributeValue represents a small, closed set of value types that can be attached
/// to log records as structured metadata. It is designed to be type-safe, allocation-aware,
/// and RTTI-free.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") AttributeValue final
    : public OneOf<Mono, bool, Int64, UInt64, double, String> {
    using OneOf::OneOf;

    template<std::convertible_to<Str> T>
    VIOLET_IMPLICIT AttributeValue(T&& value) noexcept
        : OneOf(OneOf::New<String>(VIOLET_FWD(T, value)))
    {
    }

    template<Stringify T>
    VIOLET_IMPLICIT AttributeValue(T value) noexcept
        : OneOf(OneOf::New<String>(value.ToString()))
    {
    }

    template<std::convertible_to<Int64> N>
        requires std::signed_integral<N>
    VIOLET_IMPLICIT AttributeValue(N&& value) noexcept
        : OneOf(OneOf::New<Int64>(VIOLET_FWD(N, value)))
    {
    }

    template<std::convertible_to<UInt64> N>
        requires std::unsigned_integral<N> && (!std::same_as<std::remove_cvref_t<N>, bool>)
    VIOLET_IMPLICIT AttributeValue(N&& value) noexcept
        : OneOf(OneOf::New<UInt64>(VIOLET_FWD(N, value)))
    {
    }
};

/// A structured log event.
///
/// This struct represents a single logging event produced by a `Logger` and
/// delivered to one or more sinks. It contains both unstructured text and
/// structured key-value fields for rich diagnostics.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Record final {
    chrono::TimePoint Timestamp; ///< time point at which this log record was created
    LogLevel Level; ///< severity of the log event.
    String Message; ///< primary log message
    String Logger; ///< name of the logger that emitted this record
    SourceLocation Location; ///< source location where this log was emitted
    HashMap<String, AttributeValue> Fields; ///< structured key-value pairs attached to the record

    /// Creates a new record with a given [`Clock`] for the timestamp.
    /// @param level   severity of this record
    /// @param message primary log message
    /// @param loc     source location where this log record was emitted
    template<typename C>
        requires std::derived_from<std::remove_cvref_t<C>, Clock>
    static auto Now(
        LogLevel level, Str message, C& clock, SourceLocation loc = std::source_location::current()) noexcept -> Record
    {
        return Record{.Timestamp = clock.WallNow(),
            .Level = level,
            .Message = String(message),
            .Logger = "",
            .Location = loc,
            .Fields = {}};
    }

    /// Creates a new record with the timestamp being the time this call was executed.
    ///
    /// @param level   severity of this record
    /// @param message primary log message
    /// @param loc     source location where this log record was emitted
    static auto Now(LogLevel level, Str message, SourceLocation loc = std::source_location::current()) noexcept
        -> Record
    {
        return Record::Now(level, message, Clock::System(), loc);
    }

    template<std::equality_comparable_with<String> Q>
        requires(std::is_constructible_v<String, Q>)
    auto With(Q key, AttributeValue&& value) -> Record&
    {
        (void)this->Fields.Insert(key, AttributeValue(VIOLET_MOVE(value)));
        return *this;
    }
};

} // namespace violet::experimental::log

VIOLET_TO_STRING(violet::experimental::log::LogLevel, level, {
    switch (level) {
    case violet::experimental::log::LogLevel::Trace:
        return "trace";

    case violet::experimental::log::LogLevel::Debug:
        return "debug";

    case violet::experimental::log::LogLevel::Info:
        return "info";

    case violet::experimental::log::LogLevel::Warning:
        return "warning";

    case violet::experimental::log::LogLevel::Error:
        return "error";

    case violet::experimental::log::LogLevel::Fatal:
        return "fatal";

    case violet::experimental::log::LogLevel::Off:
        return "off";
    }
});
