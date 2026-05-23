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

#if VIOLET_PLATFORM(LINUX)

#include <violet/Subprocess.h>
#include <violet/Subprocess/PipeReader.h>
#include <violet/Subprocess/__detail/Impl.unix.h>

#include <thread>

#include <fcntl.h>
#include <grp.h>
#include <sys/poll.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std::chrono_literals;

using violet::subprocess::Child;
using violet::subprocess::Command;
using violet::subprocess::ExitStatus;
using violet::subprocess::Output;
using violet::subprocess::Stdio;

using violet::Array;
using violet::Err;
using violet::Int32;
using violet::Int64;
using violet::UInt8;

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

auto makePipes(Int32 fds[2]) -> bool
{
    return ::pipe2(fds, O_CLOEXEC) == 0;
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

void drainPipes(Int32 stdout, Int32 stderr, Output& out)
{
    // clang-format off
    struct pollfd pfds[2] = {
        { .fd = stdout, .events = POLLIN, .revents = 0 },
        { .fd = stderr, .events = POLLIN, .revents = 0 }
    };
    // clang-format on

    Array<UInt8, 4096> chunk;
    Int32 openFDs = (stdout >= 0 ? 1 : 0) + (stderr >= 0 ? 1 : 0);

    while (openFDs > 0) {
        if (Int32 ret = ::poll(pfds, 2, -1); ret < 0) {
            if (errno == EINTR) {
                continue;
            }

            break;
        }

        for (Int32 i = 0; i < 2; i++) {
            if (pfds[i].fd < 0) {
                continue;
            }

            if ((pfds[i].revents & (POLLIN | POLLHUP | POLLERR)) == 0) {
                continue;
            }

            while (true) {
                Int64 num = ::read(pfds[i].fd, chunk.data(), chunk.size());
                if (num > 0) {
                    if (i == 0) {
                        out.Stdout.insert(out.Stdout.end(), chunk.begin(), chunk.begin() + num);
                    } else {
                        out.Stderr.insert(out.Stderr.end(), chunk.begin(), chunk.begin() + num);
                    }
                } else if (num == 0) {
                    ::close(pfds[i].fd);
                    pfds[i].fd = -1;
                    openFDs--;

                    break;
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    }

                    if (errno == EINTR) {
                        continue;
                    }

                    ::close(pfds[i].fd);
                    openFDs--;
                    break;
                }
            }
        }
    }
}

auto pollWait(Int32 pid, std::chrono::milliseconds timeout) -> violet::io::Result<ExitStatus>
{
    auto deadline = std::chrono::steady_clock::now() + timeout;
    auto interval = 1ms;
    constexpr auto kMaxInterval = 50ms;

    while (std::chrono::steady_clock::now() < deadline) {
        Int32 status = 0;
        pid_t ret = ::waitpid(pid, &status, WNOHANG);

        if (ret > 0) {
            return ExitStatus(status);
        }

        if (ret < 0 && errno != EINTR) {
            return Err(violet::io::Error::OSError());
        }

        auto remaining
            = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());

        auto toSleep = std::min(interval, remaining);
        if (toSleep.count() <= 0) {
            break;
        }

        std::this_thread::sleep_for(toSleep);
        interval = std::min(interval * 2, kMaxInterval);
    }

    return Err(VIOLET_IO_ERROR(TimedOut, violet::String, "process death timed-out reached"));
}

} // namespace

auto Command::doSpawn() const -> io::Result<Child>
{
    Int32 stdinPipes[2] = { -1, -1 };
    Int32 stdoutPipes[2] = { -1, -1 };
    Int32 stderrPipes[2] = { -1, -1 };
    Int32 errPipe[2] = { -1, -1 };

    if (this->n_impl->n_stdin.Piped() && !this->n_impl->n_stdin.PipedIntoFile()) {
        if (!makePipes(stdinPipes)) {
            return Err(io::Error::OSError());
        }
    }

    if (this->n_impl->n_stdout.Piped() && !this->n_impl->n_stdout.PipedIntoFile()) {
        if (!makePipes(stdoutPipes)) {
            closePipes(stdinPipes);
            return Err(io::Error::OSError());
        }
    }

    if (this->n_impl->n_stderr.Piped() && !this->n_impl->n_stderr.PipedIntoFile()) {
        if (!makePipes(stderrPipes)) {
            closePipes(stdinPipes);
            closePipes(stdoutPipes);
            return Err(io::Error::OSError());
        }
    }

    if (::pipe2(errPipe, O_CLOEXEC) < 0) {
        closePipes(stdinPipes);
        closePipes(stdoutPipes);
        closePipes(stderrPipes);
        return Err(io::Error::OSError());
    }

    Vec<CStr> argv;
    argv.reserve(this->n_impl->n_args.size() + 2);
    argv.push_back(this->n_impl->n_program.c_str());
    for (const auto& arg: this->n_impl->n_args) {
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

        setupFileDescriptor(stdinPipes[0], STDIN_FILENO, this->n_impl->n_stdin, /*readonly=*/true);
        setupFileDescriptor(stdoutPipes[1], STDOUT_FILENO, this->n_impl->n_stdout);
        setupFileDescriptor(stderrPipes[1], STDERR_FILENO, this->n_impl->n_stderr);

        if (!this->n_impl->n_extraGroupIDs.empty()) {
            if (::setgroups(this->n_impl->n_extraGroupIDs.size(), this->n_impl->n_extraGroupIDs.data()) < 0) {
                reportErrno(errno);
            }
        }

        if (auto gid = this->n_impl->n_gid; gid.HasValue()) {
            if (::setgid(*gid) < 0) {
                reportErrno(errno);
            }
        }

        if (auto uid = this->n_impl->n_uid; uid.HasValue()) {
            if (::setuid(*uid) < 0) {
                reportErrno(errno);
            }
        }

        if (auto wd = this->n_impl->n_wd; wd.HasValue()) {
            auto path = wd->Data();
            if (::chdir(path.c_str()) < 0) {
                reportErrno(errno);
            }
        }

        for (const auto& [key, value]: this->n_impl->n_environ) {
            ::setenv(key.c_str(), value.c_str(), 1);
        }

        if (auto preExec = this->n_impl->n_exec; preExec.HasValue()) {
            std::invoke(*preExec);
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        ::execvp(this->n_impl->n_program.c_str(), const_cast<char* const*>(argv.data()));
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

    child.DeathTimeout = VIOLET_MOVE(this->n_impl->n_deathTimeout);
    return child;
}

auto Child::Wait() const -> io::Result<ExitStatus>
{
    if (!this->PID) {
        return Err(VIOLET_IO_ERROR(InvalidData, String, "child is not running"));
    }

    if (!this->DeathTimeout.HasValue()) {
        Int32 status = -1;
        struct PID waited = -1;
        do {
            waited = ::waitpid(this->PID.Get(), &status, 0);
        } while (waited.Get() < 0 && errno == EINTR);

        if (waited < 0) {
            return Err(io::Error::OSError());
        }

        return ExitStatus(status);
    }

    auto pidfd = static_cast<Int32>(::syscall(SYS_pidfd_open, this->PID.Get(), 0));
    if (pidfd >= 0) {
        struct pollfd pfd{ };
        pfd.fd = pidfd;
        pfd.events = POLLIN;

        Int32 ret = ::poll(&pfd, 1, static_cast<Int32>(this->DeathTimeout->count()));
        if (ret == 0) {
            return Err(VIOLET_IO_ERROR(TimedOut, String, "process death time-out reached"));
        }

        if (ret < 0) {
            return Err(violet::io::Error::OSError());
        }
    }

    return pollWait(this->PID.Get(), this->DeathTimeout.Value());
}

auto Child::ToString() const -> String
{
    return std::format("Child(pid={})", this->PID);
}

auto Command::Spawn() const -> io::Result<Child>
{
    return this->doSpawn();
}

auto Command::Output() const -> io::Result<struct Output>
{
    auto child = VIOLET_TRY(this->Spawn());
    struct Output out;

    auto setFDAsNonBlocking = [](Int32 fd) -> void {
        if (fd >= 0) {
            Int32 flags = ::fcntl(fd, F_GETFL, 0);
            ::fcntl(fd, F_SETFD, flags | O_NONBLOCK);
        }
    };

    bool needsMultiplexing = child.Stdout.HasValue() && child.Stderr.HasValue();

    if (auto stdout = child.Stdout; stdout.HasValue()) {
        setFDAsNonBlocking(stdout->Descriptor.Get());
        if (!needsMultiplexing) {
            auto reader = GetPipeReader();
            VIOLET_ASSERT(reader != nullptr, "there is no unique PipeReader implementation available");

            reader->Register(stdout->Descriptor.Get());
            out.Stdout = VIOLET_TRY(reader->CaptureAll());
        }
    }

    if (auto stderr = child.Stderr; stderr.HasValue()) {
        setFDAsNonBlocking(stderr->Descriptor.Get());
        if (!needsMultiplexing) {
            auto reader = GetPipeReader();
            VIOLET_ASSERT(reader != nullptr, "there is no unique PipeReader implementation available");

            reader->Register(stderr->Descriptor.Get());
            out.Stderr = VIOLET_TRY(reader->CaptureAll());
        }
    }

    if (needsMultiplexing) {
        drainPipes(child.Stdout->Descriptor.Get(), child.Stderr->Descriptor.Get(), out);
    }

    out.Status = VIOLET_TRY(child.Wait());
    return out;
}

auto Command::Status() const -> io::Result<ExitStatus>
{
    auto child = VIOLET_TRY(this->Spawn());
    if (auto stdin = child.Stdin; stdin.HasValue()) {
        stdin->Descriptor.Close();
    }

    if (auto stdout = child.Stdout; stdout.HasValue()) {
        stdout->Descriptor.Close();
    }

    if (auto stderr = child.Stderr; stderr.HasValue()) {
        stderr->Descriptor.Close();
    }

    return child.Wait();
}

#endif
