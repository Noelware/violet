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
#include <violet/Violet.h>

namespace violet::io::experimental {

/// Abstract, byte-oriented input stream.
///
/// The `InputStream` abstract class defines a minimal, synchronous interface
/// for reading raw bytes from an underlying source (e.g. file, socket, memory buffer, etc.)
///
/// Implementations of **InputStream** are expected to:
/// - Return the number of bytes actually read from [`InputStream::Read`], which may be less
///   than the size of the provided buffer.
/// - Returns `0` to indicate end-of-stream (EOF) in [`InputStream::Read`] when no more data
///   is available.
/// - Report errors exclusively though the [`io::Result`] interface.
///
/// @note This interface doesn't impose buffering semantics. Use the [`BufferedInputStream`]
///       class to buffer input.
struct InputStream {
    virtual ~InputStream() = default;

    /// Reads bytes from the stream into `buf` by attempting to read up to `buf.size()`
    /// bytes.
    ///
    /// On success, the number of bytes is returned or `0` to indicate EOF (end-of-stream).
    /// On failure, returns a I/o-specific error.
    virtual auto Read(Span<UInt8> buf) noexcept -> Result<UInt> = 0;

    /// Queries the number of bytes that can be read without blocking.
    ///
    /// The returned value is a best-effort estimate of immediately available
    /// bytes. It may be zero even if the stream has not reached EOF, and it may
    /// change between calls.
    ///
    /// On success, returns the number of bytes currently available for reading,
    /// otherwise, on failure, returns an I/o-specific error.
    [[nodiscard]] virtual auto Available() const noexcept -> Result<UInt> = 0;

    /// Skip (discard) `bytes` from the stream.
    ///
    /// This will advance the stream position up to `bytes` without copying into
    /// userland storage.
    ///
    /// Implementations of **InputStream** may skip fewer bytes than requested.
    ///
    /// @param bytes max number of bytes to skip
    /// @returns on success, `void`. on failure, an i/o-specific error.
    virtual auto Skip(UInt bytes) noexcept -> Result<void> = 0;
};

/// Reads the entire remaining contents of a stream into a string.
///
/// This will continuously reads from the `src` stream until end-of-stream (EOF)
/// and appends data into a newly constructed [`String`].
///
/// This function is intended for convenience and may perform multiple reads
/// internally. The stream is left positioned at EOF on success.
///
/// @tparam Stream A type derived from `InputStream`.
/// @param src input stream to read from.
/// @note no character encoding is assumed; bytes are copied verbatim.
template<typename Stream>
    requires(std::is_base_of_v<InputStream, std::remove_cvref_t<Stream>>)
auto ReadToString(Stream& src) noexcept -> Result<String>
{
    String out;

    if (auto available = src.Available()) {
        out.reserve(available.Value());
    }

    UInt8 buf[4096];
    while (true) {
        auto read = src.Read(Span<UInt8>{ buf, sizeof(buf) });
        if (read.Err()) {
            return Err(VIOLET_MOVE(read.Error()));
        }

        const UInt8 bytes = read.Unwrap();
        if (bytes == 0) {
            break;
        }

        out.append(reinterpret_cast<const char*>(buf), bytes);
    }

    return out;
}

// /// Reads a single line from a stream.
// ///
// /// This will read bytes from the `src` stream until a line terminator is
// /// encountered or EOF (end-of-stream) is reached and stores the result in `out`.
// ///
// /// Line terminators are implementation-defined, but are typically `'\n'` or
// /// `"\r\n"`. The terminator itself is not included in `out`.
// ///
// /// @tparam Stream A type derived from `InputStream`.
// /// @param src input stream to read from.
// /// @param out destination string to receive the line contents.
// /// @note no character encoding is assumed
// /// @note if EOF is reached before any bytes are read, `out` may be left empty.
// template<typename Stream>
//     requires(std::is_base_of_v<InputStream, std::remove_cvref_t<Stream>>)
// auto ReadLine(Stream& src, String out) noexcept -> Result<void>
// {
//     out.clear();

//     UInt8 ch = '\0';
//     bool hasSeenAnything = false;

//     while (true) {
//         auto read = src.Read(Span<UInt8>{ &ch, 1 });
//         if (read.Err()) {
//             return Err(VIOLET_MOVE(read.Error()));
//         }

//         if (read.Value() == 0) {
//             return {};
//         }

//         hasSeenAnything = true;
//         if (ch == '\n') {
//             if (!out.empty() && out.back() == '\r') {
//                 out.pop_back();
//             }

//             return {};
//         }

//         out.push_back(static_cast<char>(ch));
//     }
// }

} // namespace violet::io::experimental
