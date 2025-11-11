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

#ifdef VIOLET_LINUX

#include <violet/Filesystem/Path.h>
#include <violet/IO/Descriptor.h>
#include <violet/IO/Error.h>

using violet::Err;
using violet::Ok;
using violet::String;
using violet::filesystem::Path;
using violet::io::Error;
using violet::io::FileDescriptor;
using violet::io::Result;

namespace {
auto impl(FileDescriptor descriptor) -> Result<Path>
{
    char buf[PATH_MAX];
    ssize_t len = readlink(std::format("/proc/self/fd/{}", descriptor.Get()).c_str(), buf, sizeof(buf) - 1);
    if (len == -1) {
        return Err(Error::OSError());
    }

    buf[len] = '\0';
    return Ok<Path, Error>(String(buf));
}
} // namespace

auto violet::filesystem::PathRef::FromFileDescriptor(FileDescriptor descriptor) -> Result<Path, Error>
{
    return impl(VIOLET_MOVE(descriptor));
}

auto violet::filesystem::Path::FromFileDescriptor(FileDescriptor descriptor) -> Result<Path, Error>
{
    return impl(VIOLET_MOVE(descriptor));
}

#endif
