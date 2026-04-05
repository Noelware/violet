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
#include <violet/Experimental/Threading/CancellationToken.h>

#include <thread>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::experimental::threading;
using namespace violet::events;
using namespace violet;

using namespace std::chrono_literals;
// NOLINTEND(google-build-using-namespace)

TEST(CancellationTokenSource, DefaultNotCancelled)
{
    CancellationTokenSource cts;
    EXPECT_FALSE(cts.RequestsCancellation());
}

TEST(CancellationTokenSource, CancelSetsCancelled)
{
    CancellationTokenSource cts;
    cts.Cancel();
    EXPECT_TRUE(cts.RequestsCancellation());
}

TEST(CancellationTokenSource, CancelIsIdempotent)
{
    CancellationTokenSource cts;
    cts.Cancel();
    cts.Cancel();
    cts.Cancel();
    EXPECT_TRUE(cts.RequestsCancellation());
}

TEST(CancellationTokenSource, MoveTransfersOwnership)
{
    CancellationTokenSource cts1;
    auto token = cts1.Token();

    CancellationTokenSource cts2 = VIOLET_MOVE(cts1);
    cts2.Cancel();

    EXPECT_TRUE(token.RequestsCancellation());
}

TEST(CancellationToken, ReflectsSourceState)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    EXPECT_FALSE(token.RequestsCancellation());
    cts.Cancel();
    EXPECT_TRUE(token.RequestsCancellation());
}

TEST(CancellationToken, MultipleTokensShareState)
{
    CancellationTokenSource cts;
    auto token1 = cts.Token();
    auto token2 = cts.Token();
    auto token3 = cts.Token();

    EXPECT_FALSE(token1.RequestsCancellation());
    EXPECT_FALSE(token2.RequestsCancellation());
    EXPECT_FALSE(token3.RequestsCancellation());

    cts.Cancel();

    EXPECT_TRUE(token1.RequestsCancellation());
    EXPECT_TRUE(token2.RequestsCancellation());
    EXPECT_TRUE(token3.RequestsCancellation());
}

TEST(CancellationToken, NoneTokenNeverCancelled)
{
    auto token = CancellationToken::None();
    EXPECT_FALSE(token.RequestsCancellation());
}

TEST(CancellationToken, MoveTransfersState)
{
    CancellationTokenSource cts;
    auto token1 = cts.Token();
    auto token2 = VIOLET_MOVE(token1);

    cts.Cancel();
    EXPECT_TRUE(token2.RequestsCancellation());
}

TEST(CancellationToken, OnCancelledFiresOnCancel)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    bool fired = false;
    auto guard = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { fired = true; });

    EXPECT_FALSE(fired);
    cts.Cancel();
    EXPECT_TRUE(fired);
}

TEST(CancellationToken, OnCancelledFiresImmediatelyIfAlreadyCancelled)
{
    CancellationTokenSource cts;
    cts.Cancel();

    auto token = cts.Token();

    bool fired = false;
    auto guard = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { fired = true; });

    EXPECT_TRUE(fired);
}

TEST(CancellationToken, OnCancelledMultipleListeners)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    int count = 0;
    auto guard1 = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { count++; });
    auto guard2 = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { count++; });
    auto guard3 = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { count++; });

    cts.Cancel();
    EXPECT_EQ(count, 3);
}

TEST(CancellationToken, OnCancelledFiresOnlyOnce)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    int count = 0;
    auto guard = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { count++; });

    cts.Cancel();
    cts.Cancel();
    cts.Cancel();

    EXPECT_EQ(count, 1);
}

TEST(CancellationToken, OnCancelledNoneTokenDoesNotFire)
{
    auto token = CancellationToken::None();

    bool fired = false;
    auto guard = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { fired = true; });

    EXPECT_FALSE(fired);
}

TEST(CancellationToken, GuardDisposeDeregistersListener)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    bool fired = false;
    auto guard = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { fired = true; });

    guard.Dispose();
    cts.Cancel();

    EXPECT_FALSE(fired);
}

TEST(CancellationToken, GuardDestructorDeregistersListener)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    bool fired = false;
    {
        auto guard = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { fired = true; });
        // guard goes out of scope here
    }

    cts.Cancel();
    EXPECT_FALSE(fired);
}

TEST(CancellationToken, PersistentGuardSurvivesScope)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    bool fired = false;
    {
        auto guard = token.OnCancelled([&](const CancellationRequestedEvent&) -> void { fired = true; });
        guard.Persist();
        // guard goes out of scope but listener persists
    }

    cts.Cancel();
    EXPECT_TRUE(fired);
}

TEST(CancellationToken, WaitForCancellationUnblocksOnCancel)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    std::atomic<bool> unblocked = false;
    std::thread waiter([&] -> void {
        token.WaitForCancellation();
        unblocked.store(true, std::memory_order_relaxed);
    });

    // Give the waiter thread time to enter the wait
    std::this_thread::sleep_for(50ms);
    EXPECT_FALSE(unblocked.load(std::memory_order_relaxed));

    cts.Cancel();

    waiter.join();
    EXPECT_TRUE(unblocked.load(std::memory_order_relaxed));
}

TEST(CancellationToken, WaitForCancellationReturnsImmediatelyIfAlreadyCancelled)
{
    CancellationTokenSource cts;
    cts.Cancel();

    auto token = cts.Token();

    // Should return immediately, not block
    auto start = std::chrono::steady_clock::now();
    token.WaitForCancellation();

    auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_LT(elapsed, 100ms);
}

TEST(CancellationToken, WaitForCancellationNoneTokenReturnsImmediately)
{
    auto token = CancellationToken::None();

    // No token has no state; it shouldn't block.
    auto start = std::chrono::steady_clock::now();
    token.WaitForCancellation();

    auto elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_LT(elapsed, 100ms);
}

TEST(CancellationToken, ConcurrentCancelAndObserve)
{
    CancellationTokenSource cts;

    constexpr int kThreads = 16;
    std::atomic<int> observed_count = 0;

    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (int i = 0; i < kThreads; i++) {
        threads.emplace_back([&, i] -> void {
            auto token = cts.Token();

            if (i == 0) {
                // One thread cancels
                std::this_thread::sleep_for(10ms);
                cts.Cancel();
            } else {
                // Others wait and observe
                token.WaitForCancellation();
                if (token.RequestsCancellation()) {
                    observed_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto& thread: threads) {
        thread.join();
    }

    // All non-cancelling threads should have observed cancellation
    EXPECT_EQ(observed_count.load(), kThreads - 1);
}

TEST(CancellationToken, ConcurrentOnCancelled)
{
    CancellationTokenSource cts;

    constexpr int kListeners = 64;
    std::atomic<int> fire_count = 0;

    std::vector<Emitter<CancellationRequestedEvent>::Guard> guards;
    guards.reserve(kListeners);

    // Register listeners from multiple threads
    std::vector<std::thread> threads;
    Mutex guards_mu;

    threads.reserve(kListeners);
    for (int i = 0; i < kListeners; i++) {
        threads.emplace_back([&] -> void {
            auto token = cts.Token();
            auto guard = token.OnCancelled(
                [&](const CancellationRequestedEvent&) -> void { fire_count.fetch_add(1, std::memory_order_relaxed); });

            std::lock_guard lk(guards_mu);
            guards.push_back(VIOLET_MOVE(guard));
        });
    }

    for (auto& thread: threads) {
        thread.join();
    }

    cts.Cancel();
    EXPECT_EQ(fire_count.load(), kListeners);
}

TEST(CancellationToken, ConcurrentRegisterAndCancel)
{
    // Tests the race between registering a listener and cancelling.
    // The listener should either:
    //   a) be registered and fire, OR
    //   b) see already-cancelled state and fire immediately
    // It should never silently miss the cancellation.

    for (int iteration = 0; iteration < 100; iteration++) {
        CancellationTokenSource cts;
        std::atomic<bool> fired = false;

        std::thread canceller([&] -> void { cts.Cancel(); });
        std::thread registerer([&] -> void {
            auto token = cts.Token();
            auto guard = token.OnCancelled(
                [&](const CancellationRequestedEvent&) -> void { fired.store(true, std::memory_order_relaxed); });

            guard.Persist();
        });

        canceller.join();
        registerer.join();

        EXPECT_TRUE(fired.load(std::memory_order_relaxed)) << "Missed cancellation on iteration " << iteration;
    }
}

TEST(CancellationRequestedEvent, ToStringIsNonEmpty)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    violet::String repr;
    auto guard = token.OnCancelled([&](const CancellationRequestedEvent& event) -> void { repr = event.ToString(); });

    cts.Cancel();
    EXPECT_FALSE(repr.empty());
}

TEST(CancellationRequestedEvent, StreamableToOstream)
{
    CancellationTokenSource cts;
    auto token = cts.Token();

    std::ostringstream oss;
    auto guard = token.OnCancelled([&](const CancellationRequestedEvent& event) -> void { oss << event; });

    cts.Cancel();
    EXPECT_FALSE(oss.str().empty());
}

TEST(CancellationToken, SourceDestroyedBeforeToken)
{
    CancellationToken token = CancellationToken::None();

    {
        CancellationTokenSource cts;
        token = cts.Token();
        // cts destroyed here; state is still kept alive afterwards
    }

    // Token should still be usable (state kept alive by SharedPtr)
    EXPECT_FALSE(token.RequestsCancellation());
}

TEST(CancellationToken, SourceDestroyedAfterCancel)
{
    CancellationToken token = CancellationToken::None();

    {
        CancellationTokenSource cts;
        token = cts.Token();
        cts.Cancel();
        // cts destroyed here
    }

    // Cancellation state persists
    EXPECT_TRUE(token.RequestsCancellation());
}

TEST(CancellationToken, MultipleSourcesIndependent)
{
    CancellationTokenSource cts1;
    CancellationTokenSource cts2;

    auto token1 = cts1.Token();
    auto token2 = cts2.Token();

    cts1.Cancel();

    EXPECT_TRUE(token1.RequestsCancellation());
    EXPECT_FALSE(token2.RequestsCancellation());
}
