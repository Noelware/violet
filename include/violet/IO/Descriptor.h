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

#include <violet/IO/Read.h>
#include <violet/IO/Write.h>
#include <violet/Violet.h>

namespace violet::io {

/// A zero-cost, tiny abstraction around OS-related file descriptors.
struct VIOLET_API FileDescriptor final {
#ifdef VIOLET_UNIX
    /// Value type on POSIX (macOS, Linux) that wraps a file descriptor.
    using value_type = Int32;
#else
    /// Value type for Windows and unsupported systems.
    using value_type = void*;
#endif

    VIOLET_IMPLICIT_COPY_AND_MOVE(FileDescriptor);

    VIOLET_IMPLICIT FileDescriptor() noexcept;
    VIOLET_IMPLICIT FileDescriptor(value_type value) noexcept;

    /// This will clean up the resource that this file descriptor owns.
    ///
    /// ## Platform-specific Notes
    /// - Windows: Calls `CloseHandle(HANDLE)`.
    /// - POSIX: Calls `close(fd)`.
    ~FileDescriptor();

    /// Returns **true** if this file descriptor points to a valid handle.
    [[nodiscard]] auto Valid() const noexcept -> bool;

    /// Returns the raw value type of this file descriptor.
    [[nodiscard]] auto Get() const noexcept -> value_type;

    /// Closes this file descriptor.
    void Close();

    /// Returns a text representation of this file descriptor.
    ///
    /// - Windows: `Descriptor(<hex of handle>)`
    /// - POSIX: `Descriptor(<id>)`
    [[nodiscard]] auto ToString() const noexcept -> String;

    /// @see violet::io::Readable
    [[nodiscard]] auto Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>;

    /// @see violet::io::Writable
    [[nodiscard]] auto Write(Span<const UInt8> buf) const noexcept -> io::Result<UInt>;

    /// @see violet::io::Writable
    [[nodiscard]] auto Flush() const noexcept -> io::Result<void>;

    VIOLET_EXPLICIT operator bool() const noexcept;
    VIOLET_EXPLICIT operator value_type() const noexcept;

    auto operator==(const FileDescriptor& rhs) const noexcept -> bool;
    auto operator!=(const FileDescriptor& rhs) const noexcept -> bool;
    auto operator==(value_type rhs) const noexcept -> bool;
    auto operator!=(value_type rhs) const noexcept -> bool;

private:
    struct Impl;

    SharedPtr<Impl> n_impl;
};

static_assert(Readable<FileDescriptor>, "`FileDescriptor` doesn't conform to the `Readable` concept");
static_assert(Writable<FileDescriptor>, "`FileDescriptor` doesn't conform to the `Writable` concept");

} // namespace violet::io

VIOLET_FORMATTER(violet::io::FileDescriptor);
