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
//
//! # 🌺💜 `violet/IO/Experimental/Error.h`
//! This is a next generation implementation of `violet::io::Error` that is experimental
//! and is subject to change.

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Container/Result.h>
#include <violet/Experimental/OneOf.h>
#include <violet/SourceLocation.h>

#define VIOLET_IO_ERROR_KINDS(X)                                                                                       \
    X(NotFound, "entity not found")                                                                                    \
    X(PermissionDenied, "permission denied")                                                                           \
    X(ConnectionRefused, "connection refused")                                                                         \
    X(ConnectionReset, "connection reset")                                                                             \
    X(HostUnreachable, "host unreachable")                                                                             \
    X(NetworkUnreachable, "network unreachable")                                                                       \
    X(ConnectionAborted, "connection aborted")                                                                         \
    X(NotConnected, "not connected")                                                                                   \
    X(AddrInUse, "address in use")                                                                                     \
    X(AddrNotAvailable, "address not available")                                                                       \
    X(NetworkDown, "network down")                                                                                     \
    X(BrokenPipe, "broken pipe")                                                                                       \
    X(AlreadyExists, "entity already exists")                                                                          \
    X(WouldBlock, "operation would block")                                                                             \
    X(NotADirectory, "not a directory")                                                                                \
    X(IsADirectory, "is a directory")                                                                                  \
    X(DirectoryNotEmpty, "directory not empty")                                                                        \
    X(ReadOnlyFilesystem, "read-only filesystem or storage medium")                                                    \
    X(FilesystemLoop, "filesystem loop or indirection limit (e.g. symlink loop)")                                      \
    X(StaleNetworkFileHandle, "stale network file handle")                                                             \
    X(InvalidInput, "invalid input parameter")                                                                         \
    X(InvalidData, "invalid data")                                                                                     \
    X(TimedOut, "timed out")                                                                                           \
    X(WriteZero, "write zero")                                                                                         \
    X(StorageFull, "no storage space")                                                                                 \
    X(NotSeekable, "seek on unseekable file")                                                                          \
    X(QuotaExceeded, "quota exceeded")                                                                                 \
    X(FileTooLarge, "file too large")                                                                                  \
    X(ResourceBusy, "resource busy")                                                                                   \
    X(ExecutableFileBusy, "executable file busy")                                                                      \
    X(Deadlock, "deadlock")                                                                                            \
    X(CrossesDevices, "cross-device link or rename")                                                                   \
    X(TooManyLinks, "too many links")                                                                                  \
    X(InvalidFilename, "invalid filename")                                                                             \
    X(ArgumentListTooLong, "argument list too long")                                                                   \
    X(Interrupted, "operation interrupted")                                                                            \
    X(Unsupported, "unsupported")                                                                                      \
    X(UnexpectedEof, "unexpected end of file")                                                                         \
    X(OutOfMemory, "out of memory")                                                                                    \
    X(InProgress, "in progress")

namespace violet::io::experimental {

enum struct ErrorKind : UInt8;
struct Error;

#if VIOLET_PLATFORM(UNIX)
/// This platform's native error code type.
/// @since current
using NativeErrorCode = std::remove_cvref_t<decltype(errno)>;
#elif VIOLET_PLATFORM(WINDOWS)
/// This platform's native error code type.
/// @since current
using NativeErrorCode = UInt32;
#else
using NativeErrorCode = void*;
#endif

namespace NOELDOC_HIDE error_internal {

constexpr inline Str kErrorKindMessages[] = {
#define VIOLET_IO_ERROR_KIND_MESSAGE(NAME, MESSAGE) ::violet::Str{MESSAGE},
    VIOLET_IO_ERROR_KINDS(VIOLET_IO_ERROR_KIND_MESSAGE)
#undef VIOLET_IO_ERROR_KIND_MESSAGE
};

constexpr inline UInt8 kErrorKindCount = static_cast<UInt8>(std::size(kErrorKindMessages));

} // namespace NOELDOC_HIDE error_internal

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") SimpleMessage final {
    ErrorKind Kind;
    String Message;

    constexpr VIOLET_IMPLICIT SimpleMessage(ErrorKind kind, Str message) noexcept
        : Kind(kind)
        , Message(message)
    {
    }
};

enum struct NOELDOC_EXPERIMENTAL_SINCE("current") ErrorKind : UInt8 { // NOLINT(readability-enum-initial-value)
#define VIOLET_IO_ERROR_KIND_ENUMERATOR(NAME, MESSAGE) NAME,
    VIOLET_IO_ERROR_KINDS(VIOLET_IO_ERROR_KIND_ENUMERATOR)
#undef VIOLET_IO_ERROR_KIND_ENUMERATOR
};

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") PlatformError final {
    using error_type = NativeErrorCode;

    VIOLET_IMPLICIT_COPY_AND_MOVE(PlatformError);
    ~PlatformError() = default;

    [[nodiscard]] auto Kind() const noexcept -> ErrorKind;
    [[nodiscard]] auto Get() const noexcept -> error_type
    {
        return this->n_value;
    }

    [[nodiscard]] auto ToString() const noexcept -> String;
    friend auto operator<<(std::ostream& os, const PlatformError& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

    friend auto operator==(const PlatformError& lhs, const PlatformError& rhs) noexcept -> bool
    {
        return lhs.n_value == rhs.n_value;
    }

    friend auto operator==(const PlatformError& lhs, error_type rhs) noexcept -> bool
    {
        return lhs.n_value == rhs;
    }

private:
    friend struct Error;

    VIOLET_EXPLICIT PlatformError() noexcept; // NOLINT(modernize-use-equals-delete)
    VIOLET_EXPLICIT PlatformError(error_type value) noexcept;

    error_type n_value;
};

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Error final {
    using native_error_code = NativeErrorCode;

    VIOLET_IMPLICIT_CONSTEXPR_COPY_AND_MOVE(Error);
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Error);
    ~Error() = default;

    constexpr VIOLET_IMPLICIT Error(ErrorKind kind, SourceLocation loc = std::source_location::current()) noexcept
        : Error(repr::New<ErrorKind>(kind), loc)
    {
    }

    constexpr VIOLET_IMPLICIT Error(
        ErrorKind kind, Str message, SourceLocation loc = std::source_location::current()) noexcept
        : Error(repr::New<SimpleMessage>(kind, message), loc)
    {
    }

    static auto OSError(SourceLocation loc = std::source_location::current()) -> Error;
    static auto FromOSError(native_error_code ec, SourceLocation loc = std::source_location::current()) -> Error;

    [[nodiscard]] auto RawOSError() const noexcept -> Optional<native_error_code>;
    [[nodiscard]] auto Kind() const noexcept -> ErrorKind;

    [[nodiscard]] constexpr auto Location() const noexcept -> SourceLocation
    {
        return this->n_loc;
    }

    [[nodiscard]] auto ToString() const -> String;
    friend auto operator<<(std::ostream& os, const Error& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    using repr = violet::experimental::OneOf<ErrorKind, PlatformError, SimpleMessage>;

    constexpr VIOLET_EXPLICIT Error(repr value, SourceLocation loc = std::source_location::current()) noexcept
        : n_loc(loc)
        , n_repr(VIOLET_MOVE(value))
    {
    }

    SourceLocation n_loc;
    repr n_repr;
};

/// @since current
template<typename T = void>
using Result = violet::Result<T, Error>;

} // namespace violet::io::experimental

VIOLET_TO_STRING(violet::io::experimental::ErrorKind, self, {
    VIOLET_ASSERT0(static_cast<UInt8>(self) < io::experimental::error_internal::kErrorKindCount);
    return String(violet::io::experimental::error_internal::kErrorKindMessages[static_cast<UInt8>(self)]);
});

#undef VIOLET_IO_ERROR_KINDS

/**
 * @macro VIOLET_IO_ERROR
 * @since current
 * @param KIND The `violet::io::ErrorKind` variant (without qualification).
 * @param ... An optional static message.
 *
 * Constructs a `violet::io::Error` with the given kind, capturing the call site.
 *
 * # Example
 *
 * ```cpp
 * return violet::Err(VIOLET_IO_ERROR(NotFound, "file not found"));
 * ```
 */
#define VIOLET_IO_ERROR(KIND, MSG)                                                                                     \
    ::violet::io::experimental::Error(                                                                                 \
        ::violet::io::experimental::ErrorKind::KIND, MSG, ::std::source_location::current())

VIOLET_FORMATTER(violet::io::experimental::PlatformError);
VIOLET_FORMATTER(violet::io::experimental::Error);
