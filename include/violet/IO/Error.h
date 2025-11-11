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

#pragma once

#include "violet/Container/Result.h"
#include "violet/Violet.h"

#if VIOLET_USE_RTTI
#include "violet/Container/Optional.h"
#endif

#include <variant>

namespace violet::io {

/// A list of general categories of I/O errors.
enum struct ErrorKind : UInt8 {
    NotFound,
    PermissionDenied,
    ConnectionRefused,
    ConnectionReset,
    HostUnreachable,
    NetworkUnreachable,
    ConnectionAborted,
    NotConnected,
    AddrInUse,
    AddrNotAvailable,
    NetworkDown,
    BrokenPipe,
    AlreadyExists,
    WouldBlock,
    NotADirectory,
    IsADirectory,
    DirectoryNotEmpty,
    ReadOnlyFilesystem,
    FilesystemLoop,
    StaleNetworkFileHandle,
    InvalidInput,
    InvalidData,
    TimedOut,
    WriteZero,
    StorageFull,
    NotSeekable,
    QuotaExceeded,
    FileTooLarge,
    ResourceBusy,
    ExecutableFileBusy,
    Deadlock,
    CrossesDevices,
    TooManyLinks,
    InvalidFilename,
    ArgumentListTooLong,
    Interrupted,
    Unsupported,
    UnexpectedEof,
    OutOfMemory,
    InProgress,
    Other,

    /// @internal
    __other,
};

} // namespace violet::io

VIOLET_TO_STRING(const io::ErrorKind&, self, {
#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
    VIOLET_DIAGNOSTIC_PUSH
    VIOLET_DIAGNOSTIC_IGNORE("-Wswitch")
#endif
    switch (self) {
    case io::ErrorKind::AddrInUse:
        return "address in use";

    case io::ErrorKind::AddrNotAvailable:
        return "address not avaliable";

    case io::ErrorKind::AlreadyExists:
        return "entity already exists";

    case io::ErrorKind::ArgumentListTooLong:
        return "argument list too long";

    case io::ErrorKind::BrokenPipe:
        return "broken pipe";

    case io::ErrorKind::ConnectionAborted:
        return "connection aborted";

    case io::ErrorKind::ConnectionRefused:
        return "connection refused";

    case io::ErrorKind::ConnectionReset:
        return "connection reset";

    case io::ErrorKind::CrossesDevices:
        return "cross-device link or rename";

    case io::ErrorKind::Deadlock:
        return "deadlock";

    case io::ErrorKind::DirectoryNotEmpty:
        return "directory not empty";

    case io::ErrorKind::ExecutableFileBusy:
        return "executable file busy";

    case io::ErrorKind::FileTooLarge:
        return "file too large";

    case io::ErrorKind::FilesystemLoop:
        return "filesystem loop or indirection limit (e.g. symlink loop)";

    case io::ErrorKind::HostUnreachable:
        return "host unreachable";

    case io::ErrorKind::InProgress:
        return "in progress";

    case io::ErrorKind::Interrupted:
        return "operation interrupted";

    case io::ErrorKind::InvalidData:
        return "invalid data";

    case io::ErrorKind::InvalidFilename:
        return "invalid filename";

    case io::ErrorKind::InvalidInput:
        return "invalid input parameter";

    case io::ErrorKind::IsADirectory:
        return "is a directory";

    case io::ErrorKind::NetworkDown:
        return "network down";

    case io::ErrorKind::NetworkUnreachable:
        return "network unreachable";

    case io::ErrorKind::NotADirectory:
        return "not a directory";

    case io::ErrorKind::NotConnected:
        return "not connected";

    case io::ErrorKind::NotFound:
        return "entity not found";

    case io::ErrorKind::NotSeekable:
        return "seek on unseekable file";

    case io::ErrorKind::Other:
        return "other error";

    case io::ErrorKind::OutOfMemory:
        return "out of memory";

    case io::ErrorKind::PermissionDenied:
        return "permission denied";

    case io::ErrorKind::QuotaExceeded:
        return "quota exceeded";

    case io::ErrorKind::ReadOnlyFilesystem:
        return "read-only filesystem or storage medium";

    case io::ErrorKind::ResourceBusy:
        return "resource busy";

    case io::ErrorKind::StaleNetworkFileHandle:
        return "stale network file handle";

    case io::ErrorKind::StorageFull:
        return "no storage space";

    case io::ErrorKind::TimedOut:
        return "timed out";

    case io::ErrorKind::TooManyLinks:
        return "too many links";

    case io::ErrorKind::UnexpectedEof:
        return "unexpected end of file";

    case io::ErrorKind::Unsupported:
        return "unsupported";

    case io::ErrorKind::WouldBlock:
        return "operation would block";

    case io::ErrorKind::WriteZero:
        return "write zero";
    }

#if defined(VIOLET_CLANG) || defined(VIOLET_GCC)
    VIOLET_DIAGNOSTIC_POP
#endif

    return "";
});

namespace violet::io {

struct Error;

struct PlatformError final {
#ifdef VIOLET_WINDOWS
    /// The type that is represented of `GetLastError()` for Windows
    using error_type = UInt64;
#elif defined(VIOLET_UNIX)
    /// Type that is represented by `errno` on POSIX platforms
    using error_type = Int32;
#endif

    [[nodiscard]] auto AsErrorKind() const noexcept -> ErrorKind;

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX)
    /// Returns the raw error that this [`PlatformError`] stores.
    [[nodiscard]] auto Get() const noexcept -> error_type;
#endif

    /// Returns the string representation of this platform's I/O error code.
    [[nodiscard]] auto ToString() const noexcept -> String;

private:
    friend struct violet::io::Error;

    VIOLET_EXPLICIT PlatformError();

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX)
    error_type n_value;
#endif
};

struct Error final {
    constexpr VIOLET_IMPLICIT Error(ErrorKind kind)
        : n_repr(kind)
    {
    }

    constexpr VIOLET_IMPLICIT Error(ErrorKind kind, Str message)
        : n_repr(simple_message{ kind, message })
    {
    }

    static auto OSError() -> Error;

#if VIOLET_USE_RTTI
    template<typename T, typename... Args>
    static auto New(ErrorKind kind, Args&&... args) -> Error;
#endif

    [[nodiscard]] auto RawOSError() const noexcept -> Optional<PlatformError::error_type>;
    [[nodiscard]] auto Kind() const noexcept -> ErrorKind;
    [[nodiscard]] auto ToString() const noexcept -> String;

#if VIOLET_USE_RTTI
    template<typename T>
    auto Downcast() const noexcept -> Optional<T>;
#endif

private:
    constexpr Error() = default;

    struct simple_message {
        constexpr VIOLET_EXPLICIT simple_message(ErrorKind kind, Str message)
            : n_kind(kind)
            , n_message(message)
        {
        }

        [[nodiscard]] constexpr auto Kind() const noexcept -> ErrorKind
        {
            return this->n_kind;
        }

        [[nodiscard]] constexpr auto Message() const noexcept -> Str
        {
            return this->n_message;
        }

    private:
        ErrorKind n_kind;
        Str n_message;
    };

#if VIOLET_USE_RTTI
    // clang-format off
    std::variant<
        std::monostate,      //< used for the private default constructor, considered invalid.
        ErrorKind,           //< a error with just the error kind
        simple_message,      //< a error kind with message
        PlatformError,       //< a platform based error
        Pair<ErrorKind, Any> //< custom (if RTTI is enabled)
    > n_repr;
    // clang-format on
#else
    // clang-format off
    std::variant<
        std::monostate, //< used for the private default constructor, considered invalid.
        ErrorKind,      //< a error with just the error kind
        simple_message, //< a error kind with message
        PlatformError   //< a platform based error
    > n_repr;
    // clang-format on
#endif
};

template<typename T>
using Result = violet::Result<T, Error>;

} // namespace violet::io
