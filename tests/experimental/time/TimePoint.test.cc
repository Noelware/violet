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
#include <violet/Experimental/Time/TimePoint.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace std::chrono_literals;
using namespace violet::experimental::chrono;
using namespace violet;

static_assert(std::is_trivially_copyable_v<TimePoint>);
static_assert(std::is_trivially_destructible_v<TimePoint>);
static_assert(sizeof(TimePoint) == sizeof(violet::Int64));

TEST(TimePoint, DefaultConstructsToEpoch)
{
    TimePoint t;
    EXPECT_EQ(t.ToUnixNanos(), 0);
}

TEST(TimePoint, FromUnixSeconds)
{
    auto t = TimePoint::FromUnixSeconds(1'766'534'400);
    EXPECT_EQ(t.ToUnixSeconds(), 1'766'534'400);
    EXPECT_EQ(t.ToUnixNanos(), 1'766'534'400LL * 1'000'000'000LL);
}

TEST(TimePoint, FromUnixSecondsAcceptsNegative)
{
    auto t = TimePoint::FromUnixSeconds(-5);
    EXPECT_EQ(t.ToUnixSeconds(), -5);
    EXPECT_EQ(t.ToUnixNanos(), -5LL * 1'000'000'000LL);
}

TEST(TimePoint, FromUnixMillis)
{
    auto t = TimePoint::FromUnixMillis(1'776'024'331'142);
    EXPECT_EQ(t.ToUnixMillis(), 1'776'024'331'142);
}

TEST(TimePoint, FromUnixNanos)
{
    auto t = TimePoint::FromUnixNanos(1'776'024'331'142'857'999);
    EXPECT_EQ(t.ToUnixNanos(), 1'776'024'331'142'857'999);
}

TEST(TimePoint, ToUnixSecondsTruncatesFractionalPositive)
{
    auto t = TimePoint::FromUnixMillis(1'500);
    EXPECT_EQ(t.ToUnixSeconds(), 1);
}

TEST(TimePoint, ToUnixSecondsTruncatesFractionalNegative)
{
    auto t = TimePoint::FromUnixMillis(-1'500);
    EXPECT_EQ(t.ToUnixSeconds(), -1);
}

TEST(TimePoint, ToUnixMillisExactForMillisecondInput)
{
    auto t = TimePoint::FromUnixMillis(1'776'024'331'142);
    EXPECT_EQ(t.ToUnixMillis(), 1'776'024'331'142);
}

TEST(TimePoint, ToUnixMillisTruncatesSubMillisecondNanos)
{
    auto t = TimePoint::FromUnixNanos(1'999'999); // ~2ms minus 1ns
    EXPECT_EQ(t.ToUnixMillis(), 1);
}

TEST(TimePoint, ToStdRoundTrip)
{
    auto original = TimePoint::FromUnixMillis(1'776'024'331'142);
    auto std_tp = original.ToStd();
    TimePoint back(std_tp);

    // Round-trip exact at millisecond precision; sub-platform-precision
    // nanos may truncate.
    EXPECT_EQ(back.ToUnixMillis(), original.ToUnixMillis());
}

TEST(TimePoint, FromStdPreservesWallTime)
{
    auto std_tp = std::chrono::system_clock::time_point{ std::chrono::seconds{ 1'776'024'331 } };

    TimePoint t(std_tp);
    EXPECT_EQ(t.ToUnixSeconds(), 1'776'024'331);
}

TEST(TimePoint, ToStdReturnsSystemClockTimePoint)
{
    auto t = TimePoint::FromUnixSeconds(0);
    auto std_tp = t.ToStd();
    static_assert(std::is_same_v<decltype(std_tp), std::chrono::system_clock::time_point>);
    EXPECT_EQ(std_tp.time_since_epoch().count(), 0);
}

TEST(TimePoint, AddDurationShiftsForward)
{
    auto t = TimePoint::FromUnixSeconds(1'776'024'331);
    auto later = t + Duration::Seconds(60);
    EXPECT_EQ(later.ToUnixSeconds(), 1'776'024'391);
}

TEST(TimePoint, AddNegativeDurationShiftsBackward)
{
    auto t = TimePoint::FromUnixSeconds(1'776'024'331);
    auto earlier = t + Duration::Seconds(-60);
    EXPECT_EQ(earlier.ToUnixSeconds(), 1'776'024'271);
}

TEST(TimePoint, SubtractDurationShiftsBackward)
{
    auto t = TimePoint::FromUnixSeconds(1'776'024'331);
    auto earlier = t - Duration::Seconds(60);
    EXPECT_EQ(earlier.ToUnixSeconds(), 1'776'024'271);
}

TEST(TimePoint, DifferenceBetweenTimePoints)
{
    auto a = TimePoint::FromUnixSeconds(1'776'024'331);
    auto b = TimePoint::FromUnixSeconds(1'776'024'391);
    EXPECT_EQ((b - a).AsSeconds(), 60);
    EXPECT_EQ((a - b).AsSeconds(), -60);
}

TEST(TimePoint, DifferenceAtSubSecondPrecision)
{
    auto a = TimePoint::FromUnixNanos(1'000'000'000);
    auto b = TimePoint::FromUnixNanos(1'500'000'000);
    EXPECT_EQ((b - a).AsMillis(), 500);
}

TEST(TimePoint, CompoundAssignment)
{
    auto t = TimePoint::FromUnixSeconds(1'776'024'331);
    t += Duration::Seconds(60);
    EXPECT_EQ(t.ToUnixSeconds(), 1'776'024'391);

    t -= Duration::Seconds(30);
    EXPECT_EQ(t.ToUnixSeconds(), 1'776'024'361);
}

TEST(TimePoint, CheckedAddReturnsSomeOnSuccess)
{
    auto t = TimePoint::FromUnixSeconds(1'776'024'331);
    auto result = t.CheckedAdd(Duration::Seconds(60));
    ASSERT_TRUE(result);
    EXPECT_EQ(result->ToUnixSeconds(), 1'776'024'391);
}

TEST(TimePoint, CheckedAddReturnsNoneOnOverflow)
{
    auto late = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::max() - 100);
    auto result = late.CheckedAdd(Duration::Nanoseconds(200));
    EXPECT_FALSE(result);
}

TEST(TimePoint, CheckedAddReturnsNoneOnUnderflow)
{
    auto early = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::min() + 100);
    auto result = early.CheckedAdd(Duration::Nanoseconds(-200));
    EXPECT_FALSE(result);
}

TEST(TimePoint, CheckedSubReturnsSomeOnSuccess)
{
    auto t = TimePoint::FromUnixSeconds(1'776'024'331);
    auto result = t.CheckedSub(Duration::Seconds(60));
    ASSERT_TRUE(result);
    EXPECT_EQ(result->ToUnixSeconds(), 1'776'024'271);
}

TEST(TimePoint, CheckedSubReturnsNoneOnUnderflow)
{
    auto early = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::min() + 100);
    auto result = early.CheckedSub(Duration::Nanoseconds(200));
    EXPECT_FALSE(result);
}

TEST(TimePoint, CheckedDurationSinceSuccess)
{
    auto a = TimePoint::FromUnixSeconds(1'776'024'331);
    auto b = TimePoint::FromUnixSeconds(1'776'024'391);
    auto result = b.CheckedDurationSince(a);
    ASSERT_TRUE(result);
    EXPECT_EQ(result->AsSeconds(), 60);
}

TEST(TimePoint, CheckedDurationSinceReturnsNoneOnOverflow)
{
    // Two TimePoints at the extreme ends produce a difference too
    // large for Duration (~292 years).
    auto early = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::min());
    auto late = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::max());
    auto result = late.CheckedDurationSince(early);
    EXPECT_FALSE(result);
}

#ifndef NDEBUG
TEST(TimePoint, OperatorPlusPanicsOnOverflowInDebug)
{
    auto late = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::max() - 100);
    EXPECT_DEATH({ auto _ = late + Duration::Nanoseconds(200); }, "TimePoint::operator\\+");
}

TEST(TimePoint, OperatorMinusDurationPanicsOnUnderflowInDebug)
{
    auto early = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::min() + 100);
    EXPECT_DEATH({ auto _ = early - Duration::Nanoseconds(200); }, "TimePoint::operator-");
}

TEST(TimePoint, OperatorMinusTimePointPanicsOnOverflowInDebug)
{
    auto early = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::min());
    auto late = TimePoint::FromUnixNanos(std::numeric_limits<violet::Int64>::max());
    EXPECT_DEATH({ auto _ = late - early; }, "TimePoint::operator-\\(TimePoint\\)");
}
#endif

TEST(TimePoint, Equality)
{
    auto a = TimePoint::FromUnixSeconds(1'776'024'331);
    auto b = TimePoint::FromUnixSeconds(1'776'024'331);
    auto c = TimePoint::FromUnixSeconds(1'776'024'332);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(TimePoint, EqualityAcrossUnits)
{
    auto a = TimePoint::FromUnixSeconds(1);
    auto b = TimePoint::FromUnixMillis(1000);
    auto c = TimePoint::FromUnixNanos(1'000'000'000);
    EXPECT_EQ(a, b);
    EXPECT_EQ(b, c);
}

TEST(TimePoint, Ordering)
{
    auto earlier = TimePoint::FromUnixSeconds(1'776'024'331);
    auto later = TimePoint::FromUnixSeconds(1'776'024'391);
    EXPECT_LT(earlier, later);
    EXPECT_LE(earlier, later);
    EXPECT_GT(later, earlier);
    EXPECT_GE(later, earlier);
}

TEST(TimePoint, ThreeWayComparison)
{
    auto a = TimePoint::FromUnixSeconds(100);
    auto b = TimePoint::FromUnixSeconds(200);
    EXPECT_TRUE((a <=> b) < 0);
    EXPECT_TRUE((b <=> a) > 0);
    EXPECT_TRUE((a <=> a) == 0);
}

TEST(TimePoint, IntoISO8601FromDefault)
{
    auto s = TimePoint{ }.IntoISO8601();
    EXPECT_EQ(s, "1970-01-01T00:00:00.000Z");
}

constexpr Int64 kKnownSeconds = 1'776'463'531;
constexpr Int64 kKnownMillis = 1'776'463'531'142;

TEST(TimePoint, IntoISO8601KnownValue)
{
    auto t = TimePoint::FromUnixMillis(kKnownMillis);
    EXPECT_EQ(t.IntoISO8601(), "2026-04-17T22:05:31.142Z");
}

TEST(TimePoint, IntoISO8601EmitsMillisecondPrecision)
{
    auto t = TimePoint::FromUnixNanos((kKnownMillis * 1'000'000) + 857'999);
    EXPECT_EQ(t.IntoISO8601(), "2026-04-17T22:05:31.142Z");
}

TEST(TimePoint, ParseBasicUtc)
{
    auto r = TimePoint::FromISO8601("2026-04-17T22:05:31Z");
    if (r.Err()) {
        r.Error().Print();
    }

    ASSERT_TRUE(r);
    EXPECT_EQ(r->ToUnixSeconds(), kKnownSeconds);
}

TEST(TimePoint, ParseWithMilliseconds)
{
    auto r = TimePoint::FromISO8601("2026-04-17T22:05:31.142Z");
    if (r.Err()) {
        r.Error().Print();
    }

    ASSERT_TRUE(r);
    EXPECT_EQ(r->ToUnixMillis(), kKnownMillis);
}

TEST(TimePoint, LogLineTimePoint)
{
    auto t = TimePoint::FromUnixMillis(kKnownMillis);
    auto line = std::format("[{}] event", t.IntoISO8601());
    EXPECT_EQ(line, "[2026-04-17T22:05:31.142Z] event");
}

TEST(TimePoint, IntoISO8601AlwaysEndsInZ)
{
    auto t = TimePoint::FromUnixSeconds(1'766'534'400);
    auto s = t.IntoISO8601();
    ASSERT_FALSE(s.empty());
    EXPECT_EQ(s.back(), 'Z');
}

TEST(TimePoint, IntoISO8601PreEpoch)
{
    auto t = TimePoint::FromUnixSeconds(-1); // 1969-12-31T23:59:59Z
    EXPECT_EQ(t.IntoISO8601(), "1969-12-31T23:59:59.000Z");
}

TEST(TimePoint, ParseShortFractionPadsToMillis)
{
    auto r = TimePoint::FromISO8601("2026-04-17T22:05:31.5Z");
    if (r.Err()) {
        r.Error().Print();
    }

    ASSERT_TRUE(r);
    EXPECT_EQ(r->ToUnixMillis() % 1000, 500);
}

TEST(TimePoint, ParseExtraFractionTruncates)
{
    auto r = TimePoint::FromISO8601("2026-04-17T22:05:31.142999999Z");
    if (r.Err()) {
        r.Error().Print();
    }

    ASSERT_TRUE(r);
    EXPECT_EQ(r->ToUnixMillis() % 1000, 142);
}

TEST(TimePoint, ParsePositiveOffset)
{
    auto offset_version = TimePoint::FromISO8601("2026-04-18T05:05:31+07:00").Unwrap();
    auto utc_version = TimePoint::FromISO8601("2026-04-17T22:05:31Z").Unwrap();
    EXPECT_EQ(offset_version, utc_version);
}

TEST(TimePoint, ParseNegativeOffset)
{
    auto offset_version = TimePoint::FromISO8601("2026-04-17T15:05:31-07:00").Unwrap();
    auto utc_version = TimePoint::FromISO8601("2026-04-17T22:05:31Z").Unwrap();
    EXPECT_EQ(offset_version, utc_version);
}

TEST(TimePoint, RoundTripAtMillisecondPrecision)
{
    auto original = TimePoint::FromUnixMillis(1'776'024'331'142);
    auto s = original.IntoISO8601();
    auto back = TimePoint::FromISO8601(s);
    ASSERT_TRUE(back);
    EXPECT_EQ(*back, original);
}

TEST(TimePoint, RejectsMissingTimezone)
{
    auto r = TimePoint::FromISO8601("2026-04-17T22:05:31");
    EXPECT_FALSE(r);
}

TEST(TimePoint, RejectsTrailingGarbage)
{
    auto r = TimePoint::FromISO8601("2026-04-17T22:05:31Zextra");
    EXPECT_FALSE(r);
}

TEST(TimePoint, RejectsInvalidMonth)
{
    auto r = TimePoint::FromISO8601("2026-13-01T00:00:00Z");
    EXPECT_FALSE(r);
}

TEST(TimePoint, RejectsInvalidDay)
{
    auto r = TimePoint::FromISO8601("2026-04-32T00:00:00Z");
    EXPECT_FALSE(r);
}

TEST(TimePoint, RejectsInvalidHour)
{
    auto r = TimePoint::FromISO8601("2026-04-17T25:00:00Z");
    EXPECT_FALSE(r);
}

TEST(TimePoint, RejectsTruncatedInput)
{
    auto r = TimePoint::FromISO8601("2026-04-17");
    EXPECT_FALSE(r);
}

static_assert(TimePoint{ }.ToUnixNanos() == 0);
static_assert(TimePoint::FromUnixSeconds(60).ToUnixSeconds() == 60);
static_assert(TimePoint::FromUnixSeconds(1) + Duration::Seconds(1) == TimePoint::FromUnixSeconds(2));
static_assert(TimePoint::FromUnixSeconds(10) - TimePoint::FromUnixSeconds(3) == Duration::Seconds(7));
static_assert(TimePoint::FromUnixSeconds(5) > TimePoint::FromUnixSeconds(3));

TEST(TimePoint, DeadlineComputation)
{
    auto now = TimePoint::FromUnixSeconds(1'776'024'331);
    auto timeout = Duration::Seconds(30);
    auto deadline = now + timeout;
    EXPECT_EQ(deadline.ToUnixSeconds(), 1'776'024'361);

    auto remaining = deadline - now;
    EXPECT_EQ(remaining.AsSeconds(), 30);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
