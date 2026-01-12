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

#include <csignal>
#include <violet/Violet.h>

#ifdef VIOLET_APPLE_MACOS

#include <violet/IO/Error.h>
#include <violet/Subprocess.h>
#include <violet/Subprocess/Unix.h>

#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

using violet::Int32;
using violet::UInt8;
using violet::Vec;
using violet::process::Command;

namespace {

void unblockFD(Int32 fd)
{
    Int32 result = ::fcntl(fd, F_GETFL, 0);
    if (result >= 0) {
        VIOLET_ASSERT(::fcntl(fd, F_SETFL, result | O_NONBLOCK), "unable to make file descriptor non-blocking");
    }
}

/// This will drain stdout and stderr pipes into `out` and `err` respectively.
void drainPipes(Int32 outFD, Int32 errFD, Vec<UInt8>& out, Vec<UInt8>& err, pid_t pid)
{
    if (outFD >= 0) {
        unblockFD(outFD);
    }

    if (errFD >= 0) {
        unblockFD(errFD);
    }

    bool isStdoutOpen = outFD >= 0;
    bool isStderrOpen = errFD >= 0;
    UInt8 buf[4096];

    while (isStdoutOpen || isStderrOpen) {
        pollfd fds[2];
        nfds_t nfds = 0;

        if (isStdoutOpen) {
            fds[nfds++] = { .fd = outFD, .events = POLLIN };
        } else if (isStderrOpen) {
            fds[nfds++] = { .fd = errFD, .events = POLLIN };
        }

        Int32 rc = ::poll(fds, nfds, 15 * 1000 /*15s*/);
        if (rc == 0) {
            ::kill(pid, SIGKILL);
            break;
        }

        if (rc < 0) {
            if (errno == EINTR) {
                continue;
            }

            break;
        }

        for (nfds_t i = 0; i < nfds; ++i) {
            if ((fds[i].revents & (POLLIN | POLLHUP | POLLERR)) != 0) {
                while (true) {
                    ssize_t num = ::read(fds[i].fd, buf, sizeof(buf));
                    if (num > 0) {
                        if (isStdoutOpen)
                            out.insert(out.end(), buf, buf + num);
                        else
                            err.insert(err.end(), buf, buf + num);
                    } else if (num == 0) {
                        ::close(fds[i].fd);
                        if (isStdoutOpen) {
                            isStdoutOpen = false;
                        } else if (isStderrOpen) {
                            isStderrOpen = false;
                        }

                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }

                        ::close(fds[i].fd);
                        if (isStdoutOpen) {
                            isStdoutOpen = false;
                        } else if (isStderrOpen) {
                            isStderrOpen = false;
                        }

                        break;
                    }
                }
            }
        }
    }
}

} // namespace

auto Command::Impl::Spawn() const noexcept -> io::Result<Child>
{
    posix_spawn_file_actions_t actions = { nullptr };
    posix_spawnattr_t attributes = nullptr;

    posix_spawn_file_actions_init(&actions);
    posix_spawnattr_init(&attributes);

    // Ensures that children don't inherit management pipes
    posix_spawnattr_setflags(&attributes, POSIX_SPAWN_CLOEXEC_DEFAULT);

    Int32 stdinPipes[2] = { -1, -1 };
    Int32 stdoutPipes[2] = { -1, -1 };
    Int32 stderrPipes[2] = { -1, -1 };

    auto setupPipeActions = [&](const Stdio& cfg, Int32 target, Int32 pipes[2], bool stdin) -> void {
        if (cfg.IsInherited()) {
            return;
        }

        if (cfg.IsNull()) {
            posix_spawn_file_actions_addopen(&actions, target, "/dev/null", stdin ? O_RDONLY : O_WRONLY, 0);
            return;
        }

        if (cfg.IsPipe() && !cfg.PipedToFile()) {
            if (::pipe(pipes) != 0) {
                return;
            }

            if (stdin) {
                ::posix_spawn_file_actions_adddup2(&actions, pipes[0], target);
                ::posix_spawn_file_actions_addclose(&actions, pipes[1]);
            } else {
                ::posix_spawn_file_actions_adddup2(&actions, pipes[1], target);
                ::posix_spawn_file_actions_addclose(&actions, pipes[0]);
            }

            return;
        }

        auto data = cfg.PipedFile()->Data();
        if (stdin) {
            ::posix_spawn_file_actions_addopen(&actions, target, data.c_str(), O_RDONLY, 0);
        } else {
            Int32 flags = O_CREAT | O_WRONLY | O_TRUNC;
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            ::posix_spawn_file_actions_addopen(&actions, target, data.c_str(), flags, mode);
        }
    };

    setupPipeActions(this->n_stdin_cfg, STDIN_FILENO, stdinPipes, true);
    setupPipeActions(this->n_stdin_cfg, STDIN_FILENO, stdoutPipes, false);
    setupPipeActions(this->n_stdin_cfg, STDIN_FILENO, stderrPipes, false);

    // Set the working directory, if one was provided
    if (auto wd = this->n_wd) {
        auto data = wd->Data();
        ::posix_spawn_file_actions_addchdir_np(&actions, data.c_str());
    }

    // TODO(@auguwu): make the Linux impl as `return this->useForkImpl();` and
    // use that if we have uid/gid/... since macOS doesn't have specific APIs
    // for it with the `posix_spawn` API. Sigh...

    auto argv = this->BuildArgv();
    auto env = this->n_environ.empty() ? unix::EnvBlock{} : this->BuildEnviron();

    pid_t pid = -1;
    int rc = ::posix_spawnp(&pid, this->n_command.c_str(), &actions, &attributes,
        static_cast<char* const*>(argv.data()), env.Environ.data());

    ::posix_spawn_file_actions_destroy(&actions);
    ::posix_spawnattr_destroy(&attributes);

    if (rc != 0) {
        errno = rc;
        return Err(io::Error::OSError());
    }

    Child child = { .ProcessID = pid };

    if (this->n_stdin_cfg.IsPipe()) {
        ::close(stdinPipes[0]);
        child.Stdin = ChildStdin(stdinPipes[1]);
    }

    if (this->n_stdout_cfg.IsPipe()) {
        ::close(stdoutPipes[1]);
        child.Stdout = ChildStdout(stdoutPipes[0]);
    }

    if (this->n_stderr_cfg.IsPipe()) {
        ::close(stderrPipes[1]);
        child.Stderr = ChildStderr(stderrPipes[0]);
    }

    return child;
}

auto Command::Output() const noexcept -> io::Result<struct Output>
{
    auto child = this->Spawn();
    if (child.Err()) {
        return Err(VIOLET_MOVE(child.Error()));
    }

    struct Output out;
    Int32 outFD = child->Stdout.HasValue() ? child->Stdout->Descriptor.Get() : -1;
    Int32 errFD = child->Stderr.HasValue() ? child->Stderr->Descriptor.Get() : -1;

    drainPipes(outFD, errFD, out.Stdout, out.Stderr, child->ProcessID);
    out.Status = child->Wait().Unwrap();

    return out;
}

#endif
