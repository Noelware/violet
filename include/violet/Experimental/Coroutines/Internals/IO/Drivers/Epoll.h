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

#include <violet/Language/Macros.h>

#if VIOLET_PLATFORM(LINUX)

#include <violet/Experimental/Collections/HashMap.h>
#include <violet/Experimental/Coroutines/Internals/IO/Driver.h>
#include <violet/IO/Experimental/Descriptor.h>
#include <violet/IO/Experimental/Error.h>

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals::io {

struct VIOLET_API EpollDriver final: Driver {
    VIOLET_DISALLOW_COPY_AND_MOVE(EpollDriver);
    ~EpollDriver() override;

    static auto New(DriverOptions&) noexcept -> violet::io::experimental::Result<ptr::Unique<Driver>>;

    auto TryNow(Op& op) noexcept -> bool override;
    void Submit(Op& op) noexcept override;
    void Orphan(Op* op) noexcept override;
    void Park(Optional<chrono::Instant> deadline, const Clock& clock) noexcept override;
    void Unpark() noexcept override;

private:
    template<typename, typename>
    friend struct ptr::Unique;

    VIOLET_EXPLICIT EpollDriver(
        violet::io::experimental::Descriptor epollFd, violet::io::experimental::Descriptor eventfd) noexcept
        : n_epoll(VIOLET_MOVE(epollFd))
        , n_eventfd(VIOLET_MOVE(eventfd))
    {
    }

    struct entry final {
        Op* Head = nullptr;
        Op* Tail = nullptr;
        bool Registered = false;
    };

    static void link(entry& ent, Op& op) noexcept;
    static void unlink(entry& ent, Op& op) noexcept;

    auto arm(violet::io::experimental::RawFd fd, entry& ent) noexcept -> violet::io::experimental::Result<void>;
    void dispatch(violet::io::experimental::RawFd fd, UInt32 events) noexcept;
    void forget(violet::io::experimental::RawFd fd) noexcept;

    violet::io::experimental::Descriptor n_epoll;
    violet::io::experimental::Descriptor n_eventfd;
    HashMap<violet::io::experimental::RawFd, entry> n_fds;
};

} // namespace experimental::coro::internals::io
} // namespace NOELDOC_HIDE violet

#endif
