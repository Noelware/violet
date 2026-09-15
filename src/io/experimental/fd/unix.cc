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

#include <violet/Language/Macros.h>

#if VIOLET_PLATFORM(UNIX)

#include <violet/IO/Experimental/Descriptor.h>

#include <unistd.h>

namespace violet::io::experimental::fd_internal {

auto isFdValid(RawFd fd) noexcept -> bool
{
    return fd != kInvalidRawFd;
}

// TODO(@auguwu/Noel): should `read`/`write` be split by using io_uring (fallback to epoll if not available)?

auto read(RawFd fd, Span<UInt8> buf) noexcept -> Result<UInt>
{
    if (!isFdValid(fd) || buf.empty()) {
        return 0;
    }

    Int64 bytes = 0;
    do {
        bytes = ::read(fd, buf.data(), buf.size());
    } while (bytes == -1 && errno == EINTR);

    if (bytes == -1) {
        return Err(Error::OSError());
    }

    return bytes;
}

auto write(RawFd fd, Span<const UInt8> buf) noexcept -> Result<UInt>
{
    if (!isFdValid(fd)) {
        return Err(VIOLET_IO_ERROR(InvalidInput, "cannot operate on a invalid descriptor"));
    }

    UInt total = 0;
    const UInt8* data = buf.data();
    UInt remaining = buf.size();

    while (remaining > 0) {
        Int64 bytes = ::write(fd, data, remaining);
        if (bytes == -1) {
            if (errno == EINTR) {
                continue;
            }

            return Err(Error::OSError());
        }

        total += bytes;
        data += bytes;
        remaining -= bytes;
    }

    return total;
}

auto flush(RawFd fd) noexcept -> Result<void>
{
    if (isFdValid(fd) && ::fsync(fd) == -1) {
        // We intentionally ignore `EINVAL` since this can mean that
        // the file descriptor is not valid (i.e, `STDOUT_FILENO`)
        if (errno == EINVAL) {
            return {};
        }

        return Err(Error::OSError());
    }

    return {};
}

auto close(RawFd fd) noexcept -> Result<void>
{
    if (::close(fd) == -1) {
        return Err(Error::OSError());
    }

    return {};
}

auto toString(RawFd fd) noexcept -> String
{
    if (!isFdValid(fd)) {
        return "fd(invalid)";
    }

    return std::format("fd({})", fd);
}

} // namespace violet::io::experimental::fd_internal

#endif
