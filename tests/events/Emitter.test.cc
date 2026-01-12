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
#include <violet/Events/EventEmitter.h>
#include <violet/Violet.h>

#include <thread>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::events; // NOLINT(google-build-using-namespace)

TEST(Events, ReceivesEvents)
{
    Emitter<UInt32> emitter;
    Event<UInt32> event = emitter.Event();

    UInt32 called = 0;
    auto _ = event([&](UInt32 value) -> void { called = value; }); // NOLINT(readability-identifier-length)

    emitter.Fire(42);
    ASSERT_EQ(called, 42);
}

TEST(Events, PersistListener)
{
    Emitter<UInt32> emitter;
    Event<UInt32> event = emitter.Event();

    UInt32 called = 0;
    Int64 id = -1;

    {
        auto guard
            = event([&](UInt32 value) -> void { called = value; }, true); // NOLINT(readability-identifier-length)

        id = guard.ID();

        emitter.Fire(42);
        ASSERT_EQ(called, 42);
    }

    ASSERT_TRUE(id != -1);

    emitter.Fire(69);
    ASSERT_EQ(called, 69);

    emitter.Unsubscribe(id);
    emitter.Fire(420);
    ASSERT_EQ(called, 69);
}

TEST(Events, GuardListenerStopsCallbacks)
{
    Emitter<UInt32> emitter;
    Event<UInt32> event = emitter.Event();

    UInt32 called = 0;
    {
        auto guard = event([&](UInt32 value) -> void { called = value; });
        emitter.Fire(10);

        ASSERT_EQ(called, 10);
    }

    emitter.Fire(20);
    ASSERT_EQ(called, 10);
}

TEST(Events, MultipleListenersReceiveEvents)
{
    Emitter<UInt32> emitter;
    Event<UInt32> event = emitter.Event();

    UInt32 a = 0; // NOLINT(readability-identifier-length)
    UInt32 b = 0; // NOLINT(readability-identifier-length)

    auto guardA = event([&](UInt32 value) -> void { a = value; });
    auto guardB = event([&](UInt32 value) -> void { b = value; });

    emitter.Fire(5);

    ASSERT_EQ(a, 5);
    ASSERT_EQ(b, 5);
}

TEST(Events, EmitterThreadSafe)
{
    Emitter<UInt32> emitter;
    Event<UInt32> event = emitter.Event();

    std::atomic<bool> started = false;
    Mutex mu;
    Vec<UInt64> ids(1000);

    auto worker = [&] -> void {
        while (!started.load()) {}

        for (Int32 i = 0; i < 100; i++) {
            auto guard = event([&](UInt32) -> void {});
            std::scoped_lock lk(mu);

            ids.push_back(guard.ID());
        }
    };

    std::thread t1(worker);
    std::thread t2(worker);
    std::thread t3(worker);

    started.store(true);
    t1.join();
    t2.join();
    t3.join();

    ASSERT_EQ(ids.size(), 1300);
}
