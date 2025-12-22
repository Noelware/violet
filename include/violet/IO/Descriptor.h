// 🌺💜 Violet: Extended C++ standard library
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

#pragma once

#include "violet/IO/Read.h"
#include "violet/IO/Write.h"
#include "violet/Violet.h"

#ifdef VIOLET_WINDOWS
#include <windows.h>
#endif

namespace violet::io {

/// Abstraction around a POSIX file descriptor or a Windows `HANDLE`.
struct VIOLET_API FileDescriptor final {
#ifdef VIOLET_WINDOWS
    /// Value type on Windows that wraps a file descriptor.
    using value_type = HANDLE;
#elif defined(VIOLET_UNIX)
    /// Value type on POSIX (macOS, Linux) that wraps a file descriptor.
    using value_type = Int32;
#else
    /// On a unsupported platform, this will be `void*` (which will always be `nullptr`).
    using value_type = void*;
#endif

    /// Produce a new, dummy file descriptor.
    VIOLET_IMPLICIT FileDescriptor() = default;

    /// Creates a new file descriptor from the OS value type.
    /// @param value the value.
    VIOLET_IMPLICIT FileDescriptor(value_type value);

    /// This will clean up the resource that this file descriptor owns.
    ///
    /// - Windows: Calls `CloseHandle(HANDLE)`.
    /// - POSIX: Calls `close(fd)`.
    ~FileDescriptor();

    /// Cannot *safely* copy file descriptors.
    VIOLET_IMPLICIT FileDescriptor(const FileDescriptor& other) noexcept = delete;

    /// Moves the file descriptor into a new structure. This will close `other`'s
    /// descriptor and moves `other` into `this`, producing a new file descriptor.
    VIOLET_IMPLICIT FileDescriptor(FileDescriptor&& other) noexcept;

    auto operator=(const FileDescriptor& other) noexcept -> FileDescriptor& = delete;
    auto operator=(FileDescriptor&& other) noexcept -> FileDescriptor&;

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX)
    /// Returns the raw value type.
    [[nodiscard]] auto Get() const noexcept -> value_type;
#endif

    /// Returns a text representation of this file descriptor.
    ///
    /// - Windows: `Descriptor(<hex of handle>)`
    /// - POSIX: `Descriptor(<id>)`
    [[nodiscard]] auto ToString() const noexcept -> String;

    /// Closes the file descriptor and this descriptor will be invalidated.
    void Close();

    /// Returns **true** if this file descriptor points to a valid handle.
    [[nodiscard]] auto Valid() const noexcept -> bool;

    /// Conversion operator: `violet::io::FileDescriptor` -> `bool`.
    VIOLET_EXPLICIT operator bool() const noexcept;

    [[nodiscard]] auto Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>;
    [[nodiscard]] auto Write(Span<const UInt8> buf) const noexcept -> io::Result<UInt>;
    [[nodiscard]] auto Flush() const noexcept -> io::Result<void>;

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX)
    /// Conversion operator: `violet::io::FileDescriptor` -> `value_type`.
    VIOLET_EXPLICIT operator value_type() const noexcept;
#endif

private:
#ifdef VIOLET_WINDOWS
    constexpr static const HANDLE INVALID = INVALID_HANDLE_VALUE;
#elif defined(VIOLET_UNIX)
    constexpr static const Int32 INVALID = -1;
#else
    constexpr static const void* INVALID = nullptr;
#endif

    value_type n_value = INVALID;
};

static_assert(Readable<FileDescriptor>, "`FileDescriptor` doesn't conform to the `Readable` concept");
static_assert(Writable<FileDescriptor>, "`FileDescriptor` doesn't conform to the `Writable` concept");

} // namespace violet::io

VIOLET_FORMATTER(violet::io::FileDescriptor);
