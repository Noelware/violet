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

#include "violet/filesystem/File.h"

auto Noelware::Violet::Filesystem::File::doOpen() const noexcept -> IO::Result<void>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

void Noelware::Violet::Filesystem::File::Close()
{
    /* nothing here */
}

auto Noelware::Violet::Filesystem::File::Lock() const noexcept -> IO::Result<void>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::File::SharedLock() const noexcept -> IO::Result<void>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::File::Unlock() const noexcept -> IO::Result<void>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::File::ScopedLock() -> IO::Result<struct ScopedLock>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::File::ScopedSharedLock() -> IO::Result<struct ScopedLock>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}

auto Noelware::Violet::Filesystem::File::Clone() const noexcept -> IO::Result<File>
{
    return Err(IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation"));
}
