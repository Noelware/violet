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

#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Path.h>
#include <violet/Filesystem/Permissions.h>
#include <violet/IO/Error.h>

namespace violet::filesystem {

struct TempFile;
struct TempDir;

/// Returns the system's temporary directory.
auto SystemTempDirectory() -> io::Result<Path>;

/// A builder that is used to build [`TempDir`] or [`TempFile`]s.
struct TempBuilder final {
    constexpr VIOLET_IMPLICIT TempBuilder() noexcept = default;

    constexpr auto WithRandomBytes(UInt32 bits) noexcept -> TempBuilder&
    {
        this->n_randomness = bits;
        return *this;
    }

    constexpr auto WithPrefix(Str prefix) noexcept -> TempBuilder&
    {
        this->n_prefix = prefix;
        return *this;
    }

    constexpr auto WithSuffix(Str suffix) noexcept -> TempBuilder&
    {
        this->n_suffix = suffix;
        return *this;
    }

#ifdef VIOLET_UNIX
    constexpr auto WithMode(Mode mode) noexcept -> TempBuilder&
    {
        this->n_mode = mode;
        return *this;
    }
#endif

    [[nodiscard]] auto MkFile() const noexcept -> io::Result<TempFile>;
    [[nodiscard]] auto MkDir() const noexcept -> io::Result<TempDir>;

private:
    UInt32 n_randomness = 8;
    String n_prefix = "violet-";
    String n_suffix;

#ifdef VIOLET_UNIX
    Mode n_mode = 0600;
#endif
};

/// Represents a directory that is temporarily avaliable. This also acts
/// like a RAII guard where, on drop, it'll remove the directory from disk.
struct TempDir final {
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(TempDir);
    VIOLET_DISALLOW_CONSTEXPR_COPY(TempDir);

    ~TempDir();
    VIOLET_IMPLICIT TempDir(TempDir&& other) noexcept;
    auto operator=(TempDir&& other) noexcept -> TempDir&;

    [[nodiscard]] constexpr auto Path() const noexcept -> const filesystem::Path&
    {
        return this->n_path;
    }

    auto Release() noexcept -> filesystem::Path;

private:
    friend struct TempBuilder;

    VIOLET_EXPLICIT TempDir(PathRef path) noexcept;

    bool n_released = false;
    filesystem::Path n_path;
};

struct TempFile final {
    VIOLET_DISALLOW_CONSTEXPR_COPY(TempFile);
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(TempFile);

    ~TempFile();
    VIOLET_IMPLICIT TempFile(TempFile&& other) noexcept;
    auto operator=(TempFile&& other) noexcept -> TempFile&;

    auto File() const noexcept -> const File&;
    auto Path() const noexcept -> const Optional<Path>&;
    auto Persist(PathRef dst) noexcept -> io::Result<struct File>;

private:
    friend struct TempBuilder;

    VIOLET_EXPLICIT TempFile(filesystem::File file, Optional<filesystem::Path> explicitPath = Nothing) noexcept;

    filesystem::File n_file;
    Optional<filesystem::Path> n_explicitPath;
    bool n_persist = false;
};

} // namespace violet::filesystem
