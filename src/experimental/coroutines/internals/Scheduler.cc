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

#include <violet/Experimental/Coroutines/Internals/Scheduler.h>
#include <violet/Experimental/Coroutines/Waker.h>

namespace violet::experimental::coro::internals {
namespace {
thread_local RawTask* tCurrentTask = nullptr;
}

auto CurrentTask() noexcept -> RawTask*
{
    return tCurrentTask;
}

void WakeupTask(Unsafe, RawTask* task) noexcept
{
    VIOLET_ASSUME(task != nullptr);

    UInt32 state = task->State.load(std::memory_order_relaxed);
    UInt32 next = 0;
    bool push = false;

    do {
        // task was already queued, flagged for re-queue, or finished
        if ((state & (kTaskStateScheduled | kTaskStateNotified | kTaskStateComplete)) != 0) {
            return;
        }

        push = (state & kTaskStateRunning) == 0;
        next = push ? (state | kTaskStateScheduled) : (state | kTaskStateNotified);
    } while (!task->State.compare_exchange_weak(state, next, std::memory_order_acq_rel, std::memory_order_relaxed));

    if (push) {
        RetainRawTask(Unsafe("the run queue takes one reference"), task);
        task->Owner->Push(task);
    }
}

void RunTask(Unsafe, RawTask* task) noexcept
{
    VIOLET_ASSUME(task != nullptr);

    UInt32 previous = task->State.fetch_xor(kTaskStateScheduled | kTaskStateRunning, std::memory_order_acq_rel);
    VIOLET_DEBUG_ASSERT((previous & kTaskStateLifecycleMask) == kTaskStateScheduled,
        "`RunTask` was called on a task that was not scheduled");

    std::coroutine_handle<> leaf = std::exchange(task->Leaf, nullptr);

    RawTask* outer = std::exchange(tCurrentTask, task);
    if (leaf) {
        leaf.resume();
    } else {
        task->VTable->Resume(task->Frame);
    }

    tCurrentTask = outer;

    if (task->VTable->Done(task->Frame)) {
        UInt32 finished = task->State.fetch_xor(kTaskStateRunning | kTaskStateComplete, std::memory_order_acq_rel);

        if ((finished & kTaskStateJoinWaiter) != 0) {
            RawTask* joiner = task->VTable->TakeJoinWaiter(task->Frame);
            WakeupTask(Unsafe("the join awaiter handed us its reference via the promise"), joiner);
            ReleaseRawTask(Unsafe("dropping the join awaiter's reference"), joiner);
        }

        ReleaseRawTask(Unsafe("dropping the run queue's reference"), task);
        return;
    }

    UInt32 state = task->State.load(std::memory_order_relaxed);
    while (true) {
        const bool requeue = (state & kTaskStateNotified) != 0;
        const UInt32 flags = state & ~kTaskStateLifecycleMask;
        const UInt32 next = requeue ? (flags | kTaskStateScheduled) : flags;

        if (task->State.compare_exchange_weak(state, next, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            if (requeue) {
                task->Owner->Push(task); // hand the run queue's reference straight back
            } else {
                ReleaseRawTask(Unsafe("idle: the run queue's reference is dropped"), task);
            }

            return;
        }
    }
}

} // namespace violet::experimental::coro::internals

void violet::experimental::coro::task_internal::RecordSuspendPoint(std::coroutine_handle<> leaf) noexcept
{
    if (internals::RawTask* task = internals::CurrentTask(); task != nullptr) {
        task->Leaf = leaf;
    }
}
