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

#include "violet/filesystem/Filesystem.h"
#include "violet/io/Error.h"

auto Noelware::Violet::Filesystem::Dirs::Next() noexcept -> Optional<IO::Result<DirEntry>>
{
    return Nothing;
}

auto Noelware::Violet::Filesystem::Exists(const Path&) -> IO::Result<bool>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::CreateDirectory(const Path&) -> IO::Result<void>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::CreateDirectories(const Path&) -> IO::Result<void>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::ReadDir(const Path&) -> IO::Result<Dirs>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::WalkDir(const Path&) -> IO::Result<WalkDirs>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::RemoveDirectory(const Path&) -> IO::Result<void>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}
