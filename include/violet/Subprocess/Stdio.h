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

#include <violet/Container/Optional.h>
#include <violet/Filesystem/Path.h>
#include <violet/Violet.h>

namespace violet::subprocess {

/// Describes how a standard I/O stream (stdin, stdout, or stderr) should be
/// configured when spawning a child process.
///
/// A `Stdio` value is constructed exclusively via its static factory methods:
/// [`Null()`], [`Inherit()`], and [`Pipe()`], which correspond to the three
/// supported I/O modes:
///
/// - **Null**: the stream is redirected to `/dev/null` (or the platform
///   equivalent); all writes are discarded and all reads return EOF.
///
/// - **Inherit**: the child inherits the parent's file descriptor for this
///   stream, so output goes to the same terminal/file as the parent.
///
/// - **Pipe**: a new OS pipe is created and the child's end is connected to
///   it. The parent end is exposed as a [`ChildStdin`], [`ChildStdout`], or
///   [`ChildStderr`] handle on the spawned [`Child`]. Optionally, the pipe
///   output can be redirected directly into a file path instead of being
///   handed back to the caller.
///
/// ## Example
/// ```cpp
/// // Discard the child's stdout, inherit stderr from the parent, and pipe
/// // stdin so the parent can write to it.
/// auto cmd = Command("my-tool")
///     .WithStdin(Stdio::Pipe())
///     .WithStdout(Stdio::Null())
///     .WithStderr(Stdio::Inherit());
/// ```
struct VIOLET_API NOELDOC_SINCE("26.07") Stdio final {
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Stdio);

    /// Creates a `Stdio` value that redirects the stream to `/dev/null`.
    ///
    /// Writes to a null stream are silently discarded; reads immediately
    /// return EOF. Use this when you want to suppress a child's output
    /// entirely without blocking the child.
    constexpr static auto Null() noexcept -> Stdio
    {
        return Stdio(kind_t::kNull);
    }

    /// Creates a `Stdio` value that inherits the stream from the parent process.
    ///
    /// The child's file descriptor for this stream is a duplicate of the
    /// parent's, so the child writes to (or reads from) the same
    /// terminal, file, or pipe that the parent is currently using.
    constexpr static auto Inherit() noexcept -> Stdio
    {
        return Stdio(kind_t::kInherit);
    }

    /// Creates a `Stdio` value that connects the stream to a new OS pipe.
    ///
    /// After the child is spawned, the parent end of the pipe is returned as
    /// a [`ChildStdin`], [`ChildStdout`], or [`ChildStderr`] handle (depending
    /// on which stream this `Stdio` was applied to), allowing the parent to
    /// read from or write to the child interactively.
    constexpr static auto Pipe() noexcept -> Stdio
    {
        return Stdio(kind_t::kPipe);
    }

    /// Creates a `Stdio` value that pipes the stream directly into a file.
    ///
    /// This is equivalent to shell redirection (e.g. `> file.txt`): the child
    /// writes to the stream normally, but the data is captured into `path`
    /// rather than being surfaced as a handle to the parent.
    ///
    /// @param path filesystem path of the file to receive the piped output. The file is created if it does
    /// not exist and truncated if it does.
    template<std::convertible_to<filesystem::PathRef> Path>
    constexpr static auto Pipe(Path&& path) noexcept -> Stdio
    {
        return Stdio(kind_t::kPipe, VIOLET_FWD(Path, path));
    }

    /// Returns `true` if this `Stdio` is configured as [`Null()`].
    [[nodiscard]] constexpr auto IsNull() const noexcept -> bool
    {
        return this->n_kind == kind_t::kNull;
    }

    /// Returns `true` if this `Stdio` is configured as [`Inherit()`].
    [[nodiscard]] constexpr auto Inherited() const noexcept -> bool
    {
        return this->n_kind == kind_t::kInherit;
    }

    /// Returns `true` if this `Stdio` is configured as [`Pipe()`] (with or
    /// without a target file path).
    [[nodiscard]] constexpr auto Piped() const noexcept -> bool
    {
        return this->n_kind == kind_t::kPipe;
    }

    /// Returns `true` if this `Stdio` is a pipe that redirects output into a
    /// specific file (i.e. constructed with [`Pipe(Path&&)`]).
    [[nodiscard]] constexpr auto PipedIntoFile() const noexcept -> bool
    {
        return this->Piped() && this->n_pipeInto.HasValue();
    }

    /// Returns the target file path when this `Stdio` was constructed with
    /// [`Pipe(Path&&)`], or an empty [`Optional`] otherwise.
    constexpr auto PipedFile() const noexcept -> Optional<filesystem::Path>
    {
        return this->n_pipeInto;
    }

private:
    enum struct kind_t : UInt8 {
        kNull,
        kInherit,
        kPipe
    };

    constexpr VIOLET_EXPLICIT Stdio(kind_t kind)
        : n_kind(kind)
    {
    }

    constexpr VIOLET_EXPLICIT Stdio(kind_t kind, Optional<filesystem::Path> path)
        : n_kind(kind)
        , n_pipeInto(VIOLET_MOVE(path))
    {
    }

    kind_t n_kind;
    Optional<filesystem::Path> n_pipeInto;
};

/// A handle to the stdin pipe of a running child process.
///
/// `ChildStdin` is returned when the child's stdin was configured with
/// [`Stdio::Pipe()`]. It wraps the write end of the OS pipe and satisfies the
/// [`io::Writable`] concept, so data written here is delivered to the child's
/// standard input.
///
/// Dropping (or not using) this handle will eventually cause the child to
/// observe EOF on its stdin.
struct VIOLET_API ChildStdin final {
    /// The underlying file descriptor for the write end of the pipe.
    io::FileDescriptor Descriptor;

    VIOLET_EXPLICIT ChildStdin(io::FileDescriptor::value_type fd) noexcept
        : Descriptor(fd)
    {
    }

    /// Writes `buf` to the child's stdin.
    /// @returns the number of bytes successfully written, or an I/O error.
    [[nodiscard]] auto Write(Span<const UInt8> buf) const noexcept -> io::Result<UInt>
    {
        return this->Descriptor.Write(buf);
    }

    /// Flushes the write buffer.
    ///
    /// This is a no-op for pipe-backed descriptors (the OS kernel manages
    /// pipe buffering), but the method is provided to satisfy the
    /// [`io::Writable`] concept.
    [[nodiscard]] auto Flush() const noexcept -> io::Result<void>
    {
        return { };
    }
};

static_assert(io::Writable<ChildStdin>, "`ChildStdin` doesn't conform to the `io::Writable` concept");

/// A handle to the stdout pipe of a running child process.
///
/// `ChildStdout` is returned when the child's stdout was configured with
/// [`Stdio::Pipe()`]. It wraps the read end of the OS pipe and satisfies the
/// [`io::Readable`] concept, so data produced by the child can be consumed
/// by the parent through this handle.
struct VIOLET_API ChildStdout final {
    /// The underlying file descriptor for the read end of the pipe.
    io::FileDescriptor Descriptor;

    VIOLET_EXPLICIT ChildStdout(io::FileDescriptor::value_type fd) noexcept
        : Descriptor(fd)
    {
    }

    /// Reads up to `buf.size()` bytes from the child's stdout into `buf`.
    ///
    /// @returns the number of bytes read, `0` on EOF (child closed its stdout), or
    /// an I/O error.
    [[nodiscard]] auto Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>
    {
        return this->Descriptor.Read(buf);
    }
};

static_assert(io::Readable<ChildStdout>, "`ChildStdout` doesn't conform to the `io::Readable` concept");

/// A handle to the stderr pipe of a running child process.
///
/// `ChildStderr` is returned when the child's stderr was configured with
/// [`Stdio::Pipe()`]. It wraps the read end of the OS pipe and satisfies the
/// [`io::Readable`] concept, allowing the parent to capture diagnostic output
/// from the child.
struct VIOLET_API ChildStderr final {
    /// The underlying file descriptor for the read end of the pipe.
    io::FileDescriptor Descriptor;

    VIOLET_EXPLICIT ChildStderr(io::FileDescriptor::value_type fd) noexcept
        : Descriptor(fd)
    {
    }

    /// Reads up to `buf.size()` bytes from the child's stderr into `buf`.
    ///
    /// @returns the number of bytes read, `0` on EOF (child closed its stderr), or
    /// an I/O error.
    [[nodiscard]] auto Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>
    {
        return this->Descriptor.Read(buf);
    }
};

static_assert(io::Readable<ChildStderr>, "`ChildStderr` doesn't conform to the `io::Readable` concept");

} // namespace violet::subprocess
