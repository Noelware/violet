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

#if VIOLET_PLATFORM(APPLE_MACOS)

#include <violet/Filesystem.h>

#include <sys/clonefile.h>

auto violet::filesystem::Copy(PathRef src, PathRef dst) -> io::Result<UInt64>
{
    auto in = VIOLET_TRY(File::Open(src, OpenOptions{ }.Read()));
    auto mt = VIOLET_TRY(in.Metadata());

    // Use `clonefile(2)` (ref: https://www.manpagez.com/man/2/clonefile/) is the fast-path
    // on macOS, analogous to Linux's `copy_file_range(2)`. It creates a Copy-on-Write clone
    // on APFS at near-zero cost. It is not always applicable though; non-APFS filesystems
    // will report `ENOTSUP`, cross-device copies will report `EXDEV`, and an existing
    // destination will report `EEXIST`. In those cases, we fallthrough to a userspace
    // read/write loop instead.
    const bool cloned = src.WithCStr(
        [&](CStr src) -> bool { return dst.WithCStr([&](CStr dst) -> bool { return ::clonefile(src, dst, 0) == 0; }); });

    if (cloned) {
        return mt.Size;
    }

    if (errno != ENOTSUP && errno != EXDEV && errno != EEXIST) {
        return Err(io::Error::OSError());
    }

    auto out = VIOLET_TRY(File::Open(dst, OpenOptions{ }.Create().Write().Truncate().Mode(mt.Permissions.Mode())));
    UInt64 total = 0;

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
