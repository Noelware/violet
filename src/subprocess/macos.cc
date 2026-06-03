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

#if VIOLET_PLATFORM(APPLE_MACOS)

#include <violet/Subprocess.h>
#include <violet/Subprocess/__detail/Impl.unix.h>

#include <sys/event.h>
#include <unistd.h>

using violet::subprocess::Child;
using violet::subprocess::Command;
using violet::subprocess::ExitStatus;

auto Child::Wait() const -> io::Result<ExitStatus>
{
    if (!this->PID) {
        return Err(VIOLET_IO_ERROR(InvalidData, String, "child is not running"));
    }

    if (!this->DeathTimeout.HasValue()) {
        Int32 status = -1;
        pid_t waited = -1;
        do {
            waited = ::waitpid(this->PID.Get(), &status, 0);
        } while (waited < 0 && errno == EINTR);

        if (waited < 0) {
            return Err(io::Error::OSError());
        }

        return ExitStatus(status);
    }

    Int32 queue = ::kqueue();
    if (queue < 0) {
        return Err(io::Error::OSError());
    }

    struct kevent event{ };
    EV_SET(&event, this->PID.Get(), EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, nullptr);
    if (::kevent(queue, &event, 1, nullptr, 0, nullptr) < 0) {
        Int32 savedErrno = errno;
        ::close(queue);

        // `kqueue` cannot attach an `EVFILT_PROC` filter to a process that has already
        // exited (it is now a zombie), so registration fails with `ESRCH`. This is racy:
        // the child can die between `Spawn` and reaching here (e.g. it was just sent a
        // signal it honours). In that case the child is still reapable, so fall through
        // to `waitpid` instead of surfacing the error.
        if (savedErrno != ESRCH) {
            errno = savedErrno;
            return Err(io::Error::OSError());
        }

        Int32 status = 0;
        pid_t waited = -1;
        do {
            waited = ::waitpid(this->PID.Get(), &status, 0);
        } while (waited < 0 && errno == EINTR);

        if (waited < 0) {
            return Err(io::Error::OSError());
        }

        return ExitStatus(status);
    }

    auto millis = this->DeathTimeout->count();
    struct timespec timeout{
        // clang-format off
        .tv_sec = static_cast<time_t>(millis / 1000),
        .tv_nsec = static_cast<Int64>((millis % 1000) % 1'000'000)
        // clang-format on
    };

    struct kevent fired{ };
    Int32 nev = 0;
    do {
        nev = ::kevent(queue, nullptr, 0, &fired, 1, &timeout);
    } while (nev < 0 && errno == EINTR);

    ::close(queue);
    if (nev < 0) {
        return Err(io::Error::OSError());
    }

    if (nev == 0) {
        return Err(VIOLET_IO_ERROR(TimedOut, String, "process death timed-out reached"));
    }

    // Process has exited by now, reap it
    Int32 status = 0;
    pid_t waited = -1;
    do {
        waited = ::waitpid(this->PID.Get(), &status, 0);
    } while (waited < 0 && errno == EINTR);

    if (waited < 0) {
        return Err(io::Error::OSError());
    }

    return ExitStatus(status);
}

auto Command::Spawn() -> io::Result<Child>
{
    auto requiresForkExec
        = this->n_impl->n_uid || this->n_impl->n_gid || !this->n_impl->n_extraGroupIDs.empty() || this->n_impl->n_exec;

    if (requiresForkExec) {
        return detail::SpawnAsForkExec(*this);
    }

    return detail::SpawnWithPosix(*this);
}

#endif
