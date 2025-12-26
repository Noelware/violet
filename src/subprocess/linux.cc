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

#ifdef VIOLET_LINUX

#include <violet/IO/Error.h>
#include <violet/Subprocess.h>
#include <violet/Subprocess/Unix.h>

#include <fcntl.h>
#include <grp.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

using violet::Int32;
using violet::process::Command;

namespace {

using violet::UInt8;
using violet::Vec;

void writeErrnoToPipe(violet::io::FileDescriptor fd) noexcept
{
    int err = errno;
    ::write(fd.Get(), &err, sizeof(err));
}

void makeUnblock(Int32 fd)
{
    Int32 result = ::fcntl(fd, F_GETFL, 0);
    if (result >= 0) {
        ::fcntl(fd, F_SETFL, result | O_NONBLOCK);
    }
}

void drainPipes(Int32 outFD, Int32 errFD, Vec<UInt8>& out, Vec<UInt8>& err, pid_t pid)
{
    makeUnblock(outFD);
    makeUnblock(errFD);

    bool outIsOpen = outFD >= 0;
    bool errIsOpen = errFD >= 0;
    UInt8 buf[4096];

    while (outIsOpen || errIsOpen) {
        pollfd fds[2];
        nfds_t nfds = 0;

        // Use local copies for polling; if an FD is closed, we don't add it
        if (outIsOpen)
            fds[nfds++] = { .fd = outFD, .events = POLLIN };

        if (errIsOpen)
            fds[nfds++] = { .fd = errFD, .events = POLLIN };

        if (nfds == 0)
            break;

        // 15000ms timeout
        Int32 rc = ::poll(fds, nfds, 15000);

        if (rc == 0) { // Timeout
            ::kill(pid, SIGKILL);
            break;
        }

        if (rc < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        for (nfds_t i = 0; i < nfds; ++i) {
            if (fds[i].revents & (POLLIN | POLLHUP | POLLERR)) {
                bool isOut = (fds[i].fd == outFD);
                auto& dst = isOut ? out : err;
                bool& openFlag = isOut ? outIsOpen : errIsOpen;

                while (true) {
                    ssize_t num = ::read(fds[i].fd, buf, sizeof(buf));
                    if (num > 0) {
                        dst.insert(dst.end(), buf, buf + num);
                    } else if (num == 0) { // EOF
                        ::close(fds[i].fd);
                        openFlag = false;
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // If we got EAGAIN but POLLHUP was set, no more data is coming
                            if (fds[i].revents & POLLHUP) {
                                ::close(fds[i].fd);
                                openFlag = false;
                            }
                            break;
                        }
                        // Actual error
                        ::close(fds[i].fd);
                        openFlag = false;
                        break;
                    }
                }
            }
            // Handle cases where poll returns an error bit but not POLLIN
            else if (fds[i].revents & (POLLNVAL | POLLERR)) {
                if (fds[i].fd == outFD)
                    outIsOpen = false;
                else
                    errIsOpen = false;
                ::close(fds[i].fd);
            }
        }
    }
}

} // namespace

void Command::Impl::ApplyCredentials(io::FileDescriptor pipe) const noexcept
{
    if (auto gid = this->n_gid) {
        if (::setgid(*gid) != 0) {
            writeErrnoToPipe(pipe.Get());
            _exit(127);
        }
    }

    if (!this->n_extraGids.empty()) {
        if (::setgroups(this->n_extraGids.size(), this->n_extraGids.data()) != 0) {
            writeErrnoToPipe(pipe.Get());
            _exit(127);
        }
    }

    if (auto uid = this->n_uid) {
        if (::setuid(*uid) != 0) {
            writeErrnoToPipe(pipe.Get());
            _exit(127);
        }
    }
}

void Command::Impl::ConfigureIO(Command::Impl::io_kind_t kind, int pipes[2]) const noexcept
{
    auto setupPipe = [&](const Stdio& cfg, Int32 target, Int32 pipes[2]) -> void {
        if (cfg.IsInherited())
            return;

        if (cfg.IsNull()) {
            Int32 fd = ::open("/dev/null", kind == io_kind_t::kStdin ? O_RDONLY : O_WRONLY);
            if (fd < 0) {
                _exit(127);
            }

            ::dup2(fd, target);
            ::close(fd);

            return;
        }

        if (!cfg.PipedToFile()) {
            Int32 childFD = kind == io_kind_t::kStdin ? pipes[0] : pipes[1];

            ::dup2(childFD, target);
            if (kind == io_kind_t::kStdin) {
                ::close(pipes[1]); // close write end (parent writes)
                ::close(pipes[0]); // optional: close original read end (dup2 copied it)
            } else { // stdout/stderr
                ::close(pipes[0]); // close read end (parent reads)
                ::close(pipes[1]); // optional: close original write end
            }

            return;
        }

        if (cfg.PipedToFile()) {
            auto* file = cfg.PipedFile().Value();
            auto fd
                = ::open(file->Data().c_str(), kind == io_kind_t::kStdin ? O_RDONLY : (O_CREAT | O_WRONLY | O_TRUNC));

            if (fd < 0) {
                _exit(127);
            }

            ::dup2(fd, target);
            ::close(fd);

            return;
        }

        // invalid state here
        _exit(127);
    };

    switch (kind) {
    case Command::Impl::io_kind_t::kStdin:
        setupPipe(this->n_stdin_cfg, STDIN_FILENO, pipes);
        break;

    case Command::Impl::io_kind_t::kStdout:
        setupPipe(this->n_stdout_cfg, STDOUT_FILENO, pipes);
        break;

    case Command::Impl::io_kind_t::kStderr:
        setupPipe(this->n_stderr_cfg, STDERR_FILENO, pipes);
        break;
    }
}

auto Command::Impl::Spawn() const noexcept -> io::Result<Child>
{
    auto argv = this->BuildArgv();

    unix::EnvBlock environ;
    if (!this->n_environ.empty()) {
        environ = this->BuildEnviron();
    }

    Int32 errPipes[2] = { -1, -1 };
    Int32 stdinPipes[2] = { -1, -1 };
    Int32 stdoutPipes[2] = { -1, -1 };
    Int32 stderrPipes[2] = { -1, -1 };

    auto createPipe = [](Int32 pipes[2]) -> bool { return ::pipe(pipes) == 0; };

    if (!createPipe(errPipes)) {
        return Err(io::Error::OSError());
    }

    ::fcntl(errPipes[1], F_SETFD, FD_CLOEXEC);

    if (this->n_stdin_cfg.IsPipe() && !createPipe(stdinPipes)) {
        return Err(io::Error::OSError());
    }

    if (this->n_stdout_cfg.IsPipe() && !createPipe(stdoutPipes)) {
        return Err(io::Error::OSError());
    }

    if (this->n_stderr_cfg.IsPipe() && !createPipe(stderrPipes)) {
        return Err(io::Error::OSError());
    }

    pid_t pid = ::fork();
    if (pid < 0) {
        return Err(io::Error::OSError());
    }

    if (pid == 0) {
        if (auto path = this->n_wd) {
            if (::chdir(path->Data().c_str()) != 0) {
                writeErrnoToPipe(errPipes[1]);
                _exit(127);
            }
        }

        ConfigureIO(io_kind_t::kStdin, stdinPipes);
        ConfigureIO(io_kind_t::kStdout, stdoutPipes);
        ConfigureIO(io_kind_t::kStderr, stderrPipes);

        ApplyCredentials(errPipes[1]);
        if (this->n_pre_exec) {
            std::invoke(this->n_pre_exec);
        }

        if (!this->n_environ.empty()) {
            ::execve(this->n_command.c_str(), argv.data(), environ.Environ.data());
        } else {
            ::execvp(this->n_command.c_str(), argv.data());
        }

        writeErrnoToPipe(errPipes[1]);
        _exit(127);
    }

    ::close(errPipes[1]);

    Child child;
    child.ProcessID = pid;

    auto setupParentPipe
        = [](const Stdio& cfg, io_kind_t kind, Int32 pipes[2], Optional<io::FileDescriptor>& out) -> void {
        if (!cfg.IsPipe())
            return;

        if (kind == io_kind_t::kStdin) {
            ::close(pipes[0]);
            out = pipes[1];
        } else {
            ::close(pipes[1]);
            out = pipes[0];
        }
    };

    Optional<io::FileDescriptor> stdinFD;
    Optional<io::FileDescriptor> stdoutFD;
    Optional<io::FileDescriptor> stderrFD;

    setupParentPipe(this->n_stdin_cfg, io_kind_t::kStdin, stdinPipes, stdinFD);
    setupParentPipe(this->n_stdout_cfg, io_kind_t::kStdout, stdoutPipes, stdoutFD);
    setupParentPipe(this->n_stderr_cfg, io_kind_t::kStderr, stderrPipes, stderrFD);

    Int32 execErrno = 0;
    ssize_t num = ::read(errPipes[0], &execErrno, sizeof(execErrno));
    ::close(errPipes[0]);



    if (num == sizeof(execErrno)) {
        ::waitpid(pid, nullptr, 0);

        errno = execErrno;
        return Err(io::Error::OSError());
    }

    if (stdinFD.HasValue()) {

        child.Stdin = ChildStdin(stdinFD->Get());
    }

    if (stdoutFD.HasValue()) {

        child.Stdout = ChildStdout(stdoutFD->Get());
    }

    if (stderrFD.HasValue()) {

        child.Stderr = ChildStderr(stderrFD->Get());
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

    if (outFD >= 0 || errFD >= 0) {
        drainPipes(outFD, errFD, out.Stdout, out.Stderr, child->ProcessID);
    }

    out.Status = child->Wait().Unwrap();

    return out;
}

#endif
