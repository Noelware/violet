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
//! # 🌺💜 `violet/Experimental/Logging/Logger.h`

#pragma once

#include <violet/Experimental/Logging/LogRecord.h>
#include <violet/Experimental/Own.h>
#include <violet/Experimental/Unique.h>

namespace violet::experimental::log {

struct Sink;
struct AsyncSink;
struct Logger;

/// Represents a single log entry.
///
/// A `LogEntry` is a handle returned by `Logger` when logging a message. It allows attaching
/// additional attributes before the entry is emitted.
struct NOELDOC_EXPERIMENTAL_SINCE("current") VIOLET_API Entry final {
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Entry);
    VIOLET_DISALLOW_COPY_AND_MOVE(Entry);
    ~Entry();

    /// Attaches an additional attribute to the log entry.
    ///
    /// @tparam T type that is constructible as an `AttributeValue`.
    /// @param name name/key of the attribute.
    /// @param value value to associate with the attribute.
    /// @returns a reference to `*this` to allow chaining.
    template<typename T>
        requires std::constructible_from<AttributeValue, std::remove_cvref_t<T>>
    auto With(Str name, T value) noexcept -> Entry&
    {
        this->n_record.Fields.Insert(name, value);
        return *this;
    }

private:
    friend struct Logger;

    VIOLET_EXPLICIT Entry(Logger* parent, Record record, bool emit = true) noexcept
        : n_parent(parent, NoOpDeleter())
        , n_record(VIOLET_MOVE(record))
        , n_emit(emit)
    {
    }

    ptr::Unique<Logger, NoOpDeleter> n_parent;
    Record n_record;
    bool n_emit = true;
};

/// A logger that produces log entries and manages sinks.
///
/// Provides convenience methods for logging at different levels (Trace, Debug, Info, etc.).
/// Supports adding synchronous and asynchronous sinks.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Logger final {
    VIOLET_DISALLOW_CONSTRUCTOR(Logger);
    ~Logger();

    /// Constructs a logger with a name, log level, and optional sinks.
    ///
    /// @param name unique name of the logger.
    /// @param level minimum log level.
    /// @param sinks optional synchronous sinks.
    /// @param asyncSinks optional asynchronous sinks.
    VIOLET_IMPLICIT Logger(Str name, LogLevel level, std::initializer_list<Sink*> sinks = {},
        std::initializer_list<AsyncSink*> asyncSinks = {}) noexcept
        : n_name(name)
        , n_level(level)
    {
        for (auto* sink: sinks) {
            this->n_sinks.emplace_back(sink);
        }

        for (auto* sink: asyncSinks) {
            this->n_asyncSinks.emplace_back(sink);
        }
    }

    /// Constructs a logger with a name, log level, and optional sinks.
    ///
    /// @param name unique name of the logger.
    /// @param level minimum log level.
    /// @param sinks optional synchronous sinks.
    /// @param asyncSinks optional asynchronous sinks.
    VIOLET_IMPLICIT Logger(Str name, LogLevel level, std::initializer_list<Own<Sink>> sinks,
        std::initializer_list<Own<AsyncSink>> asyncSinks = {}) noexcept
        : n_name(name)
        , n_level(level)
        , n_sinks(sinks)
        , n_asyncSinks(asyncSinks)
    {
    }

    /// Returns **true** whether logging is enabled for a given level.
    [[nodiscard]] constexpr auto Enabled(LogLevel level) const noexcept -> bool
    {
        if (level == LogLevel::Off) {
            return false;
        }

        return level >= this->n_level;
    }

    /// Sets or changes the name of this logger.
    auto WithName(Str name) noexcept -> Logger&
    {
        this->n_name = name;
        return *this;
    }

    template<typename SinkT>
        requires(!std::is_abstract_v<SinkT> && std::derived_from<SinkT, Sink>)
    auto AddSink(SinkT* sink) noexcept -> Logger&
    {
        this->n_sinks.emplace_back(sink);
        return *this;
    }

    template<typename SinkT, typename... Args>
        requires(!std::is_abstract_v<SinkT> && std::derived_from<SinkT, Sink>)
    auto AddSink(Args&&... args) noexcept -> Logger&
    {
        this->n_sinks.emplace(Own<Sink>::New<SinkT>(VIOLET_FWD(Args, args)...));
        return *this;
    }

    template<typename SinkT>
        requires(!std::is_abstract_v<SinkT> && std::derived_from<SinkT, AsyncSink>)
    auto AddAsyncSink(SinkT* sink) noexcept -> Logger&
    {
        this->n_asyncSinks.emplace_back(sink);
        return *this;
    }

    template<typename SinkT, typename... Args>
        requires(!std::is_abstract_v<SinkT> && std::derived_from<SinkT, AsyncSink>)
    auto AddSink(Args&&... args) noexcept -> Logger&
    {
        this->n_asyncSinks.emplace(Own<AsyncSink>::New<SinkT>(VIOLET_FWD(Args, args)...));
        return *this;
    }

    [[nodiscard]] auto Sinks() const noexcept -> Span<const Own<Sink>>;
    [[nodiscard]] auto AsyncSinks() const noexcept -> Span<const Own<AsyncSink>>;
    [[nodiscard]] auto Name() const noexcept -> Str;
    [[nodiscard]] auto Level() const noexcept -> LogLevel;

    [[maybe_unused]] auto Log(Record record) const noexcept -> Entry; // NOLINT(modernize-use-nodiscard)

    /// Logs a message at a specified log level.
    ///
    /// @param level log level for the message.
    /// @param message message to log.
    /// @param loc source location, defaults to the caller.
    /// @returns a log `Entry` for further attribute attachment.
    // NOLINTNEXTLINE(modernize-use-nodiscard)
    [[maybe_unused]] auto Log(
        LogLevel level, Str message, SourceLocation loc = std::source_location::current()) const noexcept -> Entry
    {
        return this->Log(Record::Now(level, message, loc));
    }

#define __method__(level)                                                                                              \
    [[maybe_unused]] auto level(Str message, SourceLocation loc = std::source_location::current()) const noexcept      \
        -> Entry                                                                                                       \
    {                                                                                                                  \
        return this->Log(Record::Now(LogLevel::level, message, loc));                                                  \
    }

    __method__(Trace);
    __method__(Debug);
    __method__(Info);
    __method__(Warning);
    __method__(Error);
    __method__(Fatal);

    // NOLINTNEXTLINE
    [[maybe_unused]] auto Warn(Str message, SourceLocation loc = std::source_location::current()) const noexcept
        -> Entry
    {
        return this->Log(Record::Now(LogLevel::Warning, message, loc));
    }

#undef __method__

private:
    friend struct LogFactory;
    friend struct Entry;

    void flush();

    String n_name;
    LogLevel n_level;
    Vec<Own<Sink>> n_sinks;
    Vec<Own<AsyncSink>> n_asyncSinks;
};

} // namespace violet::experimental::log
