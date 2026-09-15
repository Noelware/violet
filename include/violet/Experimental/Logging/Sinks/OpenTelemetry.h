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
//! # 🌺💜 `violet/Experimental/Logging/Sinks/OpenTelemetry.h`

#pragma once

#include <violet/Language/Macros.h>

#ifndef VIOLET_FEATURE_EXPERIMENTAL_LOGGING_OTEL_SINK
#define VIOLET_FEATURE_EXPERIMENTAL_LOGGING_OTEL_SINK 0
#endif

#if defined(VIOLET_FEATURE_EXPERIMENTAL_LOGGING_OTEL_SINK) && VIOLET_FEATURE(EXPERIMENTAL_LOGGING_OTEL_SINK)

#include <opentelemetry/logs/log_record.h>
#include <opentelemetry/logs/logger.h>
#include <violet/Experimental/Collections/HashMap.h>
#include <violet/Experimental/Logging/AsyncSink.h>
#include <violet/Experimental/Logging/Internals/BatchQueue.h>
#include <violet/Experimental/Logging/LogRecord.h>

namespace violet::experimental::log::sinks {

/// Forwards log records to an OpenTelemetry logger.
///
/// `OpenTelemetry` bridges Violet's structured logging with the
/// [OpenTelemetry Logs data model](https://opentelemetry.io/docs/specs/otel/logs/data-model/).
///
/// Each enqueued [`LogRecord`] is translated into an `opentelemetry::logs::LogRecord` and submitted through
/// the configured logger on a dedicated background thread.
///
/// The sink owns a single worker thread that drains the internal queue. The thread
/// is started during construction and joined during destruction, ensuring no records
/// are silently dropped on shutdown.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") OpenTelemetry final: public AsyncSink {
    /// The options to passthrough the internal batch queue.
    struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Options final
        : public internals::BatchQueue<Record>::Options {
        String Name;
        String Version;
        String SchemaUrl;
        HashMap<String, AttributeValue> Attributes;
    };

    VIOLET_DISALLOW_CONSTRUCTOR(OpenTelemetry);
    ~OpenTelemetry() override = default;

    /// Construct a new OpenTelemetry log sink by acquiring a OpenTelemetry logger
    /// from the global `LoggerProvider` using the `tag` as the instrumentation-scope
    /// name.
    ///
    /// @param tag the instrumentation-scope name passed to `LoggerProvider::GetLogger`
    VIOLET_IMPLICIT OpenTelemetry(Str tag, Options options = {}) noexcept;

    /// Constructs a new OpenTelemetry log sink from an existing logger instance.
    ///
    /// Ownership of `otel` is shared with the log sink. The provided logger must
    /// remain valid for the lifetime of this sink.
    ///
    /// @param otel a shared pointer to a configured `opentelemetry::logs::Logger`
    VIOLET_IMPLICIT OpenTelemetry(SharedPtr<opentelemetry::logs::Logger> otel, Options options = {}) noexcept;

    NOELDOC_SEE("violet::experimental::log::AsyncSink::Enqueue(const violet::experimental::log::Record&)")
    void Enqueue(const Record& record) override;

    NOELDOC_SEE("violet::experimental::log::AsyncSink::Flush()")
    void Flush() noexcept override;

private:
    SharedPtr<opentelemetry::logs::Logger> n_backend;
    internals::BatchQueue<Record> n_queue;

    auto transformToOtel(const Record& record) noexcept -> UniquePtr<opentelemetry::logs::LogRecord>;
};

} // namespace violet::experimental::log::sinks

#endif
