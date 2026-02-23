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

#ifdef VIOLET_UNIX

#include <violet/System.h>

#include <unistd.h>

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
    ::setenv(key.data(), value.data(), static_cast<int>(replace));
}

void violet::sys::RemoveEnv(Unsafe, Str key)
{
    ::unsetenv(key.data());
}

auto violet::sys::WorkingDirectory() noexcept -> io::Result<filesystem::Path>
{
    // When executed from `bazel run` or `bazel test`, we are placed in the runfiles
    // directory, so we use the `BUILD_WORKING_DIRECTORY` environment variable to get
    // the actual working directory.
#ifdef BAZEL
    if (auto wd = GetEnv("BUILD_WORKING_DIRECTORY")) {
        return filesystem::Path(wd.Value());
    }
#endif

    Array<char, PATH_MAX> buf;
    if (::getcwd(buf.data(), buf.size()) == nullptr) {
        return Err(io::Error::OSError());
    }

    return filesystem::Path(buf.data());
}

auto violet::sys::SetWorkingDir(filesystem::PathRef path) -> io::Result<void>
{
    if (::chdir(static_cast<CStr>(path)) != 0) {
        return Err(io::Error::OSError());
    }

    return {};
}

#endif
