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

#include <violet/Violet.h>

#ifdef VIOLET_UNIX

#include <violet/Filesystem.h>
#include <violet/Filesystem/Temporary.h>
#include <violet/System.h>

using violet::Err;
using violet::String;
using violet::UInt;
using violet::UInt32;
using violet::UInt8;
using violet::Vec;
using violet::filesystem::OpenOptions;
using violet::filesystem::TempBuilder;
using violet::filesystem::TempDir;
using violet::filesystem::TempFile;
using violet::io::Result;

namespace {
auto genRandomBytes(UInt32 bits) -> Result<String>
{
    auto file = OpenOptions().Read().Open("/dev/urandom");
    if (!file) {
        return Err(VIOLET_MOVE(file.Error()));
    }

    Vec<UInt8> out(bits, '\0');
    UInt read = 0;

    while (read < bits) {
        auto* ptr = out.data() + read;
        auto remaining = bits - read;

        read += VIOLET_TRY(file->Read({ ptr, remaining }));
    }

    String name;
    name.reserve(out.size() * 2);

    constexpr static char hex[] = "0123456789abcdef";
    for (const UInt8 byte: out) {
        name.push_back(hex[byte >> 4]);
        name.push_back(hex[byte & 0xF]);
    }

    return name;
}
} // namespace

auto violet::filesystem::SystemTempDirectory() -> io::Result<Path>
{
    if (auto value = sys::GetEnv("TMPDIR")) {
        return Path(*value);
    }

    // TODO(@auguwu/Noel): once we support macOS, we should
    // use `confstr` to get the temporary directory, as recommended
    // from Apple themselves.

    return Path("/tmp");
}

auto TempBuilder::MkDir() const noexcept -> io::Result<TempDir>
{
    auto sys = VIOLET_TRY(SystemTempDirectory());
    for (;;) {
        auto name = VIOLET_TRY(genRandomBytes(this->n_randomness));
        auto path = sys.Join(std::format("{}{}{}", this->n_prefix, name, this->n_suffix));
        auto dir = filesystem::CreateDirectory(path);
        if (dir.Err()) {
            auto err = dir.Error();
            if (err.RawOSError().HasValueAnd([](const auto& value) -> bool { return value != EEXIST; })) {
                return Err(VIOLET_MOVE(err));
            }

            continue;
        }

        return TempDir(path);
    }
}

auto TempBuilder::MkFile() const noexcept -> io::Result<TempFile>
{
    auto sys = VIOLET_TRY(SystemTempDirectory());
    for (;;) {
        auto name = VIOLET_TRY(genRandomBytes(this->n_randomness));
        auto path = sys.Join(std::format("{}{}{}", this->n_prefix, name, this->n_suffix));
        auto file = OpenOptions().CreateNew().Mode(this->n_mode).Open(path);
        if (file.Err()) {
            auto err = file.Error();
            if (err.RawOSError().HasValueAnd([](const auto& value) -> bool { return value != EEXIST; })) {
                std::cerr << "path: " << path.ToString() << '\n';
                return Err(VIOLET_MOVE(err));
            }

            continue;
        }

        return TempFile(VIOLET_MOVE(file.Value()), path);
    }
}

TempDir::TempDir(PathRef path) noexcept
    : n_path(path)
{
}

TempDir::TempDir(TempDir&& other) noexcept
    : n_released(other.n_released)
    , n_path(VIOLET_MOVE(other.n_path))
{
    other.n_path = {};
    other.n_released = true;
}

TempDir::~TempDir()
{
    if (!this->n_released && !this->n_path.Empty()) {
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wunused-value")

        filesystem::RemoveAllDirs(this->n_path);

        VIOLET_DIAGNOSTIC_POP

        this->n_path = {};
    }
}

auto TempDir::Release() noexcept -> filesystem::Path
{
    this->n_released = true;
    return this->n_path;
}

TempFile::TempFile(violet::filesystem::File file, Optional<violet::filesystem::Path> explicitPath) noexcept
    : n_file(VIOLET_MOVE(file))
    , n_explicitPath(VIOLET_MOVE(explicitPath))
{
}

TempFile::TempFile(TempFile&& other) noexcept
    : n_file(VIOLET_MOVE(other.n_file))
    , n_explicitPath(VIOLET_MOVE(other.n_explicitPath))
{
    other.n_explicitPath = Nothing;
}

TempFile::~TempFile()
{
    if (!this->n_persist) {
        if (auto path = this->n_explicitPath; !path->Empty()) {
            VIOLET_DIAGNOSTIC_PUSH
            VIOLET_DIAGNOSTIC_IGNORE("-Wunused-value")

            filesystem::RemoveAllDirs(*path);

            VIOLET_DIAGNOSTIC_POP

            this->n_explicitPath.Reset();
        }
    }
}

auto TempFile::File() const noexcept -> const struct File&
{
    return this->n_file;
}

auto TempFile::Path() const noexcept -> const Optional<struct Path>&
{
    return this->n_explicitPath;
}

auto TempFile::Persist(PathRef dst) noexcept -> io::Result<struct File>
{
    if (!this->n_explicitPath) {
        return Err(VIOLET_IO_ERROR(InvalidData, String, "this temporary file has already persisted somewhere else"));
    }

    auto res = filesystem::Rename(*this->n_explicitPath, dst);
    if (res.Err()) {
        // TODO(@auguwu/Noel): deal with cross-device errors (fallback to copy+delete)
        return Err(VIOLET_MOVE(res.Error()));
    }

    this->n_explicitPath.Reset();
    return filesystem::File({ this->n_file.Descriptor() });
}

#endif
