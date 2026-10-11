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

#if !VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER)
#error this file requires `liburing` to be present; follow-up on the buildsystem Violet is used to see how to enable it or define `VIOLET_FEATURE_COROUTINES_IO_URING_DRIVER` to `1` to enable it yourself
#else

#include <liburing.h>
#include <violet/Experimental/Coroutines/Internals/IO/Op.h>

namespace violet::experimental::coro::internals::io {
namespace {

constexpr auto clampLength(UInt length) noexcept -> unsigned
{
    return static_cast<unsigned>(std::min<UInt>(length, UINT_MAX));
}

} // namespace

void ReadOp::PrepareUring(io_uring_sqe& sqe) noexcept
{
    const UInt64 offset = this->Offset.HasValue() ? this->Offset.Value() : static_cast<UInt64>(-1);
    ::io_uring_prep_read(&sqe, this->Fd, this->Buffer.SpareData(), clampLength(this->Buffer.Spare()), offset);
}

void WriteOp::PrepareUring(io_uring_sqe& sqe) noexcept
{
    const UInt64 offset = this->Offset.HasValue() ? this->Offset.Value() : static_cast<UInt64>(-1);
    ::io_uring_prep_write(&sqe, this->Fd, this->Data.AsStr().data(), clampLength(this->Data.Size()), offset);
}

} // namespace violet::experimental::coro::internals::io

#endif
