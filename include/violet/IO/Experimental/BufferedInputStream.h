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

/// A buffered wrapper around an [`InputStream`].
///
/// This stream reduces the number of calls to the underlying stream by maintaining
/// an internal buffer. Reads are satisfied from this buffer whenever possible, refilling it
/// from the source stream as needed.
///
/// This is particularly useful when the wrapped stream performs expensive syscalls like
/// file descriptors, sockets, pipes, etc where amortizing reads improves performance.
///
/// ## Examples
/// ```cpp
/// #include <violet/IO/Experimental/BufferedInputStream.h>
/// #include <violet/IO/Experimental/Input/FileInputStream.h>
///
/// using namespace violet::io::experimental;
/// using namespace violet;
///
/// FileInputStream file("data.bin");
/// BufferedInputStream buffered(file);
///
/// UInt8 buf[1024];
/// auto action = buffered.Read(buf);
/// ```
///
/// ## Thread Safety
/// This type is NOT thread-safe. External synchronization is required if used
/// from multiple threads.
struct BufferedInputStream final: public InputStream {
    /// Constructs a new buffered stream that wraps over `src`.
    ///
    /// @param src underlying input stream
    /// @param bufSize size of the internal buffer in bytes, defaults to `8192`
    template<typename Stream>
        requires(std::is_base_of_v<InputStream, std::remove_cvref_t<Stream>>)
    VIOLET_IMPLICIT BufferedInputStream(Stream&& src, UInt bufSize = 8192) noexcept
        : n_src(std::make_shared<std::remove_cvref_t<Stream>>(VIOLET_FWD(Stream, src)))
        , n_buf(bufSize)
    {
    }

    /// @inheritdoc violet::io::experimental::InputStream::Read(violet::Span<violet::UInt8>)
    auto Read(Span<UInt8> buf) noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Available()
    [[nodiscard]] auto Available() const noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Skip(violet::UInt8)
    auto Skip(UInt bytes) noexcept -> Result<void> override;

private:
    violet::SharedPtr<InputStream> n_src;
    Vec<UInt8> n_buf;
    UInt n_pos = 0;
    UInt n_end = 0;

    void doRefill() noexcept;
};

} // namespace violet::io::experimental
