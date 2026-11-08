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
#include <violet/Experimental/Collections/HashSet.h>
#include <violet/Experimental/Mutex.h>
#include <violet/Experimental/Slice.h>
#include <violet/Experimental/Threading/Thread.h>

#include <latch>

#if VIOLET_FEATURE(PTHREADS)
#include <pthread.h>
#endif

namespace violet::experimental::threading {
namespace {

auto getOSThreadName() -> String
{
#if VIOLET_FEATURE(PTHREADS)
    Slice<char, 64> buf;
    ::pthread_getname_np(::pthread_self(), buf.Data(), buf.Size());

    return {buf.Data()};
#endif
}

struct CountingTask final: Runnable {
    VIOLET_DISALLOW_COPY_AND_MOVE(CountingTask);

    VIOLET_EXPLICIT CountingTask(std::atomic<Int32>& counter, Int32 by) noexcept
        : Counter(counter)
        , By(by)
    {
    }

protected:
    void Execute(const CancellationToken&) override
    {
        this->Counter.fetch_add(this->By);
    }

private:
    std::atomic<Int32>& Counter;
    Int32 By;
};

} // namespace

TEST(ThreadId, CurrentIsStable)
{
    EXPECT_EQ(ThreadId::Current(), ThreadId::Current());
}

TEST(ThreadId, MainIsCurrentOnMainThread)
{
    EXPECT_EQ(ThreadId::Main(), ThreadId::Current());
}

TEST(ThreadId, HashIsConsistent)
{
    const auto a = ThreadId::Current();
    const auto b = ThreadId::Current();

    EXPECT_EQ(std::hash<ThreadId>{}(a), std::hash<ThreadId>{}(b));
}

TEST(ThreadId, DiffersAcrossLiveThreads)
{
    const auto main = ThreadId::Current();
    std::atomic<bool> differs = false;

    auto thread = Thread::Builder().Spawn([&] -> void { differs = ThreadId::Current() != main; });
    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    thread->Join();
    EXPECT_TRUE(differs);
}

TEST(ThreadId, UniqueAmongConcurrentThreads)
{
    constexpr Int32 kThreads = 16;

    std::latch allRunning(kThreads + 1);
    Mutex mutex;
    HashSet<ThreadId> ids;
    Vec<Thread> threads;

    for (Int32 i = 0; i < kThreads; i++) {
        auto thread = Thread::Builder().Spawn([&] -> void {
            {
                MutexLock mux(mutex);
                (void)ids.Insert(ThreadId::Current());
            }

            allRunning.arrive_and_wait();
        });

        EXPECT_TRUE(thread) << "thread #" << i << " failed to create: " << thread.Error();

        threads.push_back(VIOLET_MOVE(thread).Value());
    }

    allRunning.arrive_and_wait();
    for (auto& thread: threads) {
        thread.Join();
    }

    EXPECT_EQ(ids.Size(), static_cast<UInt>(kThreads));
}

TEST(Thread, MainHandle)
{
    auto main = Thread::Main();

    EXPECT_EQ(main.Id(), ThreadId::Main());
    EXPECT_EQ(main.Name(), "main");
    EXPECT_FALSE(main.Joinable());
}

TEST(Thread, CurrentOnMainThreadIsMain)
{
    auto current = Thread::Current();

    EXPECT_EQ(current.Id(), ThreadId::Main());
    EXPECT_EQ(current.Name(), "main");
    EXPECT_FALSE(current.Joinable());
}

TEST(Thread, CurrentTemporariesDoNotJoin)
{
    for (Int32 i = 0; i < 100; i++) {
        EXPECT_EQ(Thread::Current().Id(), ThreadId::Current());
    }
}

TEST(Thread, MainIsVisibleFromOtherThreads)
{
    const auto expected = ThreadId::Current();
    std::atomic<bool> matches = false;

    auto thread = Thread::Builder().Spawn(
        [&] -> void { matches = Thread::Main().Id() == expected && ThreadId::Main() == expected; });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    thread->Join();
    EXPECT_TRUE(matches);
}

TEST(Thread, SpawnLambdaRuns)
{
    std::atomic<bool> ran = false;
    auto thread = Thread::Builder().Spawn([&] -> void { ran = true; });
    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    EXPECT_TRUE(thread->Joinable());
    thread->Join();

    EXPECT_FALSE(thread->Joinable());
    EXPECT_TRUE(ran);
}

TEST(Thread, IdMatchesInsideAndOutside)
{
    Mutex mux;
    Optional<ThreadId> inside;

    auto thread = Thread::Builder().Spawn([&] -> void {
        MutexLock lock(mux);
        inside = Thread::Current().Id();
    });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    const auto outside = thread->Id();
    thread->Join();

    ASSERT_TRUE(inside);
    EXPECT_EQ(inside, Some(outside));
    EXPECT_EQ(thread->Id(), outside);
}

TEST(Thread, NameIsVisibleFromBothSides)
{
    String inside;
    auto thread
        = Thread::Builder().WithName("worker").Spawn([&] -> void { inside = String(Thread::Current().Name()); });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();
    EXPECT_EQ(thread->Name(), "worker");
    thread->Join();

    EXPECT_EQ(inside, "worker");
}

TEST(Thread, UnnamedThreadHasEmptyName)
{
    auto thread = Thread::Builder().Spawn([] -> void { });
    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    EXPECT_TRUE(thread->Name().empty());
    thread->Join();
}

TEST(Thread, NameIsPropagatedToOs)
{
    String os;
    auto thread = Thread::Builder().WithName("violet-test").Spawn([&] -> void { os = getOSThreadName(); });
    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    thread->Join();
    EXPECT_EQ(os, "violet-test");
}

TEST(Thread, LongNameIsTruncatedForOsButNotForViolet)
{
    constexpr Str kName = "a-very-long-thread-name-that-exceeds-the-limit";

    String os;
    String inside;
    auto thread = Thread::Builder().WithName(kName).Spawn([&] -> void {
        os = getOSThreadName();
        inside = String(Thread::Current().Name());
    });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();
    thread->Join();

    EXPECT_EQ(inside, kName);
#if VIOLET_PLATFORM(LINUX)
    EXPECT_EQ(os, kName.substr(0, 15));
#elif VIOLET_PLATFORM(APPLE_MACOS)
    EXPECT_TRUE(kName.starts_with(os));
#endif
}

TEST(Thread, TinyStackSizeIsNormalized)
{
    // 1 byte is below PTHREAD_STACK_MIN; `pthread_attr_setstacksize` would EINVAL without normalization
    std::atomic<bool> ran = false;
    auto thread = Thread::Builder().WithStackSize(1).Spawn([&] -> void { ran = true; });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();
    thread->Join();

    EXPECT_TRUE(ran);
}

#if VIOLET_PLATFORM(LINUX)
TEST(Thread, StackSizeIsApplied)
{
    constexpr UInt kSize = 4 * 1024 * 1024;

    UInt actual = 0;
    auto thread = Thread::Builder().WithStackSize(kSize).Spawn([&] -> void {
        pthread_attr_t attrs;
        ASSERT_EQ(::pthread_getattr_np(::pthread_self(), &attrs), 0);

        ::pthread_attr_getstacksize(&attrs, &actual);
        ::pthread_attr_destroy(&attrs);
    });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();
    thread->Join();

    EXPECT_GE(actual, kSize);
}
#endif

TEST(Thread, CancelStopsCooperativeLoop)
{
    std::atomic<bool> started{false};
    auto thread = Thread::Builder().Spawn([&](const CancellationToken& token) -> void {
        started = true;
        started.notify_one();

        token.WaitForCancellation();
    });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    started.wait(false);
    thread->CancelAndJoin();

    EXPECT_FALSE(thread->Joinable());
}

TEST(Thread, DestructorCancelsAndJoins)
{
    std::atomic<bool> exited{false};
    {
        auto thread = Thread::Builder().Spawn([&](const auto& token) -> void {
            token.WaitForCancellation();
            exited = true;
        });

        EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();
    }

    EXPECT_TRUE(exited);
}

TEST(Thread, MoveConstructorTransfersOwnership)
{
    auto a = Thread::Builder().Spawn([](const auto& token) -> void { token.WaitForCancellation(); });
    EXPECT_TRUE(a) << "thread failed to create: " << a.Error();

    auto id = a->Id();
    auto b = VIOLET_MOVE(a).Value();
    EXPECT_FALSE(a->Joinable());
    EXPECT_TRUE(b.Joinable());
    EXPECT_EQ(b.Id(), id);

    b.CancelAndJoin();
}

TEST(Thread, MoveAssignmentJoinsPreviousThread)
{
    std::atomic<bool> firstExited = false;

    auto thread = Thread::Builder().Spawn([&](const CancellationToken& token) -> void {
        token.WaitForCancellation();
        firstExited = true;
    });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    thread = Thread::Builder().Spawn([] -> void { });
    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    EXPECT_TRUE(firstExited);
    EXPECT_TRUE(thread->Joinable());
    thread->Join();
}

TEST(Thread, DetachReleasesOwnership)
{
    std::atomic<bool> done = false;
    auto thread = Thread::Builder().Spawn([&] -> void {
        done = true;
        done.notify_one();
    });

    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    thread->Detach();
    EXPECT_FALSE(thread->Joinable());

    // the detached thread only touches `done`, which outlives this wait
    done.wait(false);
    EXPECT_TRUE(done);
}

TEST(ThreadDeathTest, JoiningNonJoinableHandleAsserts)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH({ Thread::Main().Join(); }, "not joinable");
}

TEST(ThreadDeathTest, DetachingNonJoinableHandleAsserts)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    EXPECT_DEATH({ Thread::Current().Detach(); }, "not joinable");
}

TEST(ThreadDeathTest, JoiningTwiceAsserts)
{
    GTEST_FLAG_SET(death_test_style, "threadsafe");

    auto thread = Thread::Builder().Spawn([] -> void { });
    EXPECT_TRUE(thread) << "thread failed to create: " << thread.Error();

    thread->Join();
    EXPECT_DEATH(thread->Join(), "not joinable");
}

} // namespace violet::experimental::threading
