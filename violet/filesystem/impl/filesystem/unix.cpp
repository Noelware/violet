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

#include "violet/filesystem/Filesystem.h"
#include "violet/filesystem/Path.h"
#include "violet/io/Error.h"
#include "violet/support/StringRef.h"
#include "violet/violet.h"
#include <sys/stat.h>

#ifdef VIOLET_UNIX

auto Noelware::Violet::Filesystem::Dirs::Next() noexcept -> Optional<IO::Result<Path>>
{
    if (this->n_dir == nullptr) {
        return Nothing;
    }

    // Overwrite any errors that might've happend (for now)
    errno = 0;
    if (auto* entry = ::readdir(this->n_dir)) {
        auto path = this->n_parent.Join(StringRef(static_cast<CStr>(entry->d_name)));
        return Some<IO::Result<Path>>(path);
    }

    return errno != 0 ? Some<IO::Result<Path>>(IO::Error::Platform(IO::ErrorKind::Other)) : Nothing;
}

auto Noelware::Violet::Filesystem::Exists(const Path& path) -> IO::Result<bool>
{
    struct stat st{};
    if (::lstat(static_cast<StringRef>(path), &st) == 0) {
        return true;
    }

    auto err = IO::Error::Platform(IO::ErrorKind::Other);
    assert(err.RawOSError());

    if (err.RawOSError().HasValueAnd([](int value) { return value == ENOENT || value == ENOTDIR; })) {
        return false;
    }

    return Err(err);
}

auto Noelware::Violet::Filesystem::CreateDirectory(const Path& path) -> IO::Result<void>
{
    if (path.Empty()) {
        return {};
    }

    if (mkdir(static_cast<StringRef>(path), 0755) == 0) {
        return {};
    }

    auto error = IO::Error::Platform(IO::ErrorKind::InvalidData);
    assert(error.RawOSError());

    auto err = *error.RawOSError();
    if (err == EEXIST) {
        return {};
    }

    return Err(error);
}

auto Noelware::Violet::Filesystem::CreateDirectories(const Path& path) -> IO::Result<void>
{
    if (path.Empty() || path.IsRoot()) {
        return {};
    }

    auto exists = Exists(path);
    if (!exists) {
        return Err(exists.Error());
    }

    if (auto parent = path.Parent(); !parent->Empty()) {
        auto res = CreateDirectories(*parent);
        if (!res) {
            return res;
        }
    }

    auto res = CreateDirectory(path);
    if (!res) {
        if (res.Error().RawOSError().HasValueAnd([](int err) { return err == EEXIST; })) {
            return {};
        }

        return res;
    }

    return {};
}

auto Noelware::Violet::Filesystem::ReadDir(const Path& path) -> IO::Result<Dirs>
{
    auto* dir = ::opendir(static_cast<StringRef>(path));
    if (dir == nullptr) {
        return Err(IO::Error::Platform(IO::ErrorKind::InvalidInput));
    }

    return Dirs(path, dir);
}

auto Noelware::Violet::Filesystem::RemoveDirectory(const Path& path) -> IO::Result<void>
{
    if (::rmdir(static_cast<StringRef>(path)) != 0) {
        return Err(IO::Error::Platform(IO::ErrorKind::Other));
    }

    return {};
}

#endif
