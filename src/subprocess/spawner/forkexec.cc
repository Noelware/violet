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

#include <fcntl.h>
#include <unistd.h>

using violet::Int32;
using violet::subprocess::Command;
using violet::subprocess::Stdio;

namespace {

void closePipes(Int32 fds[2])
{
    if (fds[0] >= 0) {
        ::close(fds[0]);
        fds[0] = -1;
    }

    if (fds[1] >= 0) {
        ::close(fds[1]);
        fds[1] = -1;
    }
}

void setupFileDescriptor(Int32 pipe, Int32 target, const Stdio& config, bool readonly = false)
{
    if (pipe >= 0) {
        ::dup2(pipe, target);
    } else if (config.IsNull()) {
        Int32 devNull = ::open("/dev/null", readonly ? O_RDONLY : O_WRONLY);
        if (devNull >= 0) {
            ::dup2(devNull, target);
            ::close(devNull);
        }
    } else if (config.PipedIntoFile()) {
        auto path = config.PipedFile().Unwrap();
        Int32 flags = readonly ? O_RDONLY : (O_WRONLY | O_CREAT | O_TRUNC);
        Int32 file = ::open(path.Data().c_str(), flags, 0644);
        if (file >= 0) {
            ::dup2(file, target);
            ::close(file);
        }
    }
}

} // namespace

auto violet::subprocess::detail::SpawnAsForkExec(Command& command) -> io::Result<Child>
{
    Int32 stdinPipes[2] = { -1, -1 };
    Int32 stdoutPipes[2] = { -1, -1 };
    Int32 stderrPipes[2] = { -1, -1 };
    Int32 errPipe[2] = { -1, -1 };

    if (command.n_impl->n_stdin.Piped() && !command.n_impl->n_stdin.PipedIntoFile()) {
        if (!detail::MakePipes(stdinPipes)) {
            return Err(io::Error::OSError());
        }
    }

    if (command.n_impl->n_stdout.Piped() && !command.n_impl->n_stdout.PipedIntoFile()) {
        if (!detail::MakePipes(stdoutPipes)) {
            closePipes(stdinPipes);
            return Err(io::Error::OSError());
        }
    }

    if (command.n_impl->n_stderr.Piped() && !command.n_impl->n_stderr.PipedIntoFile()) {
        if (!detail::MakePipes(stderrPipes)) {
            closePipes(stdinPipes);
            closePipes(stdoutPipes);
            return Err(io::Error::OSError());
        }
    }

    if (!detail::MakePipes(errPipe)) {
        closePipes(stdinPipes);
        closePipes(stdoutPipes);
        closePipes(stderrPipes);
        return Err(io::Error::OSError());
    }

    Vec<CStr> argv;
    argv.reserve(command.n_impl->n_args.size() + 2);
    argv.push_back(command.n_impl->n_program.c_str());
    for (const auto& arg: command.n_impl->n_args) {
        argv.push_back(arg.c_str());
    }

    argv.push_back(nullptr);

    PID pid = ::fork();
    if (pid < 0) {
        closePipes(stdinPipes);
        closePipes(stdoutPipes);
        closePipes(stderrPipes);
        closePipes(errPipe);
        return Err(io::Error::OSError());
    }

    // we are the child process
    if (pid == 0) {
        ::close(errPipe[0]);

        auto reportErrno = [errPipe](Int32 err) -> void {
            ::write(errPipe[1], &err, sizeof(err));
            ::_exit(127);
        };

        setupFileDescriptor(stdinPipes[0], STDIN_FILENO, command.n_impl->n_stdin, /*readonly=*/true);
        setupFileDescriptor(stdoutPipes[1], STDOUT_FILENO, command.n_impl->n_stdout);
        setupFileDescriptor(stderrPipes[1], STDERR_FILENO, command.n_impl->n_stderr);

        if (!command.n_impl->n_extraGroupIDs.empty()) {
            if (::setgroups(command.n_impl->n_extraGroupIDs.size(), command.n_impl->n_extraGroupIDs.data()) < 0) {
                reportErrno(errno);
            }
        }

        if (auto gid = command.n_impl->n_gid; gid.HasValue()) {
            if (::setgid(*gid) < 0) {
                reportErrno(errno);
            }
        }

        if (auto uid = command.n_impl->n_uid; uid.HasValue()) {
            if (::setuid(*uid) < 0) {
                reportErrno(errno);
            }
        }

        if (auto wd = command.n_impl->n_wd; wd.HasValue()) {
            auto path = wd->Data();
            if (::chdir(path.c_str()) < 0) {
                reportErrno(errno);
            }
        }

        for (const auto& [key, value]: command.n_impl->n_environ) {
            ::setenv(key.c_str(), value.c_str(), 1);
        }

        if (auto preExec = command.n_impl->n_exec; preExec.HasValue()) {
            std::invoke(*preExec);
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        ::execvp(command.n_impl->n_program.c_str(), const_cast<char* const*>(argv.data()));
        reportErrno(errno);
    }

    // we are back as the parent: close child-side ends
    ::close(errPipe[1]);
    if (stdinPipes[0] >= 0) {
        ::close(stdinPipes[0]);
    }

    if (stdoutPipes[1] >= 0) {
        ::close(stdoutPipes[1]);
    }

    if (stderrPipes[1] >= 0) {
        ::close(stderrPipes[1]);
    }

    Int32 childErrno = 0;
    Int64 bytes = 0;
    do {
        bytes = ::read(errPipe[0], &childErrno, sizeof(childErrno));
    } while (bytes < 0 && errno == EINTR);

    ::close(errPipe[0]);
    if (bytes == sizeof(childErrno)) {
        ::waitpid(pid.Get(), nullptr, 0);
        closePipes(stdinPipes);
        closePipes(stdoutPipes);
        closePipes(stderrPipes);

        return Err(io::Error::FromOSError(childErrno));
    }

    Child child(pid);
    if (stdinPipes[1] >= 0) {
        child.Stdin = ChildStdin(stdinPipes[1]);
    }

    if (stdoutPipes[0] >= 0) {
        child.Stdout = ChildStdout(stdoutPipes[0]);
    }

    if (stderrPipes[0] >= 0) {
        child.Stderr = ChildStderr(stderrPipes[0]);
    }

    child.DeathTimeout = VIOLET_MOVE(command.n_impl->n_deathTimeout);
    return child;
}

#endif
