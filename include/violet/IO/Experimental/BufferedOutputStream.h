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

#include <violet/IO/Error.h>
#include <violet/IO/Experimental/OutputStream.h>
#include <violet/Violet.h>

namespace violet::io::experimental {

struct BufferedOutputStream final: public OutputStream {
    VIOLET_IMPLICIT BufferedOutputStream(SharedPtr<OutputStream> stream, UInt capacity = 8192) noexcept
        : n_source(VIOLET_MOVE(stream))
        , n_capacity(capacity)
    {
        this->n_buffer.reserve(capacity);
    }

    template<typename Stream>
        requires(std::is_base_of_v<OutputStream, std::remove_cvref_t<Stream>>)
    VIOLET_IMPLICIT BufferedOutputStream(Stream&& source, UInt capacity = 8192) noexcept
        : n_source(std::make_shared<std::remove_cvref_t<Stream>>(VIOLET_FWD(Stream, source)))
        , n_capacity(capacity)
    {
        this->n_buffer.reserve(capacity);
    }

    auto Write(Span<const UInt8> data) noexcept -> io::Result<UInt> override;
    auto Flush() noexcept -> io::Result<void> override;

private:
    SharedPtr<OutputStream> n_source;
    Vec<UInt8> n_buffer;
    UInt n_capacity;

    auto doFlush() noexcept -> io::Result<UInt>;
};

} // namespace violet::io::experimental
