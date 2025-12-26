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

#pragma once

#include <violet/IO/Error.h>
#include <violet/Subprocess.h>
#include <violet/Violet.h>

namespace violet::process {

namespace unix {

    /// @internal
    struct EnvBlock final {
        Vec<char*> Environ; ///< storage of the actual environ that is passed
        Vec<String> Storage; ///< storage of `key=value` blocks
    };

} // namespace unix

/// @internal
struct Command::Impl final {
    enum struct io_kind_t : UInt8 {
        kStdin,
        kStdout,
        kStderr
    };

    constexpr VIOLET_IMPLICIT Impl() noexcept = delete;
    VIOLET_IMPLICIT Impl(Str command) noexcept
        : n_command(command)
    {
    }

    VIOLET_IMPLICIT Impl(Str command, std::initializer_list<String> args) noexcept
        : n_command(command)
        , n_args(args)
    {
    }

    VIOLET_IMPLICIT Impl(Str command, Span<String> args) noexcept
        : n_command(command)
        , n_args(args.begin(), args.end())
    {
    }

    auto BuildEnviron() const noexcept -> unix::EnvBlock;
    auto BuildArgv() const noexcept -> Vec<char*>;
    void ApplyCredentials(io::FileDescriptor pipe) const noexcept;
    void ConfigureIO(io_kind_t kind, int pipes[2]) const noexcept;
    auto Spawn() const noexcept -> io::Result<Child>;

private:
    friend struct Command;

    String n_command;
    Vec<String> n_args;
    FlatHashMap<String, String> n_environ;
    Optional<filesystem::Path> n_wd;
    Stdio n_stdin_cfg = Stdio::Inherit();
    Stdio n_stdout_cfg = Stdio::Inherit();
    Stdio n_stderr_cfg = Stdio::Inherit();
    Optional<uid_t> n_uid;
    Optional<gid_t> n_gid;
    Vec<gid_t> n_extraGids;
    Command::pre_exec_t n_pre_exec;
};

} // namespace violet::process
