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

#include "violet/io/Error.h"
#include "violet/violet.h"

#ifdef VIOLET_LINUX

// clang-format off
#include "violet/filesystem/Path.h"
// clang-format on

using Noelware::Violet::Filesystem::Path;
using Noelware::Violet::Filesystem::PathRef;

auto Path::FromDescriptor(IO::Descriptor descriptor) -> IO::Result<PathRef>
{
    return PathRef::FromDescriptor(descriptor);
}

auto PathRef::FromDescriptor(IO::Descriptor descriptor) -> IO::Result<PathRef>
{
    assert(descriptor.Valid());

    PathRef fd = std::format("/proc/self/fd/{}", descriptor.AsFD());
    Array<char, PATH_MAX> buf;

    int64 len = ::readlink(static_cast<StringRef>(fd), buf.data(), buf.size());
    if (len == -1) {
        return IO::Error::Platform(IO::ErrorKind::Other);
    }

    buf[len] = '\0';

    return PathRef(StringRef(buf.data(), buf.size()));
}

#endif
