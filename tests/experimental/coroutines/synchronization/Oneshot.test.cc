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
#include <violet/Experimental/Coroutines/Synchronization/Oneshot.h>

#include <thread>

using namespace std::chrono_literals;

namespace violet::experimental::coro::oneshot {
namespace {

auto Receives(Receiver<Int32> rx) -> Task<Optional<Int32>>
{
    co_return co_await VIOLET_MOVE(rx);
}

auto SendsThenReceives() -> Task<Optional<Int32>>
{
    auto [tx, rx] = oneshot::Channel<Int32>();
    JoinHandle receiver = Handle::Current().Spawn(Receives(VIOLET_MOVE(rx)));

    VIOLET_MOVE(tx).Send(42);
    co_return co_await VIOLET_MOVE(receiver);
}

auto DropsSender() -> Task<Optional<Int32>>
{
    auto [tx, rx] = oneshot::Channel<Int32>();
    JoinHandle receiver = Handle::Current().Spawn(Receives(VIOLET_MOVE(rx)));

    {
        [[maybe_unused]]
        Sender<Int32> dropped = VIOLET_MOVE(tx);
    }

    co_return co_await VIOLET_MOVE(receiver);
}

} // namespace

TEST(Oneshot, DeliversValue)
{
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_EQ(rt->BlockOn(SendsThenReceives()).Value(), 42);
}

TEST(Oneshot, DroppedSenderYieldsNothing)
{
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_FALSE(rt->BlockOn(DropsSender()).HasValue());
}

TEST(Oneshot, SendFromAnotherThread)
{
    auto [tx, rx] = oneshot::Channel<Int32>();
    std::thread sender([tx = VIOLET_MOVE(tx)]() mutable -> void {
        std::this_thread::sleep_for(10ms);
        VIOLET_MOVE(tx).Send(7);
    });

    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_EQ(rt->BlockOn(Receives(VIOLET_MOVE(rx))), Some(7));
    sender.join();
}

TEST(Oneshot, SendAfterReceiverDroppedFails)
{
    auto [tx, rx] = oneshot::Channel<Int32>();
    {
        [[maybe_unused]]
        Receiver<Int32> dropped = VIOLET_MOVE(rx);
    }

    ASSERT_TRUE(tx.Closed());
    ASSERT_FALSE(VIOLET_MOVE(tx).Send(1));
}

TEST(Oneshot, TryReceiveBeforeAndAfterSend)
{
    auto [tx, rx] = oneshot::Channel<Int32>();
    ASSERT_FALSE(rx.TryReceive());

    VIOLET_MOVE(tx).Send(3);
    ASSERT_EQ(rx.TryReceive(), Some(3));
}

} // namespace violet::experimental::coro::oneshot
