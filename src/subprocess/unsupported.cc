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

#include <violet/Subprocess.h>

using violet::Array;
using violet::UInt8;
using violet::Vec;
using violet::subprocess::Child;
using violet::subprocess::Command;
using violet::subprocess::Stdio;

namespace violet::subprocess {
struct Command::Impl final { };
} // namespace violet::subprocess

Command::Command(Str)
    : n_impl(new Impl())
{
}

Command::Command(Str, std::initializer_list<String>)
    : n_impl(new Impl())
{
}

Command::Command(Str, Vec<String>) // NOLINT(performance-unnecessary-value-param)
    : n_impl(new Impl())
{
}

Command::~Command()
{
    if (this->n_impl != nullptr) {
        delete this->n_impl;
        this->n_impl = nullptr;
    }
}

#define __return_this__(Method, ...)                                                                                   \
    auto Command::Method(__VA_ARGS__) -> Command&                                                                      \
    {                                                                                                                  \
        return *this;                                                                                                  \
    }

__return_this__(WithArg, Str);
__return_this__(WithArgs, std::initializer_list<String>);
__return_this__(WithArgs, Span<String>);
__return_this__(WithEnv, Str, Str);
__return_this__(WithEnvs, std::initializer_list<Pair<String, String>>);
__return_this__(WithEnvs, Span<Pair<String, String>>);
__return_this__(WithStdin, const Stdio&);
__return_this__(WithStdout, const Stdio&);
__return_this__(WithStderr, const Stdio&);
__return_this__(WithDeathTimeout, std::chrono::milliseconds);
__return_this__(WithWorkingDirectory, filesystem::PathRef);

#undef __return_this__

auto Child::Kill(Int32) const -> io::Result<void>
{
    return Err(
        VIOLET_IO_ERROR(Unsupported, String, "unsupported on platform: `violet::subprocess::Child::Kill(Int32)`"));
}

auto Command::Spawn() const -> io::Result<Child>
{
    return Err(VIOLET_IO_ERROR(Unsupported, String, "unsupported on platform: `violet::subprocess::Command::Spawn()`"));
}

auto Command::Output() const -> io::Result<struct Output>
{
    return Err(
        VIOLET_IO_ERROR(Unsupported, String, "unsupported on platform: `violet::subprocess::Command::Output()`"));
}

auto Command::Status() const -> io::Result<ExitStatus>
{
    return Err(
        VIOLET_IO_ERROR(Unsupported, String, "unsupported on platform: `violet::subprocess::Command::Status()`"));
}
