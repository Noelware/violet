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

#include <violet/IO/Experimental/BufferedOutputStream.h>

using violet::io::experimental::BufferedOutputStream;

auto BufferedOutputStream::Write(Span<const UInt8> data) noexcept -> io::Result<UInt>
{
    UInt written = 0;
    Span<const UInt8> subspan = data;

    while (!subspan.empty()) {
        UInt space = n_capacity - static_cast<UInt>(this->n_buffer.size());
        UInt spaceToWrite = std::min(space, static_cast<UInt>(subspan.size()));

        this->n_buffer.insert(this->n_buffer.end(), subspan.begin(), subspan.begin() + static_cast<Int>(spaceToWrite));
        written += spaceToWrite;
        subspan = subspan.subspan(spaceToWrite); // move forward in subspan

        if (this->n_buffer.size() >= this->n_capacity) {
            VIOLET_TRY_VOID(this->Flush());
        }
    }

    return written;
}

auto BufferedOutputStream::Flush() noexcept -> io::Result<void>
{
    if (auto result = this->doFlush(); result.Err()) {
        return Err(VIOLET_MOVE(result.Error()));
    }

    return {};
}

auto BufferedOutputStream::doFlush() noexcept -> io::Result<UInt>
{
    if (this->n_buffer.empty()) {
        return 0;
    }

    UInt written = VIOLET_TRY(this->n_source->Write(this->n_buffer));

    this->n_buffer.clear();
    VIOLET_TRY_VOID(this->n_source->Flush());

    return written;
}
