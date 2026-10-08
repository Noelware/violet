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

    auto state = task->State.load(std::memory_order_relaxed);
    auto next = kTaskStateIdle;

    do {
        // task was already queued, flagged for re-queue, or finished
        if ((state & (kTaskStateScheduled | kTaskStateNotified | kTaskStateComplete)) != 0) {
            return;
        }

        // If a worker is inside `Resumem`, it must not be pushed now (another worker could resume a
        // frame that is still executing). It'll be flagged instead.
        next = (state & kTaskStateRunning) != 0 ? (state | kTaskStateNotified) : kTaskStateScheduled;
    } while (!task->State.compare_exchange_weak(state, next, std::memory_order_acq_rel, std::memory_order_relaxed));

    if (next == kTaskStateScheduled) {
        RetainRawTask(Unsafe("task is safe to be used"), task);
        task->Owner->Push(task);
    }
}

void RunTask(Unsafe, RawTask* task) noexcept
{
    VIOLET_ASSUME(task != nullptr);

    UInt32 previous = task->State.exchange(kTaskStateRunning, std::memory_order_acq_rel);
    VIOLET_DEBUG_ASSERT(previous == kTaskStateScheduled, "`RunTask` was called on a task that was not scheduled");

    std::coroutine_handle<> leaf = std::exchange(task->Leaf, nullptr);

    RawTask* outer = std::exchange(tCurrentTask, task);
    if (leaf) {
        leaf.resume();
    } else {
        task->VTable->Resume(task->Frame);
    }

    tCurrentTask = outer;

    if (task->VTable->Done(task->Frame)) {
        task->State.store(kTaskStateComplete, std::memory_order_release);
        ReleaseRawTask(Unsafe("this is safe"), task);

        return;
    }

    UInt32 state = kTaskStateRunning;
    while (true) {
        UInt32 next = (state & kTaskStateNotified) != 0 ? kTaskStateScheduled : kTaskStateIdle;
        if (task->State.compare_exchange_weak(state, next, std::memory_order_acq_rel, std::memory_order_relaxed)) {
            if (next == kTaskStateScheduled) {
                task->Owner->Push(task);
            } else {
                ReleaseRawTask(Unsafe("things"), task);
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
