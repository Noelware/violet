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

// NOLINTBEGIN(readability-identifier-length)

#include <gtest/gtest.h>
#include <violet/Experimental/SmolString.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::experimental;
using namespace std::string_view_literals;
// NOLINTEND(google-build-using-namespace)

TEST(SmolStrings, DefaultConstructionResultsInEmpty)
{
    constexpr SmolString<16> str;

    EXPECT_EQ(str.Size(), 0);
    EXPECT_EQ(str.Capacity(), 16);
    EXPECT_TRUE(str.Empty());
    EXPECT_FALSE(str);
}

TEST(SmolStrings, ConstructFromStringLiteral)
{
    constexpr SmolString str("hello");

    EXPECT_TRUE(str);
    EXPECT_FALSE(str.Empty());
    EXPECT_EQ(str.Size(), 5);
    EXPECT_EQ(static_cast<std::string_view>(str), "hello");
}

TEST(SmolStrings, ConstructFromStringLiteralWithSizeParameter)
{
    constexpr SmolString<5> str("hello");

    EXPECT_TRUE(str);
    EXPECT_FALSE(str.Empty());
    EXPECT_EQ(str.Size(), 5);
    EXPECT_EQ(static_cast<std::string_view>(str), "hello");
}

TEST(SmolStrings, ConstructFromEmptyLiteral)
{
    constexpr SmolString s("");

    EXPECT_EQ(s.Size(), 0);
    EXPECT_TRUE(s.Empty());
}

TEST(SmolStrings, ConstructFromStringView)
{
    constexpr auto sv = "world"sv;
    SmolString<16> s(sv);

    EXPECT_EQ(s.Size(), 5);
    EXPECT_EQ(static_cast<std::string_view>(s), "world");
}

TEST(SmolStrings, ConstructFromEmptyStringView)
{
    SmolString<8> s(""sv);

    EXPECT_EQ(s.Size(), 0);
    EXPECT_TRUE(s.Empty());
}

TEST(SmolStrings, CTADDeducesCorrectCapacity)
{
    constexpr SmolString s("abc");

    static_assert(std::is_same_v<decltype(s), const SmolString<3>>);
    EXPECT_EQ(s.Capacity(), 3);
    EXPECT_EQ(s.Size(), 3);
}

TEST(SmolStrings, CTADSingleChar)
{
    constexpr SmolString s("x");

    static_assert(std::is_same_v<decltype(s), const SmolString<1>>);
    EXPECT_EQ(s.Size(), 1);
}

TEST(SmolStrings, SizeReflectsContent)
{
    SmolString<32> s("test");

    EXPECT_EQ(s.Size(), 4);
    EXPECT_EQ(s.Capacity(), 32);
    EXPECT_FALSE(s.Empty());
}

TEST(SmolStrings, CapacityIsTemplateParameter)
{
    SmolString<100> s;
    EXPECT_EQ(s.Capacity(), 100);
}

TEST(SmolStrings, PushAppendsCharacter)
{
    SmolString<8> s("ab");
    s.Push('c');

    EXPECT_EQ(s.Size(), 3);
    EXPECT_EQ(static_cast<std::string_view>(s), "abc");
}

TEST(SmolStrings, PushMultipleCharacters)
{
    SmolString<8> s;
    s.Push('h');
    s.Push('i');

    EXPECT_EQ(s.Size(), 2);
    EXPECT_EQ(static_cast<std::string_view>(s), "hi");
}

TEST(SmolStrings, PushToExactCapacity)
{
    SmolString<3> s("ab");
    s.Push('c');

    EXPECT_EQ(s.Size(), 3);
    EXPECT_EQ(s.Size(), s.Capacity());
    EXPECT_EQ(static_cast<std::string_view>(s), "abc");
}

TEST(SmolStrings, AppendStringView)
{
    SmolString<16> s("hello");
    s.Append(" world");

    EXPECT_EQ(static_cast<std::string_view>(s), "hello world");
    EXPECT_EQ(s.Size(), 11);
}

TEST(SmolStrings, AppendEmpty)
{
    SmolString<16> s("hello");
    s.Append("");

    EXPECT_EQ(static_cast<std::string_view>(s), "hello");
    EXPECT_EQ(s.Size(), 5);
}

TEST(SmolStrings, AppendChaining)
{
    SmolString<32> s("a");
    s.Append("b").Append("c").Append("d");

    EXPECT_EQ(static_cast<std::string_view>(s), "abcd");
}

TEST(SmolStrings, AppendToExactCapacity)
{
    SmolString<6> s("foo");
    s.Append("bar");

    EXPECT_EQ(s.Size(), s.Capacity());
    EXPECT_EQ(static_cast<std::string_view>(s), "foobar");
}

TEST(SmolStrings, SubscriptReadAccess)
{
    constexpr SmolString s("abc");

    EXPECT_EQ(s[0], 'a');
    EXPECT_EQ(s[1], 'b');
    EXPECT_EQ(s[2], 'c');
}

TEST(SmolStrings, SubscriptWriteAccess)
{
    SmolString<8> s("abc");
    s[0] = 'z';

    EXPECT_EQ(s[0], 'z');
    EXPECT_EQ(static_cast<std::string_view>(s), "zbc");
}

TEST(SmolStrings, RangeForLoop)
{
    SmolString<8> s("abc");
    std::string collected;

    for (char c: s) {
        collected += c;
    }

    EXPECT_EQ(collected, "abc");
}

TEST(SmolStrings, BeginEndDistance)
{
    SmolString<16> s("test");

    EXPECT_EQ(static_cast<size_t>(s.end() - s.begin()), s.Size());
}

TEST(SmolStrings, ConstIteration)
{
    const SmolString<8> s("xy");
    std::string collected;

    for (const char c: s) {
        collected += c;
    }

    EXPECT_EQ(collected, "xy");
}

TEST(SmolStrings, ConvertToStringView)
{
    SmolString<16> s("hello");
    auto sv = static_cast<std::string_view>(s);

    EXPECT_EQ(sv, "hello");
    EXPECT_EQ(sv.size(), 5);
}

TEST(SmolStrings, ConvertToBoolNonEmpty)
{
    SmolString<8> s("x");
    EXPECT_TRUE(static_cast<bool>(s));
}

TEST(SmolStrings, ConvertToBoolEmpty)
{
    SmolString<8> s;
    EXPECT_FALSE(static_cast<bool>(s));
}

TEST(SmolStrings, EqualityWithStringView)
{
    SmolString<16> s("test");

    EXPECT_TRUE(s == "test"sv);
    EXPECT_FALSE(s == "other"sv);
}

TEST(SmolStrings, EqualityWithStringViewReversed)
{
    SmolString<16> s("test");

    EXPECT_TRUE("test"sv == s);
    EXPECT_FALSE("other"sv == s);
}

TEST(SmolStrings, InequalityWithStringView)
{
    SmolString<16> s("test");

    EXPECT_TRUE(s != "other"sv);
    EXPECT_FALSE(s != "test"sv);
}

TEST(SmolStrings, EqualityEmptyStrings)
{
    SmolString<8> s;

    EXPECT_TRUE(s == ""sv);
    EXPECT_FALSE(s == "x"sv);
}

TEST(SmolStrings, EqualityDifferentContent)
{
    SmolString<8> a("abc");
    SmolString<8> b("xyz");

    EXPECT_FALSE(a == static_cast<std::string_view>(b));
}

TEST(SmolStrings, SpaceshipLessThan)
{
    SmolString<8> a("abc");
    SmolString<8> b("abd");

    EXPECT_TRUE((a <=> b) < 0);
    EXPECT_TRUE(a < b);
}

TEST(SmolStrings, SpaceshipGreaterThan)
{
    SmolString<8> a("xyz");
    SmolString<8> b("abc");

    EXPECT_TRUE((a <=> b) > 0);
    EXPECT_TRUE(a > b);
}

TEST(SmolStrings, SpaceshipEqual)
{
    SmolString<8> a("same");
    SmolString<8> b("same");

    EXPECT_TRUE((a <=> b) == 0);
}

TEST(SmolStrings, ConstexprDefaultConstruction)
{
    constexpr SmolString<8> s;

    static_assert(s.Size() == 0);
    static_assert(s.Empty());
}

TEST(SmolStrings, ConstexprLiteralConstruction)
{
    constexpr SmolString s("constexpr");

    static_assert(s.Size() == 9);
    static_assert(!s.Empty());
}

TEST(SmolStrings, ConstexprEquality)
{
    constexpr SmolString a("test");

    static_assert(a == "test"sv);
    static_assert(a != "other"sv);
}

TEST(SmolStrings, SingleCharacterString)
{
    constexpr SmolString s("x");

    EXPECT_EQ(s.Size(), 1);
    EXPECT_EQ(s[0], 'x');
    EXPECT_EQ(s.Capacity(), 1);
}

TEST(SmolStrings, AllSameCharacters)
{
    SmolString<4> s("aaaa");

    EXPECT_EQ(s.Size(), 4);
    EXPECT_TRUE(s == "aaaa"sv);
}

TEST(SmolStrings, NullBytesInContent)
{
    SmolString<8> s;
    s.Push('\0');
    s.Push('a');

    EXPECT_EQ(s.Size(), 2);
    // string_view should preserve embedded nulls
    auto sv = static_cast<std::string_view>(s);
    EXPECT_EQ(sv.size(), 2);
    EXPECT_EQ(sv[1], 'a');
}

TEST(SmolStrings, CopySemantics)
{
    SmolString<16> a("hello");
    SmolString<16> b = a;

    EXPECT_EQ(static_cast<std::string_view>(b), "hello");

    b[0] = 'H';
    EXPECT_EQ(static_cast<std::string_view>(a), "hello"); // a unchanged
    EXPECT_EQ(static_cast<std::string_view>(b), "Hello");
}

TEST(SmolStrings, MoveSemantics)
{
    SmolString<16> a("hello");
    SmolString<16> b = VIOLET_MOVE(a);

    EXPECT_EQ(static_cast<std::string_view>(b), "hello");
}

TEST(SmolStrings, TriviallyCopyable)
{
    static_assert(std::is_trivially_copyable_v<SmolString<16>>);
}

// NOLINTEND(readability-identifier-length)
