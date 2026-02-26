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
#include <violet/IO/Experimental/BufferedOutputStream.h>
#include <violet/IO/Experimental/Output/ByteArrayOutputStream.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::io::experimental;
using namespace violet;
// NOLINTEND(google-build-using-namespace)

TEST(BufferedOutputStream, WriteInChunks)
{
    Vec<UInt8> buf;
    auto baos = std::make_shared<ByteArrayOutputStream>(buf);
    BufferedOutputStream bos(baos);

    // Write `abcd`
    Span<const UInt8> data(reinterpret_cast<const UInt8*>("abcd"), 4);
    auto written = bos.Write(data);
    ASSERT_TRUE(written) << "failed to write: " << written.Error();
    EXPECT_EQ(written.Value(), 4);

    // Flush, to test if it works
    ASSERT_TRUE(bos.Flush());

    // Write `efgh`
    Span<const UInt8> data2(reinterpret_cast<const UInt8*>("efgh"), 4);
    auto written2 = bos.Write(data2);
    ASSERT_TRUE(written2) << "failed to write: " << written2.Error();
    EXPECT_EQ(written2.Value(), 4);

    // Flush, to test if it works
    ASSERT_TRUE(bos.Flush());

    EXPECT_EQ(baos->Get().size(), 8);
    EXPECT_EQ(String(baos->Get().begin(), baos->Get().end()), "abcdefgh");
}

TEST(BufferedOutputStream, FlushPartial)
{
    Vec<UInt8> buf;
    auto baos = std::make_shared<ByteArrayOutputStream>(buf);
    BufferedOutputStream bos(baos, 10);

    Span<const UInt8> data(reinterpret_cast<const UInt8*>("12345"), 5);
    auto written = bos.Write(data);
    ASSERT_TRUE(written) << "failed to write: " << written.Error();
    EXPECT_EQ(written.Value(), 5);

    ASSERT_TRUE(bos.Flush());

    EXPECT_EQ(baos->Get().size(), 5);
    EXPECT_EQ(std::string(baos->Get().begin(), baos->Get().end()), "12345");
}

TEST(BufferedOutputStream, MultipleWrites)
{
    Vec<UInt8> buf;
    auto baos = std::make_shared<ByteArrayOutputStream>(buf);
    BufferedOutputStream bos(baos, 4);

    Span<const UInt8> w1(reinterpret_cast<const UInt8*>("ab"), 2);
    Span<const UInt8> w2(reinterpret_cast<const UInt8*>("cdef"), 4);
    Span<const UInt8> w3(reinterpret_cast<const UInt8*>("gh"), 2);

    ASSERT_TRUE(bos.Write(w1));
    ASSERT_TRUE(bos.Write(w2));
    ASSERT_TRUE(bos.Write(w3));
    ASSERT_TRUE(bos.Flush());

    EXPECT_EQ(baos->Get().size(), 8);
    EXPECT_EQ(std::string(baos->Get().begin(), baos->Get().end()), "abcdefgh");
}
