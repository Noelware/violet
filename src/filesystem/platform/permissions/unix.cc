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

#include <violet/Violet.h>

#ifdef VIOLET_UNIX

#include <violet/Filesystem/Permissions.h>

#include <ostream>
#include <sys/stat.h>

using violet::String;
using violet::filesystem::Mode;
using violet::filesystem::Permissions;

auto Mode::ToString() const noexcept -> String
{
    char buf[11];

    if (S_ISREG(this->n_mode)) {
        buf[0] = '-';
    } else if (S_ISDIR(this->n_mode)) {
        buf[0] = 'd';
    } else if (S_ISLNK(this->n_mode)) {
        buf[0] = 'l';
    } else if (S_ISCHR(this->n_mode)) {
        buf[0] = 'c';
    } else if (S_ISBLK(this->n_mode)) {
        buf[0] = 'b';
    } else if (S_ISFIFO(this->n_mode)) {
        buf[0] = 'p';
    } else if (S_ISSOCK(this->n_mode)) {
        buf[0] = 's';
    } else {
        buf[0] = '?';
    }

    // usr
    buf[1] = this->OwnerCanRead() ? 'r' : '-';
    buf[2] = this->OwnerCanWrite() ? 'w' : '-';
    buf[3] = this->OwnerCanExecute()
        ? this->HasSetUID() ? 's' : 'x' // NOLINT(readability-avoid-nested-conditional-operator)
        : this->HasSetUID() ? 'S'
                            : '-';

    // grp
    buf[4] = ((this->n_mode & S_IRGRP) != 0U) ? 'r' : '-';
    buf[5] = ((this->n_mode & S_IWGRP) != 0U) ? 'w' : '-';
    buf[6] = ((this->n_mode & S_IXGRP) != 0U)
        ? this->HasSetGID() ? 's' : 'x' // NOLINT(readability-avoid-nested-conditional-operator)
        : this->HasSetGID() ? 'S'
                            : '-';

    // other
    buf[7] = this->OtherCanRead() ? 'r' : '-';
    buf[8] = this->OtherCanWrite() ? 'w' : '-';
    buf[9] = this->OtherCanExecute() ? this->Sticky() // NOLINT(readability-avoid-nested-conditional-operator)
            ? 't'
            : 'x'
        : this->Sticky()             ? 'T'
                                     : '-';

    buf[10] = '\0';
    return { buf };
}

auto Mode::operator<<(std::ostream& os) const noexcept -> std::ostream&
{
    return os << this->ToString();
}

#endif

constexpr const auto kReadonly = S_IWUSR | S_IWGRP | S_IWOTH;

auto Permissions::Readonly() const noexcept -> bool
{
    return (static_cast<mode_t>(this->n_mode) & kReadonly) == 0U;
}

void Permissions::SetReadonly(bool readonly) noexcept
{
    if (readonly) {
        // NOLINTNEXTLINE(readability-implicit-bool-conversion,modernize-use-bool-literals)
        this->n_mode &= ~kReadonly;
    } else {
        this->n_mode |= kReadonly;
    }
}

auto Permissions::Mode() const noexcept -> violet::filesystem::Mode
{
    return this->n_mode;
}

auto Permissions::ToString() const noexcept -> String
{
    return std::format("Permissions(readonly={}, mode=\"{}\")", this->Readonly(), this->Mode().ToString());
}

auto Permissions::operator<<(std::ostream& os) const noexcept -> std::ostream&
{
    return os << this->ToString();
}
