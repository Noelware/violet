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

#include "violet/container/Result.h"
#include "violet/filesystem/Path.h"
#include "violet/violet.h"

#ifdef _WIN32
#    include <windows.h>
#else
#    include <sys/types.h>
#endif

namespace Noelware::Violet::Filesystem {

struct Path;
struct File;

/// Builder for opening a [`File`], analgous to Rust's [`std::fs::OpenOptions`].
///
/// [`std::fs::OpenOptions`]: https://doc.rust-lang.org/1.90.0/std/fs/struct.OpenOptions.html
struct OpenOptions final {
    /// Creates a new builder instance.
    OpenOptions() = default;

    auto Read(bool yes = true) noexcept -> OpenOptions&
    {
        this->n_read = yes;
        return *this;
    }

    auto Write(bool yes = true) noexcept -> OpenOptions&
    {
        this->n_write = yes;
        return *this;
    }

    auto Create(bool yes = true) noexcept -> OpenOptions&
    {
        this->n_create = yes;
        return *this;
    }

    auto Truncate(bool yes = true) noexcept -> OpenOptions&
    {
        this->n_truncate = yes;
        return *this;
    }

    auto Append(bool yes = true) noexcept -> OpenOptions&
    {
        this->n_append = yes;
        return *this;
    }

#ifdef _WIN32
    auto Attributes(DWORD attributes) noexcept -> OpenOptions&
    {
        this->n_attributes = attributes;
        return *this;
    }
#else
    auto Mode(mode_t mode) noexcept -> OpenOptions&
    {
        this->n_mode = mode;
        return *this;
    }
#endif

    [[nodiscard]] auto Open() const noexcept -> Result<File, usize>;

private:
    bool n_read = false;
    bool n_write = false;
    bool n_create = false;
    bool n_append = false;
    bool n_truncate = false;
    bool n_createNew = false;

#ifdef _WIN32
    DWORD n_attributes = FILE_ATTRIBUTE_NORMAL;
#else
    mode_t n_mode = 0644;
#endif
};

/// A RAII-based wrapper around Unix file descriptors or Windows' `HANDLE`.
struct File final {
    File() = delete;
    VIOLET_EXPLICIT File(Path path)
        : n_path(VIOLET_MOVE(path))
    {
    }

private:
#if defined(_WIN32)
    HANDLE n_handle = INVALID_HANDLE_VALUE;
#else
    constexpr static auto n_invalidFd = -1;
    int32 fd = n_invalidFd;
#endif

    friend struct OpenOptions;

    Path n_path;

    void doOpen() const noexcept;
};

} // namespace Noelware::Violet::Filesystem
