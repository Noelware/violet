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

#include "violet/Filesystem/Path.h"
#include "violet/Filesystem/Permissions.h"
#include "violet/IO/Descriptor.h"
#include "violet/Support/Bitflags.h"
#include "violet/Violet.h"

namespace violet::filesystem {

struct File;

/// Configures options for opening a file.
///
/// This struct provides a builder-style API to specify on how a file should be
/// opened, analogous to Rust's [`std::fs::OpenOptions`]. You can set whether if
/// the file is opened for reading, writing, appending, truncated, or created
/// if it already doesn't exist.
///
/// ## Example
/// ```cpp
/// /* TODO: this */
/// ```
struct VIOLET_API OpenOptions final {
    /// Creates a new, default set of open options (all flags disabled).
    constexpr OpenOptions() = default;

    /// Enables or disables reading this file.
    /// @param yes whether if it should be read or not.
    constexpr auto Read(bool yes = true) noexcept -> OpenOptions&
    {
        this->populateFlag(flag::kRead, yes);
        return *this;
    }

    /// Enables or disables writing to this file.
    /// @param yes whether if it should be read or not.
    constexpr auto Write(bool yes = true) noexcept -> OpenOptions&
    {
        this->populateFlag(flag::kRead, yes);
        return *this;
    }

    /// Enables or disables ensuring this file is created if it doesn't exist.
    /// @param yes whether if it should be read or not.
    constexpr auto Create(bool yes = true) noexcept -> OpenOptions&
    {
        this->populateFlag(flag::kRead, yes);
        return *this;
    }

    /// Enables or disables appending new data to this file.
    /// @param yes whether if it should be read or not.
    constexpr auto Append(bool yes = true) noexcept -> OpenOptions&
    {
        this->populateFlag(flag::kAppend, yes);
        return *this;
    }

    /// Enables or disables overwriting file data instead of appending.
    /// @param yes whether if it should be read or not.
    constexpr auto Truncate(bool yes = true) noexcept -> OpenOptions&
    {
        this->populateFlag(flag::kTruncate, yes);
        return *this;
    }

    /// Enables or disables whether if this file will be created atomically.
    /// @param yes whether if it should be read or not.
    constexpr auto CreateNew(bool yes = true) noexcept -> OpenOptions&
    {
        this->populateFlag(flag::kCreateNew, yes);
        return *this;
    }

#ifdef VIOLET_WINDOWS
    /// Sets Windows-specific file attributes.
    constexpr auto Attributes(DWORD attrs) noexcept -> OpenOptions&
    {
        this->n_attributes = attrs;
        return *this;
    }
#elif defined(VIOLET_UNIX)
    /// Sets Unix permission mode bits.
    auto Mode(mode_t mode) noexcept -> OpenOptions&
    {
        this->n_mode = mode;
        return *this;
    }

    /// Sets Unix permission mode bits.
    auto Mode(struct Mode mode) noexcept -> OpenOptions&
    {
        this->n_mode = mode;
        return *this;
    }

    /// Sets additional Unix open flags.
    auto Flags(Int32 flags) noexcept -> OpenOptions&
    {
        this->n_flags = flags;
        return *this;
    }
#endif

    /// Opens this file and returns a file handle, or an error if any occurs.
    auto Open(PathRef path) -> io::Result<File>;

private:
    friend struct File;

    /// Bitflags representing the basic open options.
    enum struct flag : UInt8 {
        kRead = 1 << 0, ///< Marks the file to be opened for reading.
        kWrite = 1 << 1, ///< Marks the file to be opened for writing.
        kCreate = 1 << 2, ///< Ensures the file is created if it does not exist.
        kAppend = 1 << 3, ///< Appends new data instead of overwriting.
        kTruncate = 1 << 4, ///< Overwrites file data instead of appending.
        kCreateNew = 1 << 5 ///< Atomically creates a new file or fails if it exists.
    };

    Bitflags<flag> n_bits;

#ifdef VIOLET_WINDOWS
    DWORD n_attributes;
#elif defined(VIOLET_UNIX)
    struct Mode n_mode;
    Int32 n_flags;
#endif

    /// Adds or removes a flag based on the boolean `yes`.
    constexpr void populateFlag(flag flag, bool yes = true) noexcept
    {
        if (!this->n_bits.Contains(flag) && yes) {
            this->n_bits.Add(flag);
        } else if (this->n_bits.Contains(flag) && !yes) {
            this->n_bits.Remove(flag);
        }
    }
};

/// Representation of a filesystem entry's type.
///
/// This is a lightweight, value-type descriptor that classifies filesystem objects
/// such as regular files, directories, symbolic links, et al.
///
/// This is usually returned when querying a file's metadata, i.e, [`violet::filesystem::Metadata`].
///
/// ## Example
/// ```cpp
/// #include <violet/Filesystem.h>
///
/// using namespace violet;
///
/// FileType t = filesystem::Metadata("/etc/passwd")
///              .AndThen([](const Metadata& mt) -> FileType {
///                   return mt.Type();
///              });
///
/// if (t.File()) {
///     std::cout << "/etc/passwd is a regular file\n";
/// } else if (t.Directory()) {
///     std::cout << "/etc/passwd is a directory\n";
/// } else if (t.Symlink()) {
///     std::cout << "/etc/passwd is a symbolic link\n";
/// }
/// ```
///
/// ## Remarks (Unix)
/// Additional types like block, character devices, FIFOs, and sockets are supported.
///
/// ## Platform-specific behaviour
/// On Unix systems, `FileType` also includes additional methods for detecting special files
/// like block and character devices.
///
/// ## Invariants
/// - `FileType` can represent multiple attributes simultaneously, like a symbolic link
///   to a directory.
struct VIOLET_API FileType final {
    /// Creates an empty `FileType` with no tags set.
    constexpr FileType() = default;

    /// Returns **true** if this represents a regular file.
    [[nodiscard]] constexpr auto File() const noexcept -> bool
    {
        return this->n_tag.Contains(tag::kFile);
    }

    /// Returns **true** if this represents a directory.
    [[nodiscard]] constexpr auto Dir() const noexcept -> bool
    {
        return this->n_tag.Contains(tag::kDir);
    }

    /// Returns **true** if this represents a symbolic link.
    [[nodiscard]] constexpr auto Symlink() const noexcept -> bool
    {
        return this->n_tag.Contains(tag::kSymlink);
    }

#ifdef VIOLET_UNIX
    /// Returns **true** if this represents a block device.
    [[nodiscard]] constexpr auto BlockDevice() const noexcept -> bool
    {
        return this->n_tag.Contains(tag::kBlkDev);
    }

    /// Returns **true** if this represents a character device.
    [[nodiscard]] constexpr auto CharDevice() const noexcept -> bool
    {
        return this->n_tag.Contains(tag::kCharDev);
    }

    /// Returns **true** if this represents a FIFO pipe.
    [[nodiscard]] constexpr auto FIFOPipe() const noexcept -> bool
    {
        return this->n_tag.Contains(tag::kFIFO);
    }

    /// Returns **true** if this represents a Unix socket.
    [[nodiscard]] constexpr auto UnixSocket() const noexcept -> bool
    {
        return this->n_tag.Contains(tag::kSocket);
    }
#endif

private:
    friend struct violet::filesystem::File;

    enum struct tag : UInt8 {
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

    Bitflags<tag> n_tag;

    static constexpr auto mksymlink() noexcept -> FileType
    {
        FileType ft{};
        ft.n_tag.Add(tag::kSymlink);

        return ft;
    }

    static constexpr auto mkfile(bool symlink = false) noexcept -> FileType
    {
        FileType ft = {};
        ft.n_tag.Add(tag::kFile);

        if (symlink) {
            ft.n_tag.Add(tag::kSymlink);
        }

        return ft;
    }

    static constexpr auto mkdir(bool symlink = false) noexcept -> FileType
    {
        FileType ft = {};
        ft.n_tag.Add(tag::kDir);

        if (symlink) {
            ft.n_tag.Add(tag::kSymlink);
        }

        return ft;
    }

#ifdef VIOLET_UNIX
    static constexpr auto mkblkdev(bool symlink = false) noexcept -> FileType
    {
        FileType ft = {};
        ft.n_tag.Add(tag::kBlkDev);

        if (symlink) {
            ft.n_tag.Add(tag::kSymlink);
        }

        return ft;
    }

    static constexpr auto mkchardev(bool symlink = false) noexcept -> FileType
    {
        FileType ft = {};
        ft.n_tag.Add(tag::kCharDev);

        if (symlink) {
            ft.n_tag.Add(tag::kSymlink);
        }

        return ft;
    }

    static constexpr auto mkfifo(bool symlink = false) noexcept -> FileType
    {
        FileType ft = {};
        ft.n_tag.Add(tag::kFIFO);

        if (symlink) {
            ft.n_tag.Add(tag::kSymlink);
        }

        return ft;
    }

    static constexpr auto mksocket(bool symlink = false) noexcept -> FileType
    {
        FileType ft = {};
        ft.n_tag.Add(tag::kSocket);

        if (symlink) {
            ft.n_tag.Add(tag::kSymlink);
        }

        return ft;
    }
#endif
};

/// Represents filesystem metadata for a regular file, directory, special entry, etc.
///
/// This struct provides low-level information about a filesystem object, including size,
/// permissions, modification/creation timestamps, etc.
///
/// ## Example
/// ```cpp
/// #include <violet/Filesystem.h>
///
/// using namespace violet;
///
/// auto metadata = filesystem::Metadata("/etc/hosts").Unwrap();
/// std::cout << "Size: " << metadata.Size << " bytes\n";
///
/// if (metadata.Type.Directory()) {
///     std::cout << "`/etc/hosts` is a directory\n";
/// }
///
/// std::cout << "Permissions: " << std::oct << meta.Permissions << '\n';
/// std::cout << "Modified At: " << meta.ModifiedAt << '\n';
/// ```
///
/// ## Platform-specific Behaviour
/// On Unix systems, additional mode information is avaliable from the [`Mode`] struct, which exposes
/// raw POSIX mode bits (owner, group, and others) and special flags.
///
/// On Windows, `Permissions` can be zeroed or partially emulated, as access control is managed
/// as Access Control List (ACLs) rather than POSIX-style bits.
struct VIOLET_API Metadata final {
    /// The file's permissions.
    struct Permissions Permissions = {};

    /// Last modification timestamp (UNIX seconds since epoch)
    UInt64 ModifiedAt = 0;

    /// The file's type.
    FileType Type = {};

    /// The size of this file in bytes.
    UInt64 Size = 0;

    /// The creation timestamp, if avaliable.
    Optional<UInt64> CreatedAt;

    /// The accessed timestamp, if avaliable.
    Optional<UInt64> AccessedAt;
};

/// A handle to an open file on the filesystem.
///
/// This struct provides RAII-based ownership of a file, ensuring that the file is
/// automatically closed when the `File` object is destroyed. It supports reading, writing,
/// querying metadata, manipulating permissions, and integration with [`OpenOptions`] to control
/// on how the file is opened.
///
/// ## Examples
/// ### Open a file for reading and writing, creating it if it doesn't exist
/// ```cpp
/// /* TODO: this */
/// ```
///
/// ### Reading and writing (implements [`io::Readable`] and [`io::Writable`])
/// ```cpp
/// /* TODO: this */
/// ```
///
/// ## RAII Semantics
/// This struct uses RAII to manage the lifetime of the underlying, low-level file descriptor. The file
/// is automatically closed either from destruction or from an explicit [`File::Close`] call. Move semantics
/// are supported, but copying is not supported.
struct File final {
    /// RAII helper that locks a file for the duration of a scope.
    ///
    /// Constructed via [`MkScopedLock`] or [`MkSharedScopedLock`] methods.
    /// Automatically releases the lock when the object is destroyed.
    struct ScopeLock final {
        ScopeLock() = delete;

        VIOLET_IMPLICIT ScopeLock(const ScopeLock&) = delete;
        auto operator=(const ScopeLock&) -> ScopeLock& = delete;

        VIOLET_IMPLICIT ScopeLock(ScopeLock&&) noexcept;
        auto operator=(ScopeLock&&) noexcept -> ScopeLock&;

        ~ScopeLock();

    private:
        friend struct File;

        VIOLET_EXPLICIT ScopeLock(File* ptr)
            : n_file(ptr)
        {
        }

        File* n_file;
    };

    /// Creates a invalid, empty file.
    VIOLET_IMPLICIT File() = default;

    /// Creates a new file from an explicit file descriptor.
    /// @param descriptor the descriptor itself.
    VIOLET_EXPLICIT File(io::FileDescriptor&& descriptor)
        : n_fd(VIOLET_MOVE(descriptor))
    {
    }

    /// Disallow copying [`File`] objects.
    VIOLET_IMPLICIT File(const File&) = delete;

    /// Disallow replacing files with a copied version of itself.
    auto operator=(const File&) -> File& = delete;

    /// Permit moving [`File`] objects.
    VIOLET_IMPLICIT File(File&&) noexcept = default;

    /// Allow replacing files with a moved file.
    auto operator=(File&&) noexcept -> File& = default;

    /// Destructor.
    ~File();

    /// Opens the file from a path and returns the opened [`File`].
    /// @param path the path to open
    /// @param opts the options to open this file
    static auto Open(PathRef path, OpenOptions opts = {}) -> io::Result<File>;

    /// Returns the raw file descriptor that this file points to.
    [[nodiscard]] auto Descriptor() const noexcept -> io::FileDescriptor::value_type;

    /// Returns a textual representation of this file object.
    [[nodiscard]] auto ToString() const noexcept -> String;

    /// Closes the file and the file will be invalidated.
    auto Close() noexcept -> io::Result<void>;

    /// Returns **true** if this file is pointed to a valid descriptor.
    [[nodiscard]] auto Valid() const noexcept -> bool;

    /// Reads from this file and returns the amount of bytes read.
    /// @param buf buffer to read from.
    [[nodiscard]] auto Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>;

    /// Writes from this file and returns the amount of bytes written.
    /// @param buf buffer to read from.
    [[nodiscard]] auto Write(Span<const UInt8> buf) const noexcept -> io::Result<UInt>;

    /// Flushes buffered writes to the underlying filesystem.
    [[nodiscard]] auto Flush() const noexcept -> io::Result<void>;

    /// Locks the file exclusively. Blocks until the lock is acquired.
    [[nodiscard]] auto Lock() const noexcept -> io::Result<void>;

    /// Acquires a shared (read) lock on the file. Blocks until acquired.
    [[nodiscard]] auto SharedLock() const noexcept -> io::Result<void>;

    /// Unlocks the file.
    [[nodiscard]] auto Unlock() const noexcept -> io::Result<void>;

    /// Checks whether if the file is currently locked by another process or thread.
    ///
    /// This method attempts to acquire a non-blocking lock on the file. If it succeeds,
    /// the file was not locked and the lock is immediately released. If it fails,
    /// the file is currently locked by another entity.
    ///
    /// ## Platform-specific behaviour
    /// Because of the inherent race condition, the result represents the lock state
    /// **at the moment of the call**, and another process could lock the file immediately
    /// after.
    ///
    /// ### Unix
    /// This uses `flock` with `LOCK_NB`. Advisory locks are cooperative, so a file may appear unlocked even if another
    /// process ignores `flock`.
    ///
    /// ## Windows
    /// This uses `LockFileEx` with `LOCKFILE_FAIL_IMMEDIATELY`.
    [[nodiscard]] auto Locked() const noexcept -> io::Result<bool>;

    /// Returns an RAII-scoped exclusive lock.
    [[nodiscard]] auto MkScopedLock() const noexcept -> io::Result<ScopeLock>;

    /// Returns an RAII-scoped shared lock.
    [[nodiscard]] auto MkSharedScopedLock() const noexcept -> io::Result<ScopeLock>;

    /// Retrieves metadata about the file.
    [[nodiscard]] auto Metadata() const noexcept -> io::Result<struct Metadata>;

    /// Returns a clone of this file, sharing the same descriptor.
    /// @param shareFlags whether if the cloned file should share the same flags
    /// as this file.
    [[nodiscard]] auto Clone(bool shareFlags = false) const noexcept -> io::Result<File>;

    VIOLET_EXPLICIT operator bool() const noexcept;
    VIOLET_EXPLICIT operator io::FileDescriptor::value_type() const noexcept;

private:
    io::FileDescriptor n_fd;
};

} // namespace violet::filesystem
