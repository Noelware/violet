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

#include <violet/IO/Experimental/BufferedInputStream.h>

#include <cstring>

using violet::io::experimental::BufferedInputStream;

void BufferedInputStream::doRefill() noexcept
{
    if (this->n_pos < this->n_end) {
        return;
    }

    this->n_pos = 0;
    this->n_end = 0;

    Span<UInt8> span(this->n_buf.data(), this->n_buf.size());
    auto result = n_src->Read(span);

    if (result.Err()) {
        return;
    }

    this->n_end = result.Value();
}

auto BufferedInputStream::Read(Span<UInt8> buf) noexcept -> Result<UInt>
{
    UInt total = 0;
    while (!buf.empty()) {
        if (this->n_pos == this->n_end) {
            this->doRefill();
            if (this->n_end == 0) {
                break;
            }
        }

        const UInt bytes = std::min(this->n_end - this->n_pos, buf.size());
        ::memcpy(buf.data(), this->n_buf.data() + this->n_pos, bytes);

        this->n_pos += bytes;
        buf = buf.subspan(bytes);
        total += bytes;
    }

    return total;
}

auto BufferedInputStream::Available() const noexcept -> Result<UInt>
{
    return (this->n_end - this->n_pos) + this->n_src->Available().UnwrapOr(0);
}

auto BufferedInputStream::Skip(UInt bytes) noexcept -> Result<void>
{
    const UInt skipped = std::min(bytes, this->n_end - this->n_pos);

    this->n_pos += skipped;
    bytes -= skipped;

    if (bytes == 0) {
        return {};
    }

    VIOLET_TRY_VOID(this->n_src->Skip(bytes));
    return {};
}
