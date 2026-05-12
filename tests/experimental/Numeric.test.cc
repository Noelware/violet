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
#include <violet/Experimental/Numeric.h>

#include <limits>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length,readability-magic-numbers)
using namespace violet::numeric;
using violet::Int16;
using violet::Int32;
using violet::Int64;
using violet::Int8;
using violet::UInt16;
using violet::UInt32;
using violet::UInt64;
using violet::UInt8;

TEST(Numeric, WouldAddingOverflowDetectsPositiveOverflow)
{
    constexpr auto kMax = std::numeric_limits<Int32>::max();
    EXPECT_TRUE(WouldAddingOverflow<Int32>(kMax, 1));
    EXPECT_TRUE(WouldAddingOverflow<Int32>(1, kMax));
    EXPECT_FALSE(WouldAddingOverflow<Int32>(kMax - 1, 1));
}

TEST(Numeric, WouldAddingOverflowDetectsNegativeOverflow)
{
    constexpr auto kMin = std::numeric_limits<Int32>::min();
    EXPECT_TRUE(WouldAddingOverflow<Int32>(kMin, -1));
    EXPECT_TRUE(WouldAddingOverflow<Int32>(-1, kMin));
    EXPECT_FALSE(WouldAddingOverflow<Int32>(kMin + 1, -1));
}

TEST(Numeric, WouldAddingOverflowSafeOperands)
{
    EXPECT_FALSE(WouldAddingOverflow<Int32>(0, 0));
    EXPECT_FALSE(WouldAddingOverflow<Int32>(100, 200));
    EXPECT_FALSE(WouldAddingOverflow<Int32>(-100, -200));
    EXPECT_FALSE(WouldAddingOverflow<Int32>(-100, 100));
}

TEST(Numeric, WouldSubtractingOverflowDetectsPositiveOverflow)
{
    constexpr auto kMax = std::numeric_limits<Int32>::max();
    EXPECT_TRUE(WouldSubtractingOverflow<Int32>(kMax, -1));
    EXPECT_FALSE(WouldSubtractingOverflow<Int32>(kMax - 1, -1));
}

TEST(Numeric, WouldSubtractingOverflowDetectsNegativeOverflow)
{
    constexpr auto kMin = std::numeric_limits<Int32>::min();
    EXPECT_TRUE(WouldSubtractingOverflow<Int32>(kMin, 1));
    EXPECT_FALSE(WouldSubtractingOverflow<Int32>(kMin + 1, 1));
}

TEST(Numeric, WouldSubtractingOverflowSafeOperands)
{
    EXPECT_FALSE(WouldSubtractingOverflow<Int32>(0, 0));
    EXPECT_FALSE(WouldSubtractingOverflow<Int32>(300, 100));
    EXPECT_FALSE(WouldSubtractingOverflow<Int32>(-100, -200));
}

TEST(Numeric, WouldMultiplicationOverflowZeroOperands)
{
    constexpr auto kMax = std::numeric_limits<Int32>::max();
    EXPECT_FALSE(WouldMultiplicationOverflow<Int32>(0, kMax));
    EXPECT_FALSE(WouldMultiplicationOverflow<Int32>(kMax, 0));
    EXPECT_FALSE(WouldMultiplicationOverflow<Int32>(0, 0));
}

TEST(Numeric, WouldMultiplicationOverflowMinTimesMinusOne)
{
    constexpr auto kMin = std::numeric_limits<Int32>::min();
    EXPECT_TRUE(WouldMultiplicationOverflow<Int32>(kMin, -1));
    EXPECT_TRUE(WouldMultiplicationOverflow<Int32>(-1, kMin));
}

TEST(Numeric, WouldMultiplicationOverflowDetectsPositiveOverflow)
{
    constexpr auto kMax = std::numeric_limits<Int32>::max();
    EXPECT_TRUE(WouldMultiplicationOverflow<Int32>(kMax, 2));
    EXPECT_TRUE(WouldMultiplicationOverflow<Int32>(2, kMax));
    EXPECT_FALSE(WouldMultiplicationOverflow<Int32>(kMax / 2, 2));
}

TEST(Numeric, WouldMultiplicationOverflowDetectsNegativeOverflow)
{
    constexpr auto kMin = std::numeric_limits<Int32>::min();
    EXPECT_TRUE(WouldMultiplicationOverflow<Int32>(kMin, 2));
    EXPECT_TRUE(WouldMultiplicationOverflow<Int32>(2, kMin));
}

TEST(Numeric, WouldMultiplicationOverflowSafeOperands)
{
    EXPECT_FALSE(WouldMultiplicationOverflow<Int32>(7, 6));
    EXPECT_FALSE(WouldMultiplicationOverflow<Int32>(-7, 6));
    EXPECT_FALSE(WouldMultiplicationOverflow<Int32>(-7, -6));
}

TEST(Numeric, CheckedAddReturnsSomeOnSuccess)
{
    auto result = CheckedAdd<Int64>(100, 200);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(*result, 300);
}

TEST(Numeric, CheckedAddReturnsNothingOnPositiveOverflow)
{
    constexpr auto kMax = std::numeric_limits<Int64>::max();
    EXPECT_FALSE(CheckedAdd<Int64>(kMax, 1).HasValue());
}

TEST(Numeric, CheckedAddReturnsNothingOnNegativeOverflow)
{
    constexpr auto kMin = std::numeric_limits<Int64>::min();
    EXPECT_FALSE(CheckedAdd<Int64>(kMin, -1).HasValue());
}

TEST(Numeric, CheckedAddBoundaryValues)
{
    constexpr auto kMax = std::numeric_limits<Int32>::max();
    auto result = CheckedAdd<Int32>(kMax - 1, 1);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(*result, kMax);
}

TEST(Numeric, CheckedSubReturnsSomeOnSuccess)
{
    auto result = CheckedSub<Int64>(300, 100);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(*result, 200);
}

TEST(Numeric, CheckedSubReturnsNothingOnNegativeOverflow)
{
    constexpr auto kMin = std::numeric_limits<Int64>::min();
    EXPECT_FALSE(CheckedSub<Int64>(kMin, 1).HasValue());
}

TEST(Numeric, CheckedSubReturnsNothingOnPositiveOverflow)
{
    constexpr auto kMax = std::numeric_limits<Int64>::max();
    EXPECT_FALSE(CheckedSub<Int64>(kMax, -1).HasValue());
}

TEST(Numeric, CheckedMulReturnsSomeOnSuccess)
{
    auto result = CheckedMul<Int64>(7, 6);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(*result, 42);
}

TEST(Numeric, CheckedMulHandlesZero)
{
    constexpr auto kMax = std::numeric_limits<Int64>::max();
    auto result = CheckedMul<Int64>(0, kMax);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(*result, 0);
}

TEST(Numeric, CheckedMulReturnsNothingOnPositiveOverflow)
{
    constexpr auto kMax = std::numeric_limits<Int64>::max();
    EXPECT_FALSE(CheckedMul<Int64>(kMax, 2).HasValue());
}

TEST(Numeric, CheckedMulReturnsNothingOnNegativeOverflow)
{
    constexpr auto kMin = std::numeric_limits<Int64>::min();
    EXPECT_FALSE(CheckedMul<Int64>(kMin, 2).HasValue());
    EXPECT_FALSE(CheckedMul<Int64>(kMin, -1).HasValue());
}

TEST(Numeric, ParseUnsignedUInt64ParsesValid)
{
    auto result = ParseUnsigned<UInt64>("12345");
    ASSERT_TRUE(result);
    EXPECT_EQ(VIOLET_MOVE(result).Unwrap(), 12345U);
}

TEST(Numeric, ParseUnsignedUInt64FailsOnEmpty)
{
    auto result = ParseUnsigned<UInt64>("");
    EXPECT_FALSE(result);
}

TEST(Numeric, ParseUnsignedUInt64FailsOnNonNumeric)
{
    auto result = ParseUnsigned<UInt64>("abc");
    EXPECT_FALSE(result);
}

TEST(Numeric, ParseUnsignedUInt64FailsOnTrailingGarbage)
{
    auto result = ParseUnsigned<UInt64>("123abc");
    EXPECT_FALSE(result);
}

TEST(Numeric, ParseUnsignedUInt8FitsInRange)
{
    auto result = ParseUnsigned<UInt8>("255");
    ASSERT_TRUE(result);
    EXPECT_EQ(VIOLET_MOVE(result).Unwrap(), 255U);
}

TEST(Numeric, ParseUnsignedUInt8RejectsOutOfRange)
{
    auto result = ParseUnsigned<UInt8>("256");
    EXPECT_FALSE(result);
}

TEST(Numeric, ParseUnsignedUInt16FitsInRange)
{
    auto result = ParseUnsigned<UInt16>("65535");
    ASSERT_TRUE(result);
    EXPECT_EQ(VIOLET_MOVE(result).Unwrap(), 65535U);
}

TEST(Numeric, ParseUnsignedUInt16RejectsOutOfRange)
{
    auto result = ParseUnsigned<UInt16>("65536");
    EXPECT_FALSE(result);
}

TEST(Numeric, ParseUnsignedUInt32FitsInRange)
{
    auto result = ParseUnsigned<UInt32>("4294967295");
    ASSERT_TRUE(result);
    EXPECT_EQ(VIOLET_MOVE(result).Unwrap(), 4294967295U);
}

TEST(Numeric, ParseUnsignedUInt32RejectsOutOfRange)
{
    auto result = ParseUnsigned<UInt32>("4294967296");
    EXPECT_FALSE(result);
}

TEST(Numeric, ParseSignedInt64ParsesPositive)
{
    auto result = ParseSigned<Int64>("12345");
    ASSERT_TRUE(result);
    EXPECT_EQ(VIOLET_MOVE(result).Unwrap(), 12345);
}

TEST(Numeric, ParseSignedInt64ParsesNegative)
{
    auto result = ParseSigned<Int64>("-12345");
    ASSERT_TRUE(result);
    EXPECT_EQ(VIOLET_MOVE(result).Unwrap(), -12345);
}

TEST(Numeric, ParseSignedInt64FailsOnEmpty)
{
    auto result = ParseSigned<Int64>("");
    EXPECT_FALSE(result);
}

TEST(Numeric, ParseSignedInt64FailsOnTrailingGarbage)
{
    auto result = ParseSigned<Int64>("42xyz");
    EXPECT_FALSE(result);
}

TEST(Numeric, ParseSignedInt8FitsInRange)
{
    auto pos = ParseSigned<Int8>("127");
    ASSERT_TRUE(pos.Ok());
    EXPECT_EQ(VIOLET_MOVE(pos).Unwrap(), 127);

    auto neg = ParseSigned<Int8>("-128");
    ASSERT_TRUE(neg.Ok());
    EXPECT_EQ(VIOLET_MOVE(neg).Unwrap(), -128);
}

TEST(Numeric, ParseSignedInt8RejectsOutOfRange)
{
    EXPECT_TRUE(ParseSigned<Int8>("128").Err());
    EXPECT_TRUE(ParseSigned<Int8>("-129").Err());
}

TEST(Numeric, ParseSignedInt16FitsInRange)
{
    auto pos = ParseSigned<Int16>("32767");
    ASSERT_TRUE(pos.Ok());
    EXPECT_EQ(VIOLET_MOVE(pos).Unwrap(), 32767);

    auto neg = ParseSigned<Int16>("-32768");
    ASSERT_TRUE(neg.Ok());
    EXPECT_EQ(VIOLET_MOVE(neg).Unwrap(), -32768);
}

TEST(Numeric, ParseSignedInt16RejectsOutOfRange)
{
    EXPECT_TRUE(ParseSigned<Int16>("32768").Err());
    EXPECT_TRUE(ParseSigned<Int16>("-32769").Err());
}

TEST(Numeric, ParseSignedInt32FitsInRange)
{
    auto pos = ParseSigned<Int32>("2147483647");
    ASSERT_TRUE(pos.Ok());
    EXPECT_EQ(VIOLET_MOVE(pos).Unwrap(), 2147483647);

    auto neg = ParseSigned<Int32>("-2147483648");
    ASSERT_TRUE(neg.Ok());
    EXPECT_EQ(VIOLET_MOVE(neg).Unwrap(), -2147483648);
}

TEST(Numeric, ParseSignedInt32RejectsOutOfRange)
{
    EXPECT_TRUE(ParseSigned<Int32>("2147483648").Err());
    EXPECT_TRUE(ParseSigned<Int32>("-2147483649").Err());
}

TEST(Numeric, ParseDoubleParsesValid)
{
    auto result = Parse("3.14");
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(VIOLET_MOVE(result).Unwrap(), 3.14);
}

TEST(Numeric, ParseDoubleParsesInteger)
{
    auto result = Parse("42");
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(VIOLET_MOVE(result).Unwrap(), 42.0);
}

TEST(Numeric, ParseDoubleParsesNegative)
{
    auto result = Parse("-2.5");
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(VIOLET_MOVE(result).Unwrap(), -2.5);
}

TEST(Numeric, ParseDoubleFailsOnEmpty)
{
    EXPECT_TRUE(Parse("").Err());
}

TEST(Numeric, ParseDoubleFailsOnTrailingGarbage)
{
    EXPECT_TRUE(Parse("3.14xyz").Err());
}

TEST(Numeric, ParseDoubleFailsOnNonNumeric)
{
    EXPECT_TRUE(Parse("abc").Err());
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length,readability-magic-numbers)
