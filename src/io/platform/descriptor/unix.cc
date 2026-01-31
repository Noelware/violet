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

#include <unistd.h>
#include <violet/Violet.h>

#ifdef VIOLET_UNIX

#include <violet/IO/Descriptor.h>

using violet::Int32;
using violet::io::FileDescriptor;

struct FileDescriptor::Impl final {
    VIOLET_IMPLICIT_COPY_AND_MOVE(Impl);

    ~Impl()
    {
        this->doClose();
    }

    VIOLET_IMPLICIT Impl() noexcept = default;
    VIOLET_IMPLICIT Impl(Int32 fd) noexcept
        : n_fd(fd)
    {
    }

    void doClose()
    {
        if (this->n_fd != -1) {
            close(this->n_fd);
            this->n_fd = -1;
        }
    }

private:
    friend struct FileDescriptor;

    Int32 n_fd = -1;
};

FileDescriptor::FileDescriptor() noexcept
    : n_impl(std::make_shared<FileDescriptor::Impl>())
{
}

FileDescriptor::FileDescriptor(Int32 fd) noexcept
    : n_impl(std::make_shared<FileDescriptor::Impl>(fd))
{
}

FileDescriptor::~FileDescriptor() = default;

auto FileDescriptor::Valid() const noexcept -> bool
{
    if (this->n_impl == nullptr) {
        return false;
    }

    return this->n_impl->n_fd != -1;
}

auto FileDescriptor::Get() const noexcept -> Int32
{
    if (this->n_impl == nullptr) {
        return -1;
    }

    return this->n_impl->n_fd;
}

auto FileDescriptor::ToString() const noexcept -> String
{
    return std::format("FileDescriptor({})", this->Get());
}

void FileDescriptor::Close()
{
    if (this->n_impl != nullptr) {
        this->n_impl->doClose();
    }
}

auto FileDescriptor::Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>
{
    if (!this->Valid() || buf.empty()) {
        return 0;
    }

    ssize_t bytes = 0;
    do { // NOLINT(cppcoreguidelines-avoid-do-while)
        bytes = ::read(this->Get(), buf.data(), buf.size());
    } while (bytes == -1 && errno == EINTR);

    if (bytes == -1) {
        return Err(Error::OSError());
    }

    return bytes;
}

auto FileDescriptor::Write(Span<const UInt8> buf) const noexcept -> Result<UInt>
{
    if (!this->Valid()) {
        return 0;
    }

    UInt total = 0;
    const UInt8* ptr = buf.data();
    UInt remaining = buf.size();

    while (remaining > 0) {
        ssize_t bytes = ::write(this->Get(), ptr, remaining);
        if (bytes == -1) {
            if (errno == EINTR) {
                continue;
            }

            return Err(Error::OSError());
        }

        total += bytes;
        ptr += bytes;
        remaining -= bytes;
    }

    return total;
}

auto FileDescriptor::Flush() const noexcept -> io::Result<void>
{
    if (this->Valid() && ::fsync(this->Get()) == -1) {
        // For now, it is ok to ignore `EINVAL` since this means
        // that the file descriptor wasn't a valid one (i.e, `STDOUT_FILENO`)
        if (errno == EINVAL) {
            return {};
        }

        return Err(Error::OSError());
    }

    return {};
}

FileDescriptor::operator bool() const noexcept
{
    return this->Valid();
}

FileDescriptor::operator Int32() const noexcept
{
    return this->Get();
}

auto FileDescriptor::operator==(const FileDescriptor& other) const noexcept -> bool
{
    return this->Get() == other.Get();
}

auto FileDescriptor::operator!=(const FileDescriptor& other) const noexcept -> bool
{
    return this->Get() != other.Get();
}

auto FileDescriptor::operator==(Int32 other) const noexcept -> bool
{
    return this->Get() == other;
}

auto FileDescriptor::operator!=(Int32 other) const noexcept -> bool
{
    return this->Get() != other;
}

#endif
