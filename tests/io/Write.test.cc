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
#include <violet/IO/Write.h>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::io; // NOLINT(google-build-using-namespace)

TEST(Writes, StringWriteFull)
{
    String str;
    auto result = io::Write(str, Vec<UInt8>({ 'H', 'e', 'l', 'l', 'o' }));

    ASSERT_TRUE(result);
    EXPECT_EQ(result.Value(), 5);
    EXPECT_EQ(str, "Hello");
}

TEST(Writes, StringWriteEmpty)
{
    String str;
    auto result = io::Write(str, {});

    ASSERT_TRUE(result);
    EXPECT_EQ(result.Value(), 0);
    EXPECT_EQ(str, "");
}

TEST(Writes, VecWriteFull)
{
    Vec<UInt8> data;
    auto result = io::Write(data, Vec<UInt8>({ 'D', 'a', 't', 'a' }));

    ASSERT_TRUE(result);
    EXPECT_EQ(result.Value(), 4);
    EXPECT_EQ(data, Vec<UInt8>({ 'D', 'a', 't', 'a' }));
}

TEST(Writes, VecMultipleWrites)
{
    Vec<UInt8> data;
    Vec<UInt8> buf1{ 'A', 'B' };
    Vec<UInt8> buf2{ 'C', 'D', 'E' };

    auto r1 = Write(data, buf1);
    auto r2 = Write(data, buf2);

    ASSERT_TRUE(r1);
    ASSERT_TRUE(r2);
    EXPECT_EQ(r1.Value(), 2);
    EXPECT_EQ(r2.Value(), 3);
    EXPECT_EQ(data, Vec<UInt8>({ 'A', 'B', 'C', 'D', 'E' }));
}
