// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::events; // NOLINT(google-build-using-namespace)

TEST(Events, ReceivesEvents)
{
    Emitter<UInt32> emitter;
    Event<UInt32> event = emitter.Event();

    UInt32 called = 0;
    event([&](UInt32 value) -> void { called = value; });

    emitter.Fire(42);
    ASSERT_EQ(called, 42);
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

    auto worker = [&] {
        while (!started.load()) {}

        for (Int32 i = 0; i < 100; i++) {
            auto guard = event([&](UInt32) {});
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

    std::sort(ids.begin(), ids.end());
    auto it = std::unique(ids.begin(), ids.end());

    EXPECT_EQ(it, ids.end());
}
