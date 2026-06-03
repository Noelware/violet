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

#include <violet/Filesystem/Permissions.h>
#include <violet/IO/Descriptor.h>
#include <violet/Support/Bitflags.h>

namespace violet::filesystem {
namespace experimental {
    struct Dir;
}

namespace xattr {
    struct Iter;
}

struct PathRef;
struct File;
struct Metadata;

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
struct VIOLET_API NOELDOC_SINCE("26.02") OpenOptions final {
    /// Creates a new, default set of open options (all flags disabled).
    constexpr OpenOptions() = default;

    /// Enables or disables reading this file.
    constexpr auto Read() noexcept -> OpenOptions&
    {
        this->n_bits = this->n_bits.Toggle(flag::kRead);
        return *this;
    }

    /// Enables or disables writing to this file.
    constexpr auto Write() noexcept -> OpenOptions&
    {
        this->n_bits = this->n_bits.Toggle(flag::kWrite);
        return *this;
    }

    /// Enables or disables ensuring this file is created if it doesn't exist.
    constexpr auto Create() noexcept -> OpenOptions&
    {
        this->n_bits = this->n_bits.Toggle(flag::kCreate);
        return *this;
    }

    /// Enables or disables appending new data to this file.
    constexpr auto Append() noexcept -> OpenOptions&
    {
        this->n_bits = this->n_bits.Toggle(flag::kAppend);
        return *this;
    }

    /// Enables or disables overwriting file data instead of appending.
    constexpr auto Truncate() noexcept -> OpenOptions&
    {
        this->n_bits = this->n_bits.Toggle(flag::kTruncate);
        return *this;
    }

    /// Enables or disables whether if this file will be created atomically.
    constexpr auto CreateNew() noexcept -> OpenOptions&
    {
        this->n_bits = this->n_bits.Toggle(flag::kCreateNew);
        return *this;
    }

#if VIOLET_PLATFORM(WINDOWS) || VIOLET_FEATURE(NOELDOC)
    /// Sets Windows-specific file attributes.
    constexpr auto Attributes(DWORD attrs) noexcept -> OpenOptions&
    {
        this->n_attributes = attrs;
        return *this;
    }
#elif VIOLET_PLATFORM(UNIX) || VIOLET_FEATURE(NOELDOC)
    /// Sets Unix permission mode bits.
    constexpr auto Mode(mode_t mode) noexcept -> OpenOptions&
    {
        this->n_mode = mode;
        return *this;
    }

    /// Sets Unix permission mode bits.
    constexpr auto Mode(struct Mode mode) noexcept -> OpenOptions&
    {
        this->n_mode = mode;
        return *this;
    }

    /// Sets additional Unix open flags.
    constexpr auto Flags(Int32 flags) noexcept -> OpenOptions&
    {
        this->n_flags = flags;
        return *this;
    }
#endif

    /// Opens this file and returns a file handle, or an error if any occurs.
    VIOLET_API auto Open(PathRef path) -> io::Result<File>;

private:
    friend struct File;
    friend struct experimental::Dir;

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

#if VIOLET_PLATFORM(WINDOWS)
    DWORD n_attributes;
#elif VIOLET_PLATFORM(UNIX)
    struct Mode n_mode;
    Int32 n_flags;
#endif
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
struct VIOLET_API NOELDOC_SINCE("26.02") File final {
    /// RAII helper that locks a file for the duration of a scope.
    ///
    /// Constructed via [`MkScopedLock`] or [`MkSharedScopedLock`] methods.
    /// Automatically releases the lock when the object is destroyed.
    struct NOELDOC_SINCE("26.02") ScopeLock final {
        VIOLET_DISALLOW_CONSTRUCTOR(ScopeLock);
        VIOLET_DISALLOW_COPY(ScopeLock);

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

    VIOLET_DISALLOW_COPY(File);
    VIOLET_IMPLICIT_MOVE(File);

    /// Creates a invalid, empty file.
    VIOLET_IMPLICIT File() = default;

    /// Creates a new file from an explicit file descriptor.
    /// @param descriptor the descriptor itself.
    VIOLET_EXPLICIT File(io::FileDescriptor&& descriptor)
        : n_fd(VIOLET_MOVE(descriptor))
    {
    }

    /// Destructor.
    ~File();

    /// Opens the file from a path and returns the opened [`File`].
    /// @param path the path to open
    /// @param opts the options to open this file
    VIOLET_API static auto Open(PathRef path, OpenOptions opts = { }) -> io::Result<File>;

    /// Returns the raw file descriptor that this file points to.
    [[nodiscard]] VIOLET_API auto Descriptor() const noexcept -> io::FileDescriptor::value_type;

    /// Returns a textual representation of this file object.
    [[nodiscard]] VIOLET_API auto ToString() const noexcept -> String;

    /// Closes the file and the file will be invalidated.
    VIOLET_API auto Close() noexcept -> io::Result<void>;

    /// Returns **true** if this file is pointed to a valid descriptor.
    [[nodiscard]] VIOLET_API auto Valid() const noexcept -> bool;

    /// Reads from this file and returns the amount of bytes read.
    /// @param buf buffer to read from.
    [[nodiscard]] VIOLET_API auto Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>;

    /// Writes from this file and returns the amount of bytes written.
    /// @param buf buffer to read from.
    [[nodiscard]] VIOLET_API auto Write(Span<const UInt8> buf) const noexcept -> io::Result<UInt>;

    /// Flushes buffered writes to the underlying filesystem.
    [[nodiscard]] VIOLET_API auto Flush() const noexcept -> io::Result<void>;

    /// Locks the file exclusively. Blocks until the lock is acquired.
    [[nodiscard]] VIOLET_API auto Lock() const noexcept -> io::Result<void>;

    /// Acquires a shared (read) lock on the file. Blocks until acquired.
    [[nodiscard]] VIOLET_API auto SharedLock() const noexcept -> io::Result<void>;

    /// Unlocks the file.
    [[nodiscard]] VIOLET_API auto Unlock() const noexcept -> io::Result<void>;

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
    [[nodiscard]] VIOLET_API auto Locked() const noexcept -> io::Result<bool>;

    /// Returns an RAII-scoped exclusive lock.
    [[nodiscard]] VIOLET_API auto MkScopedLock() const noexcept -> io::Result<ScopeLock>;

    /// Returns an RAII-scoped shared lock.
    [[nodiscard]] VIOLET_API auto MkSharedScopedLock() const noexcept -> io::Result<ScopeLock>;

    /// Retrieves metadata about the file.
    [[nodiscard]] VIOLET_API auto Metadata() const noexcept -> io::Result<struct Metadata>;

    /// Returns a clone of this file, sharing the same descriptor.
    /// @param shareFlags whether if the cloned file should share the same flags
    /// as this file.
    [[nodiscard]] VIOLET_API auto Clone(bool shareFlags = false) const noexcept -> io::Result<File>;

    /// Returns an iterator that lists through this file's extended attributes.
    [[nodiscard]] VIOLET_API auto Attributes() const noexcept -> io::Result<xattr::Iter>;

    /// Returns an extended attribute by `key`.
    /// @param key the attribute to fetch
    [[nodiscard]] VIOLET_API auto GetAttribute(Str key) const noexcept -> io::Result<Optional<Vec<UInt8>>>;

    /// Sets an extended attribute by `key` with the given `value`.
    [[nodiscard]] VIOLET_API auto SetAttribute(Str key, Span<const UInt8> value) const noexcept -> io::Result<void>;

    /// Removes an extended attribute.
    /// @param key the attribute to remove
    [[nodiscard]] VIOLET_API auto RemoveAttribute(Str key) const noexcept -> io::Result<void>;

    VIOLET_EXPLICIT operator bool() const noexcept;
    VIOLET_EXPLICIT operator io::FileDescriptor::value_type() const noexcept;

private:
    io::FileDescriptor n_fd;

    /// Tracks whether this handle currently holds an advisory lock.
    ///
    /// Advisory `flock` locks are associated with the open file description, so a
    /// non-blocking probe on our own descriptor cannot observe (and would corrupt)
    /// a lock we already hold. We record it here so [`Locked`] can report a
    /// self-held lock without touching the kernel state.
    mutable bool n_locked = false;
};

} // namespace violet::filesystem

VIOLET_FORMATTER(violet::filesystem::File);
