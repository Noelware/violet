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

#include <violet/Experimental/Logging/AsyncSink.h>
#include <violet/Experimental/Logging/LogFactory.h>
#include <violet/Experimental/Logging/Logger.h>
#include <violet/Experimental/Logging/Sink.h>
#include <violet/Experimental/Unique.h>
#include <violet/Print.h>
#include <violet/Support/Demangle.h>

namespace violet::experimental::log {

static ptr::Unique<LogFactory> instance = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
const static Logger dummy("dummy logger", LogLevel::Off);

auto LogFactory::Get(Str name) noexcept -> Logger
{
    if (instance == nullptr || instance->n_level == LogLevel::Off) {
        return dummy;
    }

    if (auto logger = instance->n_loggers.Get(name); logger.HasValue()) {
        return logger.Value();
    }

    Logger log(name, instance->n_level);
    for (const auto& sink: instance->n_sinks) {
        log.n_sinks.emplace_back(sink);
    }

    for (const auto& sink: instance->n_asyncSinks) {
        log.n_asyncSinks.emplace_back(sink);
    }

    (void)instance->n_loggers.Insert(name, log);
    return log;
}

void LogFactory::Init(LogLevel level, std::initializer_list<Sink*> sinks, std::initializer_list<AsyncSink*> asyncSinks)
{
    if (instance != nullptr) {
        return;
    }

    instance.Reset(new LogFactory());
    VIOLET_DEBUG_ASSERT0(instance != nullptr);

    for (auto* sink: sinks) {
        instance->n_sinks.emplace_back(sink);
    }

    for (auto* sink: asyncSinks) {
        instance->n_asyncSinks.emplace_back(sink);
    }

    instance->n_level = level;
}

void LogFactory::Init(
    LogLevel level, std::initializer_list<Own<Sink>> sinks, std::initializer_list<Own<AsyncSink>> asyncSinks)
{
    if (instance != nullptr) {
        return;
    }

    instance.Reset(new LogFactory());
    for (auto sink: sinks) {
        instance->n_sinks.emplace_back(VIOLET_MOVE(sink));
    }

    for (auto sink: asyncSinks) {
        instance->n_asyncSinks.emplace_back(VIOLET_MOVE(sink));
    }

    instance->n_level = level;
}

void LogFactory::Init(LogLevel level, Vec<Own<Sink>> sinks, Vec<Own<AsyncSink>> asyncSinks)
{
    if (instance != nullptr) {
        return;
    }

    instance.Reset(new LogFactory());
    instance->n_level = level;

    if (level != LogLevel::Off) {
        instance->n_sinks = VIOLET_MOVE(sinks);
        instance->n_asyncSinks = VIOLET_MOVE(asyncSinks);
    }
}

void LogFactory::Shutdown()
{
    if (instance == nullptr) {
        return;
    }

    for (auto& [name, log]: instance->n_loggers) {
#if VIOLET_FEATURE(EXCEPTIONS)
        try {
            log.flush();
        } catch (const std::exception& ex) {
#if VIOLET_FEATURE(RTTI)
            const auto exceptionType = util::DemangleCXXName(typeid(ex).name());
#else
            const auto exceptionType = util::DemangleCXXException();
#endif

            PrintErrln("[violet/experimental/logging] caught exception [{}] when flushing logger {}: {}", exceptionType,
                name, ex.what());
        } catch (...) {
            PrintErrln("[violet/experimental/logging] caught exception with type [{}] when flushing logger {}",
                util::DemangleCXXException(), name);
        }
#else
        log.flush();
#endif
    }

    instance.Reset();
}

auto GetLogger(Str name) noexcept -> Logger
{
    return LogFactory::Get(name);
}

} // namespace violet::experimental::log
