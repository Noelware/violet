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
#include <violet/Experimental/Coroutines/Notification.h>
#include <violet/Experimental/Coroutines/Runtime.h>

#include <thread>

using namespace std::chrono_literals;

namespace violet::experimental::coro {
namespace {

auto WaitsOn(Notification* notif) -> Task<Int32>
{
    co_await notif->Wait();
    co_return 1;
}

auto NotifiesLater(Notification* notif) -> Task<void>
{
    notif->Notify();
    co_return;
}

auto ThreeWaiters(Notification* notification) -> Task<Int32>
{
    JoinHandle a = Handle::Current().Spawn(WaitsOn(notification));
    JoinHandle b = Handle::Current().Spawn(WaitsOn(notification));
    JoinHandle c = Handle::Current().Spawn(WaitsOn(notification));

    // FIFO: a, b, and c all run (and park) before the notifier does.
    co_await Handle::Current().Spawn(NotifiesLater(notification));
    co_return co_await VIOLET_MOVE(a) + co_await VIOLET_MOVE(b) + co_await VIOLET_MOVE(c);
}

auto SpawnsParkedWaiter(Notification* notification) -> Task<void>
{
    {
        [[maybe_unused]]
        JoinHandle detached = Handle::Current().Spawn(WaitsOn(notification));
    }

    co_return;
}

} // namespace

TEST(Notification, StartsUnnotified)
{
    Notification notification;
    ASSERT_FALSE(notification.WasNotified());

    notification.Notify();
    ASSERT_TRUE(notification.WasNotified());
}

TEST(Notification, AlreadyNotifiedCompletesImmediately)
{
    Notification notification;
    notification.Notify();

    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_EQ(rt->BlockOn(WaitsOn(&notification)), 1);
}

TEST(Notification, WakesEveryWaiter)
{
    Notification notification;
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_EQ(rt->BlockOn(ThreeWaiters(&notification)), 3);
}

TEST(Notification, NotifyFromAnotherThreadWakesRuntime)
{
    Notification notification;
    auto rt = Runtime::Builder::CurrentThread().Build();

    std::thread notifier([&] -> void {
        std::this_thread::sleep_for(10ms);
        notification.Notify();
    });

    ASSERT_EQ(rt->BlockOn(WaitsOn(&notification)), 1);
    notifier.join();
}

TEST(Notification, NoLostWakeupUnderRace)
{
    // `Notify` races `await_suspend` with no delay. A lost wakeup shows up as a hang (test timeout).
    for (Int32 i = 0; i < 1000; i++) {
        Notification notification;
        auto rt = Runtime::Builder::CurrentThread().Build();

        std::thread notifier([&] -> void { notification.Notify(); });
        ASSERT_EQ(rt->BlockOn(WaitsOn(&notification)), 1) << "iteration " << i;
        notifier.join();
    }
}

TEST(Notification, NotifyAfterRuntimeShutdownIsHarmless)
{
    Notification notification;
    {
        auto rt = Runtime::Builder::CurrentThread().Build();
        rt->BlockOn(SpawnsParkedWaiter(&notification));
    }

    notification.Notify();
    ASSERT_TRUE(notification.WasNotified());
}

TEST(NotificationDeathTest, NotifyingTwiceAsserts)
{
    EXPECT_DEBUG_DEATH(
        {
            Notification notification;
            notification.Notify();
            notification.Notify();
        },
        "called more than once");
}

} // namespace violet::experimental::coro
