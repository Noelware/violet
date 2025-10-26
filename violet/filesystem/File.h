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

#include "violet/filesystem/Path.h"
#include "violet/filesystem/Permissions.h"
#include "violet/io/Descriptor.h"
#include "violet/io/Error.h"
#include "violet/support/Bitflags.h"
#include "violet/violet.h"

#ifdef VIOLET_WINDOWS
#    include <windows.h>
#elif defined(VIOLET_UNIX)
#    include <dirent.h>
#    include <sys/types.h>
#endif

namespace Noelware::Violet::Filesystem {

struct Path;
struct File;

struct FileType final {
    FileType() = default;

    constexpr auto File() const noexcept -> bool;
    constexpr auto Dir() const noexcept -> bool;
    constexpr auto Symlink() const noexcept -> bool;

#ifdef VIOLET_UNIX
    constexpr auto BlockDevice() const noexcept -> bool;
    constexpr auto CharDevice() const noexcept -> bool;
    constexpr auto FIFOPipe() const noexcept -> bool;
    constexpr auto UnixSocket() const noexcept -> bool;
#endif

private:
    friend struct File;

    enum class flag : uint8 {
        kFile = 1 << 0, ///< this is a file
        kDir = 1 << 1, ///< this is a directory
        kSymlink = 1 << 2, ///< this is a symlink

#ifdef VIOLET_UNIX
        kBlkDev = 1 << 3, ///< this is a character device
        kCharDev = 1 << 4, ///< this is a character device
        kFIFO = 1 << 5, ///< this is a named pipe (FIFO).
        kSocket = 1 << 6 ///< this is a unix socket
#endif
    };

    Bitflags<flag> n_tags;

    static constexpr auto mkfile(bool symlink = false) noexcept -> FileType;
    static constexpr auto mkdir(bool symlink = false) noexcept -> FileType;

#ifdef VIOLET_UNIX
    static constexpr auto mkblkdev(bool symlink = false) noexcept -> FileType;
    static constexpr auto mkchardev(bool symlink = false) noexcept -> FileType;
    static constexpr auto mkfifo(bool symlink = false) noexcept -> FileType;
    static constexpr auto mksocket(bool symlink = false) noexcept -> FileType;
#endif
};

struct OpenOptions final {
    OpenOptions() = default;

    auto Read(bool yes = true) noexcept -> OpenOptions&;
    auto Write(bool yes = true) noexcept -> OpenOptions&;
    auto Create(bool yes = true) noexcept -> OpenOptions&;
    auto Truncate(bool yes = true) noexcept -> OpenOptions&;
    auto Append(bool yes = true) noexcept -> OpenOptions&;
    auto CreateNew(bool yes = true) noexcept -> OpenOptions&;

#ifdef VIOLET_WINDOWS
    auto Attributes(DWORD) noexcept -> OpenOptions&;
#elif defined(VIOLET_UNIX)
    auto Mode(mode_t) noexcept -> OpenOptions&;
    auto Flags(int32) noexcept -> OpenOptions&;
#endif

    [[nodiscard]] auto Open(PathRef) -> IO::Result<File>;

private:
#ifdef VIOLET_WINDOWS
    DWORD n_attributes;
#elif defined(VIOLET_UNIX)
    mode_t n_mode;
    int32 n_flags;
#endif

    friend struct File;

    enum struct flag : uint8 {
        kRead = 1 << 0, //< marks this file as it should be opened for reading
        kWrite = 1 << 1, //< marks this file as it should be opened for writing
        kCreate = 1 << 2, //< ensures that this file is created before the handle is given
        kAppend = 1 << 3, //< appends new data rather than overwriting (kTruncate)
        kTruncate = 1 << 4, //< overwrites new data rather than appending
        kCreateNew = 1 << 5 //< atomically creates the file or fails if it already exists.
    };

    Bitflags<flag> n_bits;

    void populateFlag(const flag& flag, bool yes = true) noexcept
    {
        if (!this->n_bits.Contains(flag) && yes) {
            this->n_bits.Add(flag);
        } else if (this->n_bits.Contains(flag) && !yes) {
            this->n_bits.Remove(flag);
        }
    }
};

struct Metadata final {
    struct FileType FileType;
    struct Permissions Permissions;
    uint64 Size;
};

struct ScopedLock final {
    ScopedLock() = delete;
    ~ScopedLock();

    ScopedLock(const ScopedLock&) = delete;
    auto operator=(const ScopedLock&) -> ScopedLock& = delete;

    ScopedLock(ScopedLock&&) noexcept;
    auto operator=(ScopedLock&&) noexcept -> ScopedLock&;

private:
    friend struct File;

    VIOLET_EXPLICIT ScopedLock(File* ptr);

    File* n_file;
};

struct File final {
    constexpr File() noexcept = default;
    constexpr VIOLET_EXPLICIT File(PathRef) noexcept;

    static auto FromDescriptor(IO::Descriptor, OpenOptions = {}) noexcept -> IO::Result<File>;

    ~File();

    File(const File&) noexcept = delete;
    auto operator=(const File&) -> File& = delete;

    File(File&&) noexcept;
    auto operator=(File&&) noexcept -> File&;

    constexpr auto Valid() const noexcept -> bool;

    [[nodiscard]] auto Close() -> IO::Result<void>;
    [[nodiscard]] auto Open(const OpenOptions&) -> IO::Result<void>;
    [[nodiscard]] auto Lock() const -> IO::Result<void>;
    [[nodiscard]] auto SharedLock() const -> IO::Result<void>;
    [[nodiscard]] auto Unlock() const -> IO::Result<void>;
    [[nodiscard]] auto MkScopedLock() -> IO::Result<ScopedLock>;
    [[nodiscard]] auto MkSharedScopedLock() -> IO::Result<ScopedLock>;
    [[nodiscard]] auto Metadata(bool followSymlinks = false) const -> IO::Result<Metadata>;
    [[nodiscard]] auto Clone() const -> IO::Result<File>;
    [[nodiscard]] auto Path() const noexcept -> PathRef;
    [[nodiscard]] auto Descriptor() const noexcept -> IO::Descriptor;

    constexpr VIOLET_EXPLICIT operator bool() const noexcept;

private:
    friend struct OpenOptions;

    [[nodiscard]] static auto doOpen(PathRef, OpenOptions) -> IO::Result<File>;

    IO::Descriptor n_descriptor;
    PathRef n_path;
};

} // namespace Noelware::Violet::Filesystem
