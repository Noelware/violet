// 🌺💜 Violet: Extended C++ standard library
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

#include <gtest/gtest.h>
#include <violet/IO/Read.h>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::io; // NOLINT(google-build-using-namespace)

TEST(Reads, AFullRead)
{
    Vec<UInt8> data = { 'H', 'e', 'l', 'l', 'o' };
    Vec<UInt8> buf(5);
    auto result = io::Read(data, buf);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.Value(), 5);
    EXPECT_EQ(buf, data);
}

TEST(Reads, Partial)
{
    Vec<UInt8> data = { 'A', 'B', 'C' };
    Vec<UInt8> buf(5, 0);
    auto result = io::Read(data, buf);

    ASSERT_TRUE(result);
    EXPECT_EQ(result.Value(), 3);
    EXPECT_EQ(buf, Vec<UInt8>({ 'A', 'B', 'C', 0, 0 }));
}

// TEST(Reads, ReadToString)
// {
//     Vec<UInt8> data = { 'H', 'e', 'l', 'l', 'o' };
//     auto result = io::ReadToString(data);

//     ASSERT_TRUE(result);
//     EXPECT_EQ(result.Value(), "Hello");
// }
