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
#include <violet/Experimental/Synchronization/ReadWriteLock.h>

#include <atomic>
#include <chrono>
#include <latch>
#include <string>
#include <thread>
#include <vector>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet::experimental::sync;
using namespace violet;
using namespace std::chrono_literals;

TEST(ReadWriteLock, DefaultConstructs)
{
    ReadWriteLock<Int32> lock;
    auto guard = lock.Read();
    EXPECT_EQ(*guard, 0);
}

TEST(ReadWriteLock, ConstructsFromValue)
{
    ReadWriteLock<String> lock("hello");
    auto guard = lock.Read();
    EXPECT_EQ(*guard, "hello");
}

TEST(ReadWriteLock, ConstructsFromCopy)
{
    String value = "copied";
    ReadWriteLock<String> lock(value);
    auto guard = lock.Read();
    EXPECT_EQ(*guard, "copied");
    EXPECT_EQ(value, "copied");
}

TEST(ReadWriteLock, ConstructsFromMove)
{
    String value = "moved";
    ReadWriteLock<String> lock(VIOLET_MOVE(value));
    auto guard = lock.Read();
    EXPECT_EQ(*guard, "moved");
}

TEST(ReadWriteLock, ConstructsInPlace)
{
    ReadWriteLock<String> lock(5, 'x');
    auto guard = lock.Read();
    EXPECT_EQ(*guard, "xxxxx");
}

TEST(ReadWriteLock, ReadReturnsConstRef)
{
    ReadWriteLock<Int32> lock(42);
    auto guard = lock.Read();
    EXPECT_EQ(*guard, 42);
}

TEST(ReadWriteLock, WriteAllowsMutation)
{
    ReadWriteLock<Int32> lock(0);
    {
        auto guard = lock.Write();
        *guard = 42;
    }

    {
        auto guard = lock.Read();
        EXPECT_EQ(*guard, 42);
    }
}

TEST(ReadWriteLock, ArrowOperator)
{
    ReadWriteLock<String> lock("hello");
    {
        auto guard = lock.Read();
        EXPECT_EQ(guard->size(), 5);
    }

    {
        auto guard = lock.Write();
        guard->append(" world");
    }

    {
        auto guard = lock.Read();
        EXPECT_EQ(*guard, "hello world");
    }
}

TEST(ReadWriteLock, GuardReleasesOnDestruction)
{
    ReadWriteLock<Int32> lock(0);
    {
        auto w = lock.Write();
        *w = 1;
    }

    {
        auto w = lock.Write();
        *w = 2;
    }

    auto r = lock.Read();
    EXPECT_EQ(*r, 2);
}

TEST(ReadWriteLock, MultipleReadersSimultaneous)
{
    ReadWriteLock<Int32> lock(99);
    constexpr Int32 kReaders = 8;

    std::latch started(kReaders);
    std::latch done(kReaders);
    std::atomic<Int32> activeReaders = 0;
    std::atomic<Int32> maxConcurrent = 0;

    Vec<std::thread> threads;
    threads.reserve(kReaders);
    for (Int32 i = 0; i < kReaders; i++) {
        threads.emplace_back([&] -> void {
            auto guard = lock.Read();
            EXPECT_EQ(*guard, 99);

            auto current = activeReaders.fetch_add(1, std::memory_order_relaxed) + 1;

            // Track high-water mark of concurrent readers.
            auto prev = maxConcurrent.load(std::memory_order_relaxed);
            while (prev < current && !maxConcurrent.compare_exchange_weak(prev, current, std::memory_order_relaxed)) { }

            started.count_down();
            started.wait(); // all readers holding the lock simultaneously

            activeReaders.fetch_sub(1, std::memory_order_relaxed);
            done.count_down();
        });
    }

    for (auto& t: threads) {
        t.join();
    }

    // At least 2 readers should have overlapped.
    EXPECT_GE(maxConcurrent.load(), 2);
}

TEST(ReadWriteLock, WriterExcludesReaders)
{
    ReadWriteLock<Int32> lock(0);
    std::latch writeHolding(1);
    std::atomic<bool> reader_entered = false;

    std::thread writer([&] -> void {
        auto guard = lock.Write();
        writeHolding.count_down();
        std::this_thread::sleep_for(50ms);

        // Reader should not have entered while we hold the write lock.
        EXPECT_FALSE(reader_entered.load());
        *guard = 1;
    });

    std::thread reader([&] -> void {
        // ensure writer is holding
        writeHolding.wait();

        // small delay to be sure
        std::this_thread::sleep_for(5ms);
        auto guard = lock.Read();
        reader_entered.store(true);

        // writer must have finished
        EXPECT_EQ(*guard, 1);
    });

    writer.join();
    reader.join();
}

TEST(ReadWriteLock, WriterExcludesWriters)
{
    ReadWriteLock<Int32> lock(0);
    std::latch firstHolding(1);
    std::atomic<bool> second_entered = false;

    std::thread first([&] -> void {
        auto guard = lock.Write();
        firstHolding.count_down();
        std::this_thread::sleep_for(50ms);

        EXPECT_FALSE(second_entered.load());
        *guard = 1;
    });

    std::thread second([&] -> void {
        firstHolding.wait();
        std::this_thread::sleep_for(5ms);

        auto guard = lock.Write();
        second_entered.store(true);

        EXPECT_EQ(*guard, 1);
        *guard = 2;
    });

    first.join();
    second.join();

    auto r = lock.Read();
    EXPECT_EQ(*r, 2);
}

TEST(ReadWriteLock, TryReadSucceedsWhenUnlocked)
{
    ReadWriteLock<Int32> lock(42);
    auto guard = lock.TryRead();
    ASSERT_TRUE(guard);
    EXPECT_EQ(**guard, 42);
}

TEST(ReadWriteLock, TryReadSucceedsWhileOtherReadersActive)
{
    ReadWriteLock<Int32> lock(42);
    auto r1 = lock.Read();
    auto r2 = lock.TryRead();
    ASSERT_TRUE(r2);
    EXPECT_EQ(**r2, 42);
}

TEST(ReadWriteLock, TryReadFailsWhileWriterActive)
{
    ReadWriteLock<Int32> lock(0);
    std::latch writerHolding(1);
    std::latch readerDone(1);

    std::thread writer([&] -> void {
        auto guard = lock.Write();
        writerHolding.count_down();
        readerDone.wait();
    });

    writerHolding.wait();

    auto result = lock.TryRead();
    EXPECT_FALSE(result);

    readerDone.count_down();
    writer.join();
}

TEST(ReadWriteLock, TryWriteSucceedsWhenUnlocked)
{
    ReadWriteLock<Int32> lock(0);
    auto guard = lock.TryWrite();
    ASSERT_TRUE(guard);
    **guard = 42;
}

TEST(ReadWriteLock, TryWriteFailsWhileReaderActive)
{
    ReadWriteLock<Int32> lock(0);
    auto reader = lock.Read();
    auto writer = lock.TryWrite();
    EXPECT_FALSE(writer);
}

TEST(ReadWriteLock, TryWriteFailsWhileWriterActive)
{
    ReadWriteLock<Int32> lock(0);
    std::latch holding(1);
    std::latch done(1);

    std::thread first([&] -> void {
        auto guard = lock.Write();
        holding.count_down();
        done.wait();
    });

    holding.wait();

    auto result = lock.TryWrite();
    EXPECT_FALSE(result);

    done.count_down();
    first.join();
}

TEST(ReadWriteLock, ReadWhenBlocksUntilPredicateMet)
{
    ReadWriteLock<Vec<Int32>> lock;
    std::thread producer([&] -> void {
        std::this_thread::sleep_for(20ms);
        auto guard = lock.Write();
        guard->push_back(42);
    });

    auto guard = lock.ReadWhen([](const Vec<Int32>& v) { return !v.empty(); });
    EXPECT_EQ(guard->size(), 1);
    EXPECT_EQ(guard->front(), 42);

    producer.join();
}

TEST(ReadWriteLock, WriteWhenBlocksUntilPredicateMet)
{
    ReadWriteLock<Int32> lock(0);
    std::thread setter([&] -> void {
        std::this_thread::sleep_for(20ms);
        auto guard = lock.Write();
        *guard = 10;
    });

    {
        // Wait until value is non-zero before acquiring write access.
        auto guard = lock.WriteWhen([](const int& v) -> bool { return v != 0; });
        EXPECT_EQ(*guard, 10);
        *guard = 20;
    }

    setter.join();

    // Verify final value.
    auto r = lock.Read();
}

TEST(ReadWriteLock, WriteWhenSeesUpdatedValue)
{
    ReadWriteLock<Int32> lock(0);
    std::thread setter([&] -> void {
        std::this_thread::sleep_for(20ms);
        auto guard = lock.Write();
        *guard = 5;
    });

    auto guard = lock.WriteWhen([](const int& v) -> bool { return v == 5; });
    EXPECT_EQ(*guard, 5);
    *guard = 100;

    setter.join();
}

TEST(ReadWriteLock, TryReadUntilSucceedsWhenAvailable)
{
    ReadWriteLock<Int32> lock(42);
    auto guard = lock.TryReadUntil(100ms);
    ASSERT_TRUE(guard);
    EXPECT_EQ(**guard, 42);
}

TEST(ReadWriteLock, TryReadUntilTimesOutWhileWriterHolds)
{
    ReadWriteLock<Int32> lock(0);

    std::latch holding(1);
    std::latch done(1);
    std::thread writer([&] -> void {
        auto guard = lock.Write();
        holding.count_down();
        done.wait();
    });

    holding.wait();
    auto start = std::chrono::steady_clock::now();
    auto result = lock.TryReadUntil(30ms);
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result);
    EXPECT_GE(elapsed, 25ms); // should have waited close to the timeout

    done.count_down();
    writer.join();
}

TEST(ReadWriteLock, TryReadUntilSucceedsAfterWriterReleases)
{
    ReadWriteLock<Int32> lock(0);
    std::thread writer([&] -> void {
        auto guard = lock.Write();
        *guard = 7;
        std::this_thread::sleep_for(20ms);
    });

    // Give writer time to acquire.
    std::this_thread::sleep_for(5ms);

    auto result = lock.TryReadUntil(200ms);
    ASSERT_TRUE(result);
    EXPECT_EQ(**result, 7);

    writer.join();
}

TEST(ReadWriteLock, TryWriteUntilSucceedsWhenAvailable)
{
    ReadWriteLock<Int32> lock(0);
    auto guard = lock.TryWriteUntil(100ms);
    ASSERT_TRUE(guard);
    **guard = 42;
}

TEST(ReadWriteLock, TryWriteUntilTimesOutWhileReadersHold)
{
    ReadWriteLock<Int32> lock(0);
    auto reader = lock.Read();
    auto start = std::chrono::steady_clock::now();
    auto result = lock.TryWriteUntil(30ms);
    auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result);
    EXPECT_GE(elapsed, 25ms);
}

TEST(ReadWriteLock, TryWriteUntilSucceedsAfterReadersRelease)
{
    ReadWriteLock<Int32> lock(0);
    std::thread reader([&] -> void {
        auto guard = lock.Read();
        std::this_thread::sleep_for(20ms);
    });

    std::this_thread::sleep_for(5ms);

    auto result = lock.TryWriteUntil(200ms);
    ASSERT_TRUE(result);
    **result = 99;

    reader.join();
}

TEST(ReadWriteLock, ConcurrentReadersAndWriters)
{
    ReadWriteLock<Int32> lock(0);
    constexpr Int32 kReaders = 6;
    constexpr Int32 kWriters = 3;
    constexpr Int32 kIterations = 500;

    std::atomic<Int32> readCount = 0;
    std::atomic<Int32> writeCount = 0;

    Vec<std::thread> threads;
    threads.reserve(kReaders + kWriters);

    for (Int32 i = 0; i < kReaders; i++) {
        threads.emplace_back([&] -> void {
            for (Int32 j = 0; j < kIterations; j++) {
                auto guard = lock.Read();

                // Value should never be negative (no partial writes).
                EXPECT_GE(*guard, 0);
                readCount.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (Int32 i = 0; i < kWriters; i++) {
        threads.emplace_back([&] -> void {
            for (Int32 j = 0; j < kIterations; j++) {
                auto guard = lock.Write();
                (*guard)++;

                writeCount.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t: threads) {
        t.join();
    }

    auto final_val = *lock.Read();
    EXPECT_EQ(final_val, kWriters * kIterations);
    EXPECT_EQ(readCount.load(), kReaders * kIterations);
    EXPECT_EQ(writeCount.load(), kWriters * kIterations);
}

TEST(ReadWriteLock, StressTryReadAndTryWrite)
{
    ReadWriteLock<Int32> lock(0);
    constexpr Int32 kThreads = 8;
    constexpr Int32 kIterations = 1000;

    std::atomic<Int32> successful_writes = 0;
    Vec<std::thread> threads;
    threads.reserve(kThreads);

    for (Int32 i = 0; i < kThreads; i++) {
        threads.emplace_back([&] -> void {
            for (Int32 j = 0; j < kIterations; j++) {
                if (j % 3 == 0) {
                    if (auto w = lock.TryWrite()) {
                        (**w)++;
                        successful_writes.fetch_add(1, std::memory_order_relaxed);
                    }
                } else {
                    if (auto r = lock.TryRead()) {
                        EXPECT_GE(**r, 0);
                    }
                }
            }
        });
    }

    for (auto& t: threads) {
        t.join();
    }

    auto final_val = *lock.Read();
    EXPECT_EQ(final_val, successful_writes.load());
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
