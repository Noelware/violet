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
#include <violet/Filesystem/Experimental/Dir.h>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

using violet::filesystem::PathRef;
using violet::filesystem::experimental::Dir;

using violet::io::Error;

auto Dir::Open(PathRef path) -> io::Result<Dir>
{
    const Int32 fd = ::open(path.Data().data(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        return Err(Error::OSError());
    }

    return Dir(fd);
}

auto Dir::OpenFile(PathRef path, OpenOptions opts) const -> io::Result<File>
{
    if (!this->Alive()) {
        return Err(VIOLET_IO_ERROR(InvalidData, String, "file descriptor for this directory is not alive"));
    }

    const Int32 fd
        = path.WithCStr([&](CStr path) -> auto { return ::openat(this->n_fd.Get(), path, opts.n_bits.Get()); });

    if (fd < 0) {
        return Err(Error::OSError());
    }

    return File(fd);
}

auto Dir::Descriptor() const -> io::FileDescriptor::value_type
{
    return this->n_fd.Get();
}

auto Dir::Metadata() const -> io::Result<struct Metadata>
{
    return Metadata::For(this->n_fd.Get());
}

auto Dir::Iter(Path display) const -> io::Result<Dirs>
{
    io::FileDescriptor copy = ::fcntl(this->n_fd.Get(), F_DUPFD_CLOEXEC, 0);
    if (!copy.Valid()) {
        return Err(io::Error::OSError());
    }

    DIR* dir = ::fdopendir(copy.Get());
    if (dir == nullptr) {
        auto saved = errno;
        copy.Close();

        return Err(io::Error::FromOSError(saved));
    }

    return Dirs(display, VIOLET_MOVE(copy), dir);
}

auto Dir::Walk(Path display) const -> io::Result<WalkDirs>
{
    const Int32 fd = ::openat(this->Descriptor(), ".", O_DIRECTORY | O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return Err(io::Error::OSError());
    }

    DIR* stream = ::fdopendir(fd);
    if (stream == nullptr) {
        auto saved = errno;
        ::close(fd);

        return Err(io::Error::FromOSError(saved));
    }

    return WalkDirs::fromRootStream(stream, VIOLET_MOVE(display));
}

auto Dir::Alive() const -> bool
{
    return this->n_fd.Valid();
}

void Dir::Close()
{
    this->n_fd.Close();
}

auto Dir::Release() && -> io::FileDescriptor
{
    return VIOLET_MOVE(this->n_fd);
}

Dir::operator bool() const noexcept
{
    return this->Alive();
}

Dir::operator io::FileDescriptor::value_type() const noexcept
{
    return this->n_fd.Get();
}

#endif
