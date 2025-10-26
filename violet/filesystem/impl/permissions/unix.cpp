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

#include "violet/violet.h"

#ifdef VIOLET_UNIX

// clang-format off
#include "violet/filesystem/Permissions.h"

#include <sys/stat.h>
#include <sys/types.h>
// clang-format on

using Noelware::Violet::Filesystem::Mode;
using Noelware::Violet::Filesystem::Permissions;

constexpr Mode::Mode(mode_t mode) noexcept
    : n_mode(mode)
{
}

constexpr Mode::Mode(struct stat& st) noexcept
    : n_mode(st.st_mode)
{
}

// clang-format off
#define IMPL_MODE(IDENT, BIT)                                                                                      \
    constexpr auto Mode::IDENT() const noexcept -> bool                                                            \
    {                                                                                                              \
        return (this->n_mode & BIT) != 0U;                                                                         \
    }
// clang-format on

IMPL_MODE(OwnerCanRead, S_IRUSR);
IMPL_MODE(OwnerCanWrite, S_IWUSR);
IMPL_MODE(OwnerCanExecute, S_IXUSR);

IMPL_MODE(OtherCanRead, S_IROTH);
IMPL_MODE(OtherCanWrite, S_IWOTH);
IMPL_MODE(OtherCanExecute, S_IXOTH);

IMPL_MODE(HasSetUID, S_ISUID);
IMPL_MODE(HasSetGID, S_ISGID);
IMPL_MODE(Sticky, S_ISVTX);

// clang-format off
#undef IMPL_MODE
// clang-format on

// clang-format off
#define IMPL_OP(OP)                                                                                                \
    constexpr auto Mode::operator OP(Mode mode) const noexcept -> Mode                                             \
    {                                                                                                              \
        return Mode(this->n_mode OP mode.n_mode);                                                                  \
    }                                                                                                              \
                                                                                                                   \
    constexpr auto Mode::operator OP(mode_t mode) const noexcept -> Mode {                                         \
        return Mode(this->n_mode OP mode);                                                                         \
    }
// clang-format on

IMPL_OP(|);
IMPL_OP(&);

// clang-format off
#undef IMPL_OP
// clang-format on

// clang-format off
#define IMPL_OP(OP)                                                                                                \
    constexpr auto Mode::operator OP(Mode mode) noexcept -> Mode                                                   \
    {                                                                                                              \
        this->n_mode OP mode.n_mode;                                                                               \
        return *this;                                                                                              \
    }                                                                                                              \
                                                                                                                   \
    constexpr auto Mode::operator OP(mode_t mode) noexcept -> Mode {                                               \
        this->n_mode OP mode;                                                                                      \
        return *this;                                                                                              \
    }
// clang-format on

IMPL_OP(|=);
IMPL_OP(&=);

// clang-format off
#undef IMPL_OP
// clang-format on

constexpr Mode::operator mode_t() const noexcept
{
    return this->n_mode;
}

constexpr Permissions::Permissions(struct Mode mode) noexcept
    : Mode(mode)
{
}

constexpr auto Permissions::Readonly() const noexcept -> bool
{
    return (static_cast<mode_t>(this->Mode) & 0222) != 0U;
}

constexpr void Permissions::SetReadonly(bool readonly) noexcept
{
    if (readonly) {
        // NOLINTNEXTLINE(readability-implicit-bool-conversion,modernize-use-bool-literals)
        this->Mode &= !0222;
    } else {
        this->Mode |= 0222;
    }
}

constexpr Permissions::operator Noelware::Violet::Filesystem::Mode() const noexcept
{
    return this->Mode;
}

constexpr Permissions::operator mode_t() const noexcept
{
    return static_cast<mode_t>(this->Mode);
}

#endif
