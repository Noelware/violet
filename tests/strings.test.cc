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
#include <violet/Strings.h>

using namespace violet::strings; // NOLINT(google-build-using-namespace)
using namespace violet; // NOLINT(google-build-using-namespace)

TEST(Strings, TrimStart)
{
    ASSERT_EQ(TrimStart("   hello"), "hello");
    ASSERT_EQ(TrimStart("\t\n  hello"), "hello");
    ASSERT_EQ(TrimStart("hello"), "hello");
    ASSERT_EQ(TrimStart(""), "");
}

TEST(Strings, TrimEnd)
{
    ASSERT_EQ(TrimEnd("hello   "), "hello");
    ASSERT_EQ(TrimEnd("hello\t\n  "), "hello");
    ASSERT_EQ(TrimEnd("hello"), "hello");
    ASSERT_EQ(TrimEnd(""), "");
}

TEST(Strings, Trim)
{
    EXPECT_EQ(Trim("   hello   "), "hello");
    EXPECT_EQ(Trim("\t\nhello\t"), "hello");
    EXPECT_EQ(Trim("hello"), "hello");
    EXPECT_EQ(Trim(""), "");
}

TEST(Strings, TrimWithCustomPredicate)
{
    auto isX = [](unsigned char ch) -> bool { return ch == 'X'; };
    EXPECT_EQ(TrimStart("XXXhello", isX), "hello");
    EXPECT_EQ(TrimEnd("hellXXXX", isX), "hell");
    EXPECT_EQ(Trim("XXhellXX", isX), "hell");
}

TEST(StringsTest, SplitOnce)
{
    {
        auto result = SplitOnce("key:value", ':');
        EXPECT_EQ(result.first, "key");
        ASSERT_TRUE(result.second.HasValue());
        EXPECT_EQ(result.second.Value(), "value");
    }

    {
        auto result = SplitOnce("novalue", ':');
        EXPECT_EQ(result.first, "novalue");
        EXPECT_FALSE(result.second.HasValue());
    }

    {
        auto result = SplitOnce("a:b:c", ':');
        EXPECT_EQ(result.first, "a");
        ASSERT_TRUE(result.second.HasValue());
        EXPECT_EQ(result.second.Value(), "b:c");
    }
}

TEST(StringsTest, SplitConsecutiveDelimiters)
{
    auto iter = Split("a,,c", ',');
    auto tokens = iter.Collect<Vec<Str>>();

    EXPECT_EQ(tokens.size(), 3U);
    EXPECT_EQ(tokens[0], "a");
    EXPECT_EQ(tokens[1], ""); // empty segment between commas
    EXPECT_EQ(tokens[2], "c");
}

TEST(StringsTest, SplitNoDelimiter)
{
    auto iter = Split("abc", ',');
    auto next = iter.Next();

    ASSERT_TRUE(next);
    EXPECT_EQ(next.Value(), "abc");
    EXPECT_FALSE(iter.Next());
}

TEST(StringsTest, SplitEmptyString)
{
    auto iter = Split("", ',');
    EXPECT_FALSE(iter.Next());
}
