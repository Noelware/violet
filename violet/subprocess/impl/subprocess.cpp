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

#include "violet/subprocess/Subprocess.h"

using Noelware::Violet::Process::Command;

Command::Command(String program) noexcept
    : n_program(VIOLET_MOVE(program))
{
}

Command::Command(String program, Vec<String> args) noexcept
    : n_program(VIOLET_MOVE(program))
    , n_args(VIOLET_MOVE(args))
{
}

Command::Command(String program, std::initializer_list<String> init) noexcept
    : n_program(VIOLET_MOVE(program))
    , n_args(init)
{
}

auto Command::WithArg(String arg) noexcept -> Command&
{
    this->n_args.push_back(VIOLET_MOVE(arg));
    return *this;
}

auto Command::WithArgs(std::initializer_list<String> init) noexcept -> Command&
{
    this->n_args.insert(this->n_args.end(), init);
    return *this;
}

auto Command::WithArgs(Vec<String> args) noexcept -> Command&
{
    this->n_args.insert(this->n_args.end(), args.begin(), args.end());
    return *this;
}

auto Command::WithEnvironment(std::initializer_list<Pair<const String, String>> init) noexcept -> Command&
{
    this->n_envs.insert(init);
    return *this;
}

auto Command::WithEnvironment(const absl::flat_hash_map<String, String>& map) noexcept -> Command&
{
    for (const auto& [name, value]: map) {
        this->n_envs.insert_or_assign(name, value);
    }

    return *this;
}

auto Command::WithStdin(Stdio stdio) noexcept -> Command&
{
    this->n_stdin = stdio;
    return *this;
}

auto Command::WithStdout(Stdio stdio) noexcept -> Command&
{
    this->n_stdout = stdio;
    return *this;
}

auto Command::WithStderr(Stdio stdio) noexcept -> Command&
{
    this->n_stderr = stdio;
    return *this;
}

auto Command::WithWorkingDir(Noelware::Violet::Filesystem::PathRef path) noexcept -> Command&
{
    this->n_wd = path;
    return *this;
}
