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

using violet::Array;
using violet::Err;
using violet::Int32;
using violet::Int64;
using violet::UInt8;

namespace {

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

auto Command::Spawn() -> io::Result<Child>
{
    return detail::SpawnAsForkExec(*this);
}

#endif
