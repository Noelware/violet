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
//! # 🌺💜 `violet/Experimental/Logging/LogFactory.h`

#pragma once

#include <violet/Experimental/Collections/HashMap.h>
#include <violet/Experimental/Logging/LogRecord.h>
#include <violet/Experimental/Logging/Logger.h>
#include <violet/Experimental/Own.h>

namespace violet::experimental::log {

struct Sink;
struct AsyncSink;

/// A factory-style way for constructing and managing loggers.
///
/// `LogFactory` provides a centralized way to obtain loggers by name,
/// configure logging sinks (both sync and async), and manage the global logging
/// lifecycle.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Logging/Formatters/Pattern.h>
/// #include <violet/Experimental/Logging/Sinks/Console.h>
/// #include <violet/Experimental/Logging/LogFactory.h>
///
/// using namespace violet::experimental;
/// using namespace violet::experimental::log;
/// using namespace formatters;
///
/// LogFactory::Initialize(
///     /* logLevel:   */ LogLevel::Info,
///     /* sinks:      */ { new Console(Pattern::Azalia()) },
///     /* asyncSinks: */ {}
/// );
///
/// auto logger = log::GetLogger("main");
/// logger.Info("hello, world!");
/// ```
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") LogFactory final {
    /// Retrieves a logger with the specified name.
    ///
    /// If the logger doesn't exist already, it'll be created and stored internally.
    /// If the log factory was not initialized, it'll return a dummy logger that doesn't log anything.
    ///
    /// @param name unique name of the logger to retrieve.
    /// @returns logger instance corresponding to the requested name.
    static auto Get(Str name) noexcept -> Logger;

    /// Initializes the logging factory with a specified `level` and a list of async and/or synchronous
    /// sinks.
    ///
    /// This overload allows you to do `LogFactory::Initialize(Info, { new SinkT }, { new AsyncSinkT })` where `SinkT`
    /// is a sink that implements the [`Sink`] interface and `AsyncSinkT` is a sink that implements the [`AsyncSink`]
    /// interface.
    ///
    /// @param level global log level
    /// @param sinks list of synchronous sinks for log output
    /// @param asyncSinks list of asynchronous sinks for log output
    static void Init(
        LogLevel level, std::initializer_list<Sink*> sinks = {}, std::initializer_list<AsyncSink*> asyncSinks = {});

    /// Initializes the logging factory with a specified `level` and a list of async and/or synchronous
    /// sinks.
    ///
    /// This overload allows you to do `LogFactory::Initialize(Info, { Own::New<SinkT>() }, { Own::New<AsyncSinkT>() })`
    /// where `SinkT` is a sink that implements the [`Sink`] interface and `AsyncSinkT` is a sink that implements the
    /// [`AsyncSink`] interface.
    ///
    /// @param level global log level
    /// @param sinks list of synchronous sinks for log output
    /// @param asyncSinks list of asynchronous sinks for log output
    static void Init(
        LogLevel level, std::initializer_list<Own<Sink>> sinks, std::initializer_list<Own<AsyncSink>> asyncSinks = {});

    /// Initializes the logging factory with a specified `level` and a list of async and/or synchronous
    /// sinks.
    ///
    /// This overload allows you to initialize sinks with an already existing list of async and/or sync sinks,
    /// which is useful in applications that allow configuring the interface.
    ///
    /// @param level global log level
    /// @param sinks list of synchronous sinks for log output
    /// @param asyncSinks list of asynchronous sinks for log output
    static void Init(LogLevel level, Vec<Own<Sink>> sinks, Vec<Own<AsyncSink>> asyncSinks = {});

    /// Shuts down the logging system, flushing any pending asynchronous logs.
    static void Shutdown();

private:
    VIOLET_IMPLICIT LogFactory() noexcept = default;

    LogLevel n_level = LogLevel::Off;
    HashMap<String, Logger> n_loggers;
    Vec<Own<Sink>> n_sinks;
    Vec<Own<AsyncSink>> n_asyncSinks;
};

/// Retrieve a logger from the global [`LogFactory`] by its `name`, if the
/// logging factory was setup correctly.
///
/// If [`LogFactory::Initialize()`] was never called, this will return a dummy logger
/// that doesn't emit records.
///
/// @param name logger name
VIOLET_API auto GetLogger(Str name) noexcept -> Logger;

} // namespace violet::experimental::log
