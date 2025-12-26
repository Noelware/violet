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

#include <violet/IO/Error.h>
#include <violet/Subprocess.h>
#include <violet/Subprocess/Unix.h>

using violet::String;
using violet::Vec;
using violet::process::Command;
using violet::process::Stdio;

auto Command::Impl::BuildArgv() const noexcept -> Vec<char*>
{
    Vec<char*> argv;
    argv.reserve(this->n_args.size() + 2);
    argv.push_back(const_cast<char*>(this->n_command.data())); // NOLINT(cppcoreguidelines-pro-type-const-cast)

    for (const auto& arg: this->n_args) {
        argv.push_back(const_cast<char*>(arg.c_str())); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }

    return argv;
}

auto Command::Impl::BuildEnviron() const noexcept -> unix::EnvBlock
{
    unix::EnvBlock out;
    out.Storage.reserve(this->n_environ.size());
    out.Environ.reserve(this->n_environ.size() + 1);

    for (const auto& [key, value]: this->n_environ) {
        out.Storage.emplace_back(std::format("{}={}", key, value));
    }

    for (auto& arg: out.Storage) {
        out.Environ.push_back(arg.data());
    }

    out.Environ.push_back(nullptr);
    return out;
}

Command::Command(Str command) noexcept
    : n_impl(new Impl(command))
{
}

Command::Command(Str command, std::initializer_list<String> args) noexcept
    : n_impl(new Impl(command, args))
{
}

Command::Command(Str command, Span<String> args) noexcept
    : n_impl(new Impl(command, args))
{
}

auto Command::WithArg(Str arg) noexcept -> Command&
{
    this->n_impl->n_args.emplace_back(arg);
    return *this;
}

auto Command::WithArgs(std::initializer_list<String> args) noexcept -> Command&
{
    this->n_impl->n_args.insert(this->n_impl->n_args.end(), args);
    return *this;
}

auto Command::WithArgs(Span<String> args) noexcept -> Command&
{
    this->n_impl->n_args.insert(this->n_impl->n_args.end(), args.begin(), args.end());

    return *this;
}

auto Command::WithEnv(Str key, Str value) noexcept -> Command&
{
    this->n_impl->n_environ.emplace(std::make_pair(key, value));
    return *this;
}

auto Command::WithEnvs(std::initializer_list<Pair<String, String>> environ) noexcept -> Command&
{
    this->n_impl->n_environ.insert(environ);
    return *this;
}

auto Command::WithEnvs(Span<Pair<String, String>> environ) noexcept -> Command&
{
    this->n_impl->n_environ.insert(environ.begin(), environ.end());
    return *this;
}

auto Command::WithWorkingDirectory(violet::filesystem::PathRef path) noexcept -> Command&
{
    if (path.Empty()) {
        this->n_impl->n_wd.Reset();
        return *this;
    }

    this->n_impl->n_wd = path;
    return *this;
}

auto Command::WithStdin(Stdio cfg) noexcept -> Command&
{
    this->n_impl->n_stdin_cfg = VIOLET_MOVE(cfg);
    return *this;
}

auto Command::WithStdout(Stdio cfg) noexcept -> Command&
{
    this->n_impl->n_stdout_cfg = VIOLET_MOVE(cfg);
    return *this;
}

auto Command::WithStderr(Stdio cfg) noexcept -> Command&
{
    this->n_impl->n_stderr_cfg = VIOLET_MOVE(cfg);
    return *this;
}

auto Command::WithUID(uid_t uid) noexcept -> Command&
{
    this->n_impl->n_uid = uid;
    return *this;
}

auto Command::WithGID(gid_t gid) noexcept -> Command&
{
    this->n_impl->n_gid = gid;
    return *this;
}

auto Command::WithGroups(std::initializer_list<gid_t> groups) noexcept -> Command&
{
    this->n_impl->n_extraGids.insert(this->n_impl->n_extraGids.end(), groups);
    return *this;
}

auto Command::WithGroups(Span<gid_t> groups) noexcept -> Command&
{
    this->n_impl->n_extraGids.insert(this->n_impl->n_extraGids.end(), groups.begin(), groups.end());
    return *this;
}

auto Command::WithPreExec(Command::pre_exec_t exec) noexcept -> Command&
{
    this->n_impl->n_pre_exec = VIOLET_MOVE(exec);
    return *this;
}

auto Command::Status() const noexcept -> io::Result<ExitStatus>
{
    auto child = this->n_impl->Spawn();
    if (child.Err()) {
        return Err(VIOLET_MOVE(child.Error()));
    }

    return child->Wait();
}

auto Command::Spawn() const noexcept -> io::Result<Child>
{
    return this->n_impl->Spawn();
}

#endif
