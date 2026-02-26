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
#include <violet/IO/Experimental/Output/ByteArrayOutputStream.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::io::experimental;
using namespace violet;
// NOLINTEND(google-build-using-namespace)

TEST(ByteArrayOutputStream, WriteAndFlush)
{
    ByteArrayOutputStream baos;

    Span<const UInt8> data(reinterpret_cast<const UInt8*>("hello"), 5);
    auto written = baos.Write(data);
    ASSERT_TRUE(written) << "failed to write: " << written.Error();
    EXPECT_EQ(written.Value(), 5);

    ASSERT_TRUE(baos.Flush());
    EXPECT_EQ(baos.Get().size(), 5);
    EXPECT_EQ(String(baos.Get().begin(), baos.Get().end()), "hello");
}
