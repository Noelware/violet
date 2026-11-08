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
#include <violet/Experimental/Threading/Sleep.h>
#include <violet/Experimental/Threading/ThreadPool.h>

#include <atomic>
#include <latch>

namespace violet::experimental::threading {

TEST(ThreadPool, FixedRunsAllTasks)
{
    auto pool = ThreadPool<4>::New();
    ASSERT_TRUE(pool) << pool.Error();

    std::atomic<Int32> count = 0;
    for (Int32 i = 0; i < 100; i++) {
        auto submitted = pool->Submit([&]() -> void { count++; });
        ASSERT_TRUE(submitted) << submitted.Error();
    }

    pool->WaitForIdle();
    EXPECT_EQ(count.load(), 100);
}

TEST(ThreadPool, FixedShutdownDrainsQueue)
{
    auto pool = ThreadPool<1>::New();
    ASSERT_TRUE(pool) << pool.Error();

    std::atomic<Int32> count = 0;
    for (Int32 i = 0; i < 20; i++) {
        auto submitted = pool->Submit([&]() -> void { count++; });
        ASSERT_TRUE(submitted) << submitted.Error();
    }

    pool->Shutdown();
    EXPECT_EQ(count.load(), 20);
}

TEST(ThreadPool, FixedSubmitAfterShutdownFails)
{
    auto pool = ThreadPool<2>::New();
    ASSERT_TRUE(pool) << pool.Error();

    pool->Shutdown();
    EXPECT_FALSE(pool->Submit([]() -> void { }));
}

TEST(ThreadPool, FixedShutdownNowCancelsRunningTasks)
{
    auto pool = ThreadPool<1>::New();
    ASSERT_TRUE(pool) << pool.Error();

    std::latch started(1);
    std::atomic<bool> sawCancel = false;

    auto submitted = pool->Submit([&](const CancellationToken& token) -> void {
        started.count_down();
        while (!token.RequestsCancellation()) {
            SleepFor(chrono::Duration::Milliseconds(1));
        }

        sawCancel = true;
    });

    ASSERT_TRUE(submitted) << submitted.Error();

    started.wait();
    pool->ShutdownNow();
    EXPECT_TRUE(sawCancel.load());
}

TEST(ThreadPool, FixedShutdownNowDropsQueuedTasks)
{
    auto pool = ThreadPool<1>::New();
    ASSERT_TRUE(pool) << pool.Error();

    std::latch started(1);
    std::atomic<Int32> ran = 0;

    auto blocker = pool->Submit([&](const CancellationToken& token) -> void {
        started.count_down();
        while (!token.RequestsCancellation()) {
            SleepFor(chrono::Duration::Milliseconds(1));
        }
    });

    ASSERT_TRUE(blocker) << blocker.Error();

    started.wait();
    for (Int32 i = 0; i < 10; i++) {
        auto submitted = pool->Submit([&]() -> void { ran++; });
        ASSERT_TRUE(submitted) << submitted.Error();
    }

    pool->ShutdownNow();
    EXPECT_EQ(ran.load(), 0);
}

TEST(ThreadPool, DynamicStartsWithMinThreads)
{
    auto pool = ThreadPool<>::Builder().WithMinThreads(2).WithMaxThreads(4).Build();
    ASSERT_TRUE(pool) << pool.Error();

    EXPECT_EQ(pool->Threads(), 2U);
    EXPECT_EQ(pool->MaxThreads(), 4U);
}

TEST(ThreadPool, DynamicClampsMinToMax)
{
    auto pool = ThreadPool<>::Builder().WithMinThreads(8).WithMaxThreads(2).Build();
    ASSERT_TRUE(pool) << pool.Error();

    EXPECT_EQ(pool->Threads(), 2U);
}

TEST(ThreadPool, DynamicDefaultsMaxToAvailableParallelism)
{
    auto pool = ThreadPool<>::Builder().Build();
    ASSERT_TRUE(pool) << pool.Error();

    EXPECT_GE(pool->MaxThreads(), 1U);
}

TEST(ThreadPool, DynamicSpawnsUpToMax)
{
    auto pool = ThreadPool<>::Builder().WithMaxThreads(3).Build();
    ASSERT_TRUE(pool) << pool.Error();
    EXPECT_EQ(pool->Threads(), 0U);

    std::atomic<Int32> count = 0;
    for (Int32 i = 0; i < 50; i++) {
        auto submitted = pool->Submit([&]() -> void { count++; });
        ASSERT_TRUE(submitted) << submitted.Error();
    }

    pool->WaitForIdle();
    EXPECT_EQ(count.load(), 50);
    EXPECT_GE(pool->Threads(), 1U);
    EXPECT_LE(pool->Threads(), 3U);
}

TEST(ThreadPool, DynamicRetiresIdleWorkers)
{
    auto pool
        = ThreadPool<>::Builder().WithMaxThreads(2).WithKeepAlive(chrono::Duration::Milliseconds(20)).Build();
    ASSERT_TRUE(pool) << pool.Error();

    auto first = pool->Submit([]() -> void { });
    ASSERT_TRUE(first) << first.Error();
    pool->WaitForIdle();

    SleepFor(chrono::Duration::Milliseconds(200));
    EXPECT_EQ(pool->Threads(), 0U);

    // the pool keeps working after every worker has retired
    std::atomic<bool> ran = false;
    auto second = pool->Submit([&]() -> void { ran = true; });
    ASSERT_TRUE(second) << second.Error();

    pool->WaitForIdle();
    EXPECT_TRUE(ran.load());
}

TEST(ThreadPool, DynamicKeepsMinThreadsAlive)
{
    auto pool = ThreadPool<>::Builder()
                    .WithMinThreads(1)
                    .WithMaxThreads(3)
                    .WithKeepAlive(chrono::Duration::Milliseconds(20))
                    .Build();

    ASSERT_TRUE(pool) << pool.Error();

    std::latch release(1);
    for (Int32 i = 0; i < 3; i++) {
        auto submitted = pool->Submit([&]() -> void { release.wait(); });
        ASSERT_TRUE(submitted) << submitted.Error();
    }

    release.count_down();
    pool->WaitForIdle();

    SleepFor(chrono::Duration::Milliseconds(200));
    EXPECT_EQ(pool->Threads(), 1U);
}

TEST(ThreadPool, DynamicSubmitAfterShutdownFails)
{
    auto pool = ThreadPool<>::Builder().WithMaxThreads(2).Build();
    ASSERT_TRUE(pool) << pool.Error();

    pool->Shutdown();
    EXPECT_FALSE(pool->Submit([]() -> void { }));
}

TEST(ThreadPool, TasksCanSubmitMoreTasks)
{
    auto pool = ThreadPool<>::Builder().WithMaxThreads(2).Build();
    ASSERT_TRUE(pool) << pool.Error();

    std::atomic<Int32> count = 0;
    auto submitted = pool->Submit([&]() -> void {
        count++;
        (void)pool->Submit([&]() -> void { count++; });
    });

    ASSERT_TRUE(submitted) << submitted.Error();

    // the nested submit may land after the first `WaitForIdle` returns
    while (count.load() < 2) {
        pool->WaitForIdle();
    }

    EXPECT_EQ(count.load(), 2);
}

} // namespace violet::experimental::threading
