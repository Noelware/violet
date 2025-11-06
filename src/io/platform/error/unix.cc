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

#include <violet/Violet.h>

#ifdef VIOLET_UNIX

#include <violet/Container/Optional.h>
#include <violet/IO/Error.h>

#include <cerrno>

using violet::io::ErrorKind;
using violet::io::PlatformError;

PlatformError::PlatformError()
    : n_value(errno)
{
    VIOLET_DEBUG_ASSERT(errno > 0);
}

auto PlatformError::AsErrorKind() const noexcept -> ErrorKind
{
    switch (this->n_value) {
    case E2BIG:
        return ErrorKind::ArgumentListTooLong;
    case EADDRINUSE:
        return ErrorKind::AddrInUse;
    case EADDRNOTAVAIL:
        return ErrorKind::AddrNotAvailable;
    case EBUSY:
        return ErrorKind::ResourceBusy;
    case ECONNABORTED:
        return ErrorKind::ConnectionAborted;
    case ECONNREFUSED:
        return ErrorKind::ConnectionRefused;
    case ECONNRESET:
        return ErrorKind::ConnectionReset;
    case EDEADLK:
        return ErrorKind::Deadlock;
    case EDQUOT:
        return ErrorKind::QuotaExceeded;
    case EEXIST:
        return ErrorKind::AlreadyExists;
    case EFBIG:
        return ErrorKind::FileTooLarge;
    case EHOSTUNREACH:
        return ErrorKind::HostUnreachable;
    case EINTR:
        return ErrorKind::Interrupted;
    case EINVAL:
        return ErrorKind::InvalidInput;
    case EISDIR:
        return ErrorKind::IsADirectory;
    case ELOOP:
        return ErrorKind::FilesystemLoop;
    case ENOENT:
        return ErrorKind::NotFound;
    case ENOMEM:
        return ErrorKind::OutOfMemory;
    case ENOSPC:
        return ErrorKind::StorageFull;
    case ENOSYS:
        return ErrorKind::Unsupported;
    case EMLINK:
        return ErrorKind::TooManyLinks;
    case ENAMETOOLONG:
        return ErrorKind::InvalidFilename;
    case ENETDOWN:
        return ErrorKind::NetworkDown;
    case ENETUNREACH:
        return ErrorKind::NetworkUnreachable;
    case ENOTCONN:
        return ErrorKind::NotConnected;
    case ENOTDIR:
        return ErrorKind::NotADirectory;
    case ENOTEMPTY:
        return ErrorKind::DirectoryNotEmpty;
    case EPIPE:
        return ErrorKind::BrokenPipe;
    case EROFS:
        return ErrorKind::ReadOnlyFilesystem;
    case ESPIPE:
        return ErrorKind::NotSeekable;
    case ESTALE:
        return ErrorKind::StaleNetworkFileHandle;
    case ETIMEDOUT:
        return ErrorKind::TimedOut;
    case ETXTBSY:
        return ErrorKind::ExecutableFileBusy;
    case EXDEV:
        return ErrorKind::CrossesDevices;
    case EINPROGRESS:
        return ErrorKind::InProgress;
    case EOPNOTSUPP:
        return ErrorKind::Unsupported;
    case EACCES:
    case EPERM:
        return ErrorKind::PermissionDenied;
    }

    if (this->n_value == EAGAIN || this->n_value == EWOULDBLOCK) {
        return ErrorKind::WouldBlock;
    }

    return ErrorKind::__other;
}

auto PlatformError::Get() const noexcept -> PlatformError::error_type
{
    return this->n_value;
}

auto PlatformError::ToString() const noexcept -> String
{
    return strerror(this->n_value);
}

#endif
