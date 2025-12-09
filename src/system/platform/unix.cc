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

#ifdef VIOLET_UNIX

#include <violet/System.h>

auto violet::sys::GetEnv(Str key) noexcept -> Optional<String>
{
    const auto* var = ::getenv(key.data());
    if (var != nullptr) {
        return Some<String>(var);
    }

    return Nothing;
}

void violet::sys::SetEnv(Unsafe, Str key, Str value, bool replace)
{
    VIOLET_ASSERT(::setenv(key.data(), value.data(), replace) == 0, "setenv");
}

void violet::sys::RemoveEnv(Unsafe, Str key)
{
    VIOLET_ASSERT(::unsetenv(key.data()) == 0, "unsetenv");
}

auto violet::sys::WorkingDirectory() noexcept -> io::Result<filesystem::Path>
{
    Array<char, PATH_MAX> buf;
    if (::getcwd(buf.data(), buf.size()) == nullptr) {
        return Err(io::Error::OSError());
    }

    return filesystem::Path(buf.data(), buf.size());
}

auto violet::sys::SetWorkingDir(filesystem::PathRef path) -> io::Result<void>
{
    if (::chdir(static_cast<CStr>(path)) != 0) {
        return Err(io::Error::OSError());
    }

    return {};
}

#endif
