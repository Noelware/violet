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

#include <violet/Container/Optional.h>
#include <violet/Filesystem/Permissions.h>
#include <violet/IO/Descriptor.h>
#include <violet/Support/Bitflags.h>

#include <sstream>

#if VIOLET_PLATFORM(UNIX)
#include <sys/stat.h>
#endif

namespace violet::filesystem {

struct PathRef;
struct Metadata;
struct File;

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
struct VIOLET_API NOELDOC_SINCE("26.02") FileType final {
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

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        std::ostringstream os;
        os << *this;

        return os.str();
    }

    friend auto operator<<(std::ostream& os, const FileType& self) noexcept -> std::ostream&
    {
        os << "FileType(type=";

        if (self.n_tag.Contains(tag::kFile)) {
            os << "file";
        } else if (self.n_tag.Contains(tag::kDir)) {
            os << "directory";
        } else if (self.n_tag.Contains(tag::kSymlink)) {
            os << "symbolic link";
#if VIOLET_PLATFORM(UNIX)
        } else if (self.n_tag.Contains(tag::kBlkDev)) {
            os << "block device";
        } else if (self.n_tag.Contains(tag::kCharDev)) {
            os << "char device";
        } else if (self.n_tag.Contains(tag::kFIFO)) {
            os << "fifo";
        } else if (self.n_tag.Contains(tag::kSocket)) {
            os << "unix socket";
        }
#else
        }
#endif

        return os << ')';
    }

#if VIOLET_PLATFORM(UNIX) || VIOLET_FEATURE(NOELDOC)
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
    friend struct violet::filesystem::Metadata;

    enum struct tag : UInt8 {
        kFile = 1 << 0, ///< this is a file
        kDir = 1 << 1, ///< this is a directory
        kSymlink = 1 << 2, ///< this is a symlink

#if VIOLET_PLATFORM(UNIX)
        kBlkDev = 1 << 3, ///< this is a character device
        kCharDev = 1 << 4, ///< this is a character device
        kFIFO = 1 << 5, ///< this is a named pipe (FIFO).
        kSocket = 1 << 6 ///< this is a unix socket
#endif
    };

    Bitflags<tag> n_tag;

    static constexpr auto mksymlink() noexcept -> FileType
    {
        FileType ft{ };
        ft.n_tag |= tag::kSymlink;

        return ft;
    }

    static constexpr auto mkfile() noexcept -> FileType
    {
        FileType ft = { };
        ft.n_tag |= tag::kFile;

        return ft;
    }

    static constexpr auto mkdir() noexcept -> FileType
    {
        FileType ft = { };
        ft.n_tag |= tag::kDir;

        return ft;
    }

#if VIOLET_PLATFORM(UNIX)
    static constexpr auto mkblkdev() noexcept -> FileType
    {
        FileType ft = { };
        ft.n_tag |= tag::kBlkDev;

        return ft;
    }

    static constexpr auto mkchardev() noexcept -> FileType
    {
        FileType ft = { };
        ft.n_tag |= tag::kCharDev;

        return ft;
    }

    static constexpr auto mkfifo() noexcept -> FileType
    {
        FileType ft = { };
        ft.n_tag |= tag::kFIFO;

        return ft;
    }

    static constexpr auto mksocket() noexcept -> FileType
    {
        FileType ft = { };
        ft.n_tag |= tag::kSocket;

        return ft;
    }
#endif
};

/// Whether a path-based metadata query follows a trailing symbolic link.
enum struct SymlinkResolution : UInt8 {
    /// Follow a final symbolic link to its target, report metadata about what the link points to
    /// (`stat` semantics).
    Follow,

    /// Do not follow a final symbolic link, report metadata about the link itself (`lstat` semantics).
    NoFollow
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
struct VIOLET_API NOELDOC_SINCE("26.02") Metadata final {
    /// The file's permissions.
    struct Permissions Permissions = { };

    /// Last modification timestamp (UNIX seconds since epoch)
    UInt64 ModifiedAt = 0;

    /// The file's type.
    FileType Type = { };

    /// The size of this file in bytes.
    UInt64 Size = 0;

    /// The creation timestamp, if avaliable.
    Optional<UInt64> CreatedAt;

    /// The accessed timestamp, if avaliable.
    Optional<UInt64> AccessedAt;

#if VIOLET_PLATFORM(UNIX) || VIOLET_FEATURE(NOELDOC)
    /// The inode number.
    ///
    /// **Not** a standalone file identity: inode numbers are unique only *within* a single filesystem and are
    /// recycled after deletion. To test whether two paths refer to the same file, compare the
    /// `(Device, Inode)` pair, never the inode alone.
    NOELDOC_SINCE("26.07") UInt64 Inode;

    /// The identifier of the device the file resides on. Pair with `Inode()` for file identity.
    NOELDOC_SINCE("26.07") UInt64 Device;

    /// The major number of the device the file resides on.
    NOELDOC_SINCE("26.07") UInt64 DeviceMajor;

    /// The minor number of the device the file resides on.
    NOELDOC_SINCE("26.07") UInt64 DeviceMinor;

    /// The device this file *represents*, if it is a block or character special file.
    ///
    /// ## Remarks
    /// This is distinct from [`Metadata::Device`]. **Device* * is the device the inode lives *on*, whereas this is
    /// the device a special files *points at*, i.e, for `/dev/kvm`. [`violet::Nothing`] is returned for regular files,
    /// directories, and any other non-special file. Treat this value as opaque; compare for equality, do not decode
    /// the bits by hand.
    NOELDOC_SINCE("26.07") Optional<UInt64> Rdev;

    /// The major number of the device this file represents.
    ///
    /// [`violet::Nothing`] is returned for non-special files. Together with [`Metadata::RdevMinor`] this is
    /// the usual `major:minor` form you see in `ls -l` on a device node or in `/dev`.
    NOELDOC_SINCE("26.07") NOELDOC_SINCE("26.07") Optional<UInt64> RdevMajor;

    /// The minor number of the device this file represents. Otherwise, [`violet::Nothing`] for non-special files.
    NOELDOC_SINCE("26.07") Optional<UInt64> RdevMinor;

    /// The user ID of the file's owner.
    NOELDOC_SINCE("26.07") UInt32 UserID;

    /// The group ID of the file's owning group.
    NOELDOC_SINCE("26.07") UInt32 GroupID;

    /// The number of hard links to the file.
    NOELDOC_SINCE("26.07") UInt64 HardLinks;
#endif

    /// Queries metadata for an open file descriptor.
    ///
    /// Works on any kind of descriptor, regular file, directory, socket, pipe; reporting the corresponding
    /// type in the result. Because the descriptor already refers to a resolved object, there is no symlink
    /// behavior to choose here.
    ///
    /// @param fd a raw, open file descriptor. It is *borrowed*: ownership is not taken and the descriptor is
    ///           not closed.
    ///
    /// @returns the descriptor's `Metadata`, or an `io::Error`, e.g. `EBADF` if `fd` is not a valid open
    ///          descriptor.
    NOELDOC_SINCE("26.07")
    static auto For(io::FileDescriptor::value_type fd) -> io::Result<Metadata>;

    NOELDOC_SINCE("26.07")
    static auto For(io::FileDescriptor::value_type dirfd, PathRef path,
        SymlinkResolution resolution = SymlinkResolution::Follow) -> io::Result<Metadata>;

    /// Queries metadata for the file at `path`.
    ///
    /// ## Example
    /// ```cpp
    /// auto metadata = VIOLET_TRY(Metadata::For("/etc/hostname"));
    ///
    /// // /etc/localtime is usually a symlink; NoFollow reports the link, Follow its target.
    /// auto link = VIOLET_TRY(Metadata::For("/etc/localtime", SymbolResolution::NoFollow));
    /// ```
    ///
    /// @param path the path to query.
    /// @param resolution whether a trailing symbolic link is followed (`SymbolResolution::Follow`, the default,
    ///                   resolving to the link's target) or reported as the link itself
    ///                   ([`SymbolResolution::NoFollow`]). Only the *final* component is affected;
    ///                   intermediate symlinks are always traversed.
    ///
    /// @returns the file's `Metadata`, or an `io::Error`, e.g. `ENOENT` if the path does not exist, `EACCES`
    ///          on a permission failure, or `ENOTDIR` if a path component is not a directory.
    NOELDOC_SINCE("26.07")
    static auto For(PathRef path, SymlinkResolution resolution = SymlinkResolution::Follow) -> io::Result<Metadata>;
};

} // namespace violet::filesystem
