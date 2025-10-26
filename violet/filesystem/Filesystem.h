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

#include "violet/filesystem/File.h"
#include "violet/filesystem/Path.h"
#include "violet/filesystem/Permissions.h"
#include "violet/io/Error.h"
#include "violet/violet.h"

#ifdef VIOLET_WINDOWS
#    include <windows.h>
#elif defined(VIOLET_UNIX)
#    include <dirent.h>
#endif

namespace Noelware::Violet::Filesystem {

struct Dirs;
struct WalkDirs;

// START   :: FUNCTIONS (DIRECTORY)

auto CreateDirectory(const PathRef&) -> IO::Result<void>;
auto CreateDirectories(const PathRef&) -> IO::Result<void>;
auto RemoveDirectory(const PathRef&) -> IO::Result<void>;
auto RemoveAllDirs(const PathRef&) -> IO::Result<void>;
auto ReadDir(const PathRef&) -> IO::Result<Dirs>;
auto WalkDir(const PathRef&) -> IO::Result<WalkDirs>;

// END   :: FUNCTIONS (DIRECTORY)
// START :: FUNCTIONS (FILES)

auto CreateFile(const PathRef&) -> IO::Result<File>;
auto Canonicalize(const PathRef&) -> IO::Result<Path>;
auto Copy(const PathRef&, const PathRef&) -> IO::Result<uint64>;
auto Exists(const PathRef&) -> IO::Result<bool>;
auto Metadata(const PathRef&, bool symlinks = false) -> IO::Result<Metadata>;
auto RemoveFile(const PathRef&) -> IO::Result<void>;
auto Rename(const PathRef&, const PathRef&) -> IO::Result<void>;
auto SetPermissions(const PathRef&, Permissions) -> IO::Result<void>;

// END   :: FUNCTIONS (FILES)

struct DirEntry final {};
struct Dirs final {};
struct WalkDirs final {};

} // namespace Noelware::Violet::Filesystem

/*
struct Dirs;
struct WalkDirs;

auto CreateDirectory(const Path& path) -> IO::Result<void>;
auto CreateDirectories(const Path& path) -> IO::Result<void>;
auto ReadDir(const Path& path) -> IO::Result<Dirs>;
auto WalkDir(const Path& path) -> IO::Result<WalkDirs>;
auto RemoveDirectory(const Path& path) -> IO::Result<void>;
auto RemoveAllDirs(const Path& path) -> IO::Result<void>;
auto Exists(const Path& path) -> IO::Result<bool>;

struct FileType final {
    FileType() = default;

    static constexpr auto MkFile(bool symlink = false) noexcept -> FileType
    {
        FileType ft;
        ft.n_tag.Add(Tag::kFile);

        if (symlink) {
            ft.n_tag.Add(Tag::kSymlink);
        }

        return ft;
    }

    static constexpr auto MkDir(bool symlink = false) noexcept -> FileType
    {
        FileType ft;
        ft.n_tag.Add(Tag::kDir);

        if (symlink) {
            ft.n_tag.Add(Tag::kSymlink);
        }

        return ft;
    }

    static constexpr auto MkBlockDevice() noexcept -> FileType
    {
        FileType ft;
        ft.n_tag.Add(Tag::kBlockDevice);

        return ft;
    }

    static constexpr auto MkCharDevice() noexcept -> FileType
    {
        FileType ft;
        ft.n_tag.Add(Tag::kCharDevice);

        return ft;
    }

    static constexpr auto MkFIFOPipe() noexcept -> FileType
    {
        FileType ft;
        ft.n_tag.Add(Tag::kFIFOPipe);

        return ft;
    }

    static constexpr auto MkUnixSocket() noexcept -> FileType
    {
        FileType ft;
        ft.n_tag.Add(Tag::kFIFOPipe);

        return ft;
    }

    [[nodiscard]] constexpr auto File() const noexcept -> bool
    {
        return this->n_tag.Contains(Tag::kFile);
    }

    [[nodiscard]] constexpr auto Dir() const noexcept -> bool
    {
        return this->n_tag.Contains(Tag::kDir);
    }

    [[nodiscard]] constexpr auto Symlink() const noexcept -> bool
    {
        return this->n_tag.Contains(Tag::kSymlink);
    }

    [[nodiscard]] constexpr auto BlockDevice() const noexcept -> bool
    {
        return this->n_tag.Contains(Tag::kBlockDevice);
    }

    [[nodiscard]] constexpr auto CharDevice() const noexcept -> bool
    {
        return this->n_tag.Contains(Tag::kCharDevice);
    }

    [[nodiscard]] constexpr auto FIFOPipe() const noexcept -> bool
    {
        return this->n_tag.Contains(Tag::kFIFOPipe);
    }

    [[nodiscard]] constexpr auto UnixSock() const noexcept -> bool
    {
        return this->n_tag.Contains(Tag::kUnixSock);
    }

private:
    enum struct Tag : uint8 {
        kFile = 1 << 0, ///< this is a file
        kDir = 1 << 1, ///< this is a directory
        kSymlink = 1 << 2, ///< this is a symlink
        kBlockDevice = 1 << 3, ///< this is a block device
        kCharDevice = 1 << 4, ///< this is a character device
        kFIFOPipe = 1 << 5, ///< this is a named pipe (FIFO).
        kUnixSock = 1 << 6 ///< this is a unix socket
    };

    Bitflags<Tag> n_tag = Bitflags<Tag>(0);
};

/// A entry from the [`Dirs`] or [`WalkDirs`] iterators.
struct DirEntry final {
    struct Path Path; ///< path of this entry
    struct FileType FileType; ///< the file type of this entry
};

/// A iterator that iterates through a list of files in a directory. You create this
/// with the [`Noelware::Violet::Filesystem::ReadDir`] function.
struct Dirs final: public Iterator<Dirs> {
    Dirs() = delete;

    auto Next() noexcept -> Optional<IO::Result<DirEntry>>;

private:
    friend auto Noelware::Violet::Filesystem::ReadDir(const Path&) -> IO::Result<Dirs>;

    Path n_parent;

#ifdef VIOLET_WINDOWS
    HANDLE n_handle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW n_iter;
    bool n_valid = false;
#elif defined(VIOLET_UNIX)
    DIR* n_dir = nullptr; ///< directory handle

    VIOLET_EXPLICIT Dirs(Path parent, DIR* ptr)
        : n_parent(VIOLET_MOVE(parent))
        , n_dir(ptr)
    {
    }
#endif
};

struct WalkDirs final {
    WalkDirs() = delete;

    auto Next() noexcept -> Optional<IO::Result<DirEntry>>;

private:
    friend auto Noelware::Violet::Filesystem::ReadDir(const Path&) -> IO::Result<Dirs>;

    Path n_parent;

#ifdef VIOLET_WINDOWS
    HANDLE n_handle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW n_iter;
    bool n_valid = false;
#elif defined(VIOLET_UNIX)
    DIR* n_dir = nullptr; ///< directory handle

    VIOLET_EXPLICIT WalkDirs(Path parent, DIR* ptr)
        : n_parent(VIOLET_MOVE(parent))
        , n_dir(ptr)
    {
    }
#endif
};
*/
