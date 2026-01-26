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

#include <violet/IO/Experimental/Input/StringInputStream.h>

using violet::io::experimental::StringInputStream;

auto StringInputStream::Read(Span<UInt8> buf) noexcept -> Result<UInt>
{
    const UInt remaining = this->n_pos < this->n_data.size() ? this->n_data.size() - this->n_pos : 0;
    if (remaining == 0 || buf.empty()) {
        return 0;
    }

    const UInt8 toRead = std::min(remaining, buf.size());
    ::memcpy(buf.data(), reinterpret_cast<const UInt8*>(this->n_data.data()) + this->n_pos, toRead);

    this->n_pos += toRead;
    return toRead;
}

auto StringInputStream::Available() const noexcept -> Result<UInt>
{
    if (this->n_pos >= this->n_data.size()) {
        return 0;
    }

    return this->n_data.size() - this->n_pos;
}

auto StringInputStream::Skip(UInt bytes) noexcept -> Result<void>
{
    const UInt remaining = this->n_pos < this->n_data.size() ? this->n_data.size() - this->n_pos : 0;
    const UInt8 toSkip = std::min(bytes, remaining);

    this->n_pos += toSkip;
    return {};
}
