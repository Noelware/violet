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
#include <violet/Experimental/Time/Clock.h>
#include <violet/Experimental/Time/Instant.h>

#include <thread>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet::experimental;
using namespace violet::experimental::chrono;

TEST(Instant, DefaultIsEpoch)
{
    Instant inst;
    EXPECT_EQ(inst.ToStd(), Instant::std_type{ });
}

TEST(Instant, RoundtripFromStd)
{
    auto now = std::chrono::steady_clock::now();
    Instant inst(now);
    EXPECT_EQ(inst.ToStd(), now);
}

TEST(Instant, DurationSinceEarlier)
{
    auto base = std::chrono::steady_clock::now();
    Instant earlier(base);
    Instant later(base + std::chrono::milliseconds(500));

    auto dur = later.DurationSince(earlier);
    EXPECT_EQ(dur, Duration::Milliseconds(500));
}

TEST(Instant, DurationSinceLaterClampsToZero)
{
    auto base = std::chrono::steady_clock::now();
    Instant earlier(base);
    Instant later(base + std::chrono::seconds(1));

    auto dur = earlier.DurationSince(later);
    EXPECT_EQ(dur, Duration::Zero());
}

TEST(Instant, DurationSinceSelfIsZero)
{
    Instant inst(std::chrono::steady_clock::now());
    EXPECT_EQ(inst.DurationSince(inst), Duration::Zero());
}

TEST(Instant, AddDuration)
{
    auto base = std::chrono::steady_clock::now();
    Instant inst(base);

    auto result = inst + Duration::Seconds(5);
    EXPECT_EQ(result.ToStd(), base + std::chrono::seconds(5));
}

TEST(Instant, SubtractDuration)
{
    auto base = std::chrono::steady_clock::now();
    Instant inst(base);

    auto result = inst - Duration::Seconds(3);
    EXPECT_EQ(result.ToStd(), base - std::chrono::seconds(3));
}

TEST(Instant, SubtractInstant)
{
    auto base = std::chrono::steady_clock::now();
    Instant a(base);
    Instant b(base + std::chrono::milliseconds(750));

    Duration diff = b - a;
    EXPECT_EQ(diff, Duration::Milliseconds(750));
}

TEST(Instant, PlusEqualsCompound)
{
    auto base = std::chrono::steady_clock::now();
    Instant inst(base);

    inst += Duration::Seconds(2);
    inst += Duration::Milliseconds(500);

    EXPECT_EQ(inst.ToStd(), base + std::chrono::milliseconds(2500));
}

TEST(Instant, Ordering)
{
    auto base = std::chrono::steady_clock::now();
    Instant a(base);
    Instant b(base + std::chrono::seconds(1));

    EXPECT_LT(a, b);
    EXPECT_LE(a, b);
    EXPECT_GT(b, a);
    EXPECT_GE(b, a);
    EXPECT_NE(a, b);
}

TEST(Instant, EqualityForSameTimePoint)
{
    auto tp = std::chrono::steady_clock::now();
    Instant a(tp);
    Instant b(tp);

    EXPECT_EQ(a, b);
    EXPECT_LE(a, b);
    EXPECT_GE(a, b);
}

TEST(Instant, ElapsedIsNonNegative)
{
    const auto& clock = Clock::System();
    auto before = clock.Now();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto elapsed = before.Elapsed(clock);
    EXPECT_GE(elapsed, Duration::Milliseconds(10));
}

TEST(Instant, ElapsedFutureInstantClampsToZero)
{
    const auto& clock = Clock::System();
    auto future = clock.Now() + Duration::Seconds(9999);

    EXPECT_EQ(future.Elapsed(clock), Duration::Zero());
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
