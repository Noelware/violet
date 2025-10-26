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
//
//! # 🌺💜 `violet/subprocess/Subprocess.h`

#pragma once

#include "absl/container/flat_hash_map.h"
#include "violet/container/Optional.h"
#include "violet/filesystem/Path.h"
#include "violet/io/Error.h"
#include "violet/subprocess/Stdio.h"
#include "violet/violet.h"

#include <initializer_list>

#ifdef VIOLET_WINDOWS
#    include <windows.h>
#endif

namespace Noelware::Violet {

struct Subprocess;

} // namespace Noelware::Violet

namespace Noelware::Violet::Process {

struct Output final {
    int32 Status;
    String Stdout;
    String Stderr;
};

struct Command final {
    Command() = delete;

    VIOLET_EXPLICIT Command(String) noexcept;
    VIOLET_EXPLICIT Command(String, Vec<String>) noexcept;
    VIOLET_EXPLICIT Command(String, std::initializer_list<String>) noexcept;

    auto WithArg(String) noexcept -> Command&;

    auto WithArgs(std::initializer_list<String>) noexcept -> Command&;
    auto WithArgs(Vec<String>) noexcept -> Command&;

    auto WithEnvironment(std::initializer_list<Pair<const String, String>>) noexcept -> Command&;
    auto WithEnvironment(const absl::flat_hash_map<String, String>&) noexcept -> Command&;

    auto WithStdin(Stdio) noexcept -> Command&;
    auto WithStdout(Stdio) noexcept -> Command&;
    auto WithStderr(Stdio) noexcept -> Command&;

    auto WithWorkingDir(Filesystem::PathRef) noexcept -> Command&;

    auto Spawn() noexcept -> IO::Result<Noelware::Violet::Subprocess>;
    auto Output() noexcept -> IO::Result<Output>;
    auto Status() noexcept -> IO::Result<int32>;

private:
    String n_program;
    Vec<String> n_args;
    absl::flat_hash_map<String, String> n_envs;
    Stdio n_stdin;
    Stdio n_stdout;
    Stdio n_stderr;
    Optional<Filesystem::PathRef> n_wd;
};

} // namespace Noelware::Violet::Process

namespace Noelware::Violet {

struct Subprocess final {
    Optional<Process::Stdin> Stdin;
    Optional<Process::Stdout> Stdout;
    Optional<Process::Stderr> Stderr;

    Subprocess();
    ~Subprocess();

    Subprocess(const Subprocess&) = delete;
    auto operator=(const Subprocess&) -> Subprocess& = delete;

    Subprocess(Subprocess&&) noexcept;
    auto operator=(Subprocess&&) noexcept -> Subprocess&;

    auto Alive() const noexcept -> bool;
    auto ID() const noexcept -> int32;
    auto Wait() -> IO::Result<int32>;
    auto Kill() -> IO::Result<void>;

    void Detach();

    auto Output() noexcept -> IO::Result<Process::Output>;

    VIOLET_EXPLICIT operator bool() const noexcept;

private:
    friend struct Process::Command;

    int32 n_pid = -1;

#ifdef VIOLET_WINDOWS
    HANDLE n_handle = INVALID_HANDLE_VALUE;
    HANDLE n_thread = INVALID_HANDLE_VALUE;
#endif
};

} // namespace Noelware::Violet
