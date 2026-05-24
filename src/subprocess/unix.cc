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
#include <violet/Subprocess/PipeReader.h>
#include <violet/Subprocess/__detail/Impl.unix.h>

#include <fcntl.h>
#include <sys/poll.h>
#include <unistd.h>

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

Command::Command(const Command& other)
    : n_impl(other.n_impl != nullptr ? new Impl(*other.n_impl) : nullptr)
{
}

auto Command::operator=(const Command& other) -> Command&
{
    if (this != &other) {
        if (this->n_impl != nullptr) {
            delete this->n_impl;
            this->n_impl = nullptr;
        }

        this->n_impl = other.n_impl != nullptr ? new Impl(*other.n_impl) : nullptr;
    }

    return *this;
}

Command::Command(Command&& command) noexcept
    : n_impl(std::exchange(command.n_impl, nullptr))
{
}

auto Command::operator=(Command&& other) noexcept -> Command&
{
    if (this != &other) {
        if (this->n_impl != nullptr) {
            delete this->n_impl;
            this->n_impl = nullptr;
        }

        this->n_impl = std::exchange(other.n_impl, nullptr);
    }

    return *this;
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

auto Command::Output() -> io::Result<struct Output>
{
    auto child = VIOLET_TRY(this->Spawn());
    struct Output out;

    auto setFDAsNonBlocking = [](Int32 fd) -> void {
        if (fd >= 0) {
            Int32 flags = ::fcntl(fd, F_GETFL, 0);
            ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        }
    };

    bool needsMultiplexing = child.Stdout.HasValue() && child.Stderr.HasValue();
    if (auto stdout = child.Stdout; stdout.HasValue()) {
        if (!needsMultiplexing) {
            auto reader = GetPipeReader();
            VIOLET_ASSERT(reader != nullptr, "there is no unique PipeReader implementation available");

            if (reader->WantsNonBlocking()) {
                setFDAsNonBlocking(stdout->Descriptor.Get());
            }

            reader->Register(stdout->Descriptor.Get());
            out.Stdout = VIOLET_TRY(reader->CaptureAll());
        }
    }

    if (auto stderr = child.Stderr; stderr.HasValue()) {
        if (!needsMultiplexing) {
            auto reader = GetPipeReader();
            VIOLET_ASSERT(reader != nullptr, "there is no unique PipeReader implementation available");

            if (reader->WantsNonBlocking()) {
                setFDAsNonBlocking(stderr->Descriptor.Get());
            }

            reader->Register(stderr->Descriptor.Get());
            out.Stdout = VIOLET_TRY(reader->CaptureAll());
        }
    }

    if (needsMultiplexing) {
        detail::DrainPipes(child.Stdout->Descriptor.Get(), child.Stderr->Descriptor.Get(), out);
    }

    out.Status = VIOLET_TRY(child.Wait());
    return out;
}

auto Command::Status() -> io::Result<ExitStatus>
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

auto Child::ToString() const -> String
{
    return std::format("Child(pid={})", this->PID);
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

auto violet::subprocess::detail::MakePipes(Int32 fds[2]) -> bool
{
    if (::pipe(fds) != 0) {
        return false;
    }

    ::fcntl(fds[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(fds[1], F_SETFD, FD_CLOEXEC);
    return true;
}

void violet::subprocess::detail::DrainPipes(
    violet::io::FileDescriptor::value_type stdoutFd, violet::io::FileDescriptor::value_type stderrFd, Output& out)
{
    struct pollfd pfds[2]
        = { { .fd = stdoutFd, .events = POLLIN, .revents = 0 }, { .fd = stderrFd, .events = POLLIN, .revents = 0 } };

    Array<UInt8, 4096> chunk;
    Int32 openFDs = (stdoutFd >= 0 ? 1 : 0) + (stderrFd >= 0 ? 1 : 0);

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
                    pfds[i].fd = -1;
                    openFDs--;
                    break;
                }
            }
        }
    }
}

#endif
