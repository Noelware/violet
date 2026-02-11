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

#pragma once

#include <violet/IO/Descriptor.h>
#include <violet/IO/Error.h>
#include <violet/IO/Experimental/InputStream.h>

namespace violet::io::experimental {

/// An [`InputStream`] that reads from an in-memory string.
///
/// This stream provides sequential reading, skipping, and availability queries
/// over a non-owning string ([`Str`]). It is useful for testing or processing
/// data already in-memory without performing any file or system I/O.
struct StringInputStream final: public InputStream {
    /// Constructs an empty `StringInputStream`.
    VIOLET_IMPLICIT StringInputStream() noexcept = default;

    /// Constructs a `StringInputStream` from a string.
    /// @param str the string to read from. The string is copied/moved into the stream.
    template<std::convertible_to<Str> Str>
    VIOLET_IMPLICIT StringInputStream(Str&& str)
        : n_data(VIOLET_FWD(Str, str))
    {
    }

    /// @inheritdoc violet::io::experimental::InputStream::Read(violet::Span<violet::UInt8>)
    auto Read(Span<UInt8> buf) noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Available()
    [[nodiscard]] auto Available() const noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Skip(violet::UInt8)
    auto Skip(UInt bytes) noexcept -> Result<void> override;

    /// Resets the read position to the start of the string.
    void Reset() noexcept;

    /// Returns the current read position within the buffer.
    ///
    /// This is the number of bytes already consumed from the beginning of the buffer.
    [[nodiscard]] auto Position() const noexcept -> UInt;

    /// Returns **true** if the stream has reached the end of the buffer.
    [[nodiscard]] auto EOS() const noexcept -> bool;

private:
    Str n_data;
    UInt n_pos = 0;
};

} // namespace violet::io::experimental
