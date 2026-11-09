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
#include <violet/Experimental/Synchronization/SpinLock.h>
#include <violet/Experimental/Threading/Parallelism.h>
#include <violet/Experimental/Threading/Sleep.h>
#include <violet/Experimental/Threading/Thread.h>

using namespace std::chrono_literals;

namespace violet::experimental::sync {

using threading::Thread;

namespace {

struct Counter final {
    SpinLock Lock;
    UInt Value VIOLET_GUARDED_BY(this->Lock) = 0;
};

auto getOversubscribedThreadConcurrency() noexcept -> UInt32
{
    const UInt32 hw = std::max(1UL, threading::AvailableParallelism());
    return hw * 4;
}

} // namespace

TEST(SpinLock, BasicUsage)
{
    SpinLock lock;
    EXPECT_FALSE(lock.Locked());

    lock.Lock();
    EXPECT_TRUE(lock.Locked());

    lock.Unlock();
    EXPECT_FALSE(lock.Locked());
}

TEST(SpinLock, TryLockFailsWhileHeld)
{
    SpinLock lock;
    ASSERT_TRUE(lock.TryLock());

    auto thread = Thread::Spawn([&] -> void { EXPECT_FALSE(lock.TryLock()); });

    ASSERT_TRUE(thread) << "failed to create thread: " << thread.Error();
    thread->Join();

#if VIOLET_COMPILER(CLANG)
    VIOLET_DIAGNOSTIC_PUSH
    VIOLET_DIAGNOSTIC_IGNORE("-Wthread-safety-analysis")
#endif

    lock.Unlock();
    EXPECT_TRUE(lock.TryLock());

    lock.Unlock();

#if VIOLET_COMPILER(CLANG)
    VIOLET_DIAGNOSTIC_POP
#endif
}

TEST(SpinLock, GuardReleasesOnScopeExit)
{
    SpinLock lock;
    {
        SpinLockGuard guard(lock);
        EXPECT_TRUE(lock.Locked());
    }

    EXPECT_FALSE(lock.Locked());
}

TEST(SpinLock, SleeperIsWoken)
{
    SpinLock lock;
    std::atomic<bool> acquired{false};

    lock.Lock();
    auto waiter = Thread::Spawn([&] -> void {
        SpinLockGuard guard(lock);
        acquired.store(true, std::memory_order_release);
    });

    threading::SleepFor(50ms);
    EXPECT_FALSE(acquired.load(std::memory_order_acquire));

    lock.Unlock();
    waiter->Join();

    EXPECT_TRUE(acquired.load(std::memory_order_acquire));
}

TEST(SpinLock, MutualExclusionUnderOversubscription)
{
    constexpr UInt kIterations = 20'000;
    const auto threads = getOversubscribedThreadConcurrency();

    Counter ctr;
    std::atomic<bool> go{false};
    Vec<Thread> workers;
    workers.reserve(threads);

    for (UInt32 i = 0; i < threads; i++) {
        auto thread = Thread::Spawn([&] -> void {
            while (!go.load(std::memory_order::acquire)) {
                threading::Yield();
            }

            for (UInt j = 0; j < kIterations; j++) {
                SpinLockGuard guard(ctr.Lock);
                ctr.Value++;
            }
        });

        ASSERT_TRUE(thread) << "failed to build thread #" << i << ": " << thread.Error();
        workers.push_back(VIOLET_MOVE(thread).Value());
    }

    go.store(true, std::memory_order_release);
    for (auto& worker: workers) {
        worker.Join();
    }

    SpinLockGuard guard(ctr.Lock);
    EXPECT_EQ(ctr.Value, static_cast<UInt>(threads) * kIterations);
    EXPECT_FALSE(ctr.Lock.TryLock());
}

TEST(SpinLock, TryLockNeverBreaksExclusion)
{
    constexpr UInt kIterations = 50'000;
    Counter ctr;
    std::atomic<UInt> inside{0};

    struct body final: threading::Runnable {
        VIOLET_DISALLOW_COPY_AND_MOVE(body);

        bool UseTryLock = false;
        struct Counter& Counter;
        std::atomic<UInt>& Inside;

        VIOLET_IMPLICIT body(bool useTryLock, struct Counter& ctr, std::atomic<UInt>& inside)
            : UseTryLock(useTryLock)
            , Counter(ctr)
            , Inside(inside)
        {
        }

    protected:
        void Execute([[maybe_unused]] const threading::CancellationToken& token) override
        {
            for (UInt i = 0; i < kIterations; i++) {
                if (this->UseTryLock) {
                    if (!this->Counter.Lock.TryLock()) {
                        continue;
                    }
                } else {
                    this->Counter.Lock.Lock();
                }

                EXPECT_EQ(this->Inside.fetch_add(1, std::memory_order_relaxed), 0U);
                this->Counter.Value++;
                this->Inside.fetch_sub(1, std::memory_order_relaxed);
                this->Counter.Lock.Unlock();
            }
        }
    };

    auto thread1 = Thread::Spawn<body>(false, ctr, inside);
    ASSERT_TRUE(thread1) << thread1.Error();

    auto thread2 = Thread::Spawn<body>(true, ctr, inside);
    ASSERT_TRUE(thread2) << thread2.Error();

    auto thread3 = Thread::Spawn<body>(false, ctr, inside);
    ASSERT_TRUE(thread3) << thread3.Error();

    auto thread4 = Thread::Spawn<body>(true, ctr, inside);
    ASSERT_TRUE(thread4) << thread4.Error();

    thread1->Join();
    thread2->Join();
    thread3->Join();
    thread4->Join();
}

} // namespace violet::experimental::sync
