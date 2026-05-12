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
#include <violet/Experimental/Time/Duration.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace std::chrono_literals;
using namespace violet::experimental::chrono;

static_assert(std::is_trivially_copyable_v<Duration>);
static_assert(std::is_trivially_destructible_v<Duration>);
static_assert(sizeof(Duration) == sizeof(violet::UInt64));

TEST(Duration, DefaultConstructsToZero)
{
    Duration d;
    EXPECT_EQ(d.AsNanos(), 0);
    EXPECT_TRUE(d.Zeroed());
}

TEST(Duration, ZeroFactory)
{
    EXPECT_EQ(Duration::Zero().AsNanos(), 0);
    EXPECT_TRUE(Duration::Zero().Zeroed());
}

TEST(Duration, MaxIsLargestRepresentable)
{
    EXPECT_EQ(Duration::Max().AsNanos(), std::numeric_limits<int64_t>::max());
}

TEST(Duration, FromNanos)
{
    EXPECT_EQ(Duration::Nanoseconds(0).AsNanos(), 0);
    EXPECT_EQ(Duration::Nanoseconds(500).AsNanos(), 500);
    EXPECT_EQ(Duration::Nanoseconds(-500).AsNanos(), -500);
}

TEST(Duration, Milliseconds)
{
    EXPECT_EQ(Duration::Milliseconds(1).AsNanos(), 1'000'000);
    EXPECT_EQ(Duration::Milliseconds(1500).AsMillis(), 1500);
    EXPECT_EQ(Duration::Milliseconds(-250).AsNanos(), -250'000'000);
}

TEST(Duration, Seconds)
{
    EXPECT_EQ(Duration::Seconds(1).AsNanos(), 1'000'000'000);
    EXPECT_EQ(Duration::Seconds(0).AsNanos(), 0);
    EXPECT_EQ(Duration::Seconds(-5).AsSeconds(), -5);
}

TEST(Duration, Minutes)
{
    EXPECT_EQ(Duration::Minutes(1).AsSeconds(), 60);
    EXPECT_EQ(Duration::Minutes(2).AsMillis(), 120'000);
}

TEST(Duration, Hours)
{
    EXPECT_EQ(Duration::Hours(1).AsSeconds(), 3'600);
    EXPECT_EQ(Duration::Hours(24).AsSeconds(), 86'400);
}

TEST(Duration, ConstructibleFromChronoNanoseconds)
{
    Duration d = 500ns;
    EXPECT_EQ(d.AsNanos(), 500);
}

TEST(Duration, ConstructibleFromChronoMilliseconds)
{
    Duration d = 250ms;
    EXPECT_EQ(d.AsMillis(), 250);
}

TEST(Duration, ConstructibleFromChronoSeconds)
{
    Duration d = 3s;
    EXPECT_EQ(d.AsSeconds(), 3);
}

TEST(Duration, ConstructibleFromChronoMinutes)
{
    Duration d = 2min;
    EXPECT_EQ(d.AsSeconds(), 120);
}

TEST(Duration, ToStdReturnsChronoNanoseconds)
{
    auto d = Duration::Milliseconds(500);
    auto ns = d.ToStd();
    static_assert(std::is_same_v<decltype(ns), std::chrono::nanoseconds>);
    EXPECT_EQ(ns.count(), 500'000'000);
}

TEST(Duration, AsNanosExact)
{
    EXPECT_EQ(Duration::Nanoseconds(42).AsNanos(), 42);
}

TEST(Duration, AsMicrosTruncatesTowardZero)
{
    EXPECT_EQ(Duration::Nanoseconds(1'999).AsMicros(), 1);
    EXPECT_EQ(Duration::Nanoseconds(-1'999).AsMicros(), -1);
}

TEST(Duration, AsMillisTruncatesTowardZero)
{
    EXPECT_EQ(Duration::Nanoseconds(1'999'999).AsMillis(), 1);
    EXPECT_EQ(Duration::Nanoseconds(-1'999'999).AsMillis(), -1);
}

TEST(Duration, AsSecondsTruncatesTowardZero)
{
    EXPECT_EQ(Duration::Milliseconds(1999).AsSeconds(), 1);
    EXPECT_EQ(Duration::Milliseconds(-1999).AsSeconds(), -1);
}

TEST(Duration, IsZero)
{
    EXPECT_TRUE(Duration::Zero().Zeroed());
    EXPECT_TRUE(Duration::Nanoseconds(0).Zeroed());
    EXPECT_FALSE(Duration::Nanoseconds(1).Zeroed());
    EXPECT_FALSE(Duration::Nanoseconds(-1).Zeroed());
}

TEST(Duration, Addition)
{
    auto a = Duration::Seconds(10);
    auto b = Duration::Milliseconds(500);
    EXPECT_EQ((a + b).AsMillis(), 10'500);
}

TEST(Duration, AdditionCommutative)
{
    auto a = Duration::Milliseconds(300);
    auto b = Duration::Microseconds(700);
    EXPECT_EQ((a + b), (b + a));
}

TEST(Duration, Subtraction)
{
    auto a = Duration::Seconds(10);
    auto b = Duration::Milliseconds(500);
    EXPECT_EQ((a - b).AsMillis(), 9'500);
}

TEST(Duration, SubtractionProducesNegative)
{
    auto a = Duration::Seconds(1);
    auto b = Duration::Seconds(3);
    EXPECT_EQ((a - b).AsSeconds(), -2);
}

TEST(Duration, MultiplicationByScalar)
{
    auto d = Duration::Milliseconds(250);
    EXPECT_EQ((d * 4).AsSeconds(), 1);
}

TEST(Duration, MultiplicationByNegativeScalar)
{
    auto d = Duration::Seconds(5);
    EXPECT_EQ((d * -2).AsSeconds(), -10);
}

TEST(Duration, DivisionByScalar)
{
    auto d = Duration::Seconds(10);
    EXPECT_EQ((d / 4).AsMillis(), 2'500);
}

TEST(Duration, CompoundAssignment)
{
    auto d = Duration::Seconds(5);
    d += Duration::Milliseconds(500);
    EXPECT_EQ(d.AsMillis(), 5'500);

    d -= Duration::Seconds(1);
    EXPECT_EQ(d.AsMillis(), 4'500);
}

TEST(Duration, UnaryNegation)
{
    auto d = Duration::Seconds(3);
    EXPECT_EQ((-d).AsSeconds(), -3);
    EXPECT_EQ((-(-d)).AsSeconds(), 3);
}

TEST(Duration, CheckedAddReturnsSomeOnSuccess)
{
    auto a = Duration::Seconds(1);
    auto b = Duration::Seconds(2);
    auto result = a.CheckedAdd(b);
    ASSERT_TRUE(result);
    EXPECT_EQ(result->AsSeconds(), 3);
}

TEST(Duration, CheckedAddReturnsNoneOnOverflow)
{
    auto huge = Duration::Max() - Duration::Nanoseconds(1);
    auto result = huge.CheckedAdd(Duration::Nanoseconds(2));
    EXPECT_FALSE(result);
}

TEST(Duration, CheckedAddReturnsNoneOnUnderflow)
{
    auto tiny = Duration::Min() + Duration::Nanoseconds(1);
    auto result = tiny.CheckedAdd(Duration::Nanoseconds(-2));
    EXPECT_FALSE(result);
}

TEST(Duration, CheckedSubReturnsSomeOnSuccess)
{
    auto a = Duration::Seconds(5);
    auto b = Duration::Seconds(2);
    auto result = a.CheckedSub(b);
    ASSERT_TRUE(result);
    EXPECT_EQ(result->AsSeconds(), 3);
}

TEST(Duration, CheckedSubReturnsNoneOnUnderflow)
{
    auto result = Duration::Min().CheckedSub(Duration::Nanoseconds(1));
    EXPECT_FALSE(result);
}

TEST(Duration, SaturatingAddClampsToMax)
{
    auto result = Duration::Max().SaturatingAdd(Duration::Seconds(1));
    EXPECT_EQ(result, Duration::Max());
}

TEST(Duration, SaturatingAddClampsToMin)
{
    auto result = Duration::Min().SaturatingAdd(Duration::Seconds(-1));
    EXPECT_EQ(result, Duration::Min());
}

TEST(Duration, SaturatingAddNormalCase)
{
    auto a = Duration::Seconds(5);
    auto b = Duration::Seconds(3);
    EXPECT_EQ(a.SaturatingAdd(b).AsSeconds(), 8);
}

TEST(Duration, SaturatingSubClampsToMin)
{
    auto result = Duration::Min().SaturatingSub(Duration::Seconds(1));
    EXPECT_EQ(result, Duration::Min());
}

TEST(Duration, SaturatingSubNormalCase)
{
    auto a = Duration::Seconds(5);
    auto b = Duration::Seconds(3);
    EXPECT_EQ(a.SaturatingSub(b).AsSeconds(), 2);
}

#ifndef NDEBUG
TEST(Duration, OperatorPlusPanicsOnOverflowInDebug)
{
    auto huge = Duration::Max() - Duration::Nanoseconds(1);
    EXPECT_DEATH({ auto _ = huge + Duration::Nanoseconds(2); }, "overflow");
}

TEST(Duration, OperatorMinusPanicsOnUnderflowInDebug)
{
    EXPECT_DEATH({ auto _ = Duration::Min() - Duration::Nanoseconds(1); }, "overflow");
}

TEST(Duration, OperatorTimesPanicsOnOverflowInDebug)
{
    auto d = Duration::Seconds(1'000'000'000); // 1e18 ns
    EXPECT_DEATH({ auto _ = d * 100; }, "overflow");
}

TEST(Duration, UnaryNegationPanicsOnIntMinInDebug)
{
    EXPECT_DEATH({ auto _ = -Duration::Min(); }, "negation");
}
#endif

TEST(Duration, Equality)
{
    EXPECT_EQ(Duration::Milliseconds(500), Duration::Milliseconds(500));
    EXPECT_EQ(Duration::Milliseconds(1000), Duration::Seconds(1));
    EXPECT_NE(Duration::Milliseconds(500), Duration::Milliseconds(501));
}

TEST(Duration, Ordering)
{
    EXPECT_LT(Duration::Milliseconds(100), Duration::Milliseconds(200));
    EXPECT_LE(Duration::Milliseconds(100), Duration::Milliseconds(100));
    EXPECT_GT(Duration::Milliseconds(200), Duration::Milliseconds(100));
    EXPECT_GE(Duration::Milliseconds(200), Duration::Milliseconds(200));
}

TEST(Duration, OrderingAcrossUnits)
{
    EXPECT_LT(Duration::Milliseconds(999), Duration::Seconds(1));
    EXPECT_EQ(Duration::Milliseconds(1000), Duration::Seconds(1));
    EXPECT_GT(Duration::Milliseconds(1001), Duration::Seconds(1));
}

TEST(Duration, NegativeOrdering)
{
    EXPECT_LT(Duration::Seconds(-5), Duration::Seconds(-3));
    EXPECT_LT(Duration::Seconds(-1), Duration::Zero());
    EXPECT_LT(Duration::Min(), Duration::Max());
}

TEST(Duration, ThreeWayComparison)
{
    auto a = Duration::Milliseconds(100);
    auto b = Duration::Milliseconds(200);
    EXPECT_TRUE((a <=> b) < 0);
    EXPECT_TRUE((b <=> a) > 0);
    EXPECT_TRUE((a <=> a) == 0);
}

TEST(Duration, RoundTripViaChrono)
{
    auto original = Duration::Milliseconds(1234);
    Duration back = original.ToStd();
    EXPECT_EQ(original, back);
}

TEST(Duration, RoundTripNegativeViaChrono)
{
    auto original = Duration::Milliseconds(-789);
    Duration back = original.ToStd();
    EXPECT_EQ(original, back);
}

static_assert(Duration::Zero().Zeroed());
static_assert(Duration::Seconds(1).AsMillis() == 1000);
static_assert(Duration::Milliseconds(500) + Duration::Milliseconds(500) == Duration::Seconds(1));
static_assert(Duration::Seconds(3) > Duration::Seconds(2));
static_assert(-Duration::Seconds(1) == Duration::Seconds(-1));
static_assert(Duration::Max() > Duration::Min());

TEST(Duration, OneDayInHours)
{
    EXPECT_EQ(Duration::Hours(24).AsSeconds(), 86'400);
}

TEST(Duration, ArithmeticPreservesUnits)
{
    auto timeout = Duration::Seconds(30);
    auto elapsed = Duration::Milliseconds(500);
    auto remaining = timeout - elapsed;
    EXPECT_EQ(remaining.AsMillis(), 29'500);
}

TEST(Duration, RetryBackoffComputation)
{
    // Exponential backoff: base * 2^attempt, capped.
    auto base = Duration::Milliseconds(100);
    auto cap = Duration::Seconds(30);
    auto attempt_3 = base * (1 << 3); // 800ms
    EXPECT_EQ(attempt_3.AsMillis(), 800);
    EXPECT_LT(attempt_3, cap);

    auto attempt_10 = base.SaturatingAdd(base * ((1LL << 10) - 1)); // stays reasonable

    // Just verifying it doesn't overflow or return junk
    EXPECT_GT(attempt_10.AsMillis(), 0);
}

TEST(DurationFromStr, Nanoseconds)
{
    EXPECT_EQ(Duration::FromStr("500ns").Unwrap(), Duration::Nanoseconds(500));
    EXPECT_EQ(Duration::FromStr("1ns").Unwrap(), Duration::Nanoseconds(1));
    EXPECT_EQ(Duration::FromStr("0ns").Unwrap(), Duration::Zero());
}

TEST(DurationFromStr, Microseconds)
{
    EXPECT_EQ(Duration::FromStr("500us").Unwrap(), Duration::Microseconds(500));
    EXPECT_EQ(Duration::FromStr("1us").Unwrap(), Duration::Nanoseconds(1'000));
}

TEST(DurationFromStr, Milliseconds)
{
    EXPECT_EQ(Duration::FromStr("500ms").Unwrap(), Duration::Milliseconds(500));
    EXPECT_EQ(Duration::FromStr("1ms").Unwrap(), Duration::Nanoseconds(1'000'000));
}

TEST(DurationFromStr, Seconds)
{
    EXPECT_EQ(Duration::FromStr("30s").Unwrap(), Duration::Seconds(30));
    EXPECT_EQ(Duration::FromStr("1s").Unwrap(), Duration::Milliseconds(1000));
}

TEST(DurationFromStr, Minutes)
{
    EXPECT_EQ(Duration::FromStr("5m").Unwrap(), Duration::Seconds(300));
    EXPECT_EQ(Duration::FromStr("1m").Unwrap(), Duration::Seconds(60));
}

TEST(DurationFromStr, Hours)
{
    EXPECT_EQ(Duration::FromStr("2h").Unwrap(), Duration::Seconds(7200));
    EXPECT_EQ(Duration::FromStr("1h").Unwrap(), Duration::Minutes(60));
}

TEST(DurationFromStr, Days)
{
    EXPECT_EQ(Duration::FromStr("1d").Unwrap(), Duration::Hours(24));
    EXPECT_EQ(Duration::FromStr("7d").Unwrap(), Duration::Hours(168));
}

TEST(DurationFromStr, MsNotAmbiguousWithM)
{
    // Regression test: longer unit prefixes ("ms") must match before
    // shorter ones ("m"). If MatchUnit's ordering regresses, "500ms"
    // would Duration::FromStr as "500" + "m" with leftover "s"; error.
    EXPECT_EQ(Duration::FromStr("500ms").Unwrap(), Duration::Milliseconds(500));
}

TEST(DurationFromStr, UsNotAmbiguous)
{
    EXPECT_EQ(Duration::FromStr("500us").Unwrap(), Duration::Microseconds(500));
}

TEST(DurationFromStr, NsNotAmbiguous)
{
    EXPECT_EQ(Duration::FromStr("500ns").Unwrap(), Duration::Nanoseconds(500));
}

TEST(DurationFromStr, FractionalSeconds)
{
    EXPECT_EQ(Duration::FromStr("1.5s").Unwrap(), Duration::Milliseconds(1500));
    EXPECT_EQ(Duration::FromStr("0.5s").Unwrap(), Duration::Milliseconds(500));
    EXPECT_EQ(Duration::FromStr("0.25s").Unwrap(), Duration::Milliseconds(250));
}

TEST(DurationFromStr, FractionalMillis)
{
    EXPECT_EQ(Duration::FromStr("1.5ms").Unwrap(), Duration::Microseconds(1500));
}

TEST(DurationFromStr, FractionalMinutes)
{
    EXPECT_EQ(Duration::FromStr("1.5m").Unwrap(), Duration::Seconds(90));
    EXPECT_EQ(Duration::FromStr("0.5m").Unwrap(), Duration::Seconds(30));
}

TEST(DurationFromStr, FractionalHours)
{
    EXPECT_EQ(Duration::FromStr("2.5h").Unwrap(), Duration::Minutes(150));
    EXPECT_EQ(Duration::FromStr("0.25h").Unwrap(), Duration::Minutes(15));
}

TEST(DurationFromStr, VerySmallFraction)
{
    // 0.001 seconds = 1ms = 1_000_000 ns
    EXPECT_EQ(Duration::FromStr("0.001s").Unwrap(), Duration::Microseconds(1000));
}

TEST(DurationFromStr, HoursAndMinutes)
{
    auto d = Duration::FromStr("1h30m");
    EXPECT_EQ(d->AsSeconds(), 90 * 60);
}

TEST(DurationFromStr, MinutesAndSeconds)
{
    auto d = Duration::FromStr("2m30s");
    EXPECT_EQ(d->AsSeconds(), 150);
}

TEST(DurationFromStr, ThreePartCompound)
{
    auto d = Duration::FromStr("1h30m15s");
    EXPECT_EQ(d->AsSeconds(), (90 * 60) + 15);
}

TEST(DurationFromStr, CompoundWithSubsecond)
{
    auto d = Duration::FromStr("1s500ms");
    EXPECT_EQ(d->AsMillis(), 1500);
}

TEST(DurationFromStr, CompoundOrderIsNotEnforced)
{
    // "30m1h" still Duration::FromStrs. Duration::FromStrr sums whatever it sees.
    auto d = Duration::FromStr("30m1h");
    EXPECT_EQ(d->AsMinutes(), 90);
}

TEST(DurationFromStr, AllUnitsInOneString)
{
    auto d = Duration::FromStr("1h2m3s4ms5us6ns").Unwrap();
    const auto expected = Duration::Hours(1) + Duration::Minutes(2) + Duration::Seconds(3) + Duration::Milliseconds(4)
        + Duration::Microseconds(5) + Duration::Nanoseconds(6);

    EXPECT_EQ(d, expected);
}

TEST(DurationFromStr, RejectsNegativePrefix)
{
    EXPECT_FALSE(Duration::FromStr("-5s"));
    EXPECT_FALSE(Duration::FromStr("-100ms"));
    EXPECT_FALSE(Duration::FromStr("-1h30m"));
}

TEST(DurationFromStr, RejectsPositivePrefix)
{
    // Even '+' is rejected — consistent with "no signs at all."
    EXPECT_FALSE(Duration::FromStr("+5s"));
}

TEST(DurationFromStr, RejectsNegativeFraction)
{
    EXPECT_FALSE(Duration::FromStr("-1.5s"));
    EXPECT_FALSE(Duration::FromStr("-0.5s"));
}

TEST(DurationFromStr, RejectsSignOnly)
{
    EXPECT_FALSE(Duration::FromStr("-"));
    EXPECT_FALSE(Duration::FromStr("+"));
}

TEST(DurationFromStr, RejectsInternalSign)
{
    // "1h-30m" should be rejected; negatives inside compound form
    // make no semantic sense.
    EXPECT_FALSE(Duration::FromStr("1h-30m"));
}

TEST(DurationFromStr, BareZeroWithUnit)
{
    EXPECT_EQ(Duration::FromStr("0s").Unwrap(), Duration::Zero());
    EXPECT_EQ(Duration::FromStr("0ms").Unwrap(), Duration::Zero());
    EXPECT_EQ(Duration::FromStr("0h").Unwrap(), Duration::Zero());
}

TEST(DurationFromStr, ZeroFractionIsZero)
{
    EXPECT_EQ(Duration::FromStr("0.0s").Unwrap(), Duration::Zero());
}

TEST(DurationFromStr, RejectsLeadingWhitespace)
{
    EXPECT_FALSE(Duration::FromStr(" 30s"));
}

TEST(DurationFromStr, RejectsTrailingWhitespace)
{
    EXPECT_FALSE(Duration::FromStr("30s "));
}

TEST(DurationFromStr, RejectsInternalWhitespace)
{
    EXPECT_FALSE(Duration::FromStr("1h 30m"));
    EXPECT_FALSE(Duration::FromStr("30 s"));
}

TEST(DurationFromStr, RejectsEmpty)
{
    EXPECT_FALSE(Duration::FromStr(""));
}

TEST(DurationFromStr, RejectsMissingUnit)
{
    EXPECT_FALSE(Duration::FromStr("30"));
    EXPECT_FALSE(Duration::FromStr("0"));
}

TEST(DurationFromStr, RejectsMissingNumber)
{
    EXPECT_FALSE(Duration::FromStr("s"));
    EXPECT_FALSE(Duration::FromStr("ms"));
}

TEST(DurationFromStr, RejectsUnknownUnit)
{
    EXPECT_FALSE(Duration::FromStr("30x"));
    EXPECT_FALSE(Duration::FromStr("30y"));
    EXPECT_FALSE(Duration::FromStr("30week"));
}

TEST(DurationFromStr, RejectsBareDecimalPoint)
{
    EXPECT_FALSE(Duration::FromStr(".s"));
    EXPECT_FALSE(Duration::FromStr(".5"));
}

TEST(DurationFromStr, AcceptLeadingDecimal)
{
    EXPECT_TRUE(Duration::FromStr(".5s"));
}

TEST(DurationFromStr, RejectsTrailingJunk)
{
    EXPECT_FALSE(Duration::FromStr("30s!"));
    EXPECT_FALSE(Duration::FromStr("30sabc"));
}

TEST(DurationFromStr, RejectsMultipleDecimals)
{
    EXPECT_FALSE(Duration::FromStr("1.2.3s"));
}

TEST(DurationFromStr, RejectsScientificNotation)
{
    EXPECT_FALSE(Duration::FromStr("1e3s"));
}

TEST(DurationFromStr, RejectsOverflowInDays)
{
    // Int64 nanoseconds caps at ~292 years. 500,000 days overflows.
    EXPECT_FALSE(Duration::FromStr("500000d"));
}

TEST(DurationFromStr, LargeButRepresentable)
{
    // 100 days is fine: 100 * 86400e9 = 8.64e15 ns < Int64 max (~9.2e18).
    auto result = Duration::FromStr("100d");
    ASSERT_TRUE(result);
    EXPECT_EQ(result.Unwrap().AsSeconds(), 100 * 86400);
}

TEST(DurationFromStr, MaxYearsRangeWorks)
{
    // 100 years ~= 36500d. 36500 * 86400 * 1e9 ~= 3.15e18 ns. Fits.
    auto result = Duration::FromStr("36500d");
    EXPECT_TRUE(result);
}

TEST(DurationFromStr, TypicalHttpTimeout)
{
    EXPECT_EQ(Duration::FromStr("30s").Unwrap(), Duration::Seconds(30));
}

TEST(DurationFromStr, TypicalRetryBackoff)
{
    EXPECT_EQ(Duration::FromStr("5m").Unwrap(), Duration::Minutes(5));
}

TEST(DurationFromStr, TypicalRpcDeadline)
{
    EXPECT_EQ(Duration::FromStr("250ms").Unwrap(), Duration::Milliseconds(250));
}

TEST(DurationFromStr, TypicalHealthCheckInterval)
{
    EXPECT_EQ(Duration::FromStr("10s").Unwrap(), Duration::Seconds(10));
}

TEST(DurationFromStr, TypicalCacheTtl)
{
    EXPECT_EQ(Duration::FromStr("24h").Unwrap(), Duration::Hours(24));
}

TEST(DurationFromStr, TypicalSessionLifetime)
{
    EXPECT_EQ(Duration::FromStr("7d").Unwrap(), Duration::Hours(168));
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
