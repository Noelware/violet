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

#include <violet/Experimental/Coroutines/Internals/IO/Op.h>

#include <cerrno>
#include <unistd.h>

namespace violet::experimental::coro::internals::io {
namespace {

auto attemptFrom(Int64 size) noexcept -> Attempt
{
    if (size >= 0) {
        return Attempt::Finished(size);
    }

    return errno == EAGAIN || errno == EWOULDBLOCK ? Attempt::WouldBlock() : Attempt::Finished(-errno);
}

auto toResult(Int64 result) -> violet::io::experimental::Result<UInt>
{
    if (result < 0) {
        return Err(violet::io::experimental::Error::FromOSError(errno));
    }

    return static_cast<UInt>(result);
}

} // namespace

auto ReadOp::Finish() -> BufResult<UInt, Bytes>
{
    if (this->Result > 0) {
        this->Buffer.AssumeInit(
            Unsafe("the kernel wrote `Result` bytes into the spare tail"), static_cast<UInt>(this->Result));
    }

    return {
        .Result = toResult(this->Result),
        .Buffer = VIOLET_MOVE(this->Buffer),
    };
}

auto ReadOp::TryOnce() noexcept -> Attempt
{
    void* into = this->Buffer.SpareData();
    const UInt length = this->Buffer.Spare();

    if (this->Offset.HasValue()) {
        return attemptFrom(::pread(this->Fd, into, length, static_cast<off_t>(this->Offset.Value())));
    }

    return attemptFrom(::read(this->Fd, into, length));
}

auto WriteOp::Finish() -> BufResult<UInt, Buf>
{
    return {
        .Result = toResult(this->Result),
        .Buffer = VIOLET_MOVE(this->Data),
    };
}

auto WriteOp::TryOnce() noexcept -> Attempt
{
    const auto* from = this->Data.AsStr().data();
    const UInt length = this->Data.Size();

    if (this->Offset.HasValue()) {
        return attemptFrom(::pwrite(this->Fd, from, length, static_cast<off_t>(this->Offset.Value())));
    }

    return attemptFrom(::write(this->Fd, from, length));
}

} // namespace violet::experimental::coro::internals::io
