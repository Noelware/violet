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

#if VIOLET_PLATFORM(LINUX)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <violet/Filesystem/Metadata.h>
#include <violet/Filesystem/Path.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

using violet::UInt64;
using violet::filesystem::Metadata;
using violet::filesystem::PathRef;
using violet::filesystem::SymlinkResolution;
using violet::io::FileDescriptor;
using Perms = violet::filesystem::Permissions;

namespace {
auto statxTimestampToMillis(const statx_timestamp& ts) -> UInt64
{
    return (static_cast<UInt64>(ts.tv_sec) * 1000ULL) + (static_cast<UInt64>(ts.tv_nsec) / 1'000'000ULL);
}

auto statxToMetadata(const struct statx& st) -> Metadata
{
    struct Metadata mt;
    mt.Size = st.stx_size;
    mt.ModifiedAt = statxTimestampToMillis(st.stx_mtime);
    mt.AccessedAt = statxTimestampToMillis(st.stx_atime);
    if ((st.stx_mask & STATX_BTIME) != 0U) {
        mt.CreatedAt = statxTimestampToMillis(st.stx_btime);
    }

    mt.Permissions = Perms(st.stx_mode);
    mt.Inode = st.stx_ino;
    mt.Device = makedev(st.stx_dev_major, st.stx_dev_minor);
    mt.DeviceMajor = st.stx_dev_major;
    mt.DeviceMinor = st.stx_dev_minor;

    // `Rdev` only makes sense for block/character device nodes; for everything else
    // statx zero-fills the rdev fields, which would otherwise show up as `Some(0)`
    // rather than `Nothing` and break the contract documented on `Metadata::Rdev`.
    const auto kind = st.stx_mode & S_IFMT;
    if (kind == S_IFBLK || kind == S_IFCHR) {
        mt.Rdev = makedev(st.stx_rdev_major, st.stx_rdev_minor);
        mt.RdevMajor = st.stx_rdev_major;
        mt.RdevMinor = st.stx_rdev_minor;
    }

    mt.UserID = st.stx_uid;
    mt.GroupID = st.stx_gid;
    mt.HardLinks = st.stx_nlink;

    return mt;
}
} // namespace

auto Metadata::For(FileDescriptor::value_type dirfd, PathRef path, SymlinkResolution resolution) -> io::Result<Metadata>
{
    struct statx st{ };
    Int32 flags = 0;
    if (path.Empty()) {
        flags |= AT_EMPTY_PATH;
    }

    if (resolution == SymlinkResolution::NoFollow) {
        flags |= AT_SYMLINK_NOFOLLOW;
    }

    if (path.WithCStr([&](CStr path) -> bool {
            return ::statx(dirfd, path, flags, STATX_BASIC_STATS | STATX_BTIME, &st) != -1;
        })) {
        auto mt = statxToMetadata(st);
        switch (st.stx_mode & S_IFMT) {
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

auto Metadata::For(FileDescriptor::value_type fd) -> io::Result<Metadata>
{
    return For(fd, "");
}

auto Metadata::For(PathRef path, SymlinkResolution resolution) -> io::Result<Metadata>
{
    struct statx st{ };
    const UInt32 mask = STATX_BASIC_STATS | STATX_BTIME;
    Int32 flags = 0;
    if (resolution == SymlinkResolution::NoFollow) {
        flags |= AT_SYMLINK_NOFOLLOW;
    }

    if (path.WithCStr([&](CStr data) -> bool { return ::statx(AT_FDCWD, data, flags, mask, &st) != -1; })) {
        auto mt = statxToMetadata(st);
        switch (st.stx_mode & S_IFMT) {
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
