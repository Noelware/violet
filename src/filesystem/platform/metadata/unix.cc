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

#if VIOLET_PLATFORM(UNIX)

#include <violet/Filesystem/Metadata.h>
#include <violet/Filesystem/Path.h>

#if VIOLET_PLATFORM(LINUX)
#include <sys/sysmacros.h>
#elif VIOLET_PLATFORM(APPLE_MACOS)
#include <sys/types.h>
#endif

using violet::UInt64;
using violet::filesystem::Metadata;
using violet::filesystem::PathRef;
using violet::filesystem::SymlinkResolution;
using violet::io::FileDescriptor;
using Perms = violet::filesystem::Permissions;

#if VIOLET_PLATFORM(APPLE_MACOS)
#define ST_MTIM st.st_mtimespec
#define ST_ATIM st.st_atimespec
#else
#define ST_MTIM st.st_mtim
#define ST_ATIM st.st_atim
#endif

namespace {
auto timespecToMillis(const timespec& ts) noexcept -> UInt64
{
    return (static_cast<UInt64>(ts.tv_sec) * 1000ULL) + (static_cast<UInt64>(ts.tv_nsec) / 1'000'000ULL);
}

auto statToMetadata(struct stat st) -> Metadata
{
    Metadata mt;
    mt.Size = st.st_size;
    mt.ModifiedAt = timespecToMillis(ST_MTIM);
    mt.AccessedAt = timespecToMillis(ST_ATIM);
    mt.Permissions = Perms(st.st_mode);

#if VIOLET_PLATFORM(APPLE_MACOS)
    mt.CreatedAt = timespecToMillis(st.st_birthtimespec);
#endif

    mt.Inode = st.st_ino;
    mt.Device = st.st_dev;
    mt.DeviceMajor = major(st.st_dev);
    mt.DeviceMinor = minor(st.st_dev);

    // `Rdev` only makes sense for block/character device nodes; for everything else
    // `st_rdev` is zero, which would otherwise show up as `Some(0)` rather than
    // `Nothing` and break the contract documented on `Metadata::Rdev`.
    const auto kind = st.st_mode & S_IFMT;
    if (kind == S_IFBLK || kind == S_IFCHR) {
        mt.Rdev = st.st_rdev;
        mt.RdevMajor = major(st.st_rdev);
        mt.RdevMinor = minor(st.st_rdev);
    }

    mt.UserID = st.st_uid;
    mt.GroupID = st.st_gid;
    mt.HardLinks = st.st_nlink;

    return mt;
}

} // namespace

auto Metadata::For(FileDescriptor::value_type fd) -> io::Result<Metadata>
{
    struct stat st{ };
    if (::fstat(fd, &st) < 0) {
        return Err(io::Error::OSError());
    }

    auto mt = statToMetadata(st);
    switch (st.st_mode & S_IFMT) {
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

auto Metadata::For(PathRef path, SymlinkResolution resolution) -> io::Result<Metadata>
{
    struct stat st{ };
    if (resolution == SymlinkResolution::Follow) {
        if (path.WithCStr([&](CStr path) -> bool { return ::stat(path, &st) < 0; })) {
            return Err(io::Error::OSError());
        }
    } else {
        if (path.WithCStr([&](CStr path) -> bool { return ::lstat(path, &st) < 0; })) {
            return Err(io::Error::OSError());
        }
    }

    auto mt = statToMetadata(st);
    switch (st.st_mode & S_IFMT) {
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

#endif
