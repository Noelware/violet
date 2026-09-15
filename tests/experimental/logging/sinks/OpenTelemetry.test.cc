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

#if !defined(VIOLET_FEATURE_EXPERIMENTAL_LOGGING_OTEL_SINK) || !VIOLET_FEATURE(EXPERIMENTAL_LOGGING_OTEL_SINK)
#error requires the OpenTelemetry sink to be enabled via your buildsystem for this test to compile
#endif

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <opentelemetry/logs/log_record.h>
#include <opentelemetry/logs/logger.h>
#include <violet/Experimental/Logging/Sinks/OpenTelemetry.h>
#include <violet/Experimental/Mutex.h>

namespace violet::experimental::log::sinks {
namespace common = opentelemetry::common;
namespace nostd = opentelemetry::nostd;
namespace trace = opentelemetry::trace;
namespace logs = opentelemetry::logs;

namespace {

struct CapturedRecord final {
    logs::Severity Severity = logs::Severity::kInvalid;
    String Body;
    Vec<Pair<String, String>> Attributes;
};

struct MockOtelRecord final: public logs::LogRecord {
    CapturedRecord Record;

    void SetSeverity(logs::Severity severity) noexcept override
    {
        this->Record.Severity = severity;
    }

    void SetBody(const common::AttributeValue& message) noexcept override
    {
        if (const auto* sv = nostd::get_if<nostd::string_view>(&message)) {
            this->Record.Body.assign(sv->data(), sv->size());
        }
    }

    void SetAttribute(nostd::string_view key, const common::AttributeValue& value) noexcept override
    {
        String val;
        if (const auto* sv = nostd::get_if<nostd::string_view>(&value)) {
            val.assign(sv->data(), sv->size());
        } else if (const auto* int_ = nostd::get_if<int64_t>(&value)) {
            val = violet::ToString(*int_);
        }

        this->Record.Attributes.emplace_back(String(key), VIOLET_MOVE(val));
    }

    void SetTimestamp(common::SystemTimestamp timestamp) noexcept override { }
    void SetObservedTimestamp(common::SystemTimestamp timestamp) noexcept override { }
    void SetEventId(int64_t id, nostd::string_view name = {}) noexcept override { }
    void SetTraceId(const trace::TraceId& trace_id) noexcept override { }
    void SetSpanId(const trace::SpanId& span_id) noexcept override { }
    void SetTraceFlags(const trace::TraceFlags& trace_flags) noexcept override { }
};

struct MockLogger final: public logs::Logger {
    struct Mutex Mutex;
    Vec<CapturedRecord> Records;

    const nostd::string_view GetName() noexcept override
    {
        return "mock logger";
    }

    nostd::unique_ptr<logs::LogRecord> CreateLogRecord() noexcept override
    {
        return nostd::unique_ptr<logs::LogRecord>(new MockOtelRecord());
    }

    void EmitLogRecord(nostd::unique_ptr<logs::LogRecord>&& record) noexcept override
    {
        MutexLock lock(this->Mutex);
        auto rec = VIOLET_MOVE(record);
        auto* mock = static_cast<MockOtelRecord*>(rec.get());
        this->Records.push_back(mock->Record);
    }
};

} // namespace

TEST(OpenTelemetry, ForwardsRecords)
{
    auto logger = nostd::shared_ptr<logs::Logger>(new MockLogger());
    auto* mock = static_cast<MockLogger*>(logger.get());
    {
        OpenTelemetry sink(logger);
        sink.Enqueue(Record::Now(LogLevel::Warning, "w"));
        sink.Flush();
    }

    EXPECT_EQ(mock->Records.size(), 1U);
    EXPECT_EQ(mock->Records[0].Severity, logs::Severity::kWarn);
    EXPECT_EQ(mock->Records[0].Body, "w");
    EXPECT_FALSE(mock->Records[0].Attributes.empty());
}

TEST(OpenTelemetry, ForwardsRecordsWithAttributes)
{
    auto logger = nostd::shared_ptr<logs::Logger>(new MockLogger());
    auto* mock = static_cast<MockLogger*>(logger.get());
    {
        OpenTelemetry sink(logger);
        sink.Enqueue(Record::Now(LogLevel::Warning, "w").With("hello", "world").With("uwu", 69));
        sink.Flush();
    }

    EXPECT_EQ(mock->Records.size(), 1U);
    EXPECT_EQ(mock->Records[0].Severity, logs::Severity::kWarn);
    EXPECT_EQ(mock->Records[0].Body, "w");
    EXPECT_FALSE(mock->Records[0].Attributes.empty());
    EXPECT_THAT(mock->Records[0].Attributes, testing::Contains(Pair("uwu", "69")));
    EXPECT_THAT(mock->Records[0].Attributes, testing::Contains(Pair("hello", "world")));
}

} // namespace violet::experimental::log::sinks
