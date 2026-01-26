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
#include <violet/IO/Experimental/BufferedInputStream.h>
#include <violet/IO/Experimental/Input/ByteArrayInputStream.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::io::experimental;
using namespace violet;
// NOLINTEND(google-build-using-namespace)

TEST(BufferedInputStream, ItWorks)
{
    Vec<UInt8> buf(1024);
    std::iota(buf.data(), buf.data() + buf.size(), 0);

    ByteArrayInputStream bas(buf);
    BufferedInputStream stream(bas, 128);

    Vec<UInt8> out(1024);
    auto res = stream.Read(out);
    ASSERT_TRUE(res) << "failed to call 'stream.Read()': " << VIOLET_MOVE(res.Error()).ToString();
    EXPECT_EQ(res.Value(), 1024);
    EXPECT_EQ(out, buf);
}

// TODO(@auguwu/Noel): add more tests here for correctness
