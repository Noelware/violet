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

using namespace std::chrono_literals;

namespace violet::experimental::coro {
namespace {

using chrono::Duration;

auto SleepsThenSets(bool* flag) -> Task<void>
{
    co_await Sleep(60'000ms);
    *flag = true;
}

auto SetsFlag(bool* flag) -> Task<void>
{
    *flag = true;
    co_return;
}

auto AddOne(Int32 value) -> Task<Int32>
{
    co_return value + 1;
}

auto AbortsSleeper(bool* flag) -> Task<bool>
{
    JoinHandle child = Handle::Current().Spawn(SleepsThenSets(flag));
    co_await Sleep(Duration::Milliseconds(5)); // let it park in the timer wheel

    child.Abort();
    co_return !(co_await VIOLET_MOVE(child).Cancellable()).HasValue();
}

auto AbortsBeforeFirstRun(bool* flag) -> Task<bool>
{
    JoinHandle child = Handle::Current().Spawn(SetsFlag(flag));
    child.Abort();

    co_return !(co_await VIOLET_MOVE(child).Cancellable()).HasValue();
}

auto AbortsAfterFinish() -> Task<Int32>
{
    JoinHandle child = Handle::Current().Spawn(AddOne(1));
    AbortHandle abort = child.GetAbortHandle();

    Int32 value = co_await VIOLET_MOVE(child);
    abort.Abort();

    co_return value;
}

auto MkRuntime()
{
    return Runtime::Builder::CurrentThread().EnableTimers().Build();
}

} // namespace

TEST(Abort, CancelsSleepingTask)
{
    bool finished = false;
    auto rt = MkRuntime();

    ASSERT_TRUE(rt->BlockOn(AbortsSleeper(&finished)));
    ASSERT_FALSE(finished);
}

TEST(Abort, CancelsQueuedTaskBeforeItRuns)
{
    bool ran = false;
    auto rt = MkRuntime();

    ASSERT_TRUE(rt->BlockOn(AbortsBeforeFirstRun(&ran)));
    ASSERT_FALSE(ran);
}

TEST(Abort, AbortingFinishedTaskIsNoop)
{
    auto rt = MkRuntime();
    ASSERT_EQ(rt->BlockOn(AbortsAfterFinish()), 2);
}

} // namespace violet::experimental::coro
