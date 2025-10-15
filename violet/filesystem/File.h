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
#include "violet/io/Error.h"
#include "violet/iter/Iterator.h"
#include "violet/violet.h"

#include <utility>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <dirent.h>
#    include <sys/types.h>
#endif

namespace Noelware::Violet::Filesystem {

struct OpenOptions;
struct ScopedLock;

struct Metadata final {
    uint64 Size = 0;
    uint64 Modified = 0;
    uint64 Created = 0;
    uint64 Accessed = 0;

    bool Readonly = false;
    bool Symlink = false;
    bool Directory = false;

    [[nodiscard]] constexpr auto File() const noexcept -> bool
    {
        return !this->Directory;
    }
};

/// A RAII-based wrapper around Unix file descriptors or Windows' `HANDLE`.
struct File final {
    File() = delete;
    File(Path path)
        : n_path(VIOLET_MOVE(path))
    {
    }

    ~File()
    {
        this->Close();
    }

    File(const File&) = delete;
    auto operator=(const File&) -> File& = delete;

    File(File&& other) noexcept
    {
        *this = VIOLET_MOVE(other);
    }

    auto operator=(File&& other) noexcept -> File&
    {
        if (this != &other) {
            this->Close();

#ifdef _WIN32
            this->n_handle = other.n_handle;
            other.n_handle = INVALID_HANDLE_VALUE;
#else
            this->n_fd = other.n_fd;
            other.n_fd = -1;
#endif
        }

        return *this;
    }

    static auto Open(Path path, OpenOptions opts) -> IO::Result<File>;

    [[nodiscard]] auto Valid() const noexcept -> bool
    {
#ifdef _WIN32
        return this->n_handle == INVALID_HANDLE_VALUE;
#else
        return this->n_fd != -1;
#endif
    }

    VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->Valid();
    }

    void Close();

    [[nodiscard]] auto Lock() const noexcept -> IO::Result<void>;
    [[nodiscard]] auto SharedLock() const noexcept -> IO::Result<void>;
    [[nodiscard]] auto Unlock() const noexcept -> IO::Result<void>;

    [[nodiscard]] auto ScopedLock() -> IO::Result<ScopedLock>;
    [[nodiscard]] auto ScopedSharedLock() -> IO::Result<struct ScopedLock>;

    [[nodiscard]] auto Metadata(bool followSymlinks = true) const noexcept -> IO::Result<Metadata>;
    [[nodiscard]] auto Clone() const noexcept -> IO::Result<File>;

private:
    friend struct OpenOptions;

#ifdef _WIN32
    HANDLE n_handle = INVALID_HANDLE_VALUE;
#else
    constexpr static auto n_invalidFd = -1;
    int32 n_fd = n_invalidFd;
#endif

    Path n_path;

    [[nodiscard]] auto doOpen() const noexcept -> IO::Result<void>;
};

struct ScopedLock final {
    ScopedLock() = delete;
    ~ScopedLock() noexcept(false)
    {
        if (this->n_file != nullptr) {
            auto res = this->n_file->Unlock(); // NOLINT(readability-identifier-length)
            if (!res) {
                throw VIOLET_MOVE(res.Error());
            }
        }
    }

    ScopedLock(const ScopedLock&) = delete;
    auto operator=(const ScopedLock&) -> ScopedLock& = delete;

    ScopedLock(ScopedLock&& other) noexcept
        : n_file(std::exchange(other.n_file, nullptr))
    {
    }

    auto operator=(ScopedLock&& other) noexcept(false) -> ScopedLock&
    {
        if (this != &other) {
            auto res = this->n_file->Unlock(); // NOLINT(readability-identifier-length)
            if (!res) {
                throw VIOLET_MOVE(res.Error());
            }

            this->n_file = std::exchange(other.n_file, nullptr);
        }

        return *this;
    }

private:
    friend struct File;

    VIOLET_EXPLICIT ScopedLock(File* file)
        : n_file(file)
    {
    }

    File* n_file;
};

/// Builder for opening a [`File`], analgous to Rust's [`std::fs::OpenOptions`].
///
/// [`std::fs::OpenOptions`]: https://doc.rust-lang.org/1.90.0/std/fs/struct.OpenOptions.html
struct OpenOptions final {
    OpenOptions() = default;

    auto Read(bool yes = true) noexcept -> OpenOptions&
    {
        populateFlag(Flag::kRead, yes);
        return *this;
    }

    auto Write(bool yes = true) noexcept -> OpenOptions&
    {
        populateFlag(Flag::kWrite, yes);
        return *this;
    }

    auto Create(bool yes = true) noexcept -> OpenOptions&
    {
        populateFlag(Flag::kCreate, yes);
        return *this;
    }

    auto Append(bool yes = true) noexcept -> OpenOptions&
    {
        populateFlag(Flag::kAppend, yes);
        return *this;
    }

    auto Truncate(bool yes = true) noexcept -> OpenOptions&
    {
        populateFlag(Flag::kTruncate, yes);
        return *this;
    }

    auto CreateNew(bool yes = true) noexcept -> OpenOptions&
    {
        populateFlag(Flag::kCreateNew, yes);
        return *this;
    }

#ifdef _WIN32
    auto Attributes(DWORD attributes) noexcept -> OpenOptions&
    {
        this->n_attributes = attributes;
        return *this;
    }
#else
    auto Mode(mode_t mode) noexcept -> OpenOptions&
    {
        this->n_mode = mode;
        return *this;
    }
#endif

    [[nodiscard]] auto Open(Path path) const noexcept -> IO::Result<File>
    {
        File file(VIOLET_MOVE(path));
        auto res = file.doOpen();
        if (!res) {
            return Err(res.Error());
        }

        return file;
    }

private:
    enum struct Flag : uint8 {
        kRead = 1 << 0, //< marks this file as it should be opened for reading
        kWrite = 1 << 1, //< marks this file as it should be opened for writing
        kCreate = 1 << 2, //< ensures that this file is created before the handle is given
        kAppend = 1 << 3, //< appends new data rather than overwriting (kTruncate)
        kTruncate = 1 << 4, //< overwrites new data rather than appending
        kCreateNew = 1 << 5 //< atomically creates the file or fails if it already exists.
    };

    uint8 n_flags = 0;

#ifdef _WIN32
    DWORD n_attributes = FILE_ATTRIBUTE_NORMAL;
#else
    mode_t n_mode = 0644;
#endif

    [[nodiscard]] auto containsFlag(const Flag& flag) const noexcept -> bool
    {
        return (n_flags & static_cast<uint8>(flag)) != 0;
    }

    void populateFlag(const Flag& flag, bool yes = true) noexcept
    {
        if (!containsFlag(flag) && yes) {
            this->n_flags |= static_cast<uint8>(flag);
        } else if (containsFlag(flag) && !yes) {
            this->n_flags &= static_cast<uint8>(flag);
        }
    }
};

inline auto File::Open(Path path, OpenOptions opts) -> IO::Result<File>
{
    return opts.Open(VIOLET_MOVE(path));
}

/* iterators */

/// [`Iterator`] that iterates through a list of paths from its parent path.
struct Dirs final: public Iterator<Dirs> {
    Dirs() = delete;

    VIOLET_EXPLICIT Dirs(Path parent)
        : n_parent(VIOLET_MOVE(parent))
    {
    }

    auto Next() noexcept -> Optional<Path>;

private:
    Path n_parent;

#ifdef _WIN32
    HANDLE n_handle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW n_iter;
    bool n_valid = false;
#else
    DIR* n_iter = nullptr;
#endif
};

/* free-hand functions */

/// Creates a directory that points to `path`.
auto CreateDirectory(const Path& path) -> IO::Result<void>;

/// Creates a directory and recursively create directories of parents if they don't exist.
auto CreateAllDirs(const Path& path) -> IO::Result<void>;

/// Creates a iterator that lists all the files in a directory.
auto ReadDir(const Path& path) -> IO::Result<Dirs>;

auto RemoveDirectory(const Path& path) -> IO::Result<void>;
auto RemoveAll(const Path& path) -> IO::Result<void>;

} // namespace Noelware::Violet::Filesystem
