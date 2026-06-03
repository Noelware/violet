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

#pragma once

#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Metadata.h>
#include <violet/Filesystem/Path.h>
#include <violet/Filesystem/Permissions.h>
#include <violet/IO/Error.h>
#include <violet/Iterator.h>

#if VIOLET_PLATFORM(UNIX)
#include <dirent.h>
#endif

namespace violet::filesystem {
namespace experimental {
    struct Dir;
}

struct Dirs;
struct WalkDirs;
struct OpenOptions;
struct Metadata;
struct FileType;

/// Creates a single directory at the specified `path`. Fails otherwise if the parent directories don't exist.
/// @param path the directory to create
VIOLET_API NOELDOC_SINCE("26.02") auto CreateDirectory(PathRef path) -> io::Result<void>;

/// Recursively creates all directories along the specified `path`. This is equivalent to `mkdir -p`.
/// @param path the directory to create
VIOLET_API NOELDOC_SINCE("26.02") auto CreateDirectories(PathRef path) -> io::Result<void>;

/// Removes an empty directory at `path`.
/// @param path the directory to remove.
VIOLET_API NOELDOC_SINCE("26.02") auto RemoveDirectory(PathRef path) -> io::Result<void>;

/// Recursively removes a directory and all of its content, including subdirectories and files.
/// @param path the directory to remove.
VIOLET_API NOELDOC_SINCE("26.02") auto RemoveAllDirs(PathRef path) -> io::Result<void>;

/// Returns a Violet-style iterator over the entries of the directory in `path`.
/// @param path the path to iterate over.
VIOLET_API NOELDOC_SINCE("26.02") auto ReadDir(PathRef path) -> io::Result<Dirs>;

/// Returns a recursive Violet-style iterator over the entries of the directory in `path`.
/// @param path the path to iterate over.
VIOLET_API NOELDOC_SINCE("26.02") auto WalkDir(PathRef path) -> io::Result<WalkDirs>;

/// Creates a new, empty file at `path`. This can overwrite if the file already exists
/// depending on platform semantics.
///
/// @param path the file to create
VIOLET_API NOELDOC_SINCE("26.02") auto CreateFile(PathRef path) -> io::Result<File>;

/// Returns the canonical, absolute path of `path` while resolving symbolic links and relative components.
/// @param path the file to canonicalize
VIOLET_API NOELDOC_SINCE("26.02") auto Canonicalize(PathRef path) -> io::Result<Path>;

/// Copies the contents of `srcs` into `dest`.
/// @param src source file
/// @param dest destination file
/// @returns the number of bytes, or an error if any occurs.
VIOLET_API NOELDOC_SINCE("26.02") auto Copy(PathRef src, PathRef dest) -> io::Result<UInt64>;

/// Returns metadata in the specified `path`, which includes the file size, permissions,
/// and timestamps.
///
/// @param path the path to retrieve metadata from
/// @param followSymlinks if true, this will follow the symlink tree.
/// @returns the metadata about this file, directory, etc. or an error if any occurs.
VIOLET_API
NOELDOC_SINCE("26.02")
VIOLET_DEPRECATED_BECAUSE("26.07",
    "use `Metadata::For(path, followSymlinks ? SymlinkResolution::Follow : "
    "SymlinkResolution::NoFollow) instead")
auto Metadata(PathRef path, bool followSymlinks = true) -> io::Result<struct Metadata>;

/// Returns **true** if the given path exists.
///
/// ## Remarks
/// This function may suffer from TOCTOU (Time of Check to Time of Use) race conditions
/// in multi-threaded or security contexts. Use the [`filesystem::TryExists`] method
/// instead.
///
/// @param path the path to check the existence of.
VIOLET_API NOELDOC_SINCE("26.02") auto Exists(PathRef path) -> bool;

/// Returns **true** if the given path exists.
/// @param path the path to check the existence of.
VIOLET_API NOELDOC_SINCE("26.02") auto TryExists(PathRef path) -> io::Result<bool>;

VIOLET_API NOELDOC_SINCE("26.02") auto RemoveFile(PathRef path) -> io::Result<void>;
VIOLET_API NOELDOC_SINCE("26.02") auto Rename(PathRef old, PathRef path) -> io::Result<void>;
VIOLET_API NOELDOC_SINCE("26.02") auto SetPermissions(PathRef path, Permissions perms) -> io::Result<void>;

/// Returns **true** if the given path is an executable file.
VIOLET_API NOELDOC_SINCE("26.02") auto Executable(PathRef path) -> io::Result<bool>;

/// A entry from walking through a directory.
struct VIOLET_API NOELDOC_SINCE("26.02") DirEntry final {
    /// The path of this entry.
    struct Path Path;

    /// Metadata about this entry.
    struct Metadata Metadata;
};

/// A [`Iterator`] implementation that walks through a filesystem directory non-recursively.
struct VIOLET_API NOELDOC_SINCE("26.02") Dirs final: public Iterator<Dirs> {
    VIOLET_DISALLOW_CONSTRUCTOR(Dirs);
    VIOLET_DISALLOW_COPY(Dirs);
    ~Dirs();

    VIOLET_IMPLICIT Dirs(Dirs&& other) noexcept;
    auto operator=(Dirs&& other) noexcept -> Dirs& = delete;

    /// The item that is returned from the iterator.
    using Item = io::Result<DirEntry>;

    /// Returns the next entry in the filesystem directory.
    VIOLET_API auto Next() noexcept -> Optional<Item>;

    /// When called, the iterator will be stopped and iterations will return [`violet::Nothing`].
    NOELDOC_SINCE("26.07") void StopTraversing();

private:
    friend auto violet::filesystem::ReadDir(PathRef) -> io::Result<Dirs>;
    friend struct experimental::Dir;

    /// the implementation of the iteration itself.
    struct Impl;

    template<typename... Args>
    VIOLET_EXPLICIT Dirs(Args&&... args);

    Impl* n_impl; ///< pointer to the implementation itself.
};

/// A [`Iterator`] implementation that walks through a filesystem directory recursively.
struct VIOLET_API NOELDOC_SINCE("26.02") WalkDirs final: public Iterator<WalkDirs> {
    VIOLET_DISALLOW_CONSTRUCTOR(WalkDirs);
    VIOLET_DISALLOW_COPY(WalkDirs);
    ~WalkDirs();

    VIOLET_IMPLICIT WalkDirs(WalkDirs&& other) noexcept;
    auto operator=(WalkDirs&& other) noexcept -> WalkDirs& = delete;

    /// The item that is returned from the iterator.
    using Item = io::Result<DirEntry>;

    /// Returns the next entry in the filesystem directory.
    VIOLET_API auto Next() noexcept -> Optional<Item>;

    /// When called, the iterator will be stopped and iterations will return [`violet::Nothing`].
    NOELDOC_SINCE("26.07") void StopTraversing();

private:
    friend auto violet::filesystem::WalkDir(PathRef) -> io::Result<WalkDirs>;
    friend struct experimental::Dir;

    /// the implementation of the iteration itself.
    struct Impl;

    template<typename... Args>
    VIOLET_EXPLICIT WalkDirs(Args&&... args);

    Impl* n_impl; ///< pointer to the implementation itself.

#if VIOLET_PLATFORM(UNIX)
    static auto fromRootStream(DIR* stream, Path base) -> WalkDirs;
#endif
};

} // namespace violet::filesystem
