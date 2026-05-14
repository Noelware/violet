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
#include <violet/Experimental/Synchronized.h>

#include <thread>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet::experimental;
using violet::Int32;
using violet::Vec;

TEST(Synchronized, DefaultConstructs)
{
    Synchronized<Int32> sync;
    auto guard = sync.Lock();
    EXPECT_EQ(*guard, 0);
}

TEST(Synchronized, ConstructsWithValue)
{
    Synchronized<Int32> sync(42);
    auto guard = sync.Lock();
    EXPECT_EQ(*guard, 42);
}

TEST(Synchronized, ConstructsInPlace)
{
    Synchronized<Vec<Int32>> sync(std::in_place, 3, 99);
    auto guard = sync.Lock();
    EXPECT_EQ(guard->size(), 3);
    EXPECT_EQ((*guard)[0], 99);
}

TEST(Synchronized, GuardGrantsMutableAccess)
{
    Synchronized<Int32> sync(0);
    {
        auto guard = sync.Lock();
        *guard = 42;
    }

    auto guard = sync.Lock();
    EXPECT_EQ(*guard, 42);
}

TEST(Synchronized, ArrowOperator)
{
    Synchronized<Vec<Int32>> sync;

    auto guard = sync.Lock();
    guard->push_back(1);
    guard->push_back(2);
    EXPECT_EQ(guard->size(), 2);
}

TEST(Synchronized, ConstGuardGrantsMutableAccess)
{
    Synchronized<Int32> sync(10);

    const auto guard = sync.Lock();
    *guard = 20;
    EXPECT_EQ(*guard, 20);
}

TEST(Synchronized, TryLockSucceedsWhenUnlocked)
{
    Synchronized<Int32> sync(7);

    auto maybe_guard = sync.TryLock();
    ASSERT_TRUE(maybe_guard);
    EXPECT_EQ(**maybe_guard, 7);
}

TEST(Synchronized, TryLockFailsWhenLocked)
{
    Synchronized<Int32> sync(0);

    auto guard = sync.Lock();
    auto maybe_guard = sync.TryLock();
    EXPECT_FALSE(maybe_guard);
}

TEST(Synchronized, GuardMoveTransfersOwnership)
{
    Synchronized<Int32> sync(5);

    auto guard = sync.Lock();
    auto moved = VIOLET_MOVE(guard);
    EXPECT_EQ(*moved, 5);

    // Original guard should be nulled out; only `moved` releases the lock.
}

TEST(Synchronized, MoveConstruct)
{
    Synchronized<Int32> src(99);
    Synchronized<Int32> dst(VIOLET_MOVE(src));

    auto guard = dst.Lock();
    EXPECT_EQ(*guard, 99);
}

TEST(Synchronized, ConcurrentIncrements)
{
    Synchronized<Int32> sync(0);
    constexpr Int32 kThreads = 8;
    constexpr Int32 kIncrementsPerThread = 10000;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (Int32 i = 0; i < kThreads; ++i) {
        threads.emplace_back([&sync] -> void {
            for (Int32 j = 0; j < kIncrementsPerThread; ++j) {
                auto guard = sync.Lock();
                *guard += 1;
            }
        });
    }

    for (auto& t: threads) {
        t.join();
    }

    auto guard = sync.Lock();
    EXPECT_EQ(*guard, kThreads * kIncrementsPerThread);
}

TEST(Synchronized, ConcurrentTryLockContention)
{
    Synchronized<Int32> sync(0);
    std::atomic<bool> locked{ false };
    std::atomic<bool> done{ false };

    // Hold the lock for the duration of the test
    std::thread holder([&] -> void {
        auto guard = sync.Lock();
        locked.store(true, std::memory_order_release);
        while (!done.load(std::memory_order_acquire)) {
            // spin
        }
    });

    // Wait until the holder actually has the lock
    while (!locked.load(std::memory_order_acquire)) { }

    // TryLock must fail while the lock is held
    auto maybe_guard = sync.TryLock();
    EXPECT_FALSE(maybe_guard);

    done.store(true, std::memory_order_release);
    holder.join();

    // Confirm we can acquire after release
    auto guard = sync.Lock();
    EXPECT_EQ(*guard, 0);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
