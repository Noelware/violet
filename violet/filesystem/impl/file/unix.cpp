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
#include "violet/io/Descriptor.h"
#include "violet/filesystem/File.h"
#include "violet/filesystem/Path.h"
#include "violet/io/Error.h"

#include <fcntl.h>
#include <sys/types.h>
// clang-format on

using Noelware::Violet::Filesystem::File;
using Noelware::Violet::Filesystem::FileType;
using Noelware::Violet::Filesystem::Metadata;
using Noelware::Violet::Filesystem::OpenOptions;
using Noelware::Violet::Filesystem::PathRef;
using Noelware::Violet::Filesystem::ScopedLock;

////// --=-- START :: FileType --=-- //////

constexpr auto FileType::File() const noexcept -> bool
{
    return this->n_tags.Contains(flag::kFile);
}

constexpr auto FileType::Dir() const noexcept -> bool
{
    return this->n_tags.Contains(flag::kDir);
}

constexpr auto FileType::Symlink() const noexcept -> bool
{
    return this->n_tags.Contains(flag::kSymlink);
}

constexpr auto FileType::BlockDevice() const noexcept -> bool
{
    return this->n_tags.Contains(flag::kBlkDev);
}

constexpr auto FileType::CharDevice() const noexcept -> bool
{
    return this->n_tags.Contains(flag::kCharDev);
}

constexpr auto FileType::FIFOPipe() const noexcept -> bool
{
    return this->n_tags.Contains(flag::kFIFO);
}

constexpr auto FileType::UnixSocket() const noexcept -> bool
{
    return this->n_tags.Contains(flag::kSocket);
}

constexpr auto FileType::mkfile(bool symlink) noexcept -> FileType
{
    FileType ft{};
    ft.n_tags.Add(flag::kFile);

    if (symlink) {
        ft.n_tags.Add(flag::kSymlink);
    }

    return ft;
}

constexpr auto FileType::mkdir(bool symlink) noexcept -> FileType
{
    FileType ft{};
    ft.n_tags.Add(flag::kDir);

    if (symlink) {
        ft.n_tags.Add(flag::kSymlink);
    }

    return ft;
}

constexpr auto FileType::mkblkdev(bool symlink) noexcept -> FileType
{
    FileType ft{};
    ft.n_tags.Add(flag::kBlkDev);

    if (symlink) {
        ft.n_tags.Add(flag::kSymlink);
    }

    return ft;
}

constexpr auto FileType::mkchardev(bool symlink) noexcept -> FileType
{
    FileType ft{};
    ft.n_tags.Add(flag::kCharDev);

    if (symlink) {
        ft.n_tags.Add(flag::kSymlink);
    }

    return ft;
}

constexpr auto FileType::mkfifo(bool symlink) noexcept -> FileType
{
    FileType ft{};
    ft.n_tags.Add(flag::kFIFO);

    if (symlink) {
        ft.n_tags.Add(flag::kSymlink);
    }

    return ft;
}

constexpr auto FileType::mksocket(bool symlink) noexcept -> FileType
{
    FileType ft{};
    ft.n_tags.Add(flag::kSocket);

    if (symlink) {
        ft.n_tags.Add(flag::kSymlink);
    }

    return ft;
}

////// --=-- END :: FileType --=-- //////

////// --=-- START :: OpenOptions --=-- //////

auto OpenOptions::Read(bool yes) noexcept -> OpenOptions&
{
    this->populateFlag(flag::kRead, yes);
    return *this;
}

auto OpenOptions::Write(bool yes) noexcept -> OpenOptions&
{
    this->populateFlag(flag::kWrite, yes);
    return *this;
}

auto OpenOptions::Create(bool yes) noexcept -> OpenOptions&
{
    this->populateFlag(flag::kCreate, yes);
    return *this;
}

auto OpenOptions::Append(bool yes) noexcept -> OpenOptions&
{
    this->populateFlag(flag::kAppend, yes);
    return *this;
}

auto OpenOptions::Truncate(bool yes) noexcept -> OpenOptions&
{
    this->populateFlag(flag::kTruncate, yes);
    return *this;
}

auto OpenOptions::CreateNew(bool yes) noexcept -> OpenOptions&
{
    this->populateFlag(flag::kCreateNew, yes);
    return *this;
}

auto OpenOptions::Mode(mode_t mode) noexcept -> OpenOptions&
{
    this->n_mode = mode;
    return *this;
}

auto OpenOptions::Flags(int32 flags) noexcept -> OpenOptions&
{
    this->n_flags |= flags;
    return *this;
}

auto OpenOptions::Open(PathRef path) -> IO::Result<File>
{
    auto ret = File::doOpen(path, *this);
    if (!ret) {
        return ret.Error();
    }

    return VIOLET_MOVE(ret.Value());
}

////// --=-- END :: OpenOptions --=-- //////

////// --=-- START :: ScopedLock --=-- //////

ScopedLock::ScopedLock(File* ptr)
    : n_file(ptr)
{
}

ScopedLock::~ScopedLock()
{
    if (this->n_file != nullptr) {
        std::ignore = this->n_file->Unlock();
        std::ignore = this->n_file->Close();
    }
}

ScopedLock::ScopedLock(ScopedLock&& other) noexcept
    : n_file(std::exchange(other.n_file, nullptr))
{
}

auto ScopedLock::operator=(ScopedLock&& other) noexcept -> ScopedLock&
{
    if (this != &other) {
        if (this->n_file != nullptr) {
            std::ignore = this->n_file->Unlock();
            std::ignore = this->n_file->Close();
        }

        this->n_file = std::exchange(other.n_file, nullptr);
    }

    return *this;
}

////// --=-- END :: ScopedLock --=-- //////

////// --=-- START :: File --=-- //////

constexpr File::File(PathRef path) noexcept
    : n_path(path)
{
}

File::~File()
{
    std::ignore = this->Close();
}

File::File(File&& other) noexcept
    : n_descriptor(std::exchange(other.n_descriptor, {}))
    , n_path(std::exchange(other.n_path, {}))
{
}

auto File::operator=(File&& other) noexcept -> File&
{
    if (this != &other) {
        std::ignore = this->Close();

        n_descriptor = std::exchange(other.n_descriptor, {});
        n_path = std::exchange(other.n_path, {});
    }

    return *this;
}

constexpr auto File::Valid() const noexcept -> bool
{
    return this->n_descriptor.Valid();
}

constexpr File::operator bool() const noexcept
{
    return this->Valid();
}

auto File::Close() -> IO::Result<void>
{
    if (this->Valid()) {
        if (::close(this->n_descriptor.AsFD()) == -1) {
            return IO::Error::Platform(IO::ErrorKind::Other);
        }
    }

    return {};
}

auto File::Open(const OpenOptions& opts) -> IO::Result<void>
{
    if (!this->Valid()) {
        auto file = File::doOpen(this->n_path, opts);
        if (!file) {
            return file.Error();
        }

        *this = VIOLET_MOVE(file.Value());
    }

    return {};
}

auto File::Lock() const -> IO::Result<void>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto File::SharedLock() const -> IO::Result<void>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto File::Unlock() const -> IO::Result<void>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto File::MkScopedLock() -> IO::Result<ScopedLock>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto File::MkSharedScopedLock() -> IO::Result<ScopedLock>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto File::Metadata(bool) const -> IO::Result<Filesystem::Metadata>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto File::Clone() const -> IO::Result<File>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto File::doOpen(PathRef path, OpenOptions opts) -> IO::Result<File>
{
    // Build the flags
    int32 flags = 0;
    if (opts.n_bits.Contains(OpenOptions::flag::kRead) && opts.n_bits.Contains(OpenOptions::flag::kRead)) {
        flags |= O_RDWR;
    } else if (opts.n_bits.Contains(OpenOptions::flag::kRead)) {
        flags |= O_RDONLY;
    } else if (opts.n_bits.Contains(OpenOptions::flag::kWrite)) {
        flags |= O_WRONLY;
    }

    if (opts.n_bits.Contains(OpenOptions::flag::kAppend)) {
        flags |= O_APPEND;
    }

    if (opts.n_bits.Contains(OpenOptions::flag::kCreate)) {
        flags |= O_CREAT;
    }

    if (opts.n_bits.Contains(OpenOptions::flag::kTruncate)) {
        flags |= O_TRUNC;
    }

    if (opts.n_bits.Contains(OpenOptions::flag::kCreateNew)) {
        flags |= O_CREAT | O_EXCL;
    }

    if (opts.n_flags != 0) {
        flags |= opts.n_flags;
    }

    int fd = ::open(static_cast<StringRef>(path), flags, opts.n_mode);
    if (fd == -1) {
        return IO::Error::Platform(IO::ErrorKind::Other);
    }

    File file;
    file.n_descriptor = IO::Descriptor(fd);
    file.n_path = path;

    return file;
}

auto File::Path() const noexcept -> PathRef
{
    return this->n_path;
}

auto File::Descriptor() const noexcept -> IO::Descriptor
{
    return this->n_descriptor;
}

#endif
