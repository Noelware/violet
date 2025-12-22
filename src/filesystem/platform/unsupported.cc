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

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX) || defined(VIOELT_LINUX) || defined(VIOLET_APPLE_MACOS)
#else
#warning "This platform doesn't support the Noelware.Violet.Filesystem library"

#include <violet/Filesystem.h>
#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Path.h>
#include <violet/IO/Error.h>

using violet::CStr;
using violet::UInt64;
using violet::filesystem::Dirs;
using violet::filesystem::PathRef;
using violet::filesystem::WalkDirs;

struct Dirs::Impl final {
    Impl() = delete;
};

auto Dirs::Next() noexcept -> Optional<Dirs::Item>
{
    abort();
}

#define MK_UNSUPPORTED_OP(NAME, RETURN, ...)                                                                           \
    auto violet::filesystem::NAME(__VA_ARGS__) -> RETURN                                                               \
    {                                                                                                                  \
        return Err(VIOLET_IO_ERROR(Unsupported, String, "unsupported operation"));                                     \
    }

MK_UNSUPPORTED_OP(ReadDir, io::Result<Dirs>, PathRef);

struct WalkDirs::Impl final {
    Impl() = delete;
};

auto WalkDirs::Next() noexcept -> Optional<WalkDirs::Item>
{
    abort();
}

auto violet::filesystem::Exists(PathRef) -> bool
{
    return false;
}

MK_UNSUPPORTED_OP(WalkDir, io::Result<WalkDirs>, PathRef);
MK_UNSUPPORTED_OP(Metadata, io::Result<struct Metadata>, PathRef, bool);
MK_UNSUPPORTED_OP(CreateDirectory, io::Result<void>, PathRef);
MK_UNSUPPORTED_OP(CreateDirectories, io::Result<void>, PathRef);
MK_UNSUPPORTED_OP(RemoveDirectory, io::Result<void>, PathRef);
MK_UNSUPPORTED_OP(RemoveAllDirs, io::Result<void>, PathRef);
MK_UNSUPPORTED_OP(CreateFile, io::Result<File>, PathRef);
MK_UNSUPPORTED_OP(Canonicalize, io::Result<Path>, PathRef);
MK_UNSUPPORTED_OP(TryExists, io::Result<bool>, PathRef);
MK_UNSUPPORTED_OP(RemoveFile, io::Result<void>, PathRef);
MK_UNSUPPORTED_OP(SetPermissions, io::Result<void>, PathRef, Permissions);
MK_UNSUPPORTED_OP(Rename, io::Result<void>, PathRef, PathRef);

#undef MK_UNSUPPORTED_OP

#endif
