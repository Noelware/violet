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
//
//! # 🌺💜 `violet/filesystem/Permissions.h`

#pragma once

#include "violet/violet.h"

#ifdef VIOLET_UNIX
#    include <sys/stat.h>
#    include <sys/types.h>
#endif

namespace Noelware::Violet::Filesystem {

#ifdef VIOLET_UNIX
/// Represents a Unix file mode's permissions.
///
/// This is a lightweight wrapper around the `mode_t` type that provides:
/// - Better accessors for file permissions (i.e, if owner can read/write/exec)
/// - A `ToString()` method that formats `mode_t` like `ls -l` (`-rwxr-xr-x`)
/// - Comparison and bitwise operators for convenience.
///
/// ## Example
/// ```cpp
/// #include "violet/filesystem/Permissions.h"
///
/// using namespace Noelware::Violet::Filesystem;
///
/// struct stat st{};
/// lstat("/usr/bin/clang", &st);
///
/// Mode mode(st.st_mode);
/// std::cout << mode << '\n'; // -rw-r--r--
/// ```
struct Mode final {
    /// Constructs a default [`Mode`] with zero bits.
    constexpr Mode() = default;

    /// Constructs this [`Mode`] with a `mode_t` value.
    /// @param mode the mode to use
    constexpr VIOLET_EXPLICIT Mode(mode_t mode) noexcept;

    /// Constructs a [`Mode`] object with the result of a [`stat(2)`] or [`lstat(2)`] call.
    /// @param st the stat object itself
    constexpr VIOLET_EXPLICIT Mode(struct stat& st) noexcept;

    /// Returns **true** if the owner of this file can read this file.
    [[nodiscard]] constexpr auto OwnerCanRead() const noexcept -> bool;

    /// Returns **true** if the owner of this file can write into this file.
    [[nodiscard]] constexpr auto OwnerCanWrite() const noexcept -> bool;

    /// Returns **true** if the owner of this file can execute this file.
    [[nodiscard]] constexpr auto OwnerCanExecute() const noexcept -> bool;

    /// Returns **true** if others can read this file.
    [[nodiscard]] constexpr auto OtherCanRead() const noexcept -> bool;

    /// Returns **true** if others can write into this file.
    [[nodiscard]] constexpr auto OtherCanWrite() const noexcept -> bool;

    /// Returns **true** if others can execute this file.
    [[nodiscard]] constexpr auto OtherCanExecute() const noexcept -> bool;

    /// Returns **true** if the set-user-ID bit is set.
    [[nodiscard]] constexpr auto HasSetUID() const noexcept -> bool;

    /// Returns **true** if the set-group-ID bit is set.
    [[nodiscard]] constexpr auto HasSetGID() const noexcept -> bool;

    /// Returns **true** if the **sticky** bit is set.
    [[nodiscard]] constexpr auto Sticky() const noexcept -> bool;

    constexpr auto operator<=>(const Mode&) const noexcept = default;
    constexpr auto operator==(const Mode&) const noexcept -> bool = default;
    constexpr auto operator|(Mode rhs) const noexcept -> Mode;
    constexpr auto operator|(mode_t rhs) const noexcept -> Mode;
    constexpr auto operator&(Mode rhs) const noexcept -> Mode;
    constexpr auto operator&(mode_t rhs) const noexcept -> Mode;
    constexpr auto operator|=(Mode rhs) noexcept -> Mode;
    constexpr auto operator|=(mode_t rhs) noexcept -> Mode;
    constexpr auto operator&=(Mode rhs) noexcept -> Mode;
    constexpr auto operator&=(mode_t rhs) noexcept -> Mode;

    constexpr VIOLET_EXPLICIT operator mode_t() const noexcept;

private:
    mode_t n_mode;
};
#endif

/// A cross-platform, lightweight file permission abstraction.
///
/// This provides a unified representation of querying and modifying a file's permissions. This will
/// not call the filesystem itself to set the permissions itself.
///
/// ## Windows
/// On Windows, this will hold a `DWORD` of the file attributes for this file.
///
/// ## POSIX (Linux, macOS)
/// On POSIX-compatible systems, this will hold a [`Mode`] struct that wraps around `mode_t`.
struct Permissions final {
#ifdef VIOLET_UNIX
    struct Mode Mode; ///< POSIX file mode representation, also encapsulates `mode_t`-based permissions.
#endif

    /// Constructs a empty set of permissions for this file.
    constexpr Permissions() noexcept = default;

#ifdef VIOLET_WINDOWS
    /// Constructs a [`Permissions`] object from a `DWORD` representing the file attributes.
    /// @param attrs the attributes that this file sets (i.e, `FILE_ATTRIBUTE_READONLY`).
    constexpr VIOLET_EXPLICIT Permissions(DWORD attrs) noexcept;
#elif defined(VIOLET_UNIX)
    constexpr VIOLET_EXPLICIT Permissions(struct Mode mode) noexcept;
#endif

    [[nodiscard]] constexpr auto Readonly() const noexcept -> bool;
    constexpr void SetReadonly(bool readonly) noexcept;

    constexpr auto operator<=>(const Permissions&) const noexcept = default;
    constexpr auto operator==(const Permissions&) const noexcept -> bool = default;

#ifdef VIOLET_WINDOWS
    constexpr VIOLET_EXPLICIT operator DWORD() const noexcept;
#elif defined(VIOLET_UNIX)
    constexpr VIOLET_EXPLICIT operator Noelware::Violet::Filesystem::Mode() const noexcept;
    constexpr VIOLET_EXPLICIT operator mode_t() const noexcept;
#endif

private:
#ifdef VIOLET_WINDOWS
    DWORD n_attrs; ///< Underlying Windows file attributes.
#endif
};

} // namespace Noelware::Violet::Filesystem
