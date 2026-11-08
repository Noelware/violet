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
#include <violet/Experimental/Language/Int128.h>

using namespace violet::experimental::int128_literals;

namespace violet::experimental {
namespace {

constexpr UInt64 kAllOnes = ~UInt64{0};

constexpr auto Bit(UInt32 n) -> UInt128
{
    return UInt128(1) << n;
}

struct Rng final {
    UInt64 State;

    auto Next() -> UInt64
    {
        UInt64 z = (State += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }

    auto NextU128() -> UInt128
    {
        // Mix in narrow values so the 64-bit fast paths get exercised too.
        switch (Next() % 4) {
        case 0:
            return Next();

        case 1:
            return UInt128::FromParts(Next(), Next() >> (Next() % 64));

        default:
            return UInt128::FromParts(Next(), Next());
        }
    }
};

} // namespace

TEST(UInt128, DefaultIsZero)
{
    UInt128 value;
    EXPECT_EQ(value.HighBits(), 0U);
    EXPECT_EQ(value.LowBits(), 0U);
}

TEST(UInt128, ConstructFromUnsigned)
{
    UInt128 value(42U);
    EXPECT_EQ(value.HighBits(), 0U);
    EXPECT_EQ(value.LowBits(), 42U);
}

TEST(UInt128, ConstructFromNegativeSignExtends)
{
    EXPECT_EQ(UInt128(-1), UInt128::Max());
    EXPECT_EQ(UInt128(Int64{-2}).HighBits(), kAllOnes);
}

TEST(UInt128, FromParts)
{
    auto value = UInt128::FromParts(1, 2);
    EXPECT_EQ(value.LowBits(), 1U);
    EXPECT_EQ(value.HighBits(), 2U);
}

TEST(UInt128, MinAndMax)
{
    EXPECT_EQ(UInt128::Min(), 0);
    EXPECT_EQ(UInt128::Max().HighBits(), kAllOnes);
    EXPECT_EQ(UInt128::Max().LowBits(), kAllOnes);
}

TEST(UInt128, NarrowingConversionTruncates)
{
    auto value = UInt128::FromParts(5, 7);
    EXPECT_EQ(static_cast<UInt64>(value), 5U);
    EXPECT_EQ(static_cast<violet::UInt8>(UInt128(0x1FF)), 0xFF);
}

TEST(UInt128, BoolConversion)
{
    EXPECT_FALSE(static_cast<bool>(UInt128(0)));
    EXPECT_TRUE(static_cast<bool>(UInt128(1)));
    EXPECT_TRUE(static_cast<bool>(Bit(100)));
}

TEST(UInt128, FloatingPointConversion)
{
    EXPECT_EQ(UInt128(3.9), 3);
    EXPECT_EQ(UInt128(0x1p100), Bit(100));
    EXPECT_EQ(static_cast<double>(Bit(100)), 0x1p100);
    EXPECT_EQ(static_cast<double>(UInt128(12345)), 12345.0);
}

TEST(UInt128, AdditionCarriesIntoHighLimb)
{
    EXPECT_EQ(UInt128::FromParts(kAllOnes, 0) + 1, UInt128::FromParts(0, 1));
}

TEST(UInt128, AdditionWraps)
{
    EXPECT_EQ(UInt128::Max() + 1, 0);
}

TEST(UInt128, SubtractionBorrowsFromHighLimb)
{
    EXPECT_EQ(UInt128::FromParts(0, 1) - 1, UInt128::FromParts(kAllOnes, 0));
}

TEST(UInt128, SubtractionWraps)
{
    EXPECT_EQ(UInt128(0) - 1, UInt128::Max());
}

TEST(UInt128, Multiplication)
{
    EXPECT_EQ(UInt128(6) * 7, 42);
    EXPECT_EQ(Bit(64) * Bit(63), Bit(127));

    // (2^64 - 1)^2 = 2^128 - 2^65 + 1
    UInt128 max64(kAllOnes);
    EXPECT_EQ(max64 * max64, UInt128::FromParts(1, kAllOnes - 1));
}

TEST(UInt128, MultiplicationWraps)
{
    EXPECT_EQ(UInt128::Max() * 2, UInt128::Max() - 1);
    EXPECT_EQ(Bit(64) * Bit(64), 0);
}

TEST(UInt128, Division)
{
    EXPECT_EQ(UInt128(100) / 7, 14);
    EXPECT_EQ(UInt128(100) % 7, 2);
    EXPECT_EQ(UInt128(5) / 7, 0);
    EXPECT_EQ(UInt128(5) % 7, 5);

    EXPECT_EQ(UInt128::Max() / UInt128::Max(), 1);
    EXPECT_EQ(UInt128::Max() / 1, UInt128::Max());
    EXPECT_EQ(Bit(127) / Bit(63), Bit(64));

    EXPECT_EQ(UInt128::Max() / 10, 34028236692093846346337460743176821145_u128);
    EXPECT_EQ(UInt128::Max() % 10, 5);
}

TEST(UInt128, DivRemSatisfiesDivisionIdentity)
{
    const UInt128 values[] = {
        1,
        7,
        kAllOnes,
        Bit(64),
        Bit(64) + 1,
        Bit(100) + 12345,
        UInt128::FromParts(0xFEDCBA9876543210ULL, 0x0123456789ABCDEFULL),
        UInt128::FromParts(1, 1),
        UInt128::FromParts(0, kAllOnes),
        UInt128::Max(),
        UInt128::Max() - 1,
    };

    for (auto dividend: values) {
        for (auto divisor: values) {
            auto [quotient, remainder] = UInt128::DivRem(dividend, divisor);
            EXPECT_LT(remainder, divisor) << dividend << " / " << divisor;
            EXPECT_EQ((quotient * divisor) + remainder, dividend) << dividend << " / " << divisor;
        }
    }
}

TEST(UInt128, DivRemRandomized)
{
    Rng rng{0xC0FFEE};
    for (int i = 0; i < 10'000; i++) {
        auto dividend = rng.NextU128();
        auto divisor = rng.NextU128();
        if (divisor == 0) {
            continue;
        }

        auto [quotient, remainder] = UInt128::DivRem(dividend, divisor);
        ASSERT_LT(remainder, divisor) << dividend << " / " << divisor;
        ASSERT_EQ((quotient * divisor) + remainder, dividend) << dividend << " / " << divisor;
    }
}

TEST(UInt128, IncrementAndDecrement)
{
    UInt128 value = kAllOnes;

    EXPECT_EQ(++value, Bit(64));
    EXPECT_EQ(value--, Bit(64));
    EXPECT_EQ(value, kAllOnes);
    EXPECT_EQ(value++, kAllOnes);
    EXPECT_EQ(--value, kAllOnes);
}

TEST(UInt128, CompoundAssignment)
{
    UInt128 value = 10;

    value += 5;
    EXPECT_EQ(value, 15);

    value -= 3;
    EXPECT_EQ(value, 12);

    value *= 4;
    EXPECT_EQ(value, 48);

    value /= 5;
    EXPECT_EQ(value, 9);

    value %= 4;
    EXPECT_EQ(value, 1);

    value <<= 100;
    EXPECT_EQ(value, Bit(100));

    value >>= 99;
    EXPECT_EQ(value, 2);

    value |= 5;
    EXPECT_EQ(value, 7);

    value &= 6;
    EXPECT_EQ(value, 6);

    value ^= 3;
    EXPECT_EQ(value, 5);
}

TEST(UInt128, UnaryOperators)
{
    EXPECT_EQ(-UInt128(1), UInt128::Max());
    EXPECT_EQ(~UInt128(0), UInt128::Max());
    EXPECT_EQ(+UInt128(5), 5);
    EXPECT_TRUE(!UInt128(0));
}

TEST(UInt128, BitwiseOperators)
{
    auto a = UInt128::FromParts(0xF0F0, 0xFF00);
    auto b = UInt128::FromParts(0x0FF0, 0x0FF0);

    EXPECT_EQ(a & b, UInt128::FromParts(0x00F0, 0x0F00));
    EXPECT_EQ(a | b, UInt128::FromParts(0xFFF0, 0xFFF0));
    EXPECT_EQ(a ^ b, UInt128::FromParts(0xFF00, 0xF0F0));
}

TEST(UInt128, ShiftLeft)
{
    UInt128 one = 1;
    EXPECT_EQ(one << 0, 1);
    EXPECT_EQ(one << 63, UInt128::FromParts(UInt64{1} << 63, 0));
    EXPECT_EQ(one << 64, UInt128::FromParts(0, 1));
    EXPECT_EQ(one << 65, UInt128::FromParts(0, 2));
    EXPECT_EQ(one << 127, UInt128::FromParts(0, UInt64{1} << 63));
    EXPECT_EQ(UInt128(kAllOnes) << 4, UInt128::FromParts(kAllOnes << 4, 0xF));
}

TEST(UInt128, ShiftRight)
{
    auto top = Bit(127);
    EXPECT_EQ(top >> 0, top);
    EXPECT_EQ(top >> 63, Bit(64));
    EXPECT_EQ(top >> 64, Bit(63));
    EXPECT_EQ(top >> 127, 1);
    EXPECT_EQ(UInt128::FromParts(0, 0xF) >> 4, UInt128::FromParts(0xF000'0000'0000'0000ULL, 0));
}

TEST(UInt128, BitQueries)
{
    EXPECT_EQ(UInt128(0).LeadingZeros(), 128U);
    EXPECT_EQ(UInt128(1).LeadingZeros(), 127U);
    EXPECT_EQ(Bit(64).LeadingZeros(), 63U);
    EXPECT_EQ(UInt128::Max().LeadingZeros(), 0U);

    EXPECT_EQ(UInt128(0).TrailingZeros(), 128U);
    EXPECT_EQ(Bit(100).TrailingZeros(), 100U);
    EXPECT_EQ(UInt128(1).TrailingZeros(), 0U);

    EXPECT_EQ(UInt128::Max().CountOnes(), 128U);
    EXPECT_EQ(UInt128::FromParts(0b1011, 0b11).CountOnes(), 5U);

    EXPECT_EQ(UInt128(0).BitWidth(), 0U);
    EXPECT_EQ(Bit(100).BitWidth(), 101U);

    EXPECT_TRUE(Bit(100).PowerOfTwo());
    EXPECT_FALSE((Bit(100) + 1).PowerOfTwo());
    EXPECT_FALSE(UInt128(0).PowerOfTwo());
}

TEST(UInt128, Rotate)
{
    EXPECT_EQ(UInt128(1).RotateLeft(127), Bit(127));
    EXPECT_EQ(Bit(127).RotateLeft(1), 1);
    EXPECT_EQ(UInt128(1).RotateRight(1), Bit(127));
    EXPECT_EQ(UInt128(5).RotateLeft(128), 5);
    EXPECT_EQ(UInt128(5).RotateRight(0), 5);
}

TEST(UInt128, SwapBytes)
{
    auto value = UInt128::FromParts(0x0102030405060708ULL, 0x090A0B0C0D0E0F10ULL);
    EXPECT_EQ(value.SwapBytes(), UInt128::FromParts(0x100F0E0D0C0B0A09ULL, 0x0807060504030201ULL));
    EXPECT_EQ(value.SwapBytes().SwapBytes(), value);
}

TEST(UInt128, ComparisonOrdersByHighLimbFirst)
{
    EXPECT_GT(UInt128::FromParts(0, 1), UInt128::FromParts(kAllOnes, 0));
    EXPECT_LT(UInt128::FromParts(kAllOnes, 0), UInt128::FromParts(0, 1));
    EXPECT_EQ(UInt128::FromParts(3, 4), UInt128::FromParts(3, 4));
    EXPECT_NE(UInt128::FromParts(3, 4), UInt128::FromParts(4, 3));
}

TEST(UInt128, ComparisonWithBuiltins)
{
    EXPECT_TRUE(UInt128(5) == 5);
    EXPECT_TRUE(5 == UInt128(5));
    EXPECT_TRUE(4 < UInt128(5));
    EXPECT_TRUE(Bit(64) > kAllOnes);
}

TEST(UInt128, CheckedArithmetic)
{
    EXPECT_FALSE(UInt128::Max().CheckedAdd(1).HasValue());
    EXPECT_EQ(UInt128(1).CheckedAdd(2).Value(), 3);

    EXPECT_FALSE(UInt128(0).CheckedSub(1).HasValue());
    EXPECT_EQ(UInt128(3).CheckedSub(2).Value(), 1);

    EXPECT_FALSE(Bit(64).CheckedMul(Bit(64)).HasValue());
    EXPECT_FALSE(UInt128::Max().CheckedMul(2).HasValue());
    EXPECT_FALSE(UInt128(kAllOnes).CheckedMul(Bit(65)).HasValue());
    EXPECT_EQ(Bit(64).CheckedMul(Bit(63)).Value(), Bit(127));
    EXPECT_EQ(UInt128::Max().CheckedMul(1).Value(), UInt128::Max());

    EXPECT_FALSE(UInt128(1).CheckedDiv(0).HasValue());
    EXPECT_EQ(UInt128(10).CheckedDiv(3).Value(), 3);
}

TEST(UInt128, SaturatingArithmetic)
{
    EXPECT_EQ(UInt128::Max().SaturatingAdd(1), UInt128::Max());
    EXPECT_EQ(UInt128(0).SaturatingSub(1), 0);
    EXPECT_EQ(Bit(64).SaturatingMul(Bit(64)), UInt128::Max());
    EXPECT_EQ(UInt128(3).SaturatingMul(4), 12);
}

TEST(UInt128, Pow)
{
    EXPECT_EQ(UInt128(2).Pow(127), Bit(127));
    EXPECT_EQ(UInt128(2).Pow(128), 0);
    EXPECT_EQ(UInt128(10).Pow(38), 100000000000000000000000000000000000000_u128);
    EXPECT_EQ(UInt128(12345).Pow(0), 1);
}

TEST(UInt128, ToString)
{
    EXPECT_EQ(UInt128(0).ToString(), "0");
    EXPECT_EQ(UInt128(12345).ToString(), "12345");
    EXPECT_EQ(UInt128::Max().ToString(), "340282366920938463463374607431768211455");
    EXPECT_EQ(UInt128(255).ToString(16), "ff");
    EXPECT_EQ(UInt128(255).ToString(16, true), "FF");
    EXPECT_EQ(UInt128(5).ToString(2), "101");
    EXPECT_EQ(UInt128::Max().ToString(16), "ffffffffffffffffffffffffffffffff");
    EXPECT_EQ(UInt128(35).ToString(36), "z");
}

TEST(UInt128, Parse)
{
    EXPECT_EQ(UInt128::Parse("0").Value(), 0);
    EXPECT_EQ(UInt128::Parse("+42").Value(), 42);
    EXPECT_EQ(UInt128::Parse("340282366920938463463374607431768211455").Value(), UInt128::Max());
    EXPECT_EQ(UInt128::Parse("ff", 16).Value(), 255);
    EXPECT_EQ(UInt128::Parse("FF", 16).Value(), 255);
    EXPECT_EQ(UInt128::Parse("101", 2).Value(), 5);

    EXPECT_FALSE(UInt128::Parse("340282366920938463463374607431768211456").HasValue());
    EXPECT_FALSE(UInt128::Parse("").HasValue());
    EXPECT_FALSE(UInt128::Parse("+").HasValue());
    EXPECT_FALSE(UInt128::Parse("-1").HasValue());
    EXPECT_FALSE(UInt128::Parse("12a").HasValue());
    EXPECT_FALSE(UInt128::Parse("2", 2).HasValue());
    EXPECT_FALSE(UInt128::Parse("1", 1).HasValue());
    EXPECT_FALSE(UInt128::Parse("1", 37).HasValue());
}

TEST(UInt128, ToStringParseRoundTrip)
{
    Rng rng{42};
    for (int i = 0; i < 1'000; i++) {
        auto value = rng.NextU128();
        for (UInt32 radix: {2U, 8U, 10U, 16U, 36U}) {
            auto parsed = UInt128::Parse(value.ToString(radix), radix);
            ASSERT_TRUE(parsed.HasValue()) << value << " (radix " << radix << ")";
            ASSERT_EQ(parsed.Value(), value) << "radix " << radix;
        }
    }
}

TEST(UInt128, Literals)
{
    EXPECT_EQ(0_u128, 0);
    EXPECT_EQ(42_u128, 42);
    EXPECT_EQ(0xFF_u128, 255);
    EXPECT_EQ(0b1010_u128, 10);
    EXPECT_EQ(017_u128, 15);
    EXPECT_EQ(1'000'000_u128, 1'000'000);
    EXPECT_EQ(0xFFFF'FFFF'FFFF'FFFF'FFFF'FFFF'FFFF'FFFF_u128, UInt128::Max());
    EXPECT_EQ(340282366920938463463374607431768211455_u128, UInt128::Max());
}

TEST(UInt128, OutputStream)
{
    std::ostringstream os;
    os << UInt128::Max();
    EXPECT_EQ(os.str(), "340282366920938463463374607431768211455");

    os.str("");
    os << std::hex << UInt128(255);
    EXPECT_EQ(os.str(), "ff");

    os.str("");
    os << std::showbase << std::uppercase << std::hex << UInt128(255);
    EXPECT_EQ(os.str(), "0XFF");

    os.str("");
    os << std::showbase << std::nouppercase << std::oct << UInt128(8);
    EXPECT_EQ(os.str(), "010");

    os.str("");
    os << std::showbase << std::dec << UInt128(255);
    EXPECT_EQ(os.str(), "255");
}

TEST(UInt128, InputStream)
{
    std::istringstream is("12345 ff nope");
    UInt128 a;
    UInt128 b;
    UInt128 c;

    is >> a;
    EXPECT_EQ(a, 12345);

    is >> std::hex >> b;
    EXPECT_EQ(b, 255);

    is >> std::dec >> c;
    EXPECT_TRUE(is.fail());
}

TEST(UInt128, Format)
{
    EXPECT_EQ(std::format("{}", UInt128::Max()), "340282366920938463463374607431768211455");
    EXPECT_EQ(std::format("{:x}", UInt128(255)), "ff");
    EXPECT_EQ(std::format("{:X}", UInt128(255)), "FF");
    EXPECT_EQ(std::format("{:#x}", UInt128(255)), "0xff");
    EXPECT_EQ(std::format("{:#X}", UInt128(255)), "0XFF");
    EXPECT_EQ(std::format("{:#b}", UInt128(5)), "0b101");
    EXPECT_EQ(std::format("{:#o}", UInt128(8)), "010");
    EXPECT_EQ(std::format("{:+}", UInt128(5)), "+5");
    EXPECT_EQ(std::format("{:6}", UInt128(42)), "    42");
    EXPECT_EQ(std::format("{:>6}", UInt128(42)), "    42");
    EXPECT_EQ(std::format("{:*<6}", UInt128(42)), "42****");
    EXPECT_EQ(std::format("{:^6}", UInt128(42)), "  42  ");
    EXPECT_EQ(std::format("{:06}", UInt128(42)), "000042");
    EXPECT_EQ(std::format("{:#06x}", UInt128(255)), "0x00ff");
}

TEST(UInt128, Hash)
{
    std::hash<UInt128> hasher;
    EXPECT_EQ(hasher(Bit(100)), hasher(Bit(100)));

    std::unordered_set<UInt128> set{1, Bit(64), Bit(100), UInt128::Max()};
    EXPECT_EQ(set.size(), 4U);
    EXPECT_TRUE(set.contains(Bit(64)));
    EXPECT_FALSE(set.contains(Bit(65)));
}

TEST(UInt128, NumericLimits)
{
    using Limits = std::numeric_limits<UInt128>;

    EXPECT_TRUE(Limits::is_specialized);
    EXPECT_FALSE(Limits::is_signed);
    EXPECT_TRUE(Limits::is_integer);
    EXPECT_EQ(Limits::digits, 128);
    EXPECT_EQ(Limits::digits10, 38);
    EXPECT_EQ(Limits::min(), 0);
    EXPECT_EQ(Limits::max(), UInt128::Max());
}

TEST(UInt128, TypeTraits)
{
    static_assert(sizeof(UInt128) == 16);
    static_assert(std::is_trivially_copyable_v<UInt128>);
    static_assert(std::is_same_v<std::common_type_t<UInt128, int>, UInt128>);
    static_assert(std::is_same_v<std::common_type_t<UInt64, UInt128>, UInt128>);
    static_assert(std::is_convertible_v<int, UInt128>);
    static_assert(!std::is_convertible_v<UInt128, int>);
    static_assert(!std::is_convertible_v<Int128, UInt128>);
}

TEST(UInt128, UsableInConstantExpressions)
{
    constexpr auto sum = UInt128::Max() + 1;
    constexpr auto product = Bit(64) * Bit(63);
    constexpr auto quotient = UInt128::Max() / 10;
    constexpr auto remainder = UInt128::Max() % 10;
    constexpr auto shifted = Bit(127) >> 64;
    constexpr auto parsed = UInt128::Parse("340282366920938463463374607431768211455");
    constexpr auto literal = 0xDEAD'BEEF_u128;

    EXPECT_EQ(sum, 0);
    EXPECT_EQ(product, Bit(127));
    EXPECT_EQ(quotient, 34028236692093846346337460743176821145_u128);
    EXPECT_EQ(remainder, 5);
    EXPECT_EQ(shifted, Bit(63));
    EXPECT_EQ(parsed.Value(), UInt128::Max());
    EXPECT_EQ(literal, 0xDEADBEEF);
}

#if VIOLET_FEATURE(NATIVE_INT128)
TEST(UInt128, MatchesNativeUnsignedInt128)
{
    using Native = unsigned __int128;

    Rng rng{0xBADC0DE};
    for (int i = 0; i < 10'000; i++) {
        auto a = rng.NextU128();
        auto b = rng.NextU128();
        auto shift = static_cast<UInt32>(rng.Next() % 128);

        auto na = static_cast<Native>(a);
        auto nb = static_cast<Native>(b);

        ASSERT_EQ(a + b, UInt128(na + nb));
        ASSERT_EQ(a - b, UInt128(na - nb));
        ASSERT_EQ(a * b, UInt128(na * nb));
        ASSERT_EQ(a << shift, UInt128(na << shift));
        ASSERT_EQ(a >> shift, UInt128(na >> shift));
        ASSERT_EQ(a < b, na < nb);

        if (b != 0) {
            ASSERT_EQ(a / b, UInt128(na / nb)) << a << " / " << b;
            ASSERT_EQ(a % b, UInt128(na % nb)) << a << " % " << b;
        }
    }
}
#endif

TEST(Int128, DefaultIsZero)
{
    Int128 value;
    EXPECT_EQ(value, 0);
    EXPECT_FALSE(value.Negative());
    EXPECT_FALSE(value.Positive());
}

TEST(Int128, ConstructFromNegativeSignExtends)
{
    Int128 value(-1);
    EXPECT_EQ(value.HighBits(), -1);
    EXPECT_EQ(value.LowBits(), kAllOnes);
}

TEST(Int128, MinAndMax)
{
    EXPECT_EQ(Int128::Min().HighBits(), std::numeric_limits<Int64>::min());
    EXPECT_EQ(Int128::Min().LowBits(), 0U);
    EXPECT_EQ(Int128::Max().HighBits(), std::numeric_limits<Int64>::max());
    EXPECT_EQ(Int128::Max().LowBits(), kAllOnes);
}

TEST(Int128, ConversionToUnsignedIsBitwise)
{
    EXPECT_EQ(static_cast<UInt128>(Int128(-1)), UInt128::Max());
    EXPECT_EQ(static_cast<Int128>(UInt128::Max()), -1);
    EXPECT_EQ(static_cast<UInt128>(Int128::Min()), Bit(127));
}

TEST(Int128, NarrowingConversionTruncates)
{
    EXPECT_EQ(static_cast<Int64>(Int128(-5)), -5);
    EXPECT_EQ(static_cast<violet::Int32>(Int128(-1) << 64), 0);
}

TEST(Int128, FloatingPointConversion)
{
    EXPECT_EQ(Int128(-2.5), -2);
    EXPECT_EQ(Int128(2.5), 2);
    EXPECT_EQ(Int128(-0x1p100), Int128(-1) << 100);
    EXPECT_EQ(static_cast<double>(Int128(-1) << 100), -0x1p100);
    EXPECT_EQ(static_cast<double>(Int128(-12345)), -12345.0);
}

} // namespace violet::experimental
