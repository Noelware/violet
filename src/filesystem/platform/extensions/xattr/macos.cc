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
#include <violet/IO/Descriptor.h>
#include <violet/IO/Error.h>

#include <cerrno>
#include <sys/xattr.h>

using value_type = violet::io::FileDescriptor::value_type;

auto violet::filesystem::xattr::Get(value_type fd, Str key) noexcept -> io::Result<Optional<Vec<UInt8>>>
{
    ssize_t size = ::fgetxattr(fd, key.data(), nullptr, 0, 0, 0);
    if (size == -1) {
        if (errno == ENODATA) {
            return Ok<Optional<Vec<UInt8>>, io::Error>(Nothing);
        }

        return Err(io::Error::OSError());
    }

    Vec<UInt8> buf(size);
    ssize_t rc = ::fgetxattr(fd, key.data(), buf.data(), buf.size(), 0, 0);
    if (rc < 0) {
        return Err(io::Error::OSError());
    }

    return Some<Vec<UInt8>>(buf);
}

auto violet::filesystem::xattr::Set(value_type fd, Str key, Span<const UInt8> value) noexcept -> io::Result<void>
{
    if (::fsetxattr(fd, key.data(), value.data(), value.size(), 0, 0) < 0) {
        return Err(io::Error::OSError());
    }

    return {};
}

auto violet::filesystem::xattr::Remove(value_type fd, Str key) -> io::Result<void>
{
    if (::fremovexattr(fd, key.data(), 0) < 0) {
        return Err(io::Error::OSError());
    }

    return {};
}

#endif
