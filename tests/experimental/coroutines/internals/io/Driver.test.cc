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

#include "tests/experimental/coroutines/internals/io/fixtures/IODriverTest.h"

#include <violet/Experimental/Buf.h>
#include <violet/Experimental/Coroutines/IO.h>
#include <violet/Experimental/Coroutines/Sleep.h>

#include <csignal>
#include <thread>

#if VIOLET_PLATFORM(UNIX)
#include <sys/socket.h>
#endif

namespace violet::experimental::coro::internals::io {
namespace {

using chrono::Duration;
using violet::io::experimental::AsFd;
using violet::io::experimental::RawFd;

template<AsFd Fd>
auto ReadOnce(Fd fd) -> Task<String>
{
    auto [result, buf] = co_await coro::io::Read(fd, Bytes(4096));
    const UInt size = result.Unwrap();

    co_return String(reinterpret_cast<const char*>(buf.Data()), size); // NOLINT
}

template<AsFd Fd>
auto WriteStr(Fd fd, Str data) -> Task<UInt>
{
    auto [result, _] = co_await coro::io::Write(fd, Buf::CopyFrom(data));
    co_return result.Unwrap();
}

template<AsFd In, AsFd Out>
auto WriteThenRead(In in, Out out) -> Task<String>
{
    co_await WriteStr(in, "hello");
    co_return co_await ReadOnce(out);
}

template<AsFd Fd>
auto WriteLater(Fd fd, Int32 millis, Str data) -> Task<void>
{
    co_await Sleep(Duration::Milliseconds(millis));
    co_await WriteStr(fd, data);
}

template<AsFd In, AsFd Out>
auto ReadWithLateWriter(In in, Out out) -> Task<String>
{
    {
        [[maybe_unused]]
        JoinHandle writer = Handle::Current().Spawn(WriteLater(in, 10, "late"));
    }

    co_return co_await ReadOnce(out);
}

template<AsFd Fd>
auto DrainExactly(Fd fd, UInt total) -> Task<UInt>
{
    UInt received = 0;
    while (received < total) {
        auto [result, _] = co_await coro::io::Read(fd, Bytes(65536));
        const UInt n = result.Unwrap();
        if (n == 0) {
            break;
        }

        received += n;
    }

    co_return received;
}

template<AsFd In, AsFd Out>
auto PumpBytes(In in, Out out, UInt size) -> Task<UInt>
{
    JoinHandle reader = Handle::Current().Spawn(DrainExactly(out, size));

    auto [written, _] = co_await WriteAll(in, Buf::FromOwner(String(size, 'x')), false);
    static_cast<void>(written.Unwrap());

    co_return co_await VIOLET_MOVE(reader);
}

auto ManyReaders(Vec<testing::Pipes>* pipes) -> Task<String>
{
    Vec<JoinHandle<String>> readers;
    for (auto& pipe: *pipes) {
        readers.push_back(Handle::Current().Spawn(ReadOnce(pipe.Read.Borrow())));
    }

    // Write in reverse order: completions must reach the right tasks, not just "the next one".
    for (UInt i = pipes->size(); i-- > 0;) {
        co_await WriteStr((*pipes)[i].Write.Borrow(), std::to_string(i));
    }

    String joined;
    for (auto& reader: readers) {
        joined += co_await VIOLET_MOVE(reader);
    }

    co_return joined;
}

template<AsFd In, AsFd Out>
auto ReadAppendsToSpare(In in, Out out) -> Task<String>
{
    Bytes buffer(64);
    buffer.Extend("prefix:");

    co_await WriteStr(in, "data");
    auto [result, filled] = co_await coro::io::Read(out, VIOLET_MOVE(buffer));
    static_cast<void>(result.Unwrap());

    co_return String(reinterpret_cast<const char*>(filled.Data()), filled.Size());
}

template<AsFd Fd>
auto WriteToClosedReader(Fd in) -> Task<bool>
{
    auto [result, _] = co_await coro::io::Write(in, Buf::FromStatic("lost"));
    co_return static_cast<bool>(result.Err());
}

template<AsFd Fd>
auto AbortPendingRead(Fd out) -> Task<bool>
{
    JoinHandle reader = Handle::Current().Spawn(ReadOnce(out));
    co_await Sleep(Duration::Milliseconds(5)); // let it park in the driver

    reader.Abort();
    co_return !(co_await VIOLET_MOVE(reader).Cancellable()).HasValue();
}

auto SpawnAndPark(Task<String> task) -> Task<void>
{
    {
        [[maybe_unused]] JoinHandle detached = Handle::Current().Spawn(VIOLET_MOVE(task));
    }

    co_await Sleep(Duration::Milliseconds(5));
}

template<AsFd Fd>
auto FillThenParkWrite(Fd in) -> Task<void>
{
    // Fill the pipe so the next write must wait, then leave that write pending.
    const String chunk(65536, 'f');
    const auto fd = in.Borrow();
    while (::write(fd.Get(), chunk.data(), chunk.size()) > 0) { }

    {
        [[maybe_unused]] JoinHandle writer = Handle::Current().Spawn(WriteLater(in, 0, "stuck"));
    }

    co_await Sleep(Duration::Milliseconds(5));
}

} // namespace
} // namespace violet::experimental::coro::internals::io
