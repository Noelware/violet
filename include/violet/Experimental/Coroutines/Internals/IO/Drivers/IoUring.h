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

#if VIOLET_PLATFORM(LINUX) && VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER)

#include <violet/Experimental/Coroutines/Internals/IO/Driver.h>
#include <violet/IO/Experimental/Descriptor.h>
#include <violet/IO/Experimental/Error.h>

struct io_uring;

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals::io {

struct VIOLET_API IoUringDriver final: Driver {
    VIOLET_DISALLOW_COPY_AND_MOVE(IoUringDriver);
    ~IoUringDriver() override;

    static auto New(DriverOptions& options) noexcept -> violet::io::experimental::Result<ptr::Unique<Driver>>;

    auto TryNow(Op& op) noexcept -> bool override;
    void Submit(Op& op) noexcept override;
    void Orphan(Op* op) noexcept override;
    void Park(Optional<chrono::Instant> deadline, const Clock& clock) noexcept override;
    void Unpark() noexcept override;

private:
    template<typename, typename>
    friend struct ptr::Unique;

    struct RingDeleter final {
        void operator()(io_uring* ring) const noexcept;
    };

    VIOLET_EXPLICIT IoUringDriver(
        ptr::Unique<io_uring, RingDeleter> ring, violet::io::experimental::Descriptor eventfd) noexcept
        : n_ring(VIOLET_MOVE(ring))
        , n_eventfd(VIOLET_MOVE(eventfd))
    {
        this->armWake();
    }

    auto acquireSqe() noexcept -> struct io_uring_sqe*;
    void armWake() noexcept;
    void drain() noexcept;
    void complete(void* data, Int32 res) noexcept;

    ptr::Unique<io_uring, RingDeleter> n_ring;
    violet::io::experimental::Descriptor n_eventfd;
    UInt64 n_wakeValue = 0;
    bool n_wakeArmed = false;
    bool n_closing = false;
    UInt n_inflight = 0;
};

} // namespace experimental::coro::internals::io
} // namespace NOELDOC_HIDE violet

#endif
