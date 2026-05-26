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

#include <violet/Experimental/Synchronization/WaitGroup.h>

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace std::chrono_literals;
using namespace violet::experimental::sync;
using namespace violet;

TEST(WaitGroup, WaitReturnsImmediatelyWhenCountIsZero)
{
    WaitGroup wg;
    wg.Wait(); // should not block
}

TEST(WaitGroup, SingleAddAndDone)
{
    WaitGroup wg;
    wg.Add(1);

    std::thread t([&wg] -> void { wg.Done(); });

    wg.Wait();
    t.join();
}

TEST(WaitGroup, MultipleAddsBeforeWait)
{
    constexpr int kThreads = 10;
    WaitGroup wg;
    wg.Add(kThreads);

    std::atomic<int> completed{ 0 };
    Vec<std::thread> threads;
    threads.reserve(kThreads);

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&] -> void {
            completed.fetch_add(1, std::memory_order_relaxed);
            wg.Done();
        });
    }

    wg.Wait();
    EXPECT_EQ(completed.load(std::memory_order_relaxed), kThreads);

    for (auto& t: threads) {
        t.join();
    }
}

TEST(WaitGroup, IncrementalAdds)
{
    WaitGroup wg;
    std::atomic<int> completed{ 0 };
    Vec<std::thread> threads;

    for (int i = 0; i < 5; ++i) {
        wg.Add(1);
        threads.emplace_back([&] -> void {
            completed.fetch_add(1, std::memory_order_relaxed);
            wg.Done();
        });
    }

    wg.Wait();
    EXPECT_EQ(completed.load(std::memory_order_relaxed), 5);

    for (auto& t: threads) {
        t.join();
    }
}

TEST(WaitGroup, AddWithDeltaGreaterThanOne)
{
    WaitGroup wg;
    wg.Add(3);

    std::atomic<int> completed{ 0 };

    std::thread t1([&] -> void {
        completed.fetch_add(1, std::memory_order_relaxed);
        wg.Done();
    });

    std::thread t2([&] -> void {
        completed.fetch_add(1, std::memory_order_relaxed);
        wg.Done();
    });

    std::thread t3([&] -> void {
        completed.fetch_add(1, std::memory_order_relaxed);
        wg.Done();
    });

    wg.Wait();
    EXPECT_EQ(completed.load(std::memory_order_relaxed), 3);

    t1.join();
    t2.join();
    t3.join();
}

TEST(WaitGroup, WaitBlocksUntilAllDone)
{
    WaitGroup wg;
    wg.Add(1);

    std::atomic<bool> work_done{ false };
    std::thread t([&] -> void {
        std::this_thread::sleep_for(50ms);
        work_done.store(true, std::memory_order_release);
        wg.Done();
    });

    wg.Wait();
    EXPECT_TRUE(work_done.load(std::memory_order_acquire));

    t.join();
}

TEST(WaitGroup, MultipleWaiters)
{
    WaitGroup wg;
    wg.Add(1);

    std::atomic<int> waiters_released{ 0 };

    std::thread w1([&] -> void {
        wg.Wait();
        waiters_released.fetch_add(1, std::memory_order_relaxed);
    });

    std::thread w2([&] -> void {
        wg.Wait();
        waiters_released.fetch_add(1, std::memory_order_relaxed);
    });

    std::thread w3([&] -> void {
        wg.Wait();
        waiters_released.fetch_add(1, std::memory_order_relaxed);
    });

    // give waiters time to block
    std::this_thread::sleep_for(20ms);

    wg.Done();

    w1.join();
    w2.join();
    w3.join();

    EXPECT_EQ(waiters_released.load(std::memory_order_relaxed), 3);
}

TEST(WaitGroup, ReuseAfterWait)
{
    WaitGroup wg;

    // first round
    wg.Add(1);
    std::thread t1([&] -> void { wg.Done(); });
    wg.Wait();

    t1.join();

    // second round on the same WaitGroup
    wg.Add(1);
    std::thread t2([&] -> void { wg.Done(); });
    wg.Wait();
    t2.join();
}

TEST(WaitGroup, DoneCalledFromManyThreadsSimultaneously)
{
    constexpr int kThreads = 100;
    WaitGroup wg;
    wg.Add(kThreads);

    std::atomic<bool> start{ false };
    Vec<std::thread> threads;
    threads.reserve(kThreads);

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&] -> void {
            while (!start.load(std::memory_order_acquire)) {
                // spin until all threads are ready
            }

            wg.Done();
        });
    }

    start.store(true, std::memory_order_release);
    wg.Wait();

    for (auto& t: threads) {
        t.join();
    }
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
