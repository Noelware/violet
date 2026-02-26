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

#include <violet/IO/Experimental/Output/StringOutputStream.h>

using violet::io::experimental::StringOutputStream;

StringOutputStream::StringOutputStream(String buf) noexcept
    : n_buf(VIOLET_MOVE(buf))
{
}

auto StringOutputStream::Write(Span<const UInt8> data) noexcept -> io::Result<UInt>
{
    this->n_buf.append(reinterpret_cast<const char*>(data.data()), data.size());
    return static_cast<UInt>(data.size());
}

auto StringOutputStream::Flush() noexcept -> io::Result<void>
{
    return {};
}

auto StringOutputStream::Get() const noexcept -> Str
{
    return this->n_buf;
}

auto StringOutputStream::Take() const&& noexcept -> String
{
    return VIOLET_MOVE(this->n_buf);
}
