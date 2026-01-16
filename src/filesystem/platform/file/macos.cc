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

#include <violet/Violet.h>

#ifdef VIOLET_APPLE_MACOS

#include <violet/Filesystem/File.h>

#include <cerrno>
#include <sys/file.h>
#include <sys/stat.h>

using violet::filesystem::File;

auto File::Lock() const noexcept -> io::Result<void>
{
    if (!this->Valid()) {
        return VIOLET_IO_ERROR(InvalidInput, String, "current file is not valid");
    }

    if (::flock(this->n_fd.Get(), LOCK_EX) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto File::SharedLock() const noexcept -> io::Result<void>
{
    if (!this->Valid()) {
        return VIOLET_IO_ERROR(InvalidInput, String, "current file is not valid");
    }

    if (::flock(this->n_fd.Get(), LOCK_SH) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto File::Unlock() const noexcept -> io::Result<void>
{
    if (!this->Valid()) {
        return VIOLET_IO_ERROR(InvalidInput, String, "current file is not valid");
    }

    if (::flock(this->n_fd.Get(), LOCK_UN) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto File::Locked() const noexcept -> io::Result<bool>
{
    if (::flock(this->n_fd.Get(), LOCK_SH | LOCK_NB) == 0) {
        ::flock(this->n_fd.Get(), LOCK_UN);
        return false;
    }

    return errno == EWOULDBLOCK;
}

#endif
