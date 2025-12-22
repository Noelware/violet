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

#include "absl/numeric/int128.h"

#include <gtest/gtest.h>
#include <violet/Numeric/Int128.h>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::numeric; // NOLINT(google-build-using-namespace)

TEST(UInt128, DoubleSmallValues)
{
    UInt128 val = 0;
    EXPECT_DOUBLE_EQ(uint128::ToDouble(val), 0.0);

    val = 1;
    EXPECT_DOUBLE_EQ(uint128::ToDouble(val), 1.0);

    val = 42;
    EXPECT_DOUBLE_EQ(uint128::ToDouble(val), 42.0);
}

TEST(UInt128, LargeValues)
{
    UInt128 val = absl::MakeUint128(1, 0); // 2^64
    EXPECT_DOUBLE_EQ(uint128::ToDouble(val), 18446744073709551616.0); // 2^64

    val = absl::MakeUint128(0x123456789ABCDEF0ULL, 0x0FEDCBA987654321ULL);
    double expected
        = (static_cast<double>(0x123456789ABCDEF0ULL) * kTwoPow64) + static_cast<double>(0x0FEDCBA987654321ULL);

    EXPECT_DOUBLE_EQ(uint128::ToDouble(val), expected);
}

TEST(Int128, PositiveValues)
{
    absl::int128 val = absl::MakeInt128(0x1, 0x0); // 2^64
    EXPECT_DOUBLE_EQ(int128::ToDouble(val), 18446744073709551616.0);

    val = absl::MakeInt128(0x12345678ULL, 0x9ABCDEF0ULL);

    double expected = (static_cast<double>(0x12345678ULL) * kTwoPow64) + static_cast<double>(0x9ABCDEF0ULL);
    EXPECT_DOUBLE_EQ(int128::ToDouble(val), expected);
}

TEST(Int128, NegativeVaalues)
{
    absl::int128 val = -absl::MakeInt128(1, 0); // -2^64
    EXPECT_DOUBLE_EQ(int128::ToDouble(val), -18446744073709551616.0);

    val = -absl::MakeInt128(0x12345678ULL, 0x9ABCDEF0ULL);

    double expected = -((static_cast<double>(0x12345678ULL) * kTwoPow64) + static_cast<double>(0x9ABCDEF0ULL));
    EXPECT_DOUBLE_EQ(int128::ToDouble(val), expected);
}

TEST(UInt128ToDouble, ZeroAndOne)
{
    EXPECT_DOUBLE_EQ(uint128::ToDouble(0), 0.0);
    EXPECT_DOUBLE_EQ(uint128::ToDouble(1), 1.0);
}

TEST(Int128ToDouble, Zero)
{
    EXPECT_DOUBLE_EQ(int128::ToDouble(0), 0.0);
}
