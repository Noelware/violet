// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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
//
//! # 🌺💜 `violet/subprocess/Stdio.h`

#pragma once

#include "violet/io/Descriptor.h"
#include "violet/io/Error.h"
#include "violet/io/Read.h"
#include "violet/io/Write.h"
#include "violet/violet.h"

#ifdef VIOLET_WINDOWS
#    include <windows.h>
#endif

namespace Noelware::Violet::Process {

struct Stdio final {
    constexpr static auto Piped() noexcept -> Stdio;
    constexpr static auto Inherit() noexcept -> Stdio;
    constexpr static auto Null() noexcept -> Stdio;

    constexpr auto IsPiped() const noexcept -> bool;
    constexpr auto IsInherited() const noexcept -> bool;
    constexpr auto IsNull() const noexcept -> bool;

private:
    friend struct Command;
    friend struct Stdin;
    friend struct Stdout;
    friend struct Stderr;

    Stdio() = default;

    enum struct kind : uint8 {
        kPiped = 0, ///< a new pipe is arranged for this process
        kInherit = 1, ///< the file descriptor of the process is inherited from the parent process
        kNull = 2, ///< /dev/null
        kUnknown = std::numeric_limits<uint8>::max(), ///< unknown state (default)
    };

    kind n_kind = kind::kUnknown;
};

/// Represents a handle or file descriptor of a process' child standard input.
struct Stdin final {
    Stdin() = default;

    /// Creates a [`Stdin`] handle from a [`IO::Descriptor`] instance
    /// @param descriptor the descriptor
    VIOLET_EXPLICIT Stdin(IO::Descriptor descriptor, Stdio stdio) noexcept;

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX)
    /// Creates a [`Stdin`] handle from the native type.
    /// @param ty the handle or fd itself.
    VIOLET_EXPLICIT Stdin(IO::Descriptor::native_type ty, Stdio stdio) noexcept;
#endif

    [[nodiscard("always check errors before discarding")]] auto Write(Span<const uint8> buf) -> IO::Result<usize>;

    auto Flush() -> IO::Result<void>;

    /// Returns **true** if this [`Stdin`] handle is considered a valid
    /// file descriptor or handle.
    [[nodiscard]] auto Valid() const noexcept -> bool;

    /// Returns a string representation of this `Stdin` handle
    [[nodiscard]] auto ToString() const noexcept -> String;

    VIOLET_OSTREAM_IMPL(const Stdin&)
    {
        return os << self.ToString();
    }

private:
    IO::Descriptor n_descriptor;
    Stdio n_stdio;
};

static_assert(IO::Writable<Stdin>, "`Stdin` doesn't implement `IO::Writable` concept");

/// Represents a handle or file descriptor of a process' child standard output.
struct Stdout final {
    Stdout() = default;

    /// Creates a [`Stdout`] handle from a [`IO::Descriptor`] instance
    /// @param descriptor the descriptor
    VIOLET_EXPLICIT Stdout(IO::Descriptor descriptor, Stdio stdio) noexcept;

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX)
    /// Creates a [`Stdout`] handle from the native type.
    /// @param ty the handle or fd itself.
    VIOLET_EXPLICIT Stdout(IO::Descriptor::native_type ty, Stdio stdio) noexcept;
#endif

    /// Reads data from this handle and writes into `buf` the incoming data.
    ///
    /// ## Remarks
    /// On Linux, this will be blocking via the [`read()`] syscall. We don't explicitlly
    /// make all files from this handle with the `O_NONBLOCK` flag
    ///
    /// @param buf the buffer to write into that contains the data from this file
    /// @returns the result of the read.
    [[nodiscard("always check errors before discarding")]] auto Read(Span<uint8> buf) -> IO::Result<usize>;

    /// Returns **true** if this [`Stdout`] handle is considered a valid
    /// file descriptor or handle.
    [[nodiscard]] auto Valid() const noexcept -> bool;

    /// Returns a string representation of this `Stdout` handle
    [[nodiscard]] auto ToString() const noexcept -> String;

    VIOLET_OSTREAM_IMPL(const Stdout&)
    {
        return os << self.ToString();
    }

private:
    IO::Descriptor n_descriptor;
    Stdio n_stdio;
};

static_assert(IO::Readable<Stdout>, "`Stdout` doesn't implement `IO::Readable` concept");

/// Represents a handle or file descriptor of a process' child standard error stream.
struct Stderr final {
    Stderr() = default;

    /// Creates a [`Stderr`] handle from a [`IO::Descriptor`] instance
    /// @param descriptor the descriptor
    VIOLET_EXPLICIT Stderr(IO::Descriptor descriptor, Stdio stdio) noexcept;

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX)
    /// Creates a [`Stdout`] handle from the native type.
    /// @param ty the handle or fd itself.
    VIOLET_EXPLICIT Stderr(IO::Descriptor::native_type ty, Stdio stdio) noexcept;
#endif

    /// Reads data from this handle and writes into `buf` the incoming data.
    ///
    /// ## Remarks
    /// On Linux, this will be blocking via the [`read()`] syscall. We don't explicitlly
    /// make all files from this handle with the `O_NONBLOCK` flag
    ///
    /// @param buf the buffer to write into that contains the data from this file
    /// @returns the result of the read.
    [[nodiscard("always check errors before discarding")]] auto Read(Span<uint8> buf) -> IO::Result<usize>;

    /// Returns **true** if this [`Stderr`] handle is considered a valid
    /// file descriptor or handle.
    [[nodiscard]] auto Valid() const noexcept -> bool;

    /// Returns a string representation of this `Stdout` handle
    [[nodiscard]] auto ToString() const noexcept -> String;

    VIOLET_OSTREAM_IMPL(const Stderr&)
    {
        return os << self.ToString();
    }

private:
    IO::Descriptor n_descriptor;
    Stdio n_stdio;
};

static_assert(IO::Readable<Stderr>, "`Stderr` doesn't implement `IO::Readable` concept");

} // namespace Noelware::Violet::Process
