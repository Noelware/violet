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
//
//! # 🌺💜 `violet/Experimental/Filesystem/Dir.h`

#pragma once

#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Path.h>
#include <violet/IO/Descriptor.h>
#include <violet/Iterator.h>

#if VIOLET_PLATFORM(LINUX)
#include <violet/Filesystem/Permissions.h>
#endif

namespace violet::filesystem {
struct WalkDirs;
struct Dirs;
} // namespace violet::filesystem

namespace violet::filesystem::experimental {

/// A handle to a open directory.
///
/// `Dir` owns the underlying file descriptor and resolves paths passed to its methods *relative to this
/// directory*, rather than the process working directory. The descriptor is closed automatically when the `Dir` is
/// destroyed.
///
/// ## Example
/// ```cpp
/// #include <violet/Filesystem/Experimental/Dir.h>
/// #include <violet/IO/Read.h>
///
/// using namespace violet::filesystem::experimental;
/// using namespace violet::io;
///
/// Dir dir = VIOLET_TRY(Dir::Open("foo"));
/// auto file = VIOLET_TRY(dir.OpenFile("bar.txt"));
/// auto contents = VIOLET_TRY(ReadToString(file));
/// assert(contents == "Hello, world!");
/// ```
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("26.07") Dir final {
    VIOLET_DISALLOW_CONSTRUCTOR(Dir);
    VIOLET_DISALLOW_COPY(Dir);
    VIOLET_IMPLICIT_MOVE(Dir);
    ~Dir() = default;

    /// Adopts an owned file descriptor, taking ownership of it.
    ///
    /// The descriptor must refer to an open directory. Ownership transfers to the `Dir`, which closes it on
    /// destruction.
    ///
    /// @param descriptor an owned file descriptor for an open directory.
    VIOLET_IMPLICIT Dir(io::FileDescriptor descriptor) noexcept
        : n_fd(VIOLET_MOVE(descriptor))
    {
    }

    /// Adopts a raw file descriptor, taking ownership of it.
    ///
    /// Equivalent to the `io::FileDescriptor` constructor, but accepts the raw underlying value (e.g. an
    /// `int` returned from a Unix-based C API). The descriptor must refer to an open directory; ownership
    /// transfers to the `Dir`. This overload is explicit because, unlike an [`io::FileDescriptor`], a raw
    /// value carries no ownership guarantee on its own.
    ///
    /// @param descriptor a raw, owned file descriptor for an open directory.
    VIOLET_EXPLICIT Dir(io::FileDescriptor::value_type descriptor) noexcept
        : n_fd(descriptor)
    {
    }

    /// Opens the directory at `path`.
    ///
    /// @param path the path of the directory to open.
    /// @param opts extra options when opening this directory
    /// @returns a `Dir` handle, or an `io::Error` if `path` does not exist, is not a directory, or cannot be
    ///   opened (e.g. insufficient permissions).
    ///
    /// ## Example
    /// ```cpp
    /// auto dir = VIOLET_TRY(Dir::Open("/var/lib/eous"));
    /// auto config = VIOLET_TRY(dir.OpenFile("config.toml")); // resolved relative to /var/lib/eous
    /// ```
    static auto Open(PathRef path) -> io::Result<Dir>;

    /// Opens a file at `path`, resolved relative to this directory.
    ///
    /// @param path the file path, taken relative to this directory.
    /// @param opts options when opening the file
    /// @returns a `File` handle, or an `io::Error` on failure.
    [[nodiscard]] auto OpenFile(PathRef path, OpenOptions opts = { }) const -> io::Result<File>;

    /// Returns the raw file descriptor backing this handle, without transferring ownership.
    ///
    /// The descriptor remains owned by this `Dir` and is closed when the `Dir` is destroyed, do not close
    /// it yourself.
    [[nodiscard]] auto Descriptor() const -> io::FileDescriptor::value_type;

    /// Queries the metadata (type, permissions, timestamps, etc.) of this directory.
    /// @returns the directory's `Metadata`, or an `io::Error` if it could not be retrieved.
    [[nodiscard]] auto Metadata() const -> io::Result<struct Metadata>;

    /// Returns an iterator over the immediate entries of this directory.
    ///
    /// Yields each entry once and does not descend into subdirectories; use [`Dir::Walk`] for a recursive traversal.
    ///
    /// @returns a `Dirs` iterator, or an `io::Error` if the directory could not be read.
    [[nodiscard]] auto Iter(Path display = { }) const -> io::Result<Dirs>;

    /// Returns an iterator that recursively walks this directory tree.
    ///
    /// Unlike `Iter`, which yields only the immediate entries, `Walk` descends into subdirectories.
    ///
    /// @returns a `WalkDirs` iterator, or an `io::Error` if the walk could not be started.
    [[nodiscard]] auto Walk(Path display = { }) const -> io::Result<WalkDirs>;

    /// Returns **true** if the directory's file descriptor is open and alive.
    [[nodiscard]] auto Alive() const -> bool;

    /// Closes the directory, releasing the underlying descriptor.
    ///
    /// After a successful close this handle no longer refers to an open directory (`operator bool` returns
    /// `false`). Closing also happens automatically on destruction, so calling `Close` is only needed when
    /// you want to observe the result (e.g. to surface a deferred error) or to release the descriptor early.
    ///
    /// @returns nothing on success, or an `io::Error` if the close failed.
    void Close();

    /// Consumes this handle and returns its underlying file descriptor, transferring ownership to the caller.
    ///
    /// After `Release`, the descriptor is no longer owned by this `Dir`, so it is **not** closed when the
    /// (now moved-from) handle is destroyed, ownership has passed to the returned [`io::FileDescriptor`]. Use it
    /// to move the descriptor into another wrapper or to hand it to code that will take over its lifetime,
    /// without risking a double close.
    ///
    /// ## Example
    /// ```cpp
    /// auto dir = VIOLET_TRY(Dir::Open("/var/lib/eous"));
    /// auto fd = VIOLET_MOVE(dir).Release();
    /// // `dir` is now empty and will not close anything; `fd` owns the descriptor.
    /// ```
    [[nodiscard]] auto Release() && -> io::FileDescriptor;

    VIOLET_EXPLICIT operator bool() const noexcept;
    VIOLET_EXPLICIT operator io::FileDescriptor::value_type() const noexcept;

private:
    io::FileDescriptor n_fd;
};

} // namespace violet::filesystem::experimental
