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

#include <violet/Experimental/Buf.h>
#include <violet/Experimental/Coroutines/Internals/IO/Driver.h>
#include <violet/Experimental/Coroutines/Task.h>
#include <violet/Experimental/Coroutines/Waker.h>
#include <violet/IO/Experimental/Descriptor.h>

#if VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER)
struct io_uring_sqe;
#endif

namespace violet::experimental::coro {

template<typename T, typename B>
struct BufResult final {
    io::experimental::Result<T> Result;
    B Buffer;
};

namespace NOELDOC_HIDE internals {
namespace io {

struct Op;

void CompleteOperation(Unsafe, Op* op, Int64 result) noexcept;

struct VIOLET_API OpAwaitBase {
protected:
    VIOLET_DISALLOW_COPY(OpAwaitBase);

    VIOLET_EXPLICIT OpAwaitBase(Op* op) noexcept
        : n_op(op)
    {
    }

    OpAwaitBase(OpAwaitBase&& other) noexcept
        : n_op(std::exchange(other.n_op, nullptr))
        , n_driver(std::exchange(other.n_driver, nullptr))
    {
    }

    auto operator=(OpAwaitBase&&) -> OpAwaitBase& = delete;
    ~OpAwaitBase();

    /// Attempts the op inline (readiness drivers). `true` means no suspension is needed.
    auto Ready() noexcept -> bool;

    /// Registers the current task's waker and hands the op to the driver.
    void Suspend() noexcept;

    Op* n_op;
    Driver* n_driver = nullptr;
};

/// The readiness an op waits for on readiness-based drivers (epoll/kqueue).
enum struct Interest : UInt8 {
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    ReadWrite = Read | Write
};

/// Where an op is in its lifecycle.
enum struct OpStatus : UInt8 {
    Idle, ///< constructed, not submitted yet
    Inflight, ///< owned by the driver's queues (and possibly the kernel)
    Done ///< `Result` is set. The operation is unlinked from the driver
};

#if VIOLET_PLATFORM(UNIX)
/// The outcome of trying an op's syscall once on a readiness-based driver.
struct Attempt final {
    bool Done = false;
    Int64 Result = 0; ///< bytes transferred, or `-errno`

    [[nodiscard]]
    constexpr static auto WouldBlock() noexcept -> Attempt
    {
        return {};
    }

    [[nodiscard]] constexpr static auto Finished(Int64 result) noexcept -> Attempt
    {
        return {
            .Done = true,
            .Result = result,
        };
    }
};
#endif

/// A single I/O operation. Derive from it per syscall (`ReadOp`, `WriteOp`, `AcceptOp`, ...).
///
/// ## Remarks
/// * Always heap-allocated: the driver deletes orphaned ops through `Op*`, hence the virtual destructor.
/// * Fields are written by the driver and the awaiter only; treat them as internal.
struct Op {
    bool ClosesDescriptor = false;
    violet::io::experimental::RawFd Fd = violet::io::experimental::kInvalidRawFd;
    Interest Wanted = Interest::None;
    OpStatus Status = OpStatus::Idle;
    bool Orphaned = false;
    Int64 Result = 0;
    Waker Waiter;

    Op* Previous = nullptr;
    Op* Next = nullptr;

    VIOLET_DISALLOW_COPY_AND_MOVE(Op);
    VIOLET_IMPLICIT Op() noexcept = default;
    virtual ~Op() = default;

#if VIOLET_PLATFORM(UNIX)
    virtual auto TryOnce() noexcept -> Attempt = 0;
#endif

#if VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER)
    virtual void PrepareUring(io_uring_sqe& sqe) noexcept = 0;

    virtual auto NormalizeUringResult(Int64 result) noexcept -> Int64
    {
        return result;
    }
#endif

    virtual void OnOrphanComplete(Int64 result) noexcept
    {
        static_cast<void>(result);
    }
};

struct ReadOp final: Op {
    Bytes Buffer;
    Optional<UInt64> Offset;

    template<violet::io::experimental::AsFd FileDescriptor>
    VIOLET_IMPLICIT ReadOp(FileDescriptor fd, Bytes buf, Optional<UInt64> offset = Nothing) noexcept
        : Buffer(VIOLET_MOVE(buf))
        , Offset(VIOLET_MOVE(offset))
    {
        this->Fd = fd.Borrow();
        this->Wanted = Interest::Read;
    }

    auto TryOnce() noexcept -> Attempt override;

#if VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER)
    void PrepareUring(io_uring_sqe& sqe) noexcept override;
#endif

    auto Finish() -> BufResult<UInt, Bytes>;
};

struct WriteOp final: Op {
    Buf Data;
    Optional<UInt64> Offset;

    template<violet::io::experimental::AsFd FileDescriptor>
    VIOLET_IMPLICIT WriteOp(FileDescriptor fd, Buf buf, Optional<UInt64> offset = Nothing) noexcept
        : Data(VIOLET_MOVE(buf))
        , Offset(VIOLET_MOVE(offset))
    {
        this->Fd = fd.Borrow();
        this->Wanted = Interest::Write;
    }

    auto TryOnce() noexcept -> Attempt override;

#if VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER)
    void PrepareUring(io_uring_sqe& sqe) noexcept override;
#endif

    auto Finish() -> BufResult<UInt, Buf>;
};

struct RecvOp final: Op {
    Bytes Buffer;

    template<violet::io::experimental::AsFd FileDescriptor>
    VIOLET_IMPLICIT RecvOp(FileDescriptor fd, Bytes buf) noexcept
        : Buffer(VIOLET_MOVE(buf))
    {
        this->Fd = fd.Borrow();
        this->Wanted = Interest::Read;
    }

    auto TryOnce() noexcept -> Attempt override;

#if VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER)
    void PrepareUring(io_uring_sqe& sqe) noexcept override;
#endif
};

struct SendOp final: Op {
    Buf Data;

    template<violet::io::experimental::AsFd FileDescriptor>
    VIOLET_IMPLICIT SendOp(FileDescriptor fd, Buf data) noexcept
        : Data(VIOLET_MOVE(data))
    {
        this->Fd = fd.Borrow();
        this->Wanted = Interest::Write;
    }

    auto TryOnce() noexcept -> Attempt override;

#if VIOLET_FEATURE(COROUTINES_IO_URING_DRIVER)
    void PrepareUring(io_uring_sqe& sqe) noexcept override;
#endif
};

// TODO(@auguwu/Noel): Add `AcceptOp`, `ConnectOp`, `CloseOp`, `PollOp`, and `FsyncOp` in the future

VIOLET_API auto WriteAll(violet::io::experimental::RawFd fd, Buf data, bool socket) -> Task<BufResult<UInt, Buf>>;

template<std::derived_from<Op> O>
struct [[nodiscard("I/O operations are lazy; `co_await` them to poll them")]] NOELDOC_EXPERIMENTAL_SINCE("current")
    OpAwaiter final: internals::io::OpAwaitBase {
    VIOLET_DISALLOW_COPY(OpAwaiter);

    NOELDOC_HIDE VIOLET_EXPLICIT OpAwaiter(O* op)
        : internals::io::OpAwaitBase(op)
    {
    }

    VIOLET_IMPLICIT OpAwaiter(OpAwaiter&&) noexcept = default;
    auto operator=(OpAwaiter&&) -> OpAwaiter& = delete;
    ~OpAwaiter() = default;

    auto await_ready() noexcept -> bool
    {
        return this->Ready();
    }

    auto await_suspend(std::coroutine_handle<>) noexcept
    {
        this->Suspend();
    }

    auto await_resume() -> decltype(auto)
    {
        return static_cast<O*>(this->n_op)->Finish();
    }
};

} // namespace io
} // namespace NOELDOC_HIDE internals
} // namespace violet::experimental::coro
