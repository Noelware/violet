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

#include "violet/support/numeric/Traits.h"
#include "violet/violet.h"

#include <gtest/gtest.h>

using Noelware::Violet::int16;
using Noelware::Violet::int32;
using Noelware::Violet::int64;
using Noelware::Violet::int8;
using Noelware::Violet::uint16;
using Noelware::Violet::uint32;
using Noelware::Violet::uint64;
using Noelware::Violet::uint8;
using Noelware::Violet::Numeric::Traits;

TEST(NumericalTraits, UnsignedWrappingAdd)
{
    ASSERT_EQ(Traits<uint8>::WrappingAdd(255, 1), 0);
    ASSERT_EQ(Traits<uint16>::WrappingAdd(65535, 1), 0);
    ASSERT_EQ(Traits<uint32>::WrappingAdd(4294967295, 1), 0);
    ASSERT_EQ(Traits<uint64>::WrappingAdd(18446744073709551615ULL, 1), 0);
}

TEST(NumericalTraits, UnsignedWrappingSub)
{
    ASSERT_EQ(Traits<uint8_t>::WrappingSub(0, 1), 255);
    ASSERT_EQ(Traits<uint16_t>::WrappingSub(0, 1), 65535);
    ASSERT_EQ(Traits<uint32_t>::WrappingSub(0, 1), 4294967295U);
    ASSERT_EQ(Traits<uint64_t>::WrappingSub(0, 1), 18446744073709551615ULL);
}

TEST(NumericalTraits, UnsignedWrappingMul)
{
    ASSERT_EQ(Traits<uint8_t>::WrappingMul(200, 2), 144); // 400 % 256 = 144
    ASSERT_EQ(Traits<uint16_t>::WrappingMul(60000, 2), 54464); // 120000 % 65536 = 54464
    ASSERT_EQ(Traits<uint32_t>::WrappingMul(3000000000U, 2), 1705032704U); // modulo 2^32
    ASSERT_EQ(Traits<uint64_t>::WrappingMul(9223372036854775808ULL, 2), 0ULL);
}

TEST(NumericalTraits, UnsignedWrappingDiv)
{
    ASSERT_EQ(Traits<uint8_t>::WrappingDiv(255, 2), 127);
    ASSERT_EQ(Traits<uint16_t>::WrappingDiv(65535, 2), 32767);
    ASSERT_EQ(Traits<uint32_t>::WrappingDiv(4294967295U, 2), 2147483647U);
    ASSERT_EQ(Traits<uint64_t>::WrappingDiv(18446744073709551615ULL, 2), 9223372036854775807ULL);
}

TEST(NumericalTraits, SignedWrappingAdd)
{
    ASSERT_EQ(Traits<int8>::WrappingAdd(127, 1), -128);
    ASSERT_EQ(Traits<int16_t>::WrappingAdd(-32768, -1), 32767);
    ASSERT_EQ(Traits<int32_t>::WrappingAdd(-2147483648, -1), 2147483647);
    ASSERT_EQ(Traits<int64_t>::WrappingAdd(-9223372036854775807LL - 1, -1), 9223372036854775807LL);
}

TEST(NumericalTraits, SignedWrappingSub)
{
    ASSERT_EQ(Traits<int8_t>::WrappingSub(127, -1), -128);
    ASSERT_EQ(Traits<int16_t>::WrappingSub(32767, -1), -32768);
    ASSERT_EQ(Traits<int32_t>::WrappingSub(2147483647, -1), -2147483648);
    ASSERT_EQ(Traits<int64_t>::WrappingSub(9223372036854775807LL, -1), -9223372036854775807LL - 1);
}

TEST(NumericalTraits, SignedWrappingMul)
{
    ASSERT_EQ(Traits<int8_t>::WrappingMul(100, 3), 44); // 300 mod 256 = 44
    ASSERT_EQ(Traits<int16_t>::WrappingMul(20000, 4), 14464);
    ASSERT_EQ(Traits<int32_t>::WrappingMul(1073741824, 4), 0); // wraps 2^32 boundary
    ASSERT_EQ(Traits<int64_t>::WrappingMul(4611686018427387904LL, 4),
        0LL); // wraps 2^64 boundary
}

TEST(NumericalTraits, SignedWrappingDiv)
{
    // Normal division
    ASSERT_EQ(Traits<int8_t>::WrappingDiv(127, 2), 63);
    ASSERT_EQ(Traits<int16_t>::WrappingDiv(32767, 2), 16383);
    ASSERT_EQ(Traits<int32_t>::WrappingDiv(2147483647, 2), 1073741823);
    ASSERT_EQ(Traits<int64_t>::WrappingDiv(9223372036854775807LL, 2), 4611686018427387903LL);

    // Special Rust case: MIN / -1 wraps to MIN
    ASSERT_EQ(Traits<int8_t>::WrappingDiv(-128, -1), -128);
    ASSERT_EQ(Traits<int16_t>::WrappingDiv(-32768, -1), -32768);
    ASSERT_EQ(Traits<int32_t>::WrappingDiv(-2147483648, -1), -2147483648);
    ASSERT_EQ(Traits<int64_t>::WrappingDiv(-9223372036854775807LL - 1, -1), -9223372036854775807LL - 1);
}
