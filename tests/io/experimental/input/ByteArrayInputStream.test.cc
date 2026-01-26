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
#include <violet/IO/Experimental/Input/ByteArrayInputStream.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::io::experimental;
using namespace violet;
// NOLINTEND(google-build-using-namespace)

TEST(ByteArrayInputStream, ItWorks)
{
    Vec<UInt8> buf({ 'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd' });
    ByteArrayInputStream stream(buf);

    Vec<UInt8> buf2(2);
    auto res1 = stream.Read(buf2);
    ASSERT_TRUE(res1) << "failed to call `stream.Read()`: " << VIOLET_MOVE(res1.Error()).ToString();
    ASSERT_EQ(buf2, Vec<UInt8>({ 'h', 'e' }));
    ASSERT_EQ(res1.Value(), 2);

    auto available = stream.Available().Unwrap();
    ASSERT_EQ(available, 9);
}

TEST(ByteArrayInputStream, ShouldWorkIfEmptyBufferReceived)
{
    Vec<UInt8> buf({ 'h', 'e', 'l', 'l', 'o', ',', 'w', 'o', 'r', 'l', 'd' });
    ByteArrayInputStream stream(buf);

    Vec<UInt8> buf3;
    auto res2 = stream.Read(buf3);
    ASSERT_TRUE(res2) << "failed to call `stream.Read()`: " << VIOLET_MOVE(res2.Error()).ToString();
    ASSERT_TRUE(buf3.empty());
    ASSERT_EQ(res2.Value(), 0);

    auto available = stream.Available().Unwrap();
    ASSERT_EQ(available, 11);
}

TEST(ByteArrayInputStream, ShouldWorkIfStreamBufIsEmpty)
{
    Vec<UInt8> buf;
    ByteArrayInputStream stream(buf);

    Vec<UInt8> buf2;
    auto res = stream.Read(buf2);
    ASSERT_TRUE(res) << "failed to call `stream.Read()`: " << VIOLET_MOVE(res.Error()).ToString();
    ASSERT_TRUE(buf2.empty());
    ASSERT_EQ(res.Value(), 0);

    auto available = stream.Available().Unwrap();
    ASSERT_EQ(available, 0);
}
