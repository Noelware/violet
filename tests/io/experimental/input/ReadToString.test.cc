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
#include <violet/IO/Experimental/InputStream.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::io::experimental;
using namespace violet;
// NOLINTEND(google-build-using-namespace)

TEST(ReadToString, ReadsEntireContent)
{
    const String expected = "hello, world";
    Vec<UInt8> data(expected.begin(), expected.end());
    ByteArrayInputStream stream(data);

    auto result = ReadToString(stream);
    ASSERT_TRUE(result) << "ReadToString failed: " << result.Error();
    ASSERT_EQ(*result, expected);
}

TEST(ReadToString, EmptyStreamReturnsEmptyString)
{
    Vec<UInt8> data;
    ByteArrayInputStream stream(data);

    auto result = ReadToString(stream);
    ASSERT_TRUE(result) << "ReadToString failed: " << result.Error();
    ASSERT_TRUE(result->empty());
}

TEST(ReadToString, ReadsAcrossMultipleChunks)
{
    // Produce a string larger than the 4096-byte internal buffer
    const String pattern = "abcdefgh";
    String expected;
    expected.reserve(8192 + pattern.size());
    while (expected.size() < 8192) {
        expected += pattern;
    }

    Vec<UInt8> data(expected.begin(), expected.end());
    ByteArrayInputStream stream(data);

    auto result = ReadToString(stream);
    ASSERT_TRUE(result) << "ReadToString failed: " << result.Error();
    ASSERT_EQ(*result, expected);
}

TEST(ReadToString, ReadsExactlyOneBufBoundary)
{
    // Exactly 4096 bytes — the internal read buffer size — to hit the boundary cleanly
    const String expected(4096, 'x');
    Vec<UInt8> data(expected.begin(), expected.end());
    ByteArrayInputStream stream(data);

    auto result = ReadToString(stream);
    ASSERT_TRUE(result) << "ReadToString failed: " << result.Error();
    ASSERT_EQ(result->size(), 4096U);
    ASSERT_EQ(*result, expected);
}

TEST(ReadToString, StreamIsAtEOSAfterRead)
{
    const String expected = "violet";
    Vec<UInt8> data(expected.begin(), expected.end());
    ByteArrayInputStream stream(data);

    auto result = ReadToString(stream);
    ASSERT_TRUE(result);
    ASSERT_TRUE(stream.EOS());
    ASSERT_EQ(stream.Available().Unwrap(), 0U);
}

TEST(ReadToString, PreservesArbitraryBytes)
{
    // Ensure non-text bytes are copied verbatim (no encoding assumptions)
    Vec<UInt8> data = { 0x00, 0x01, 0xFF, 0xFE, 0x7F, 0x80 };
    ByteArrayInputStream stream(data);

    auto result = ReadToString(stream);
    ASSERT_TRUE(result) << "ReadToString failed: " << result.Error();

    const String& str = result.Unwrap();
    ASSERT_EQ(str.size(), 6U);
    ASSERT_EQ(static_cast<UInt8>(str[0]), 0x00U);
    ASSERT_EQ(static_cast<UInt8>(str[1]), 0x01U);
    ASSERT_EQ(static_cast<UInt8>(str[2]), 0xFFU);
    ASSERT_EQ(static_cast<UInt8>(str[3]), 0xFEU);
    ASSERT_EQ(static_cast<UInt8>(str[4]), 0x7FU);
    ASSERT_EQ(static_cast<UInt8>(str[5]), 0x80U);
}
