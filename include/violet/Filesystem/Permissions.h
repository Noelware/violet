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

#include "violet/Violet.h"

#include <ostream>

#ifdef VIOLET_UNIX
#include <sys/stat.h>
#include <sys/types.h>
#elif defined(VIOLET_WINDOWS)
#include <windows.h>
#endif

namespace violet::filesystem {

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
/// #include <violet/Filesystem/Permissions.h>
///
/// using namespace violet::filesystem;
///
/// struct stat st{};
/// lstat("/usr/bin/clang", &st);
///
/// Mode mode(st.st_mode);
/// std::cout << mode << '\n'; // -rw-r--r--
/// ```
struct VIOLET_API Mode final {
    constexpr VIOLET_IMPLICIT Mode() = default;
    constexpr VIOLET_IMPLICIT Mode(mode_t mode) noexcept
        : n_mode(mode)
    {
    }

    constexpr VIOLET_EXPLICIT Mode(struct stat& st) noexcept
        : n_mode(st.st_mode)
    {
    }

    [[nodiscard]] auto ToString() const noexcept -> String;
    friend auto operator<<(std::ostream& os, const Mode& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

#define IMPL_MODE(IDENT, BIT)                                                                                          \
    constexpr auto IDENT() const noexcept -> bool                                                                      \
    {                                                                                                                  \
        return (this->n_mode & BIT) != 0U;                                                                             \
    }

    IMPL_MODE(OwnerCanRead, S_IRUSR);
    IMPL_MODE(OwnerCanWrite, S_IWUSR);
    IMPL_MODE(OwnerCanExecute, S_IXUSR);

    IMPL_MODE(OtherCanRead, S_IROTH);
    IMPL_MODE(OtherCanWrite, S_IWOTH);
    IMPL_MODE(OtherCanExecute, S_IXOTH);

    IMPL_MODE(HasSetUID, S_ISUID);
    IMPL_MODE(HasSetGID, S_ISGID);
    IMPL_MODE(Sticky, S_ISVTX);

#undef IMPL_MODE

#define IMPL_OP(OP)                                                                                                    \
    constexpr auto operator OP(Mode mode) const noexcept -> Mode                                                       \
    {                                                                                                                  \
        return Mode(this->n_mode OP mode.n_mode);                                                                      \
    }                                                                                                                  \
                                                                                                                       \
    constexpr auto operator OP(mode_t mode) const noexcept -> Mode                                                     \
    {                                                                                                                  \
        return Mode(this->n_mode OP mode);                                                                             \
    }

    IMPL_OP(|);
    IMPL_OP(&);

#undef IMPL_OP

#define IMPL_OP(OP)                                                                                                    \
    constexpr auto operator OP(Mode mode) noexcept -> Mode                                                             \
    {                                                                                                                  \
        this->n_mode OP mode.n_mode;                                                                                   \
        return *this;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    constexpr auto operator OP(mode_t mode) noexcept -> Mode                                                           \
    {                                                                                                                  \
        this->n_mode OP mode;                                                                                          \
        return *this;                                                                                                  \
    }

    IMPL_OP(|=);
    IMPL_OP(&=);

#undef IMPL_OP

    constexpr VIOLET_EXPLICIT operator mode_t() const noexcept
    {
        return this->n_mode;
    }

    constexpr auto operator==(mode_t mode) const noexcept -> bool
    {
        return this->n_mode == mode;
    }

    constexpr auto operator!=(mode_t mode) const noexcept -> bool
    {
        return this->n_mode != mode;
    }

    constexpr auto operator<=>(const Mode&) const noexcept = default;
    constexpr auto operator==(const Mode&) const noexcept -> bool = default;

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
struct VIOLET_API Permissions final {
    constexpr VIOLET_IMPLICIT Permissions() noexcept = default;

#ifdef VIOLET_WINDOWS
    /// Constructs a [`Permissions`] object from a `DWORD` representing the file attributes.
    /// @param attrs the attributes that this file sets (i.e, `FILE_ATTRIBUTE_READONLY`).
    constexpr VIOLET_EXPLICIT Permissions(DWORD attrs) noexcept
        : n_attrs(attrs)
    {
    }
#elif defined(VIOLET_UNIX)
    constexpr VIOLET_EXPLICIT Permissions(struct Mode mode) noexcept
        : n_mode(mode)
    {
    }
#endif

    [[nodiscard]] auto Readonly() const noexcept -> bool;
    void SetReadonly(bool readonly) noexcept;

#ifdef VIOLET_WINDOWS
    auto Attributes() const noexcept -> DWORD;
#elif defined(VIOLET_UNIX)
    [[nodiscard]] auto Mode() const noexcept -> struct Mode;
#endif

    [[nodiscard]] auto ToString() const noexcept -> String;
    friend auto operator<<(std::ostream& os, const Permissions& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

    constexpr auto operator<=>(const Permissions&) const noexcept = default;
    constexpr auto operator==(const Permissions&) const noexcept -> bool = default;

private:
#ifdef VIOLET_WINDOWS
    DWORD n_attrs;
#elif defined(VIOLET_UNIX)
    violet::filesystem::Mode n_mode;
#endif
};

} // namespace violet::filesystem

VIOLET_FORMATTER(violet::filesystem::Permissions);

#ifdef VIOLET_UNIX
VIOLET_FORMATTER(violet::filesystem::Mode);
#endif
