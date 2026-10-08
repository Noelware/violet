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
#include <violet/Experimental/Coroutines/Runtime.h>
#include <violet/Experimental/Coroutines/Sleep.h>

using namespace violet::experimental::chrono;

namespace violet::experimental::coro {
namespace {

auto SleepThenReturns(Duration dur) -> Task<Int32>
{
    co_await Sleep(dur);
    co_return 7;
}

auto SleepThenLog(Vec<Int32>* log, Int32 millis) -> Task<void>
{
    co_await Sleep(Duration::Milliseconds(millis));
    log->push_back(millis);
}

auto SleepsInOrder(Vec<Int32>* log) -> Task<void>
{
    // Spawned slowest-first, so FIFO order alone would get it wrong.
    JoinHandle slow = Handle::Current().Spawn(SleepThenLog(log, 30));
    JoinHandle mid = Handle::Current().Spawn(SleepThenLog(log, 20));
    JoinHandle fast = Handle::Current().Spawn(SleepThenLog(log, 10));

    co_await VIOLET_MOVE(slow);
    co_await VIOLET_MOVE(mid);
    co_await VIOLET_MOVE(fast);
}

auto SetFlagAfter(bool* flag, Int32 millis) -> Task<void>
{
    co_await Sleep(Duration::Milliseconds(millis));
    *flag = true;
}

auto DetachesSleeperThenOutlivesIt(bool* flag) -> Task<void>
{
    {
        [[maybe_unused]]
        JoinHandle detached = Handle::Current().Spawn(SetFlagAfter(flag, 10));
    }

    co_await Sleep(Duration::Milliseconds(30));
}

auto DetachesLongSleeper() -> Task<void>
{
    {
        [[maybe_unused]]
        JoinHandle detached = Handle::Current().Spawn(SetFlagAfter(nullptr, 60'000));
    }

    co_return;
}

} // namespace

TEST(Sleep, CompletesAndNeverEndsEarly)
{
    auto rt = Runtime::Builder::CurrentThread().EnableTimers().Build();
    Instant start = Clock::System().Now();

    ASSERT_EQ(rt->BlockOn(SleepThenReturns(Duration::Milliseconds(10))), 7);
    ASSERT_GE(start.Elapsed(Clock::System()).AsMillis(), 10);
}

TEST(Sleep, ZeroDurationCompletes)
{
    auto rt = Runtime::Builder::CurrentThread().EnableTimers().Build();
    ASSERT_EQ(rt->BlockOn(SleepThenReturns(Duration::Zero())), 7);
}

TEST(Sleep, WakesInDeadlineOrder)
{
    Vec<Int32> log;
    auto rt = Runtime::Builder::CurrentThread().EnableTimers().Build();
    rt->BlockOn(SleepsInOrder(&log));

    ASSERT_EQ(log, (Vec<Int32>{10, 20, 30}));
}

TEST(Sleep, DetachedSleeperStaysAlive)
{
    bool flag = false;
    auto rt = Runtime::Builder::CurrentThread().EnableTimers().Build();
    rt->BlockOn(DetachesSleeperThenOutlivesIt(&flag));

    ASSERT_TRUE(flag);
}

TEST(Sleep, RuntimeShutdownWithPendingSleeper)
{
    {
        auto rt = Runtime::Builder::CurrentThread().EnableTimers().Build();
        rt->BlockOn(DetachesLongSleeper());
    }

    SUCCEED();
}

} // namespace violet::experimental::coro
