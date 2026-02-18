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

#ifdef VIOLET_LINUX

#include <violet/Filesystem.h>
#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Path.h>

#include <unistd.h>

using violet::UInt64;
using violet::filesystem::OpenOptions;
using violet::filesystem::PathRef;

auto violet::filesystem::Copy(PathRef src, PathRef dest) -> io::Result<UInt64>
{
    auto in = File::Open(src, OpenOptions().Read());
    if (in.Err()) {
        return Err(io::Error::OSError());
    }

    auto out = File::Open(dest, OpenOptions().Write().Create().Truncate().Mode(0644));
    if (out.Err()) {
        return Err(io::Error::OSError());
    }

    ssize_t bytes = 0;
    ssize_t total = 0;
    while (true) {
        bytes = copy_file_range(
            /*infd=*/in->Descriptor(),
            /*pinoff=*/nullptr,
            /*outfd=*/out->Descriptor(),
            /*poutoff=*/nullptr,
            /*length=*/1 << 20, // TODO(@auguwu): is 1MiB/chunk ok or should this be customizable?
            /*flags=*/0);

        if (bytes == 0) {
            break;
        }

        if (bytes < 0) {
            if (errno == EINTR) {
                continue;
            }

            return Err(io::Error::OSError());
        }

        total += bytes;
    }

    return total;
}

#endif
