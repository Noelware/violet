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

#if VIOLET_PLATFORM(UNIX)

#include <violet/Filesystem.h>
#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Path.h>
#include <violet/IO/Error.h>

#include <climits> // IWYU pragma: keep
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using violet::CStr;
using violet::UInt64;
using violet::filesystem::Dirs;
using violet::filesystem::PathRef;
using violet::filesystem::WalkDirs;

struct Dirs::Impl final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Impl);
    VIOLET_IMPLICIT Impl(PathRef root, DIR* entry)
        : n_root(root)
        , n_entries(entry)
    {
    }

    VIOLET_IMPLICIT Impl(PathRef root, io::FileDescriptor&& fd, DIR* entry)
        : n_root(root)
        , n_entries(entry)
        , n_fd(VIOLET_MOVE(fd))
    {
    }

    VIOLET_IMPLICIT Impl(Path root, DIR* entry)
        : n_root(VIOLET_MOVE(root))
        , n_entries(entry)
    {
    }

    VIOLET_IMPLICIT Impl(Path root, io::FileDescriptor&& fd, DIR* entry)
        : n_root(VIOLET_MOVE(root))
        , n_entries(entry)
        , n_fd(VIOLET_MOVE(fd))
    {
    }

    ~Impl()
    {
        this->close();
    }

    auto Next() -> Optional<io::Result<DirEntry>>
    {
        if (this->n_entries == nullptr) {
            return Nothing;
        }

        struct dirent* ent = nullptr;
        while (true) {
            errno = 0;
            ent = ::readdir(this->n_entries);
            if (ent == nullptr) {
                if (errno != 0) {
                    return Err(io::Error::OSError());
                }

                return Nothing;
            }

            const Str name(ent->d_name);
            if (name == "." || name == "..") {
                continue;
            }

            break;
        }

        const Int32 dirfd = ::dirfd(this->n_entries);
        auto metadata = Metadata::For(dirfd, ent->d_name, SymlinkResolution::NoFollow);
        if (metadata.Err()) {
            return Err(VIOLET_MOVE(metadata).Error());
        }

        Path path(this->n_root.Join(ent->d_name));
        return Ok<DirEntry>(VIOLET_MOVE(path), VIOLET_MOVE(metadata.Value()));
    }

    void stopTraversing()
    {
        this->close();
    }

private:
    Path n_root;
    DIR* n_entries;
    Optional<io::FileDescriptor> n_fd;

    void close()
    {
        if (this->n_entries != nullptr) {
            ::closedir(this->n_entries);
            this->n_entries = nullptr;
        }
    }
};

template<typename... Args>
Dirs::Dirs(Args&&... args)
    : n_impl(new Impl(VIOLET_FWD(Args, args)...))
{
}

// `Dirs::Impl` is only complete in this TU (and other TUs that implement `Dirs`), so the forwarding
// constructor above can only be instantiated here. Methods that are outside this TU (like `Dir::Iter`),
// and try to construct a `Dirs(Args&&...)` will fail with a linker error. So, that's why it is explicitlly
// instantiated here.
template Dirs::Dirs(Path&, io::FileDescriptor&&, DIR*&);

Dirs::Dirs(Dirs&& other) noexcept
    : n_impl(std::exchange(other.n_impl, nullptr))
{
}

Dirs::~Dirs()
{
    if (this->n_impl != nullptr) {
        delete this->n_impl;
        this->n_impl = nullptr;
    }
}

auto Dirs::Next() noexcept -> Optional<Dirs::Item>
{
    VIOLET_ASSERT0(this->n_impl != nullptr);
    return this->n_impl->Next();
}

void Dirs::StopTraversing()
{
    if (this->n_impl != nullptr) {
        this->n_impl->stopTraversing();
    }
}

auto violet::filesystem::ReadDir(PathRef path) -> io::Result<Dirs>
{
    DIR* entries = nullptr;
    if (path.WithCStr([&](CStr path) -> bool {
            entries = ::opendir(path);
            return entries == nullptr;
        })) {
        return Err(io::Error::OSError());
    }

    return Dirs(path, entries);
}

namespace {
struct dirdestruct final {
    auto operator()(DIR* dir) -> void
    {
        if (dir != nullptr) {
            ::closedir(dir);
        }
    }
};
} // namespace

struct WalkDirs::Impl final {
    struct Frame final {
        violet::UniquePtr<DIR, dirdestruct> Entry = nullptr;
        struct filesystem::Path Path;
    };

    VIOLET_DISALLOW_COPY_AND_MOVE(Impl);

    VIOLET_IMPLICIT Impl() = default;
    VIOLET_EXPLICIT Impl(Vec<Frame> stack)
        : n_stack(VIOLET_MOVE(stack))
    {
    }

    ~Impl()
    {
        this->close();
    }

    auto Next() -> Optional<WalkDirs::Item>
    {
        // happens when a directory we couldn't descend into (recorded from the previous call) surfaces
        // here as its own error item; emitted *after* we already yielded that directory's entry, so
        // nothing is silently dropped
        if (this->n_pending.HasValue()) {
            auto error = VIOLET_MOVE(this->n_pending).Value();
            this->n_pending = Nothing;

            return Err(error);
        }

        while (!this->n_stack.empty()) {
            DIR* stream = this->n_stack.back().Entry.get();
            const Int32 dirfd = ::dirfd(stream);

            errno = 0;
            dirent* ent = ::readdir(stream);
            if (ent == nullptr) {
                if (errno != 0) {
                    auto saved = errno;
                    this->n_stack.pop_back();

                    return Err(io::Error::FromOSError(saved));
                }

                // reached EOF
                this->n_stack.pop_back();
                continue;
            }

            CStr name = ent->d_name;
            if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
                continue;
            }

            Path path(this->n_stack.back().Path.Join(name));
            auto metadata = Metadata::For(dirfd, name, SymlinkResolution::NoFollow);
            if (metadata.Err()) {
                return Err(VIOLET_MOVE(metadata.Error()));
            }

            if (metadata->Type.Dir()) {
                auto subdir = openSubdir(dirfd, name);
                if (subdir.Err()) {
                    this->n_pending = VIOLET_MOVE(subdir.Error());
                } else {
                    this->n_stack.push_back({ .Entry = VIOLET_MOVE(subdir.Value()), .Path = path });
                }
            }

            return DirEntry(VIOLET_MOVE(path), VIOLET_MOVE(metadata.Value()));
        }

        return Nothing;
    }

    void stopTraversing()
    {
        this->close();
    }

private:
    Vec<Frame> n_stack;
    Optional<io::Error> n_pending;

    void close()
    {
        this->n_stack.clear();
    }

    static auto openSubdir(Int32 dirfd, CStr name) -> io::Result<UniquePtr<DIR, dirdestruct>>
    {
        const Int32 fd = ::openat(dirfd, name, O_DIRECTORY | O_NOFOLLOW | O_RDONLY | O_CLOEXEC);
        if (fd < 0) {
            return Err(io::Error::OSError());
        }

        DIR* subdir = ::fdopendir(fd);
        if (subdir == nullptr) {
            auto saved = errno;
            ::close(fd);

            return Err(io::Error::FromOSError(saved));
        }

        return UniquePtr<DIR, dirdestruct>(subdir);
    }
};

template<typename... Args>
WalkDirs::WalkDirs(Args&&... args)
    : n_impl(new Impl(VIOLET_FWD(Args, args)...))
{
}

WalkDirs::WalkDirs(WalkDirs&& other) noexcept
    : n_impl(std::exchange(other.n_impl, nullptr))
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
    VIOLET_ASSERT0(this->n_impl != nullptr);
    return this->n_impl->Next();
}

void WalkDirs::StopTraversing()
{
    if (this->n_impl != nullptr) {
        this->n_impl->stopTraversing();
    }
}

auto WalkDirs::fromRootStream(DIR* stream, Path base) -> WalkDirs
{
    Vec<Impl::Frame> stack;
    stack.push_back(Impl::Frame(UniquePtr<DIR, dirdestruct>(stream), VIOLET_MOVE(base)));

    return WalkDirs(VIOLET_MOVE(stack));
}

auto violet::filesystem::WalkDir(PathRef path) -> io::Result<WalkDirs>
{
    DIR* ent = nullptr;
    if (path.WithCStr([&](CStr path) -> bool {
            ent = ::opendir(path);
            return ent == nullptr;
        })) {
        return Err(io::Error::OSError());
    }

    return WalkDirs::fromRootStream(ent, path);
}

auto violet::filesystem::Metadata(PathRef path, bool followSymlinks) -> io::Result<struct Metadata>
{
    return Metadata::For(path, followSymlinks ? SymlinkResolution::Follow : SymlinkResolution::NoFollow);
}

auto violet::filesystem::CreateDirectory(PathRef path) -> io::Result<void>
{
    if (path.Empty()) {
        return VIOLET_IO_ERROR(InvalidInput, String, "directory path must not be empty");
    }

    if (path.WithCStr([&](CStr path) -> bool { return ::mkdir(path, 0755) == -1; })) {
        return Err(io::Error::OSError());
    }

    return { };
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
            if (current == "/") {
                continue; // skips root
            }

            auto result = CreateDirectory(Str(current.data(), current.size()));
            if (result.Err()) {
                const auto& err = result.Error();
                if (err.RawOSError().MapOr(false, [](Int err) -> bool { return err == EEXIST || err == EISDIR; })) {
                    continue;
                }

                return Err(err);
            }
        }
    }

    return { };
}

auto violet::filesystem::RemoveDirectory(PathRef path) -> io::Result<void>
{
    if (path.WithCStr([&](CStr path) -> bool { return ::rmdir(path) == -1; })) {
        return Err(io::Error::OSError());
    }

    return { };
}

auto violet::filesystem::RemoveAllDirs(PathRef path) -> io::Result<void>
{
    auto iter = VIOLET_TRY(WalkDir(path));
    Vec<Path> dirs;
    for (auto result: iter) {
        if (result.Err()) {
            iter.StopTraversing();
            return Err(VIOLET_MOVE(result).Error());
        }

        if (result->Metadata.Type.Dir()) {
            dirs.push_back(VIOLET_MOVE(result->Path));
        } else {
            if (auto removed = filesystem::RemoveFile(result->Path); removed.Err()) {
                iter.StopTraversing();
                return Err(VIOLET_MOVE(removed).Error());
            }
        }
    }

    Drop(VIOLET_MOVE(iter));
    while (!dirs.empty()) {
        if (auto removed = filesystem::RemoveDirectory(dirs.back()); removed.Err()) {
            return Err(VIOLET_MOVE(removed).Error());
        }

        dirs.pop_back();
    }

    return RemoveDirectory(path);
}

auto violet::filesystem::CreateFile(PathRef path) -> io::Result<File>
{
    return OpenOptions().Write().Create().Truncate().Open(path);
}

auto violet::filesystem::Canonicalize(PathRef path) -> io::Result<Path>
{
    Array<char, PATH_MAX> buf;
    if (path.WithCStr([&](CStr path) -> bool { return ::realpath(path, buf.data()) == nullptr; })) {
        return Err(io::Error::OSError());
    }

    return Path(buf.data());
}

auto violet::filesystem::Exists(PathRef path) -> bool
{
    return path.WithCStr([&](CStr path) -> bool { return ::access(path, F_OK) != -1; });
}

auto violet::filesystem::TryExists(PathRef path) -> io::Result<bool>
{
#if VIOLET_PLATFORM(APPLE_MACOS)
#define O_PATH O_RDONLY
#endif

    auto file = File::Open(path, OpenOptions().Flags(O_PATH | O_NOFOLLOW));
    if (file.Err()) {
        auto err = file.UnwrapErr();
        if (err.RawOSError().HasValueAnd(
                [](const io::PlatformError::error_type& err) -> bool { return err == ENOENT; })) {
            return false;
        }

        return Err(err);
    }

    struct stat st{ };
    if (::fstat(file->Descriptor(), &st) < 0) {
        return Err(io::Error::OSError());
    }

    return true;
}

auto violet::filesystem::RemoveFile(PathRef path) -> io::Result<void>
{
    if (path.WithCStr([&](CStr path) -> bool { return ::unlink(path) == -1; })) {
        return Err(io::Error::OSError());
    }

    return { };
}

auto violet::filesystem::SetPermissions(PathRef path, Permissions perms) -> io::Result<void>
{
    if (path.WithCStr([&](CStr path) -> bool { return ::chmod(path, static_cast<mode_t>(perms.Mode())); })) {
        return Err(io::Error::OSError());
    }

    return { };
}

auto violet::filesystem::Rename(PathRef old, PathRef newPath) -> io::Result<void>
{
    if (old.WithCStr([&](CStr old) -> bool {
            return newPath.WithCStr([&](CStr path) -> bool { return ::rename(old, path) == -1; });
        })) {
        return Err(io::Error::OSError());
    }

    return { };
}

auto violet::filesystem::Executable(PathRef path) -> io::Result<bool>
{
    auto mt = VIOLET_TRY(Metadata::For(path, SymlinkResolution::Follow));
    if (!mt.Type.File()) {
        return false;
    }

    auto mode = mt.Permissions.Mode();
    return (mode | (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
}

#endif
