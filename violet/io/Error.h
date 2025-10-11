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

#include "violet/container/Optional.h"
#include "violet/support/Demangle.h"
#include "violet/violet.h"

#include <any>
#include <utility>
#include <variant>

namespace Noelware::Violet::IO {

/// A list of general categories of I/O errors.
enum struct ErrorKind : uint8 {
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
};
} // namespace Noelware::Violet::IO

VIOLET_TO_STRING(const IO::ErrorKind&, self, {
    switch (self) {
    case IO::ErrorKind::AddrInUse:
        return "address in use";

    case IO::ErrorKind::AddrNotAvailable:
        return "address not avaliable";

    case IO::ErrorKind::AlreadyExists:
        return "entity already exists";

    case IO::ErrorKind::ArgumentListTooLong:
        return "argument list too long";

    case IO::ErrorKind::BrokenPipe:
        return "broken pipe";

    case IO::ErrorKind::ConnectionAborted:
        return "connection aborted";

    case IO::ErrorKind::ConnectionRefused:
        return "connection refused";

    case IO::ErrorKind::ConnectionReset:
        return "connection reset";

    case IO::ErrorKind::CrossesDevices:
        return "cross-device link or rename";

    case IO::ErrorKind::Deadlock:
        return "deadlock";

    case IO::ErrorKind::DirectoryNotEmpty:
        return "directory not empty";

    case IO::ErrorKind::ExecutableFileBusy:
        return "executable file busy";

    case IO::ErrorKind::FileTooLarge:
        return "file too large";

    case IO::ErrorKind::FilesystemLoop:
        return "filesystem loop or indirection limit (e.g. symlink loop)";

    case IO::ErrorKind::HostUnreachable:
        return "host unreachable";

    case IO::ErrorKind::InProgress:
        return "in progress";

    case IO::ErrorKind::Interrupted:
        return "operation interrupted";

    case IO::ErrorKind::InvalidData:
        return "invalid data";

    case IO::ErrorKind::InvalidFilename:
        return "invalid filename";

    case IO::ErrorKind::InvalidInput:
        return "invalid input parameter";

    case IO::ErrorKind::IsADirectory:
        return "is a directory";

    case IO::ErrorKind::NetworkDown:
        return "network down";

    case IO::ErrorKind::NetworkUnreachable:
        return "network unreachable";

    case IO::ErrorKind::NotADirectory:
        return "not a directory";

    case IO::ErrorKind::NotConnected:
        return "not connected";

    case IO::ErrorKind::NotFound:
        return "entity not found";

    case IO::ErrorKind::NotSeekable:
        return "seek on unseekable file";

    case IO::ErrorKind::Other:
        return "other error";

    case IO::ErrorKind::OutOfMemory:
        return "out of memory";

    case IO::ErrorKind::PermissionDenied:
        return "permission denied";

    case IO::ErrorKind::QuotaExceeded:
        return "quota exceeded";

    case IO::ErrorKind::ReadOnlyFilesystem:
        return "read-only filesystem or storage medium";

    case IO::ErrorKind::ResourceBusy:
        return "resource busy";

    case IO::ErrorKind::StaleNetworkFileHandle:
        return "stale network file handle";

    case IO::ErrorKind::StorageFull:
        return "no storage space";

    case IO::ErrorKind::TimedOut:
        return "timed out";

    case IO::ErrorKind::TooManyLinks:
        return "too many links";

    case IO::ErrorKind::UnexpectedEof:
        return "unexpected end of file";

    case IO::ErrorKind::Unsupported:
        return "unsupported";

    case IO::ErrorKind::WouldBlock:
        return "operation would block";

    case IO::ErrorKind::WriteZero:
        return "write zero";
    }
});

namespace Noelware::Violet::IO {
struct Error;

/// A platform-based I/O error. This shouldn't be used directly.
struct PlatformError final {
#ifdef _WIN32
    /// The type that is represented of `GetLastError()` for Windows
    using error_type = uint64;
#else
    /// The type that is represented of `errno` for Unix (macOS, Linux) systems
    using error_type = int32;
#endif

    /// Returns the value of this [`PlatformError`].
    [[nodiscard]] auto Value() const noexcept -> error_type
    {
        return this->n_value;
    }

    /// Returns the error message associated with the platform's I/O error. Can return an empty
    /// string if there is no support for this platform.
    [[nodiscard]] auto ToString() const -> String;

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
#ifdef _WIN32
        return this->n_value != 0;
#else
        if (this->n_value == -1) {
            return false;
        }

        return this->n_value != 0;
#endif
    }

private:
    friend struct Noelware::Violet::IO::Error;

    PlatformError();

    error_type n_value;
};

struct Error final {
    static auto Platform(ErrorKind kind) noexcept -> Error
    {
        return { kind, PlatformError() };
    }

    template<typename T, typename... Args>
    static auto New(ErrorKind kind, Args&&... args) -> Error
    {
        return Error{ std::in_place_t{}, kind, std::make_any<T>(VIOLET_FWD(Args, args)...) };
    }

    VIOLET_EXPLICIT Error(std::in_place_t, ErrorKind kind, std::any value)
        : n_kind(kind)
        , n_value(value)
    {
    }

    [[nodiscard]] auto RawOSError() const noexcept -> Optional<PlatformError::error_type>
    {
        try {
            auto platform = std::get<PlatformError>(this->n_value);
            return Some<PlatformError::error_type>(platform.Value());
        } catch (...) {
            return Nothing;
        }
    }

    [[nodiscard]] auto Kind() const noexcept -> ErrorKind
    {
        return this->n_kind;
    }

    template<typename T>
    [[nodiscard]] auto Downcast() const noexcept -> Optional<T>
    {
        // First, we need to check if we are in the `std::any` variant
        try {
            auto any = std::get<std::any>(this->n_value);
            try {
                auto downcasted = std::any_cast<T>(any);
                return Some<T>(downcasted);
            } catch (const std::bad_any_cast&) {
                return Nothing;
            }
        } catch (const std::bad_variant_access&) {
            return Nothing; // we are `PlatformError`, so we can't downcast
        }
    }

    VIOLET_OSTREAM_IMPL(const Error&)
    {
        os << "I/O error";

        try {
            auto platform = std::get<PlatformError>(self.n_value);
            os << " (system): ";
            os << platform.ToString();

            return os;
        } catch (const std::bad_variant_access&) {
            // continue
        }

        if (auto msg = self.Downcast<Str>()) {
            os << " [" << Noelware::Violet::ToString(self.n_kind) << "]";
            return os << " (custom): " << *msg;
        }

        if (auto msg = self.Downcast<String>()) {
            os << " [" << Noelware::Violet::ToString(self.n_kind) << "]";
            return os << " (custom): " << *msg;
        }

        os << ": " << Noelware::Violet::ToString(self.n_kind);

        const auto& type = std::get<Any>(self.n_value).type();
        return os << " «type '" << Utility::DemangleCXXName(type.name()) << '@' << type.hash_code() << "'»";
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        std::stringstream os;
        os << *this;

        return os.str();
    }

    VIOLET_IMPL_EQUALITY_SINGLE(Error, lhs, rhs, { return lhs.n_kind == rhs.n_kind; });

private:
    Error(ErrorKind kind, PlatformError platform)
        : n_kind(kind)
        , n_value(platform)
    {
    }

    ErrorKind n_kind;
    std::variant<PlatformError, std::any> n_value;
};

} // namespace Noelware::Violet::IO
