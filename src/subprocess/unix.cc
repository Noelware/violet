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

#if VIOLET_PLATFORM(UNIX)

#include <violet/Subprocess.h>
#include <violet/Subprocess/__detail/Impl.unix.h>

using violet::Array;
using violet::UInt8;
using violet::Vec;
using violet::subprocess::Child;
using violet::subprocess::Command;
using violet::subprocess::Stdio;

Command::Impl::Impl(Str program)
    : Impl(program, { })
{
}

Command::Impl::Impl(Str program, Vec<String> args)
    : n_program(program)
    , n_args(VIOLET_MOVE(args))
{
}

Command::Impl::Impl(Str program, std::initializer_list<String> args)
    : n_program(program)
    , n_args(args)
{
}

Command::Command(Str program)
    : n_impl(new Impl(program))
{
}

Command::Command(Str program, std::initializer_list<String> args)
    : n_impl(new Impl(program, args))
{
}

Command::Command(Str program, Vec<String> args)
    : n_impl(new Impl(program, VIOLET_MOVE(args)))
{
}

Command::~Command()
{
    if (this->n_impl != nullptr) {
        delete this->n_impl;
        this->n_impl = nullptr;
    }
}

auto Command::WithArg(Str arg) -> Command&
{
    this->n_impl->n_args.emplace_back(arg);
    return *this;
}

auto Command::WithArgs(std::initializer_list<String> args) -> Command&
{
    this->n_impl->n_args.insert(this->n_impl->n_args.end(), args.begin(), args.end());
    return *this;
}

auto Command::WithArgs(Span<String> args) -> Command&
{
    this->n_impl->n_args.insert(this->n_impl->n_args.end(), args.begin(), args.end());
    return *this;
}

auto Command::WithEnv(Str key, Str value) -> Command&
{
    this->n_impl->n_environ.insert(std::make_pair(key, value));
    return *this;
}

auto Command::WithEnvs(std::initializer_list<Pair<String, String>> envs) -> Command&
{
    this->n_impl->n_environ.insert(envs.begin(), envs.end());
    return *this;
}

auto Command::WithEnvs(Span<Pair<String, String>> envs) -> Command&
{
    this->n_impl->n_environ.insert(envs.begin(), envs.end());
    return *this;
}

auto Command::WithStdin(const Stdio& cfg) -> Command&
{
    this->n_impl->n_stdin = cfg;
    return *this;
}

auto Command::WithStdout(const Stdio& cfg) -> Command&
{
    this->n_impl->n_stdout = cfg;
    return *this;
}

auto Command::WithStderr(const Stdio& cfg) -> Command&
{
    this->n_impl->n_stderr = cfg;
    return *this;
}

auto Command::WithDeathTimeout(std::chrono::milliseconds ms) -> Command&
{
    this->n_impl->n_deathTimeout = ms;
    return *this;
}

auto Command::WithWorkingDirectory(filesystem::PathRef path) -> Command&
{
    this->n_impl->n_wd = filesystem::Path(path.ToString());
    return *this;
}

auto Child::Kill(Int32 signal) const -> io::Result<void>
{
    if (!this->PID) {
        return Err(VIOLET_IO_ERROR(InvalidData, String, "child is not running"));
    }

    if (::kill(this->PID.Get(), signal) < 0) {
        return Err(io::Error::OSError());
    }

    return { };
}

#endif
