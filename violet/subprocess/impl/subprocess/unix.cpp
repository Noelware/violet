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

#include "violet/io/Error.h"
#include "violet/violet.h"

#ifdef VIOLET_UNIX

// clang-format off
#include "violet/subprocess/Subprocess.h"

#include <initializer_list>

#include <fcntl.h>
#include <sys/wait.h>
// clang-format on

using Noelware::Violet::Subprocess;
using Noelware::Violet::Process::Command;
using Noelware::Violet::Process::Output;

//////// --=-- START :: COMMAND --=-- ////////
auto Command::Spawn() noexcept -> IO::Result<Subprocess>
{
    Array<int32, 2> stdinPipes{ -1, -1 };
    Array<int32, 2> stdoutPipes{ -1, -1 };
    Array<int32, 2> stderrPipes{ -1, -1 };

    VIOLET_DISABLE_WARNING_PUSH
    VIOLET_DISABLE_WARNING("undefined-inline")

    if (this->n_stdin.IsPiped()) {
        ::pipe(stdinPipes.data());
    }

    if (this->n_stdout.IsPiped()) {
        ::pipe(stdoutPipes.data());
    }

    if (this->n_stderr.IsPiped()) {
        ::pipe(stderrPipes.data());
    }

    VIOLET_DISABLE_WARNING_POP

    pid_t pid = fork();
    if (pid < 0) {
        return Err(IO::Error::Platform(IO::ErrorKind::Other));
    }

    if (pid == 0) {
        if (this->n_stdin.IsPiped()) {
            ::close(stdinPipes[1]);
            ::dup2(stdinPipes[0], STDIN_FILENO);
            ::close(stdinPipes[0]);
        } else if (this->n_stdin.IsNull()) {
            int fd = ::open("/dev/null", O_RDONLY);
            ::dup2(fd, STDIN_FILENO);
            ::close(fd);
        }

        if (this->n_stdout.IsPiped()) {
            ::close(stdoutPipes[1]);
            ::dup2(stdoutPipes[0], STDOUT_FILENO);
            ::close(stdoutPipes[0]);
        } else if (this->n_stdin.IsNull()) {
            int fd = ::open("/dev/null", O_RDONLY);
            ::dup2(fd, STDOUT_FILENO);
            ::close(fd);
        }

        if (this->n_stderr.IsPiped()) {
            ::close(stderrPipes[1]);
            ::dup2(stderrPipes[0], STDERR_FILENO);
            ::close(stderrPipes[0]);
        } else if (this->n_stdin.IsNull()) {
            int fd = ::open("/dev/null", O_RDONLY);
            ::dup2(fd, STDERR_FILENO);
            ::close(fd);
        }

        if (auto wd = this->n_wd) {
            ::chdir(static_cast<StringRef>(wd.Value()));
        }

        // Prepare `argv` from our `args`
        Vec<char*> argv = { static_cast<char*>(this->n_program.data()) };
        for (auto& arg: this->n_args) {
            argv.push_back(arg.data());
        }

        argv.push_back(nullptr);

        // Prepare our environment. If the environment in this command hasn't
        // been modified, it'll inherit from the parent's environment.
        Vec<char*> environ;
        Vec<String> envstr;

        for (auto& [key, value]: this->n_envs) {
            envstr.push_back(std::format("{}={}", key, value));
        }

        for (auto& str: envstr) {
            environ.push_back(str.data());
        }

        environ.push_back(nullptr);

        environ.empty() ? execvp(this->n_program.data(), argv.data())
                        : execve(this->n_program.data(), argv.data(), environ.data());

        _exit(127);
    }

    Subprocess process;
    process.n_pid = pid;

    if (this->n_stdin.IsPiped()) {
        ::close(stdinPipes[0]);
        process.Stdin = Process::Stdin(stdinPipes[1], VIOLET_MOVE(this->n_stdin));
    }

    if (this->n_stdout.IsPiped()) {
        ::close(stdoutPipes[0]);
        process.Stdout = Process::Stdout(stdoutPipes[1], VIOLET_MOVE(this->n_stdout));
    }

    if (this->n_stdout.IsPiped()) {
        ::close(stderrPipes[0]);
        process.Stderr = Process::Stderr(stderrPipes[1], VIOLET_MOVE(this->n_stderr));
    }

    return process;
}

auto Command::Output() noexcept -> IO::Result<Process::Output>
{
    auto proc = this->Spawn();
    if (!proc) {
        return proc.Error();
    }

    return proc.Value().Output();
}

auto Command::Status() noexcept -> IO::Result<int32>
{
    auto proc = this->Spawn();
    if (!proc) {
        return proc.Error();
    }

    return proc.Value().Wait();
}

Subprocess::Subprocess() = default;
Subprocess::~Subprocess()
{
    if (auto stdin = this->Stdin; stdin->Valid()) {
        // stdin->Close();
    }

    if (auto stdout = this->Stdout; stdout->Valid()) {
        // stdout->Close();
    }

    if (auto stderr = this->Stderr; stderr->Valid()) {
        // stderr->Close();
    }

    // if the pid of this subprocess is valid
    if (this->Alive()) {
        int32 status = 0;

        pid_t ret = ::waitpid(this->n_pid, &status, WNOHANG);
        if (ret == 0) {
            // The child is still running even if we call `waitpid`, we'll kill the
            // child with `SIGTERM`.
            ::kill(this->n_pid, SIGTERM);
        }

        // Do a simple sanity check if `pid` == `ret` as if `ret` is equal
        // to `pid`, the process has been killed already
        assert(this->n_pid == ret);
    }
}

auto Subprocess::Alive() const noexcept -> bool
{
    return this->n_pid > 0;
}

Subprocess::operator bool() const noexcept
{
    return this->Alive();
}

auto Subprocess::ID() const noexcept -> int32
{
    return this->n_pid;
}

auto Subprocess::Wait() -> IO::Result<int32>
{
    if (!this->Alive()) {
        errno = EINVAL;
        return IO::Error::Platform(IO::ErrorKind::InvalidData);
    }

    int status = 0;
    if (::waitpid(this->n_pid, &status, 0) == -1) {
        return IO::Error::Platform(IO::ErrorKind::Other);
    }

    this->n_pid = -1;
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

auto Subprocess::Kill() -> IO::Result<void>
{
    if (this->n_pid <= 0) {
        errno = ESRCH;
        return IO::Error::Platform(IO::ErrorKind::Other);
    }

    if (::kill(this->n_pid, SIGKILL) == -1) {
        return IO::Error::Platform(IO::ErrorKind::Other);
    }

    this->n_pid = -1;
    return {};
}

void Subprocess::Detach()
{
    this->n_pid = -1;
}

auto Subprocess::Output() noexcept -> IO::Result<Process::Output>
{
    Process::Output output;
    if (this->Stdout) {
        if (auto stdout = this->Stdout.Take()) {
            if (auto result = IO::ReadToString(*stdout)) {
                output.Stdout = result.Value();
            }
        }
    }

    if (this->Stderr) {
        if (auto stderr = this->Stderr.Take()) {
            if (auto result = IO::ReadToString(*stderr)) {
                output.Stdout = result.Value();
            }
        }
    }

    auto status = this->Wait();
    if (status.IsErr()) {
        return status.Error();
    }

    output.Status = status.Value();
    return output;
}

#endif
