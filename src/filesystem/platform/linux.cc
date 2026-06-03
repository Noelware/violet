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

#if VIOLET_PLATFORM(LINUX)

#include <violet/Filesystem.h>
#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Metadata.h>
#include <violet/Filesystem/Path.h>

#include <cerrno>
#include <unistd.h>

using violet::Span;
using violet::UInt;
using violet::UInt64;
using violet::UInt8;
using violet::filesystem::OpenOptions;
using violet::filesystem::PathRef;

auto violet::filesystem::Copy(PathRef src, PathRef dst) -> io::Result<UInt64>
{
    auto in = VIOLET_TRY(File::Open(src, OpenOptions{ }.Read()));

    // Mirror the source's permission bits onto the destination, like `cp` does
    auto mt = VIOLET_TRY(in.Metadata());
    auto out = VIOLET_TRY(File::Open(dst, OpenOptions{ }.Create().Write().Truncate().Mode(mt.Permissions.Mode())));

    UInt64 total = 0;

    // Using `copy_file_range(2)` (ref: https://www.man7.org/linux/man-pages/man2/copy_file_range.2.html)
    // is the fast-path on Linux (and Android if we plan on supporting it in the future). It lets the kernel
    // copy in-place (reflinks, server-side copies, etc.). It is not always applicable though, cross-filesystem
    // copies on kernels < 5.3 will fail with `EXDEV`, sandboxes that block the syscall report `ENOSYS`, and some
    // filesystems / pseudo-files reject it with either EINVAL, EOPNOTSUPP, EPERM, or EBADF. If that's the case
    // we fallthrough to the userspace read/write loop and resume from where we are.
    while (true) {
        ssize_t bytes = copy_file_range(
            /*infd=*/in.Descriptor(),
            /*pinoff=*/nullptr,
            /*outfd=*/out.Descriptor(),
            /*poutoff=*/nullptr,
            /*length=*/1 << 20, // TODO(@auguwu): is 1MiB/chunk ok or should this be customizable?
            /*flags=*/0);

        if (bytes == 0) {
            return total;
        }

        if (bytes < 0) {
            if (errno == EINTR) {
                continue;
            }

            // `copy_file_range(2)` is unusable in this context; use userspace read/write loop instead
            if (errno == ENOSYS || errno == EXDEV || errno == EINVAL || errno == EOPNOTSUPP || errno == EPERM
                || errno == EBADF) {
                break;
            }

            return Err(io::Error::OSError());
        }

        total += static_cast<UInt64>(bytes);
    }

    constexpr static auto kBufSize = 1 << 16; // 64KiB
    Array<UInt8, kBufSize> buf{ };
    while (true) {
        UInt read = VIOLET_TRY(in.Read(buf));
        if (read == 0) {
            break;
        }

        UInt written = 0;
        while (written < read) {
            written += VIOLET_TRY(out.Write({ buf.data() + written, read - written }));
        }

        total += read;
    }

    return total;
}

#endif
