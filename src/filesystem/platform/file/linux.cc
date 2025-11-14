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

#include <cerrno>
#include <violet/Violet.h>

#ifdef VIOLET_LINUX

#include <violet/Filesystem/File.h>

#include <sys/file.h>
#include <sys/stat.h>

using violet::UInt64;
using violet::filesystem::File;

namespace {

auto getMillisecondsFromTimespec(const statx_timestamp& ts) noexcept -> UInt64
{
    return (static_cast<UInt64>(ts.tv_sec) * 1000ULL) + (static_cast<UInt64>(ts.tv_nsec) / 1'000'000ULL);
}

} // namespace

auto File::Lock() const noexcept -> io::Result<void>
{
    if (!this->Valid()) {
#if VIOLET_USE_RTTI
        return Err(io::Error::New<String>(io::ErrorKind::InvalidInput, "file is not valid"));
#else
        return Err(io::Error(io::ErrorKind::InvalidInput));
#endif
    }

    if (::flock(this->n_fd.Get(), LOCK_EX) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto File::SharedLock() const noexcept -> io::Result<void>
{
    if (!this->Valid()) {
#if VIOLET_USE_RTTI
        return Err(io::Error::New<String>(io::ErrorKind::InvalidInput, "file is not valid"));
#else
        return Err(io::Error(io::ErrorKind::InvalidInput));
#endif
    }

    if (::flock(this->n_fd.Get(), LOCK_SH) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto File::Unlock() const noexcept -> io::Result<void>
{
    if (!this->Valid()) {
#if VIOLET_USE_RTTI
        return Err(io::Error::New<String>(io::ErrorKind::InvalidInput, "file is not valid"));
#else
        return Err(io::Error(io::ErrorKind::InvalidInput));
#endif
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

auto File::Metadata() const noexcept -> io::Result<struct Metadata>
{
    struct statx sx{};
    if (::statx(this->n_fd.Get(), "", AT_EMPTY_PATH, STATX_BASIC_STATS | STATX_BTIME, &sx) != -1) {
        struct Metadata mt;
        mt.Size = sx.stx_size;
        mt.ModifiedAt = getMillisecondsFromTimespec(sx.stx_mtime);
        mt.AccessedAt = Some<UInt64>(getMillisecondsFromTimespec(sx.stx_atime));

        if ((sx.stx_mask & STATX_BTIME) != 0U) {
            mt.CreatedAt = Some<UInt64>(getMillisecondsFromTimespec(sx.stx_btime));
        }

        mt.Permissions = Permissions(sx.stx_mode);

        switch (sx.stx_mode & S_IFMT) {
        case S_IFREG:
            mt.Type = FileType::mkfile();
            break;

        case S_IFDIR:
            mt.Type = FileType::mkdir();
            break;

        case S_IFLNK:
            mt.Type = FileType::mksymlink();
            break;

        case S_IFCHR:
            mt.Type = FileType::mkchardev();
            break;

        case S_IFBLK:
            mt.Type = FileType::mkblkdev();
            break;

        case S_IFIFO:
            mt.Type = FileType::mkfifo();
            break;

        case S_IFSOCK:
            mt.Type = FileType::mksocket();
            break;

        default:
            break;
        }

        return mt;
    }

    return Err(io::Error::OSError());
}

#endif
