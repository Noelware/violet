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

namespace violet::subprocess {

/// Interface for reading from a subprocess pipe.
///
/// `PipeReader` defines the contract for capturing output from a child process' standard
/// output or error when the corresponding [`Stdio`](violet::subprocess::Stdio) is configured
/// as [`Stdio::Pipe()`](violet::subprocess::Stdio::Pipe). Implementations are responsible for
/// consuming data from the underlying file descriptor and buffering it for later retrival.
///
/// ## Example
/// ```cpp
/// #include <violet/Subprocess/PipeReader.h>
///
/// struct AReader: public violet::subprocess::PipeReader {
///     void Register(violet::io::FileDescriptor::value_type fd) noexcept override { /* ... */ }
///     auto CaptureAll() const noexcept -> violet::io::Result<violet::Vec<violet::UInt8>> override { /*...*/ }
/// };
/// ```
struct VIOLET_API NOELDOC_SINCE("26.07") PipeReader {
    virtual ~PipeReader() = default;

    /// Associate this reader with a given pipe file descriptor.
    ///
    /// Usually called once after the child process is spawned. Implementations should
    /// store `fd` and prepare to drain it (e.g. by spawning a reader thread or asynchronous I/O).
    ///
    /// @param fd read end of the pipe connected to the child's standard output or error.
    [[nodiscard]] virtual auto Register(io::FileDescriptor::value_type fd) -> violet::io::Result<void> = 0;

    /// Returns all bytes captured from the pipe so far.
    ///
    /// This method blocks until the pipe is drained (i.e. the write end is closed
    /// by the child process), then returned the accumulated bytes. The returned [`Vec`]
    /// may be empty if the child produced no output or if the pipe was not registered
    /// via [`PipeReader::Register`].
    [[nodiscard]] virtual auto CaptureAll() const -> io::Result<Vec<UInt8>> = 0;

    /// Returns whether the pipe reader requires the file descriptor to be set
    /// to non-blocking mode (`O_NONBLOCK`) before reading.
    ///
    /// Readers that perform their own asynchronous I/O internally (such as
    /// `io_uring`) should return `false`, as the kernel handles waiting for
    /// data availability. Readers that rely on readiness-based multiplexing
    /// (such as `epoll` or `kqueue`) should return `true` so that reads do
    /// not block the calling thread.
    ///
    /// The default implementation returns `true`.
    [[nodiscard]] virtual auto WantsNonBlocking() const -> bool
    {
        return true;
    }
};

/// Returns the [`PipeReader`] implementation to use.
VIOLET_API auto GetPipeReader() noexcept -> UniquePtr<PipeReader>;

} // namespace violet::subprocess
