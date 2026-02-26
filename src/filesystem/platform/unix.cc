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
#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Path.h>
#include <violet/IO/Error.h>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using violet::CStr;
using violet::UInt64;
using violet::filesystem::Dirs;
using violet::filesystem::PathRef;
using violet::filesystem::WalkDirs;

namespace {
auto getMillisecondsFromTimespec(const timespec& ts) noexcept -> UInt64
{
    return (static_cast<UInt64>(ts.tv_sec) * 1000ULL) + (static_cast<UInt64>(ts.tv_nsec) / 1'000'000ULL);
}
} // namespace

struct Dirs::Impl final {
    Impl(PathRef root, DIR* entry)
        : n_root(root)
        , n_entries(entry)
    {
    }

    ~Impl()
    {
        if (this->n_entries != nullptr)
            ::closedir(this->n_entries);
    }

    Impl(const Dirs::Impl&) = delete;
    auto operator=(const Dirs::Impl&) -> Dirs::Impl& = delete;

    Impl(Impl&&) = delete;
    auto operator=(Impl&&) -> Dirs::Impl& = delete;

    auto Next() noexcept -> Optional<io::Result<DirEntry>>
    {
        struct dirent* entry; // NOLINT(cppcoreguidelines-init-variables)
        while ((entry = ::readdir(this->n_entries)) != nullptr) {
            String name = entry->d_name;
            if (name == "." || name == "..")
                continue;

            this->n_entry = entry;
            break;
        }

        Path path = this->n_root.Join(this->n_entry->d_name);

        // We **could** make this optimized if we only wanted to return a file type, but it's probably
        // a lot more useful to return the metadata of the file.
        auto metadata = filesystem::Metadata(static_cast<PathRef>(path), /*followSymlinks=*/false);
        if (metadata.Err()) {
            return Some<io::Result<DirEntry>>(Err(metadata.Error()));
        }

        DirEntry direntry = { .Path = path, .Metadata = VIOLET_MOVE(metadata.Value()) };
        return Some<io::Result<DirEntry>>(Ok<DirEntry, io::Error>(direntry));
    }

private:
    PathRef n_root;
    DIR* n_entries;
    struct dirent* n_entry = nullptr;
};

template<typename... Args>
    requires(std::is_constructible_v<Dirs::Impl, Args...>)
Dirs::Dirs(Args&&... args)
    : n_impl(new Impl(VIOLET_FWD(Args, args)...))
{
}

Dirs::~Dirs() noexcept
{
    if (this->n_impl != nullptr) {
        delete this->n_impl;
        this->n_impl = nullptr;
    }
}

auto Dirs::Next() noexcept -> Optional<Dirs::Item>
{
    return this->n_impl->Next();
}

auto violet::filesystem::ReadDir(PathRef path) -> io::Result<Dirs>
{
    DIR* entries = nullptr;
    if ((entries = ::opendir(static_cast<CStr>(path))) == nullptr) {
        return Err(io::Error::OSError());
    }

    return Dirs(path, entries);
}

struct WalkDirs::Impl final {
    struct Frame final {
        DIR* DirEntry = nullptr;
        struct filesystem::Path Path;
    };

    VIOLET_DISALLOW_COPY_AND_MOVE(Impl);

    VIOLET_EXPLICIT Impl(Vec<Frame> stack)
        : n_stack(VIOLET_MOVE(stack))
    {
    }

    ~Impl()
    {
        for (const auto& frame: this->n_stack) {
            VIOLET_DEBUG_ASSERT(frame.DirEntry != nullptr, "existence of a dirent pointer failed");
            ::closedir(frame.DirEntry);
        }
    }

    auto Next() -> Optional<WalkDirs::Item>
    {
        while (!this->n_stack.empty()) {
            auto& frame = this->n_stack.back();

            errno = 0;
            dirent* ent = ::readdir(frame.DirEntry);
            if (ent == nullptr) {
                ::closedir(frame.DirEntry);
                this->n_stack.pop_back();

                continue;
            }

            CStr name = ent->d_name;
            if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
                continue;
            }

            Path path = frame.Path.Join(name);
            auto metadata = filesystem::Metadata(static_cast<PathRef>(path), /*followSymlinks=*/false);
            if (metadata.Err()) {
                return Err(VIOLET_MOVE(metadata.Error()));
            }

            if (metadata->Type.Dir()) {
                if (DIR* sub = ::opendir(path.Data().c_str()); sub != nullptr) {
                    this->n_stack.push_back({ .DirEntry = sub, .Path = path });
                }
            }

            return DirEntry{ .Path = path, .Metadata = metadata.Value() };
        }

        return Nothing;
    }

private:
    Vec<Frame> n_stack;
};

template<typename... Args>
    requires(std::is_constructible_v<WalkDirs::Impl, Args...>)
WalkDirs::WalkDirs(Args&&... args)
    : n_impl(new Impl(VIOLET_FWD(Args, args)...))
{
}

WalkDirs::~WalkDirs()
{
    if (this->n_impl != nullptr) {
        delete this->n_impl;
        this->n_impl = nullptr;
    }
}

auto WalkDirs::Next() noexcept -> Optional<WalkDirs::Item>
{
    return this->n_impl->Next();
}

auto violet::filesystem::Metadata::FromPosix(struct stat st) noexcept -> Metadata
{

#ifdef VIOLET_APPLE_MACOS
#define ST_MTIM st.st_mtimespec
#define ST_ATIM st.st_atimespec
#else
#define ST_MTIM st.st_mtim
#define ST_ATIM st.st_atim
#endif

    Metadata mt;
    mt.Size = st.st_size;
    mt.ModifiedAt = getMillisecondsFromTimespec(ST_MTIM);
    mt.AccessedAt = Some<UInt64>(getMillisecondsFromTimespec(ST_ATIM));
    mt.Permissions = static_cast<struct Permissions>(st.st_mode);

#ifdef VIOLET_APPLE_MACOS
    mt.CreatedAt = Some<UInt64>(getMillisecondsFromTimespec(st.st_birthtimespec));
#endif

    if (S_ISREG(st.st_mode)) {
        mt.Type = FileType::mkfile();
    } else if (S_ISDIR(st.st_mode)) {
        mt.Type = FileType::mkdir();
    } else if (S_ISLNK(st.st_mode)) {
        mt.Type = FileType::mksymlink();
    } else if (S_ISCHR(st.st_mode)) {
        mt.Type = FileType::mkchardev();
    } else if (S_ISBLK(st.st_mode)) {
        mt.Type = FileType::mkblkdev();
    } else if (S_ISFIFO(st.st_mode)) {
        mt.Type = FileType::mkfifo();
    } else if (S_ISSOCK(st.st_mode)) {
        mt.Type = FileType::mksocket();
    }

    return mt;
}

auto violet::filesystem::Metadata(PathRef path, bool followSymlinks) -> io::Result<struct Metadata>
{
    struct stat st{};
    if (followSymlinks) {
        if (::stat(static_cast<CStr>(path), &st) < 0) {
            return Err(io::Error::OSError());
        }

        return Ok<struct Metadata, io::Error>(violet::filesystem::Metadata::FromPosix(st));
    }

    if (::lstat(static_cast<CStr>(path), &st) < 0) {
        return Err(io::Error::OSError());
    }

    return Ok<struct Metadata, io::Error>(violet::filesystem::Metadata::FromPosix(st));
}

auto violet::filesystem::CreateDirectory(PathRef path) -> io::Result<void>
{
    if (path.Empty()) {
        return VIOLET_IO_ERROR(InvalidInput, String, "directory path must not be empty");
    }

    if (::mkdir(static_cast<CStr>(path), 0755) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto violet::filesystem::CreateDirectories(PathRef path) -> io::Result<void>
{
    if (path.Empty()) {
        return VIOLET_IO_ERROR(InvalidInput, String, "directory path must not be empty");
    }

    Str view = static_cast<Str>(path);
    String current;
    current.reserve(view.size());

    if (view.front() == '/') {
        current = "/";
    }

    for (UInt i = 0; i < view.size(); i++) {
        char ch = view[i];
        current.push_back(ch);

        if (ch == '/' || i + 1 == view.size()) {
            if (current == "/")
                continue; // skips root

            auto result = CreateDirectory(path);
            if (result.Err()) {
                const auto& err = result.Error();
                if (err.RawOSError().MapOr(false, [](Int err) -> bool { return err == EEXIST || err == EISDIR; })) {
                    continue;
                }

                return Err(err);
            }
        }
    }

    return {};
}

auto violet::filesystem::RemoveDirectory(PathRef path) -> io::Result<void>
{
    if (::rmdir(static_cast<CStr>(path)) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto violet::filesystem::RemoveAllDirs(PathRef path) -> io::Result<void>
{
    // TODO(@auguwu): once `Noelware.Violet.Iterators` library is avaliable,
    // switch to `WalkDir()` iterator

    DIR* dir = ::opendir(static_cast<CStr>(path));
    if (dir == nullptr) {
        // If we can't open the directory, then fail
        return Err(io::Error::OSError());
    }

    struct dirent* ent{};
    while ((ent = ::readdir(dir)) != nullptr) {
        CStr name = ent->d_name;
        if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
            continue;
        }

        Path child = path.Join(name);
        auto metadata = filesystem::Metadata(static_cast<PathRef>(child), false);
        if (metadata.Err()) {
            ::closedir(dir);
            return Err(metadata.Error());
        }

        auto removed = filesystem::RemoveDirectory(static_cast<PathRef>(child));
        if (removed.Err()) {
            ::closedir(dir);
            return Err(removed.Error());
        }
    }

    ::closedir(dir);
    return RemoveDirectory(path);
}

auto violet::filesystem::CreateFile(PathRef path) -> io::Result<File>
{
    return OpenOptions().Write().Create().Truncate().Open(path);
}

auto violet::filesystem::Canonicalize(PathRef path) -> io::Result<Path>
{
    Array<char, PATH_MAX> buf;
    if (::realpath(static_cast<CStr>(path), buf.data()) == nullptr) {
        return Err(io::Error::OSError());
    }

    return Path(buf.data());
}

auto violet::filesystem::Exists(PathRef path) -> bool
{
    return ::access(static_cast<CStr>(path), F_OK) != -1;
}

auto violet::filesystem::TryExists(PathRef path) -> io::Result<bool>
{
    auto file = File::Open(path, OpenOptions().Flags(O_PATH | O_NOFOLLOW));
    if (file.Err()) {
        auto err = file.UnwrapErr();
        if (err.RawOSError().HasValueAnd(
                [](const io::PlatformError::error_type& err) -> bool { return err == ENOENT; })) {
            return false;
        }

        return Err(err);
    }

    struct stat st{};
    if (::fstat(file->Descriptor(), &st) < 0) {
        Int32 saved = errno;
        auto _ = file->Close(); // NOLINT(readability-identifier-length)

        errno = saved;
        return Err(io::Error::OSError());
    }

    auto _ = VIOLET_MOVE(file.Value()).Close(); // NOLINT(readability-identifier-length)
    return true;
}

auto violet::filesystem::RemoveFile(PathRef path) -> io::Result<void>
{
    if (::unlink(static_cast<CStr>(path)) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto violet::filesystem::SetPermissions(PathRef path, Permissions perms) -> io::Result<void>
{
    if (::chmod(static_cast<CStr>(path), static_cast<mode_t>(perms.Mode())) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto violet::filesystem::Rename(PathRef old, PathRef newPath) -> io::Result<void>
{
    if (::rename(static_cast<CStr>(old), static_cast<CStr>(newPath)) == -1) {
        return Err(io::Error::OSError());
    }

    return {};
}

#endif
