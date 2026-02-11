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
#include <violet/IO/Experimental/InputStream.h>

namespace violet::io::experimental {

/// A simple [`InputStream`] implementation that reads from a contiguous byte array.
///
/// This stream allows reading, skipping, and querying data from an in-memory buffer without
/// performing any allocations or syscalls. This is primarily used for testing or for wrapping
/// data that is already loaded in memory.
///
/// ## Example
/// ```cpp
/// #include <violet/IO/Experimental/Input/ByteArrayInputStream.h>
///
/// using namespace violet;
/// using namespace violet::io::experimental;
///
/// const UInt8 data[] = {1, 2, 3, 4, 5};
/// ByteArrayInputStream stream(data);
///
/// Array<UInt8, 2> buf;
/// auto action = stream.Read(buf); // reads 2 bytes
/// ```
struct ByteArrayInputStream final: public InputStream {
    /// Creates an empty byte array input stream.
    VIOLET_IMPLICIT ByteArrayInputStream() noexcept = default;

    /// Creates a byte array input stream from a contiguous buffer.
    /// @param buf read-only span of bytes to read from.
    VIOLET_IMPLICIT ByteArrayInputStream(Span<const UInt8> buf) noexcept;

    /// @inheritdoc violet::io::experimental::InputStream::Read(violet::Span<violet::UInt8>)
    auto Read(Span<UInt8> buf) noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Available()
    [[nodiscard]] auto Available() const noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Skip(violet::UInt8)
    auto Skip(UInt bytes) noexcept -> Result<void> override;

    /// Resets the read position to the beginning of the buffer.
    void Reset() noexcept;

    /// Returns the current read position within the buffer.
    ///
    /// This is the number of bytes already consumed from the beginning of the buffer.
    [[nodiscard]] auto Position() const noexcept -> UInt;

    /// Returns **true** if the stream has reached the end of the buffer.
    [[nodiscard]] auto EOS() const noexcept -> bool;

private:
    Span<const UInt8> n_buf;
    UInt n_pos = 0;
};

} // namespace violet::io::experimental
