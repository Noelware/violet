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

#include <violet/IO/Descriptor.h>
#include <violet/IO/Error.h>
#include <violet/IO/Experimental/InputStream.h>

namespace violet::io::experimental {

/// An [`InputStream`] that reads from the process' standard input (`stdin`).
///
/// This stream wraps the process' standard input file descriptor and provides a
/// sequential read inferface. It supports reading, skipping, and querying the
/// number of bytes available.
///
/// ## Example
/// ```cpp
/// #include <violet/IO/Experimental/Input/StdinInputStream.h>
///
/// using namespace violet;
/// using namespace violet::io::experimental::StdinInputStream;
///
/// StdinInputStream stream;
/// UInt8 buf[1024];
/// auto action = stream.Read(buf);
/// ```
struct StdinInputStream final: public InputStream {
    /// Constructs a new stdin-based input stream.
    VIOLET_IMPLICIT StdinInputStream() noexcept;

    /// @inheritdoc violet::io::experimental::InputStream::Read(violet::Span<violet::UInt8>)
    auto Read(Span<UInt8> buf) noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Available()
    [[nodiscard]] auto Available() const noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Skip(violet::UInt8)
    auto Skip(UInt bytes) noexcept -> Result<void> override;

private:
    io::FileDescriptor n_descriptor;
};

} // namespace violet::io::experimental
