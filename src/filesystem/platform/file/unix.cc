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

#include <violet/Filesystem/Extensions/XAttr.h>
#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Path.h>
#include <violet/IO/Descriptor.h>
#include <violet/IO/Error.h>

#include <fcntl.h>
#include <unistd.h>

using violet::filesystem::File;

File::ScopeLock::ScopeLock(File::ScopeLock&& other) noexcept
    : n_file(std::exchange(other.n_file, nullptr))
{
}

auto File::ScopeLock::operator=(File::ScopeLock&& other) noexcept -> File::ScopeLock&
{
    if (this != &other) {
        this->n_file = std::exchange(other.n_file, nullptr);
    }

    return *this;
}

File::ScopeLock::~ScopeLock()
{
    std::ignore = this->n_file->Unlock();
    this->n_file = nullptr;
}

File::~File()
{
    std::ignore = this->Close();
}

auto File::Open(PathRef path, OpenOptions opts) -> io::Result<File>
{
    Int32 flags = 0;

    if (opts.n_bits.Contains(OpenOptions::flag::kRead) && opts.n_bits.Contains(OpenOptions::flag::kWrite)) {
        flags |= O_RDWR;
    } else if (opts.n_bits.Contains(OpenOptions::flag::kRead)) {
        flags |= O_RDONLY;
    } else if (opts.n_bits.Contains(OpenOptions::flag::kWrite)) {
        flags |= O_WRONLY;
    }

    if (opts.n_bits.Contains(OpenOptions::flag::kAppend)) {
        flags |= O_APPEND;
    }

    if (opts.n_bits.Contains(OpenOptions::flag::kTruncate)) {
        flags |= O_TRUNC;
    }

    if (opts.n_bits.Contains(OpenOptions::flag::kCreate)) {
        flags |= O_CREAT;
    }

    if (opts.n_bits.Contains(OpenOptions::flag::kCreateNew)) {
        flags |= O_EXCL;
    }

    mode_t mode = 0666;
    if (opts.n_mode != 0) {
        mode = static_cast<mode_t>(opts.n_mode);
    }

    Int32 fd = ::open(static_cast<CStr>(path), flags, mode);
    if (fd < 0) {
        return Err(io::Error::OSError());
    }

    return File(fd);
}

auto File::Descriptor() const noexcept -> io::FileDescriptor::value_type
{
    return this->n_fd.Get();
}

auto File::ToString() const noexcept -> String
{
    return std::format("File({})", this->n_fd.ToString());
}

auto File::Valid() const noexcept -> bool
{
    return this->n_fd.Valid();
}

auto File::Close() noexcept -> io::Result<void>
{
    if (this->Valid()) {
        if (::close(this->n_fd.Get()) == -1) {
            return Err(io::Error::OSError());
        }

        this->n_fd = -1;
    }

    return {};
}

auto File::Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>
{
    return this->n_fd.Read(buf);
}

auto File::Write(Span<const UInt8> buf) const noexcept -> io::Result<UInt>
{
    return this->n_fd.Write(buf);
}

auto File::Flush() const noexcept -> io::Result<void>
{
    return this->n_fd.Flush();
}

auto File::MkScopedLock() const noexcept -> io::Result<ScopeLock>
{
    auto result = this->Lock();
    if (result) {
        return Ok<ScopeLock, io::Error>(
            // Safety: `ScopeLock` doesn't affect the data inside of `this`, so it is ok
            // in this context and `File::Unlock` doesn't modify data either.
            ScopeLock(const_cast<File*>(this))); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }

    return Err(result.Error());
}

auto File::MkSharedScopedLock() const noexcept -> io::Result<ScopeLock>
{
    auto result = this->SharedLock();
    if (result) {
        return Ok<ScopeLock, io::Error>(
            // Safety: `ScopeLock` doesn't affect the data inside of `this`, so it is ok
            // in this context and `File::Unlock` doesn't modify data either.
            ScopeLock(const_cast<File*>(this))); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }

    return Err(result.Error());
}

auto File::Clone(bool shareFlags) const noexcept -> io::Result<File>
{
    if (!this->Valid()) {
#if VIOLET_USE_RTTI
        return Err(io::Error::New<String>(
            io::ErrorKind::Unsupported, "cannot `dup` this file as it points to an invalid descriptor"));
#else
        return Err(io::Error(io::ErrorKind::Unsupported));
#endif
    }

    Int32 newFD = ::dup(this->n_fd.Get());
    if (newFD == -1) {
        return Err(io::Error::OSError());
    }

    if (shareFlags) {
        Int32 flags = fcntl(this->n_fd.Get(), F_GETFL);
        ::fcntl(this->n_fd.Get(), F_SETFL, flags);
    }

    return File(newFD);
}

auto File::GetAttribute(Str key) const noexcept -> io::Result<Optional<Vec<UInt8>>>
{
    return xattr::Get(this->n_fd.Get(), key);
}

auto File::SetAttribute(Str key, Span<const UInt8> value) const noexcept -> io::Result<void>
{
    return xattr::Set(this->n_fd.Get(), key, value);
}

auto File::RemoveAttribute(Str key) const noexcept -> io::Result<void>
{
    return xattr::Remove(this->n_fd.Get(), key);
}

File::operator bool() const noexcept
{
    return this->Valid();
}

File::operator Int32() const noexcept
{
    return this->n_fd.Get();
}

#endif
