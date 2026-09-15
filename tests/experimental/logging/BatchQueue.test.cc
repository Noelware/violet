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
#include <violet/Experimental/Logging/Internals/BatchQueue.h>
#include <violet/Experimental/Synchronized.h>

namespace violet::experimental::log::internals {

TEST(BatchQueue, ConsumesSinglePushedItem)
{
    Synchronized<Vec<Int32>> received;
    BatchQueue<Int32> queue([&](Vec<Int32> batch) -> void {
        received.With([batch = VIOLET_MOVE(batch)](auto& data) -> void {
            for (const auto& item: batch) {
                data.push_back(item);
            }
        });
    });

    queue.Push(42);
    queue.Flush();

    received.With([](const auto& batch) -> void {
        ASSERT_EQ(batch.size(), 1);
        EXPECT_EQ(batch[0], 42);
    });
}

TEST(BatchQueue, FlushReturnsImmediatelyWhenQueueIsEmpty)
{
    BatchQueue<Int32> queue([]([[maybe_unused]] Vec<int>) -> void { });
    queue.Flush(); // should not block
}

TEST(BatchQueue, PreservesPushOrder)
{
    Synchronized<Vec<Int32>> received;
    BatchQueue<Int32> queue([&](Vec<Int32> batch) -> void {
        received.With([batch = VIOLET_MOVE(batch)](auto& data) -> void {
            for (const auto& item: batch) {
                data.push_back(item);
            }
        });
    });

    constexpr Int32 kItems = 100;
    for (Int32 i = 0; i < kItems; ++i) {
        queue.Push(i);
    }

    queue.Flush();
    received.With([&](const auto& batch) -> void {
        ASSERT_EQ(batch.size(), kItems);
        for (Int32 i = 0; i < kItems; ++i) {
            EXPECT_EQ(batch[i], i);
        }
    });
}

TEST(BatchQueue, RespectsMaxBatchSize)
{
    std::atomic<UInt> maxObservedBatch{0};
    Synchronized<UInt> totalReceived(0);

    BatchQueue<Int32>::Options options{.MaxBatchSize = 2};
    BatchQueue<Int32> queue(
        [&](const Vec<Int32>& batch) -> void {
            auto size = batch.size();
            auto prev = maxObservedBatch.load(std::memory_order_relaxed);
            while (size > prev && !maxObservedBatch.compare_exchange_weak(prev, size)) { }

            totalReceived.With([batch](auto& received) -> void { received += batch.size(); });
        },
        VIOLET_MOVE(options));

    for (Int32 i = 0; i < 9; ++i) {
        queue.Push(i);
    }

    queue.Flush();

    EXPECT_LE(maxObservedBatch.load(std::memory_order_relaxed), 2u);
    EXPECT_EQ(*totalReceived.Lock(), 9u);
}

TEST(BatchQueue, NeverInvokesConsumerWithEmptyBatch)
{
    std::atomic<bool> sawEmptyBatch{false};
    BatchQueue<int> queue([&](const Vec<Int32>& batch) -> void {
        if (batch.empty()) {
            sawEmptyBatch.store(true, std::memory_order_relaxed);
        }
    });

    for (Int32 i = 0; i < 20; ++i) {
        queue.Push(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    queue.Flush();
    EXPECT_FALSE(sawEmptyBatch.load(std::memory_order_relaxed));
}

TEST(BatchQueue, DestructorDrainsPendingItems)
{
    Synchronized<Vec<Int32>> received;
    {
        BatchQueue<Int32> queue([&](const Vec<Int32>& batch) -> void {
            received.With([batch](auto& data) -> void {
                for (const auto& item: batch) {
                    data.push_back(item);
                }
            });
        });

        for (Int32 i = 0; i < 50; ++i) {
            queue.Push(i);
        }
    }

    auto guard = received.Lock();
    EXPECT_EQ(guard->size(), 50);
}

TEST(BatchQueue, ConcurrentPushersAllItemsConsumed)
{
    constexpr int kThreads = 8;
    constexpr int kItemsPerThread = 500;

    std::atomic<Int32> totalConsumed{0};
    BatchQueue<Int32> queue([&](const Vec<Int32>& batch) -> void {
        totalConsumed.fetch_add(static_cast<Int32>(batch.size()), std::memory_order_relaxed);
    });

    Vec<std::thread> threads;
    threads.reserve(kThreads);

    for (Int32 t = 0; t < kThreads; ++t) {
        threads.emplace_back([&] -> void {
            for (Int32 i = 0; i < kItemsPerThread; ++i) {
                queue.Push(i);
            }
        });
    }

    for (auto& t: threads) {
        t.join();
    }

    queue.Flush();
    EXPECT_EQ(totalConsumed.load(std::memory_order_relaxed), kThreads * kItemsPerThread);
}

TEST(BatchQueue, FlushWaitsForInFlightConsumerCall)
{
    std::atomic<bool> consumerFinished{false};
    BatchQueue<Int32> queue([&](const auto&) -> void {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        consumerFinished.store(true, std::memory_order_release);
    });

    queue.Push(1);
    queue.Flush();

    EXPECT_TRUE(consumerFinished.load(std::memory_order_acquire));
}

TEST(BatchQueue, MultipleFlushCyclesOnSameQueue)
{
    Synchronized<Vec<Int32>> received;
    BatchQueue<Int32> queue([&](const Vec<Int32>& batch) -> void {
        received.With([batch](auto& data) -> void {
            for (const auto& item: batch) {
                data.push_back(item);
            }
        });
    });

    queue.Push(1);
    queue.Flush();

    queue.Push(2);
    queue.Flush();

    auto guard = received.Lock();
    ASSERT_EQ(guard->size(), 2);
    EXPECT_EQ((*guard)[0], 1);
    EXPECT_EQ((*guard)[1], 2);
}

} // namespace violet::experimental::log::internals
