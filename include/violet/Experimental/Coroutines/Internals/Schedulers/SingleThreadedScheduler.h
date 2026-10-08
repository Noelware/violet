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

#include <violet/Experimental/Coroutines/Internals/Parker.h>
#include <violet/Experimental/Coroutines/Internals/Scheduler.h>

#include <atomic>
#include <thread>

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals {

/// A scheduler implementation that resumes every task on the single thread that owns it.
///
/// A task pushed from the owning thread goes onto a plain, intrusive FIFO (no atomic) queue. A task
/// pushed from any other thread, such as timers or I/O completions firing a `Waker`, goes onto a
/// lock-free inbox. The owner moves the inbox into the FIFO queue before resumes.
struct VIOLET_API SingleThreadedScheduler final: public Scheduler {
    VIOLET_DISALLOW_COPY_AND_MOVE(SingleThreadedScheduler);

    VIOLET_IMPLICIT SingleThreadedScheduler() noexcept
        : n_owner(std::this_thread::get_id())
    {
    }

    ~SingleThreadedScheduler() override
    {
        this->drainRemote();
        while (auto* task = this->popLocal()) {
            ReleaseRawTask(Unsafe("our run queue owns one reference"), task);
        }
    }

    NOELDOC_SEE(
        "violet::experimental::coro::internals::Scheduler::Push(violet::experimental::coro::internals::RawTask*)")
    void Push(RawTask* task) noexcept override;

    NOELDOC_SEE("violet::experimental::coro::internals::Scheduler::Push(violet::experimental::coro::internals::RawTask*"
                ", violet::experimental::coro::internals::DriveContext)")
    void BlockOn(RawTask* root, DriveContext cx) override;

    NOELDOC_SEE("violet::experimental::coro::internals::Scheduler::Unpark()")
    void Unpark() noexcept override;

    /// Resumes all queued task until the local queue and the inbox are empty. Returns the number
    /// of tasks resumed.
    auto RunUntilIdle() noexcept -> UInt;

    /// Parks the owning thread until `Unpark` or a remote `Push`. Exposed for tests.
    void Park() noexcept;

private:
    void pushLocal(RawTask* task) noexcept;
    void drainRemote() noexcept;

    [[nodiscard]]
    auto popLocal() noexcept -> RawTask*;

    [[nodiscard]]
    auto isOwnerThread() const noexcept -> bool
    {
        return std::this_thread::get_id() == this->n_owner;
    }

    std::thread::id n_owner;
    Parker n_parker;
    RawTask* n_head = nullptr;
    RawTask* n_tail = nullptr;
    std::atomic<RawTask*> n_remote{nullptr};
};

} // namespace experimental::coro::internals
} // namespace NOELDOC_HIDE violet
