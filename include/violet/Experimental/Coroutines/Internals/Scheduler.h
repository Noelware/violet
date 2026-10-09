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

#include <violet/Experimental/Coroutines/Internals/RawTask.h>
#include <violet/Experimental/Coroutines/Internals/SpawnedTask.h>
#include <violet/Experimental/Coroutines/Internals/TaskList.h>
#include <violet/Experimental/Time/Clock.h>

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// Coroutines: Scheduling Model
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// An executor only ever deals with the root tasks (`RawTask`s produced by a `SpawnedTask<T>`). User
// tasks awaited inside a root are resumed through symmetric transfer, so when the innermost awaiter
// suspends, control unwinds all the way back to the executor's `Resume` call. From the executor's
// point of view, "the task suspended" therefore means "the whole chain is parked on something."
//
// To get rescheduled, whatever the chain is parked on (timers, I/O completion, a channel), needs a
// handle to the *root*. The executor publishes the root it is currently resuming through [`CurrentTask()`],
// and leaf awaiters capture it as a [`Waker`]. Waking pushes the root back onto its owner's run queue.
//
// ## Why do tasks need a state machine?
// Wakeups can come from any thread at any time, including:
//
// * twice in a row, before the task runs again. Without a guard, the task would be linked into
//   the run queue twice, corrupting the intrusive `Next` list.
// * while the task is running (the leaf registered a waker, and the I/O completed before `Resume` returned).
//   Pushing it immediately would let another worker resume a frame that is still executing.
//
// ## Reference Ownership
// * A task sitting in a run queue owns one reference, held by the queue.
// * The worker that pops it takes over that reference and either passes it back to the queue (re-scheduled),
//   or releases it (idle / complete).
// * Every `Waker` owns one reference, so the frame cannot be free'd while something can still wake it.

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals {
namespace timers {
struct Driver;
}

/// The current root task the calling thread is currently resuming, or `nullptr` outside of a task.
[[nodiscard]]
VIOLET_API auto CurrentTask() noexcept -> RawTask*;

/// Makes `task` runnable again.
///
/// ## Safety
/// The caller must hold a reference to `task` for the duration of the call (a `Waker` does).
VIOLET_API
void WakeupTask(Unsafe, RawTask* task) noexcept;

/// Resumes a task popped from a run queue.
///
/// The caller takes over the queue's reference. When this returns, that reference has either
/// been handed back to the owner (`task->Owner` (the task was woken while running)) or released.
VIOLET_API
void RunTask(Unsafe, RawTask* task) noexcept;

/// Completes `task` as cancelled and destroys its frame. The header lives until its last reference goes.
///
/// ## Safety
/// `task` must not be running. The caller must hold a reference.
VIOLET_API
void CancelTask(Unsafe, RawTask* task) noexcept;

/// Contextual understanding of a runtime handle without importing `Runtime.h`.
struct VIOLET_API DriveContext final {
    const Clock& TimeSource; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    timers::Driver* Timers = nullptr;
};

/// Something that accepts runnable root tasks.
struct VIOLET_API Scheduler {
    TaskList Tasks;

    virtual ~Scheduler() = default;

    /// Enqueues `task`, taking ownership of one reference.
    virtual void Push(RawTask* task) noexcept = 0;

    /// Blocks the calling thread until `root` completes, driving tasks and timers.
    virtual void BlockOn(RawTask* root, DriveContext cx) = 0;

    /// Wakes whichever thread is parked in `BlockOn` so it re-checks timers and queues.
    virtual void Unpark() noexcept = 0;

    /// Cancels all tasks that this scheduler owns.
    void CancelAll() noexcept
    {
        while (auto* task = this->Tasks.PopFront()) {
            CancelTask(Unsafe("the owned list's reference keeps `task` alive"), task);
            ReleaseRawTask(Unsafe("dropping the owned list's reference"), task);
        }
    }
};

} // namespace experimental::coro::internals
} // namespace NOELDOC_HIDE violet
