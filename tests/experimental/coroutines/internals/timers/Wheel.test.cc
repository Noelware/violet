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
#include <violet/Experimental/Coroutines/Internals/Timers/Wheel.h>

#include <algorithm>
#include <random>

namespace violet::experimental::coro::internals {
namespace {

void AdvanceAndLog(TimerWheel& wheel, UInt64 now, Vec<UInt64>& log)
{
    wheel.Advance(now, [&log](TimerEntry* ent) noexcept -> void { log.push_back(ent->Deadline); });
}

auto MkEntry(UInt64 deadline) -> TimerEntry
{
    TimerEntry entry;
    entry.Deadline = deadline;
    return entry;
}

} // namespace

TEST(TimerWheel, EmptyWheelHasNoExpiration)
{
    TimerWheel wheel;
    ASSERT_TRUE(wheel.Empty());
    ASSERT_FALSE(wheel.NextExpiration());
}

TEST(TimerWheel, DoesNotFireEarly)
{
    TimerWheel wheel;
    TimerEntry entry = MkEntry(100);
    wheel.Insert(entry);

    Vec<UInt64> log;
    AdvanceAndLog(wheel, 99, log);
    ASSERT_TRUE(log.empty());
    ASSERT_TRUE(entry.Linked);

    AdvanceAndLog(wheel, 100, log);
    ASSERT_EQ(log, (Vec<UInt64>{100}));
    ASSERT_FALSE(entry.Linked);
    ASSERT_TRUE(wheel.Empty());
}

TEST(TimerWheel, FiresInDeadlineOrderAcrossLevels)
{
    TimerWheel wheel;
    Vec<TimerEntry> entries = {MkEntry(4100), MkEntry(5), MkEntry(70), MkEntry(3), MkEntry(64), MkEntry(262'200)};
    for (TimerEntry& entry: entries) {
        wheel.Insert(entry);
    }

    Vec<UInt64> log;
    AdvanceAndLog(wheel, 1'000'000, log);

    ASSERT_EQ(log, (Vec<UInt64>{3, 5, 64, 70, 4100, 262'200}));
}

TEST(TimerWheel, CascadesToExactTick)
{
    TimerWheel wheel;
    TimerEntry entry = MkEntry(130); // level 1, must cascade to level 0 before firing
    wheel.Insert(entry);

    Vec<UInt64> log;
    for (UInt64 now = 1; now < 130; now++) {
        AdvanceAndLog(wheel, now, log);
        ASSERT_TRUE(log.empty()) << "fired early at tick " << now;
    }

    AdvanceAndLog(wheel, 130, log);
    ASSERT_EQ(log, (Vec<UInt64>{130}));
}

TEST(TimerWheel, PastDeadlineFiresOnNextAdvance)
{
    TimerWheel wheel(50);
    TimerEntry entry = MkEntry(10);
    wheel.Insert(entry);

    ASSERT_EQ(wheel.NextExpiration().Value(), 50U);

    Vec<UInt64> log;
    AdvanceAndLog(wheel, 50, log);
    ASSERT_EQ(log, (Vec<UInt64>{10}));
}

TEST(TimerWheel, RemoveMidListEntry)
{
    TimerWheel wheel;
    TimerEntry a = MkEntry(20);
    TimerEntry b = MkEntry(20);
    TimerEntry c = MkEntry(20);
    wheel.Insert(a);
    wheel.Insert(b);
    wheel.Insert(c);

    wheel.Remove(b);
    ASSERT_FALSE(b.Linked);
    ASSERT_EQ(wheel.Size(), 2U);

    Vec<UInt64> log;
    AdvanceAndLog(wheel, 20, log);
    ASSERT_EQ(log.size(), 2U);
    ASSERT_TRUE(wheel.Empty());
}

TEST(TimerWheel, OverflowEntryFiresAtDeadline)
{
    TimerWheel wheel;
    TimerEntry entry = MkEntry(TimerWheel::kMaxDuration + 5);
    wheel.Insert(entry);

    Vec<UInt64> log;
    AdvanceAndLog(wheel, TimerWheel::kMaxDuration + 4, log);
    ASSERT_TRUE(log.empty());

    AdvanceAndLog(wheel, TimerWheel::kMaxDuration + 5, log);
    ASSERT_EQ(log, (Vec<UInt64>{TimerWheel::kMaxDuration + 5}));
}

TEST(TimerWheel, NextExpirationIsALowerBound)
{
    TimerWheel wheel;
    TimerEntry entry = MkEntry(4100);
    wheel.Insert(entry);

    // Jumping straight to each reported expiration must never fire early, and must reach the deadline.
    Vec<UInt64> log;
    while (log.empty()) {
        UInt64 next = wheel.NextExpiration().Value();
        ASSERT_LE(next, 4100U);

        AdvanceAndLog(wheel, next, log);
        if (next < 4100) {
            ASSERT_TRUE(log.empty());
        }
    }

    ASSERT_EQ(log, (Vec<UInt64>{4100}));
}

TEST(TimerWheel, FireMayReinsertItself)
{
    TimerWheel wheel;
    TimerEntry entry = MkEntry(10);
    wheel.Insert(entry);

    Int32 fires = 0;
    wheel.Advance(100, [&](TimerEntry* fired) noexcept {
        fires++;
        if (fires < 5) {
            fired->Deadline += 10; // periodic: 10, 20, 30, 40, 50
            wheel.Insert(*fired);
        }
    });

    ASSERT_EQ(fires, 5);
    ASSERT_TRUE(wheel.Empty());
}

TEST(TimerWheel, RandomizedMatchesSortedOrder)
{
    std::mt19937_64 rng(0xBEEF);
    std::uniform_int_distribution<UInt64> deadlineDist(0, UInt64{1} << 20);
    std::uniform_int_distribution<UInt64> stepDist(1, 5000);

    Vec<TimerEntry> entries(2000);
    TimerWheel wheel;
    for (TimerEntry& entry: entries) {
        entry.Deadline = deadlineDist(rng);
        wheel.Insert(entry);
    }

    Vec<UInt64> log;
    UInt64 now = 0;
    while (!wheel.Empty()) {
        now += stepDist(rng);
        wheel.Advance(now, [&](TimerEntry* entry) noexcept -> void {
            ASSERT_LE(entry->Deadline, now) << "fired before its deadline";
            log.push_back(entry->Deadline);
        });
    }

    ASSERT_EQ(log.size(), entries.size());
    ASSERT_TRUE(std::ranges::is_sorted(log));
}

} // namespace violet::experimental::coro::internals
