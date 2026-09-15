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

#if VIOLET_PLATFORM(UNIX)

#include <gtest/gtest.h>
#include <violet/Experimental/Logging/LogRecord.h>
#include <violet/Experimental/Logging/Sinks/Console.h>
#include <violet/Experimental/Slice.h>
#include <violet/Strings.h>

#include <csignal>
#include <fcntl.h>
#include <thread>
#include <unistd.h>

namespace violet::experimental::log::sinks {

static_assert(std::derived_from<Console, Sink>, "`Console` must be usable wherever a `Sink` is expected");
static_assert(!std::is_copy_constructible_v<Console>, "a console sink owns a descriptor, it must not be copyable");
static_assert(!std::is_move_constructible_v<Console>, "`VIOLET_DISALLOW_COPY_AND_MOVE` should have pinned `Console`");
static_assert(std::is_default_constructible_v<Console>, "the stream parameter defaults to `Stream::Stdout`");

namespace {
namespace io = violet::io::experimental;

struct FixedFormatter final: public Formatter {
    VIOLET_EXPLICIT FixedFormatter(String text) noexcept
        : n_text(VIOLET_MOVE(text))
    {
    }

    [[nodiscard]] auto Format(const Record&) const -> String override
    {
        return this->n_text;
    }

private:
    String n_text;
};

struct RecordingFormatter final: public Formatter {
    Vec<Record>* Records;

    VIOLET_EXPLICIT RecordingFormatter(Vec<Record>* seen) noexcept
        : Records(seen)
    {
    }

    [[nodiscard]] auto Format(const Record& record) const -> String override
    {
        this->Records->push_back(VIOLET_MOVE(record));
        return std::format("[{}] {}", record.Level, record.Message);
    }
};

template<io::AsFd Fd>
auto drain(Fd desc) -> String
{
    const auto fd = desc.Borrow();
    String out;
    Slice<char, 4096> buf{};

    while (true) {
        const auto readBytes = ::read(fd.Get(), buf.Data(), buf.Size());
        if (readBytes > 0) {
            out.append(buf.Data(), static_cast<UInt>(readBytes));
            continue;
        }

        if (readBytes < 0 && errno == EINTR) {
            continue;
        }

        break;
    }

    return out;
}

struct ConsoleSink: public ::testing::Test {
protected:
    template<io::AsFd Fd, typename Fun>
        requires(callable<Fun> && callable_returns<Fun, void>)
    auto Capture(Fd target, Fun&& fun, SourceLocation loc = std::source_location::current()) -> String
    {
        const auto fd = target.Borrow();

        Slice<Int32, 2> fds{-1, -1};
        if (::pipe(fds.Data()) != 0) {
            ADD_FAILURE_AT(loc.File.data(), loc.Line)
                << "failed to create pipe for target [" << fd << "]: " << ::strerror(errno);

            return {};
        }

        ::fcntl(fds[0], F_SETFL, O_NONBLOCK);

        const auto saved = ::dup(fd.Get());
        if (saved == -1 || ::dup2(fds[1], fd.Get()) == -1) {
            ADD_FAILURE_AT(loc.File.data(), loc.Line)
                << "failed to redirect file descriptor [" << fd << "]: " << ::strerror(errno);

            return {};
        }

        // the target `fd` is now the only write end we care about.
        ::close(fds[1]);
        std::invoke(VIOLET_FWD(Fun, fun));

        // failure paths go through the iostream
        std::cout.flush();
        std::cerr.flush();

        ::dup2(saved, fd.Get());
        ::close(saved);

        const auto readFd = io::Descriptor::FromRaw(fds[0]);
        const auto out = drain(readFd.Borrow());
        return out;
    }
};

constexpr inline auto kStdoutFd = io::Descriptor::Borrowed(STDOUT_FILENO);
constexpr inline auto kStderrFd = io::Descriptor::Borrowed(STDERR_FILENO);

} // namespace

TEST_F(ConsoleSink, WritesNothingWhenNoFormatterIsAttached)
{
    const auto out = this->Capture(kStdoutFd, [] -> void {
        Console console;
        console.Emit(Record::Now(LogLevel::Info, "hello world"));
    });

    EXPECT_TRUE(out.empty());
}

TEST_F(ConsoleSink, WritesTheFormattedRecordFollowedByANewline)
{
    const auto out = this->Capture(kStdoutFd, [] -> void {
        Console console(FixedFormatter("violet is a flower"));
        console.Emit(Record::Now(LogLevel::Info, "this is ignored"));
    });

    EXPECT_EQ(out, "violet is a flower\n");
}

TEST_F(ConsoleSink, DefaultsToStandardOutput)
{
    const auto out = this->Capture(kStdoutFd, [] -> void {
        Console console(FixedFormatter("stdout"));
        console.Emit(Record::Now(LogLevel::Info, "hi"));
    });

    EXPECT_EQ(out, "stdout\n");
}

TEST_F(ConsoleSink, WritesToStandardErrorWhenAskedTo)
{
    const auto out = this->Capture(kStderrFd, [] -> void {
        Console console(Console::Stream::Stderr, FixedFormatter("stderr"));
        console.Emit(Record::Now(LogLevel::Info, "hi"));
    });

    EXPECT_EQ(out, "stderr\n");
}

TEST_F(ConsoleSink, HandsTheRecordToTheFormatterUntouched)
{
    Vec<Record> seen;
    const auto out = this->Capture(kStdoutFd, [&seen] -> void {
        Console console;
        console.WithFormatter(new RecordingFormatter(&seen));

        auto record = Record::Now(LogLevel::Warning, "disk is nearly full");
        record.Logger = "eousd::storage::poolMgmt";
        (void)record.Fields.Insert("remaining.size", AttributeValue(static_cast<UInt64>(512)));
        (void)record.Fields.Insert("remaining.unit", AttributeValue("MiB"));

        console.Emit(record);
    });

    ASSERT_EQ(seen.size(), 1);
    EXPECT_EQ(seen[0].Level, LogLevel::Warning);
    EXPECT_EQ(seen[0].Message, "disk is nearly full");
    EXPECT_EQ(seen[0].Logger, "eousd::storage::poolMgmt");
    EXPECT_EQ(seen[0].Fields.Size(), 2);
    EXPECT_EQ(out, "[warning] disk is nearly full\n");
}

TEST_F(ConsoleSink, GivesEachRecordItsOwnLine)
{
    Vec<Record> seen;
    const auto out = this->Capture(kStdoutFd, [&seen] -> void {
        Console console{RecordingFormatter(&seen)};

        console.Emit(Record::Now(LogLevel::Trace, "first"));
        console.Emit(Record::Now(LogLevel::Debug, "second"));
        console.Emit(Record::Now(LogLevel::Fatal, "third"));
    });

    EXPECT_EQ(seen.size(), 3);
    EXPECT_EQ(out, "[trace] first\n[debug] second\n[fatal] third\n");
}

TEST_F(ConsoleSink, StillEmitsALineWhenTheFormatterProducesNothing)
{
    const auto out = this->Capture(kStdoutFd, [] -> void {
        Console console{FixedFormatter("")};
        console.Emit(Record::Now(LogLevel::Info, "empty"));
    });

    EXPECT_EQ(out, "\n");
}

TEST_F(ConsoleSink, WithFormatterReplacesThePreviousFormatter)
{
    const auto out = this->Capture(kStdoutFd, [] -> void {
        Console console{FixedFormatter("old")};
        console.WithFormatter(new FixedFormatter("new"));
        console.Emit(Record::Now(LogLevel::Info, "hi"));
    });

    EXPECT_EQ(out, "new\n") << "new formatter should win over old formatter";
}

TEST_F(ConsoleSink, WithStreamSwitchesFromStandardOutputToStandardError)
{
    String stdout;
    const auto stderr = this->Capture(kStderrFd, [this, &stdout] -> void {
        stdout = this->Capture(kStdoutFd, [] -> void {
            Console console{FixedFormatter("redirected")};
            console.WithStream(Console::Stream::Stderr);
            console.Emit(Record::Now(LogLevel::Error, "hi"));
        });
    });

    ASSERT_TRUE(stdout.empty()) << "nothing should never reach stdout, got: " << stdout;
    EXPECT_EQ(stderr, "redirected\n");
}

#if VIOLET_FEATURE(EXCEPTIONS)
TEST_F(ConsoleSink, ReportsAFailedWriteOnStandardErrorInsteadOfThrowing)
{
    ::signal(SIGPIPE, SIG_IGN);

    const auto stderr = this->Capture(kStderrFd, [] -> void {
        Slice<Int32, 2> fds({-1, -1});
        ASSERT_EQ(::pipe(fds.Data()), 0) << ::strerror(errno);

        const auto saved = ::dup(kStdoutFd.Get());
        ASSERT_NE(saved, -1) << ::strerror(errno);
        ASSERT_NE(::dup2(fds[1], kStdoutFd.Get()), -1) << ::strerror(errno);

        ::close(fds[1]);
        ::close(fds[0]);

        {
            Console console{FixedFormatter("into the void")};
            EXPECT_NO_THROW(console.Emit(Record::Now(LogLevel::Error, "into the void")));
        }

        ASSERT_NE(::dup2(saved, kStdoutFd.Get()), -1) << ::strerror(errno);
        ::close(saved);
    });

    EXPECT_NE(stderr.find("[violet/logging@fatal"), String::npos) << stderr;
    EXPECT_NE(stderr.find("received fatal error when writing to stream"), String::npos) << stderr;
    EXPECT_NE(stderr.find("Unix.test.cc"), String::npos)
        << "the fallback should name the source location of the record, got: " << stderr;
}
#endif

TEST_F(ConsoleSink, FlushIsANoOpThatDoesNotDisturbTheStream)
{
    const auto out = this->Capture(kStdoutFd, [] {
        Console console{FixedFormatter("before")};

        console.Emit(Record::Now(LogLevel::Info, "hi"));
        console.Flush();
        console.Emit(Record::Now(LogLevel::Info, "hi"));
    });

    EXPECT_EQ(out, "before\nbefore\n");
}

TEST_F(ConsoleSink, FlushOnAConsoleWithoutAFormatterIsHarmless)
{
    Console console;
    EXPECT_NO_THROW(console.Flush());
}

TEST_F(ConsoleSink, InterleavesNothingWhenEmittingFromManyThreads)
{
    constexpr UInt kThreads = 8;
    constexpr UInt kPerThread = 32;

    const auto out = this->Capture(kStdoutFd, [] -> void {
        Console console{FixedFormatter("concurrent")};

        Vec<std::thread> workers;
        workers.reserve(kThreads);

        for (UInt id = 0; id < kThreads; id++) {
            workers.emplace_back([&console] -> void {
                for (UInt i = 0; i < kPerThread; i++) {
                    console.Emit(Record::Now(LogLevel::Info, "hi"));
                }
            });
        }

        for (auto& w: workers) {
            w.join();
        }
    });

    const auto lines = strings::Lines(out).Count();
    ASSERT_EQ(lines,
        // the `+ 1` is there because `strings::Lines` will observe empty newlines as well
        (kThreads * kPerThread) + 1);

    for (const auto& line: strings::Lines(out)) {
        if (line.empty()) {
            continue;
        }

        EXPECT_EQ(line, "concurrent");
    }
}

} // namespace violet::experimental::log::sinks

#endif
