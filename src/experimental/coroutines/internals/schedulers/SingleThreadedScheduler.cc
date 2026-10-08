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

#include <violet/Experimental/Coroutines/Internals/Schedulers/SingleThreadedScheduler.h>
#include <violet/Experimental/Coroutines/Internals/Timers/Driver.h>

namespace violet::experimental::coro::internals {

void SingleThreadedScheduler::Push(RawTask* task) noexcept
{
    VIOLET_DEBUG_ASSERT0(task != nullptr);

    if (this->isOwnerThread()) {
        this->pushLocal(task);
        return;
    }

    RawTask* head = this->n_remote.load(std::memory_order_relaxed);
    do {
        task->Next = head;
    } while (!this->n_remote.compare_exchange_weak(head, task, std::memory_order_release, std::memory_order_relaxed));

    if (head == nullptr) {
        this->n_parker.Unpark();
    }
}

void SingleThreadedScheduler::Park() noexcept
{
    this->n_parker.Park();
}

void SingleThreadedScheduler::Unpark() noexcept
{
    this->n_parker.Unpark();
}

auto SingleThreadedScheduler::RunUntilIdle() noexcept -> UInt
{
    VIOLET_DEBUG_ASSERT(this->isOwnerThread(), "`RunUntilIdle` was called off the owning thread");

    UInt resumed = 0;
    while (true) {
        this->drainRemote();

        RawTask* task = this->popLocal();
        if (task == nullptr) {
            return resumed;
        }

        RunTask(Unsafe("popped from our own queue"), task);
        resumed++;
    }
}

void SingleThreadedScheduler::BlockOn(RawTask* root, [[maybe_unused]] DriveContext cx)
{
    VIOLET_DEBUG_ASSERT(this->isOwnerThread(), "`BlockOn` was called off the owning thread");

    auto isFinished = [root] -> bool {
        auto state = root->State.load(std::memory_order_acquire);
        return (state & kTaskStateComplete) != 0;
    };

    while (true) {
        this->RunUntilIdle();
        if (isFinished()) {
            return;
        }

        if (cx.Timers == nullptr) {
            this->n_parker.Park();
            continue;
        }

        if (cx.Timers->FireExpired(cx.TimeSource.Now()) > 0) {
            continue;
        }

        if (auto next = cx.Timers->NextDeadline()) {
            this->n_parker.ParkUntil(*next);
        } else {
            this->n_parker.Park();
        }

        cx.Timers->FireExpired(cx.TimeSource.Now());
    }
}

void SingleThreadedScheduler::pushLocal(RawTask* task) noexcept
{
    VIOLET_ASSUME(task != nullptr);

    task->Next = nullptr;
    if (this->n_tail != nullptr) {
        this->n_tail->Next = task;
    } else {
        this->n_head = task;
    }

    this->n_tail = task;
}

auto SingleThreadedScheduler::popLocal() noexcept -> RawTask*
{
    RawTask* task = this->n_head;
    if (task == nullptr) {
        return nullptr;
    }

    this->n_head = task->Next;
    if (this->n_head == nullptr) {
        this->n_tail = nullptr;
    }

    task->Next = nullptr;
    return task;
}

void SingleThreadedScheduler::drainRemote() noexcept
{
    if (this->n_remote.load(std::memory_order_relaxed) == nullptr) {
        return;
    }

    RawTask* stack = this->n_remote.exchange(nullptr, std::memory_order_acquire);
    RawTask* reversed = nullptr;
    while (stack != nullptr) {
        RawTask* next = stack->Next;
        stack->Next = reversed;
        reversed = stack;
        stack = next;
    }

    while (reversed != nullptr) {
        RawTask* next = reversed->Next;
        this->pushLocal(reversed);

        reversed = next;
    }
}

} // namespace violet::experimental::coro::internals
