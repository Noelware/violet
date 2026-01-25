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

#include <violet/IO/Experimental/Input/ByteArrayInputStream.h>

using violet::Span;
using violet::UInt8;
using violet::io::ByteArrayInputStream;

ByteArrayInputStream::ByteArrayInputStream(Span<const UInt8> buf) noexcept
    : n_buf(buf)
{
}

auto ByteArrayInputStream::Read(Span<UInt8> buf) noexcept -> Result<UInt>
{
    if (this->n_pos >= this->n_buf.size()) {
        return 0;
    }

    const UInt bytesToRead = std::min(buf.size(), this->n_buf.size() - this->n_pos);
    ::memcpy(buf.data(), this->n_buf.data() + this->n_pos, bytesToRead);

    this->n_pos += bytesToRead;
    return bytesToRead;
}

auto ByteArrayInputStream::Available() const noexcept -> Result<UInt>
{
    return this->n_buf.size() - this->n_pos;
}

auto ByteArrayInputStream::Skip(UInt bytes) noexcept -> Result<void>
{
    const UInt skipped = std::min(bytes, this->Available().UnwrapOr(0));
    this->n_pos += skipped;

    return {};
}

void ByteArrayInputStream::Reset() noexcept
{
    this->n_pos = 0;
}
