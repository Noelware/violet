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

#include "violet/filesystem/File.h"
#include "violet/filesystem/Filesystem.h"
#include "violet/filesystem/Path.h"
#include "violet/io/Error.h"

#include <random>
#include <utility>

namespace Noelware::Violet::Filesystem {

/// Returns the path of the system's temporary directory.
///
/// - **Unix** (macOS, Linux): Uses the `$TMPDIR` environment variable or `/tmp` if it doesn't exist.
/// - **Windows**: Uses the `GetTempPathW` function to retrieve the path.
auto SystemTempDirectory() -> IO::Result<Path>;

struct TempDir final {
    static auto New(StringRef prefix = "violet-tmp-") -> IO::Result<TempDir>
    {
        auto base = SystemTempDirectory();
        if (!base) {
            return Err(base.Error());
        }

        Filesystem::Path dir = base.Value().Join(generateRandomizedName(prefix));
        auto created = CreateDirectory(dir);
        if (!created) {
            return Err(created.Error());
        }

        return TempDir(VIOLET_MOVE(dir));
    }

    ~TempDir()
    {
        if (!this->n_keep) {
            assert(Filesystem::RemoveDirectory(this->n_path));
        }
    }

    TempDir(const TempDir&) = delete;
    auto operator=(const TempDir&) -> TempDir& = delete;

    TempDir(TempDir&& other) noexcept
        : n_path(std::exchange(other.n_path, {}))
        , n_keep(std::exchange(other.n_keep, false))
    {
    }

    auto operator=(TempDir&& other) noexcept -> TempDir&
    {
        if (this != &other) {
            this->~TempDir();

            n_path = std::exchange(other.n_path, {});
            n_keep = std::exchange(other.n_keep, false);
        }

        return *this;
    }

    [[nodiscard]] constexpr auto Path() noexcept -> Filesystem::Path
    {
        return this->n_path;
    }

    [[nodiscard]] auto Join(StringRef path) const -> Filesystem::Path
    {
        return this->n_path.Join(path);
    }

    auto CreateFile(StringRef path) -> IO::Result<File>
    {
        OpenOptions opts = OpenOptions().Read().Write().Create().Truncate();
        return File::Open(this->n_path.Join(path), opts);
    }

    void Keep(bool yes = true) noexcept
    {
        this->n_keep = yes;
    }

    [[nodiscard]] auto WillKeep() const noexcept -> bool
    {
        return this->n_keep;
    }

private:
    VIOLET_EXPLICIT
    TempDir(struct Path path)
        : n_path(VIOLET_MOVE(path))
    {
    }

    Filesystem::Path n_path;
    bool n_keep = false;

    static auto generateRandomizedName(StringRef prefix) noexcept -> String
    {
        static thread_local std::mt19937 rng(std::random_device{}());
        static constexpr char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";

        std::uniform_int_distribution<usize> distrib(0, sizeof(chars) - 2);
        String out(prefix.Data(), prefix.Length());
        for (usize i = 0; i < 8; i++) {
            out += chars[distrib(rng)];
        }

        return out;
    }
};

} // namespace Noelware::Violet::Filesystem
