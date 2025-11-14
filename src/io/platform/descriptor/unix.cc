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

#include <violet/Violet.h>

#ifdef VIOLET_UNIX

#include <violet/IO/Descriptor.h>

#include <utility>

using violet::io::FileDescriptor;

FileDescriptor::FileDescriptor(Int32 value)
    : n_value(value)
{
}

FileDescriptor::FileDescriptor(FileDescriptor&& other) noexcept
    : n_value(std::exchange(other.n_value, INVALID))
{
}

FileDescriptor::~FileDescriptor()
{
    this->Close();
}

auto FileDescriptor::operator=(FileDescriptor&& other) noexcept -> FileDescriptor&
{
    if (this != &other) {
        if (this->n_value != INVALID) {
            ::close(this->n_value);
        }

        this->n_value = std::exchange(other.n_value, INVALID);
    }

    return *this;
}

auto FileDescriptor::Get() const noexcept -> value_type
{
    return this->n_value;
}

auto FileDescriptor::ToString() const noexcept -> String
{
    return std::format("FileDescriptor({})", this->n_value);
}

auto FileDescriptor::Valid() const noexcept -> bool
{
    return this->n_value != INVALID;
}

void FileDescriptor::Close()
{
    if (this->n_value != INVALID) {
        ::close(this->n_value);
        this->n_value = INVALID;
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

#endif
