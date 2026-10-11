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

#pragma once

#include <violet/Experimental/Coroutines/Internals/IO/Op.h>

namespace violet::experimental::coro::io {

template<violet::io::experimental::AsFd Fd>
[[nodiscard]]
auto Read(Fd fd, Bytes buf) -> internals::io::OpAwaiter<internals::io::ReadOp>
{
    if (buf.Spare() == 0) {
        buf.Reserve(4096);
    }

    return internals::io::OpAwaiter<internals::io::ReadOp>(new internals::io::ReadOp(fd, VIOLET_MOVE(buf), Nothing));
}

template<violet::io::experimental::AsFd Fd>
[[nodiscard]]
auto Write(Fd fd, Buf data) -> internals::io::OpAwaiter<internals::io::WriteOp>
{
    return internals::io::OpAwaiter<internals::io::WriteOp>(new internals::io::WriteOp(fd, VIOLET_MOVE(data), Nothing));
}

template<violet::io::experimental::AsFd Fd>
[[nodiscard]]
auto Recv(Fd fd, Bytes buf) -> internals::io::OpAwaiter<internals::io::RecvOp>
{
    if (buf.Spare() == 0) {
        buf.Reserve(4096);
    }

    return internals::io::OpAwaiter<internals::io::RecvOp>(new internals::io::RecvOp(fd, VIOLET_MOVE(buf), Nothing));
}

template<violet::io::experimental::AsFd Fd>
[[nodiscard]]
auto Send(Fd fd, Buf data) -> internals::io::OpAwaiter<internals::io::SendOp>
{
    return internals::io::OpAwaiter<internals::io::SendOp>(new internals::io::SendOp(fd, VIOLET_MOVE(data), Nothing));
}

} // namespace violet::experimental::coro::io
