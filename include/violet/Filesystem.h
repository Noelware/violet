// 🌺💜 Violet: Extended C++ standard library
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

#include "violet/Filesystem/File.h"
#include "violet/Filesystem/Path.h"
#include "violet/Filesystem/Permissions.h"
#include "violet/IO/Error.h"
#include "violet/Violet.h"

namespace violet::filesystem {

struct Dirs;
struct WalkDirs;
struct OpenOptions;
struct Metadata;
struct FileType;

/// Creates a single directory at the specified `path`. Fails otherwise if the parent directories don't exist.
/// @param path the directory to create
auto CreateDirectory(PathRef path) -> io::Result<void>;

/// Recursively creates all directories along the specified `path`. This is equivalent to `mkdir -p`.
/// @param path the directory to create
auto CreateDirectories(PathRef path) -> io::Result<void>;

/// Removes an empty directory at `path`.
/// @param path the directory to remove.
auto RemoveDirectory(PathRef path) -> io::Result<void>;

/// Recursively removes a directory and all of its content, including subdirectories and files.
/// @param path the directory to remove.
auto RemoveAllDirs(PathRef path) -> io::Result<void>;

/// Returns a Violet-style iterator over the entries of the directory in `path`.
/// @param path the path to iterate over.
auto ReadDir(PathRef path) -> io::Result<Dirs>;

/// Returns a recursive Violet-style iterator over the entries of the directory in `path`.
/// @param path the path to iterate over.
auto WalkDir(PathRef path) -> io::Result<WalkDirs>;

/// Creates a new, empty file at `path`. This can overwrite if the file already exists
/// depending on platform semantics.
///
/// @param path the file to create
auto CreateFile(PathRef path) -> io::Result<File>;

/// Returns the canonical, absolute path of `path` while resolving symbolic links and relative components.
/// @param path the file to canonicalize
auto Canonicalize(PathRef path) -> io::Result<Path>;

/// Copies the contents of `srcs` into `dest`.
/// @param src source file
/// @param dest destination file
/// @returns the number of bytes, or an error if any occurs.
auto Copy(PathRef src, PathRef dest) -> io::Result<UInt64>;

/// Returns metadata in the specified `path`, which includes the file size, permissions,
/// and timestamps.
///
/// @param path the path to retrieve metadata from
/// @param followSymlinks if true, this will follow the symlink tree.
/// @returns the metadata about this file, directory, etc. or an error if any occurs.
auto Metadata(PathRef path, bool followSymlinks = true) -> io::Result<Metadata>;

/// Returns **true** if the given path exists.
///
/// ## Remarks
/// This function may suffer from TOCTOU (Time of Check to Time of Use) race conditions
/// in multi-threaded or security contexts. Use the [`filesystem::TryExists`] method
/// instead.
///
/// @param path the path to check the existence of.
auto Exists(PathRef path) -> bool;

/// Returns **true** if the given path exists.
/// @param path the path to check the existence of.
auto TryExists(PathRef path) -> io::Result<bool>;

auto RemoveFile(PathRef path) -> io::Result<void>;
auto Rename(PathRef old, PathRef path) -> io::Result<void>;
auto SetPermissions(PathRef path, Permissions perms) -> io::Result<void>;

/// A entry from walking through a directory.
struct VIOLET_API DirEntry final {
    /// The path of this entry.
    struct Path Path;

    /// Metadata about this entry.
    struct Metadata Metadata;
};

/// A [`Iterator`] implementation that walks through a filesystem directory non-recursively.
struct VIOLET_API Dirs final {
    /// The item that is returned from the iterator.
    using Item = io::Result<DirEntry>;

    Dirs() = delete;
    ~Dirs() = default;

    Dirs(const Dirs&) = delete;
    auto operator=(const Dirs&) -> Dirs& = delete;

    Dirs(Dirs&&) = default;
    auto operator=(Dirs&&) -> Dirs& = delete;

    /// Returns the next entry in the filesystem directory.
    auto Next() noexcept -> Optional<Item>;

private:
    friend auto violet::filesystem::ReadDir(PathRef) -> io::Result<Dirs>;

    /// the implementation of the iteration itself.
    struct Impl;

    template<typename... Args>
        requires(std::is_constructible_v<Impl, Args...>)
    VIOLET_EXPLICIT Dirs(Args&&... args);

    Impl* n_impl; ///< pointer to the implementation itself.
};

/// A [`Iterator`] implementation that walks through a filesystem directory recursively.
struct VIOLET_API WalkDirs final {
    /// The item that is returned from the iterator.
    using Item = io::Result<DirEntry>;

    WalkDirs() = delete;

    /// Returns the next entry in the filesystem directory.
    auto Next() noexcept -> Optional<Item>;

private:
    friend auto violet::filesystem::WalkDir(PathRef) -> io::Result<WalkDirs>;

    /// the implementation of the iteration itself.
    struct Impl;

    template<typename... Args>
        requires(std::is_constructible_v<Impl, Args...>)
    VIOLET_EXPLICIT WalkDirs(Args&&... args);

    Impl* n_impl; ///< pointer to the implementation itself.
};

} // namespace violet::filesystem
