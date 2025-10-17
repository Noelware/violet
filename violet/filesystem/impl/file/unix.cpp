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

#include "violet/violet.h"

#ifdef VIOLET_UNIX

// clang-format off
#include "violet/filesystem/File.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
// clang-format on

auto Noelware::Violet::Filesystem::File::doOpen() const noexcept -> IO::Result<void>
{
    return {};
}

void Noelware::Violet::Filesystem::File::Close()
{
    if (this->n_fd >= 0) {
        ::close(this->n_fd);
        this->n_fd = -1;
    }
}

auto Noelware::Violet::Filesystem::File::Lock() const noexcept -> IO::Result<void>
{
    if (flock(this->n_fd, LOCK_EX) == -1) {
        return IO::Error::Platform(IO::ErrorKind::ExecutableFileBusy);
    }

    return {};
}

auto Noelware::Violet::Filesystem::File::SharedLock() const noexcept -> IO::Result<void>
{
    if (flock(this->n_fd, LOCK_SH) == -1) {
        return IO::Error::Platform(IO::ErrorKind::ExecutableFileBusy);
    }

    return {};
}

auto Noelware::Violet::Filesystem::File::Unlock() const noexcept -> IO::Result<void>
{
    if (flock(this->n_fd, LOCK_UN) == -1) {
        return IO::Error::Platform(IO::ErrorKind::Other);
    }

    return {};
}

auto Noelware::Violet::Filesystem::File::ScopedLock() -> IO::Result<struct ScopedLock>
{
    if (this->Lock()) {
        return Noelware::Violet::Filesystem::ScopedLock(this);
    }

    return IO::Error::Platform(IO::ErrorKind::ExecutableFileBusy);
}

auto Noelware::Violet::Filesystem::File::ScopedSharedLock() -> IO::Result<struct ScopedLock>
{
    if (this->SharedLock()) {
        return Noelware::Violet::Filesystem::ScopedLock(this);
    }

    return IO::Error::Platform(IO::ErrorKind::ExecutableFileBusy);
}

auto Noelware::Violet::Filesystem::File::Clone() const noexcept -> IO::Result<File>
{
    int32 dupfd = 0;
    if ((dupfd = fcntl(this->n_fd, F_DUPFD_CLOEXEC, 0)) == -1) {
        return IO::Error::Platform(IO::ErrorKind::ExecutableFileBusy);
    }

    auto res = Path::FromFileDescriptor(dupfd);
    assert(res.IsOk());

    File file(res.Value());
    file.n_fd = dupfd;

    return file;
}

#endif
