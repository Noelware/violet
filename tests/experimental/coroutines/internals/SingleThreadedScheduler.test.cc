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

#include <gtest/gtest.h>
#include <violet/Experimental/Coroutines/BasicAwaiter.h>
#include <violet/Experimental/Coroutines/Internals/Schedulers/SingleThreadedScheduler.h>
#include <violet/Experimental/Coroutines/Task.h>

namespace violet::experimental::coro::internals {
namespace {

auto YieldNow() noexcept -> decltype(auto)
{
    return MkBasicAwaiter([](std::coroutine_handle<>) noexcept -> void {
        WakeupTask(Unsafe("the worker we are resuming holds a reference"), CurrentTask());
    });
}

struct park_on final {
    RawTask** Slot;

    [[nodiscard]]
    static auto await_ready() noexcept -> bool
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<>) const noexcept
    {
        RawTask* root = CurrentTask();
        RetainRawTask(Unsafe("running inside of `root`"), root);
        *this->Slot = root;
    }

    static void await_resume() noexcept { }
};

struct DropFlag final {
    bool* Dropped;

    VIOLET_EXPLICIT DropFlag(bool& dropped) noexcept
        : Dropped(std::addressof(dropped))
    {
    }

    DropFlag(DropFlag&& other) noexcept
        : Dropped(std::exchange(other.Dropped, nullptr))
    {
    }

    ~DropFlag()
    {
        if (this->Dropped != nullptr) {
            *this->Dropped = true;
        }
    }
};

template<typename T>
void Spawn(SingleThreadedScheduler& scheduler, Task<T> task)
{
    RawTask* raw = MkSpawnedTask(VIOLET_MOVE(task)).IntoRaw();
    raw->Owner = std::addressof(scheduler);
    raw->State.store(kTaskStateScheduled, std::memory_order_relaxed);

    scheduler.Push(raw);
}

void WakeAndRelease(RawTask* task)
{
    WakeupTask(Unsafe("we hold the reference taken in `ParkOn`"), task);
    ReleaseRawTask(Unsafe("dropping the reference taken in `ParkOn`"), task);
}

auto Record(Vec<Int32>* log, Int32 id) -> Task<void>
{
    log->push_back(id);
    co_return;
}

auto YieldThenRecord(Vec<Int32>* log, Int32 id) -> Task<void>
{
    log->push_back(id);
    co_await YieldNow();
    log->push_back(id + 100);
}

auto ParkThenRecord(RawTask** slot, Vec<Int32>* log, Int32 id) -> Task<void>
{
    co_await park_on{slot};
    log->push_back(id);
}

auto HoldsFlag(DropFlag) -> Task<void>
{
    co_return;
}

} // namespace

TEST(SingleThreadedScheduler, EmptyQueueRunsNothing)
{
    SingleThreadedScheduler sched;
    ASSERT_EQ(sched.RunUntilIdle(), 0U);
}

TEST(SingleThreadedScheduler, RunsTasksInFifoOrder)
{
    SingleThreadedScheduler sched;
    Vec<Int32> log;

    Spawn(sched, Record(&log, 0));
    Spawn(sched, Record(&log, 1));
    Spawn(sched, Record(&log, 2));

    ASSERT_EQ(sched.RunUntilIdle(), 3U);
    ASSERT_EQ(log, (Vec<Int32>{0, 1, 2}));
}

TEST(SingleThreadedScheduler, YieldRequeuesBehindOtherTasks)
{
    SingleThreadedScheduler sched;
    Vec<Int32> log;

    Spawn(sched, YieldThenRecord(&log, 0));
    Spawn(sched, Record(&log, 1));

    ASSERT_EQ(sched.RunUntilIdle(), 3U);
    ASSERT_EQ(log, (Vec<Int32>{0, 1, 100}));
}

TEST(SingleThreadedScheduler, WakeFromOwnerThreadResumesTask)
{
    SingleThreadedScheduler sched;
    Vec<Int32> log;
    RawTask* parked = nullptr;

    Spawn(sched, ParkThenRecord(&parked, &log, 0));
    ASSERT_EQ(sched.RunUntilIdle(), 1U);
    ASSERT_NE(parked, nullptr);
    ASSERT_TRUE(log.empty());

    WakeAndRelease(parked);
    ASSERT_EQ(sched.RunUntilIdle(), 1U);
    ASSERT_EQ(log, (Vec<Int32>{0}));
}

TEST(SingleThreadedScheduler, RemoteWakesKeepArrivalOrder)
{
    SingleThreadedScheduler sched;
    Vec<Int32> log;
    RawTask* parked[3] = {};

    for (Int32 i = 0; i < 3; i++) {
        Spawn(sched, ParkThenRecord(&parked[i], &log, i));
    }

    ASSERT_EQ(sched.RunUntilIdle(), 3U);

    std::thread([&] -> void {
        for (RawTask* task: parked) {
            WakeAndRelease(task);
        }
    }).join();

    ASSERT_EQ(sched.RunUntilIdle(), 3U);
    ASSERT_EQ(log, (Vec<Int32>{0, 1, 2}));
}

TEST(SingleThreadedScheduler, ParkReturnsAfterRemoteWake)
{
    SingleThreadedScheduler sched;
    Vec<Int32> log;
    RawTask* parked = nullptr;

    Spawn(sched, ParkThenRecord(&parked, &log, 0));
    ASSERT_EQ(sched.RunUntilIdle(), 1U);

    std::thread waker([&] -> void {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        WakeAndRelease(parked);
    });

    sched.Park();
    waker.join();

    ASSERT_EQ(sched.RunUntilIdle(), 1U);
    ASSERT_EQ(log, (Vec<Int32>{0}));
}

TEST(SingleThreadedScheduler, DestructorReleasesQueuedTasks)
{
    bool dropped = false;
    {
        SingleThreadedScheduler sched;

        Spawn(sched, HoldsFlag(DropFlag{dropped}));
        ASSERT_FALSE(dropped);
    }

    ASSERT_TRUE(dropped);
}

} // namespace violet::experimental::coro::internals
