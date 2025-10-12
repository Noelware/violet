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

#include "violet/support/StringRef.h"

#include <gtest/gtest.h>

using namespace Noelware::Violet; // NOLINT

TEST(StringRefs, BasicFunctionality)
{
    StringRef abc("abc");
    EXPECT_EQ(abc.Length(), 3);
    EXPECT_EQ(abc.Data(), "abc");
    EXPECT_TRUE(abc.StartsWith('a'));
    EXPECT_TRUE(abc.StartsWith("ab"));
    EXPECT_TRUE(abc.First());
    EXPECT_TRUE(abc.Last());

    StringRef empty;
    EXPECT_EQ(empty, 0);
    EXPECT_FALSE(empty.StartsWith('a'));
    EXPECT_FALSE(empty.StartsWith("ab"));
    EXPECT_FALSE(empty.First());
    EXPECT_FALSE(empty.Last());
}

TEST(StringRefs, Trimming)
{
    StringRef hello("  \t\nhello world \r\n");

    EXPECT_TRUE(hello.TrimStart().StartsWith("hello"));
}

TEST(StringRefs, Strip) {}
TEST(StringRefs, Split) {}
