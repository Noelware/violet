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

#include <violet/Language/Macros.h>

#if defined(VIOLET_FEATURE_EXPERIMENTAL_LOGGING_OTEL_SINK) && VIOLET_FEATURE(EXPERIMENTAL_LOGGING_OTEL_SINK)

#include <violet/Experimental/Logging/Sinks/OpenTelemetry.h>

#include <opentelemetry/logs/provider.h>

namespace violet::experimental::log::sinks {
namespace logs = opentelemetry::logs;
namespace otel = opentelemetry::common;

namespace {

auto ConvertAttributes(const HashMap<String, AttributeValue>& attributes)
    -> Vec<Pair<const String, otel::AttributeValue>>
{
    Vec<Pair<const String, otel::AttributeValue>> converted;
    converted.reserve(attributes.Size());

    for (const auto& [key, value]: attributes) {
        if (value.Holds<Mono>()) {
            continue;
        }

        converted.emplace_back(key,
            value.Match(
                // clang-format off
                [](const bool& value)   -> otel::AttributeValue { return value; },
                [](const Int64& value)  -> otel::AttributeValue { return value; },
                [](const UInt64& value) -> otel::AttributeValue { return value; },
                [](const double& value) -> otel::AttributeValue { return value; },
                [](const String& value) -> otel::AttributeValue { return value; },
                [](Mono)                -> otel::AttributeValue { VIOLET_UNREACHABLE(); }
                // clang-format on
                ));
    }

    return converted;
}

auto logLevelToOtelSeverity(LogLevel level) -> logs::Severity
{
    switch (level) {
    case LogLevel::Trace:
        return logs::Severity::kTrace;

    case LogLevel::Debug:
        return logs::Severity::kDebug;

    case LogLevel::Info:
        return logs::Severity::kInfo;

    case LogLevel::Warning:
        return logs::Severity::kWarn;

    case LogLevel::Error:
        return logs::Severity::kError;

    case LogLevel::Fatal:
        return logs::Severity::kFatal;

    default:
        VIOLET_UNREACHABLE();
    }
}

} // namespace

OpenTelemetry::OpenTelemetry(Str tag, Options options) noexcept
    : OpenTelemetry(logs::Provider::GetLoggerProvider()->GetLogger(
                        tag, options.Name, options.Version, options.SchemaUrl, ConvertAttributes(options.Attributes)),
          VIOLET_MOVE(options))
{
}

OpenTelemetry::OpenTelemetry(SharedPtr<logs::Logger> logger, Options opts) noexcept
    : n_backend(VIOLET_MOVE(logger))
    , n_queue(
          [this](Vec<Record> batch) -> void {
              auto records = VIOLET_MOVE(batch);
              for (auto& record: records)
                  this->n_backend->EmitLogRecord(this->transformToOtel(record));
          },
          VIOLET_MOVE(opts))
{
}

void OpenTelemetry::Enqueue(const Record& record)
{
    this->n_queue.Push(record);
}

void OpenTelemetry::Flush() noexcept
{
    this->n_queue.Flush();
}

auto OpenTelemetry::transformToOtel(const Record& record) noexcept -> UniquePtr<logs::LogRecord>
{
    auto log = this->n_backend->CreateLogRecord();
    log->SetObservedTimestamp(otel::SystemTimestamp(record.Timestamp.ToStd()));
    log->SetSeverity(logLevelToOtelSeverity(record.Level));
    log->SetBody(record.Message);

    if (!record.Location.File.empty()) {
        log->SetAttribute("code.filepath", record.Location.File);
    }

    if (record.Location.Line > 0) {
        log->SetAttribute("code.lineno", record.Location.Line);
    }

    if (!record.Location.Function.empty()) {
        log->SetAttribute("code.function", record.Location.Function);
    }

    for (const auto& [key, value]: record.Fields) {
        if (value.Holds<Mono>()) {
            continue;
        }

        log->SetAttribute(key,
            value.Match(
                // clang-format off
                [](const bool& value)   -> otel::AttributeValue { return value; },
                [](const Int64& value)  -> otel::AttributeValue { return value; },
                [](const UInt64& value) -> otel::AttributeValue { return value; },
                [](const double& value) -> otel::AttributeValue { return value; },
                [](const String& value) -> otel::AttributeValue { return value; },
                [](Mono)                -> otel::AttributeValue { VIOLET_UNREACHABLE(); }
                // clang-format on
                ));
    }

    return log;
}

} // namespace violet::experimental::log::sinks

#endif
