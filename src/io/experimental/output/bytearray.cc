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

#include <violet/IO/Experimental/Output/ByteArrayOutputStream.h>

using violet::io::experimental::ByteArrayOutputStream;

ByteArrayOutputStream::ByteArrayOutputStream(Vec<UInt8> buf) noexcept
    : n_buf(VIOLET_MOVE(buf))
{
}

auto ByteArrayOutputStream::Write(Span<const UInt8> data) noexcept -> io::Result<UInt>
{
    this->n_buf.insert(this->n_buf.end(), data.begin(), data.end());
    return static_cast<UInt>(this->n_buf.size());
}

auto ByteArrayOutputStream::Flush() noexcept -> io::Result<void>
{
    return {};
}

auto ByteArrayOutputStream::Get() const noexcept -> const Vec<UInt8>&
{
    return this->n_buf;
}

auto ByteArrayOutputStream::Take() const&& noexcept -> Vec<UInt8>
{
    return VIOLET_MOVE(this->n_buf);
}
