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

#include <violet/IO/Experimental/InputStream.h>
#include <violet/Violet.h>

namespace violet::io::experimental {

struct BufferedInputStream final: public InputStream {
    template<typename Stream>
        requires(std::is_base_of_v<InputStream, std::remove_cvref_t<Stream>>)
    VIOLET_IMPLICIT BufferedInputStream(Stream&& src, UInt bufSize = 8192) noexcept
        : n_src(std::make_unique<std::remove_cvref_t<Stream>>(VIOLET_FWD(Stream, src)))
        , n_buf(bufSize)
    {
    }

    auto Read(Span<UInt8> buf) noexcept -> Result<UInt> override;
    [[nodiscard]] auto Available() const noexcept -> Result<UInt> override;
    auto Skip(UInt bytes) noexcept -> Result<void> override;

private:
    violet::UniquePtr<InputStream> n_src;
    Vec<UInt8> n_buf;
    UInt n_pos = 0;
    UInt n_end = 0;

    void doRefill() noexcept;
};

} // namespace violet::io::experimental
