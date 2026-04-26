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
#include <violet/Container/Result.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;

TEST(Err, ConstructFromLvalue)
{
    std::string msg = "failed";
    Err<std::string> err(msg);
    EXPECT_EQ(err.Error(), "failed");
}

TEST(Err, ConstructFromRvalue)
{
    Err<std::string> err(std::string("failed"));
    EXPECT_EQ(err.Error(), "failed");
}

TEST(Err, InPlaceConstruction)
{
    Err<std::string> err(5, 'x');
    EXPECT_EQ(err.Error(), "xxxxx");
}

TEST(Err, ConvertingMoveConstruct)
{
    // Assumes io::Error is convertible to anyhow::Error or similar.
    // Use int -> long as a stand-in.
    Err<Int32> inner(42);
    Err<Int64> outer(VIOLET_MOVE(inner));
    EXPECT_EQ(outer.Error(), 42L);
}

TEST(Err, Equality)
{
    Err<Int32> a(1);
    Err<Int32> b(1);
    Err<Int32> c(2);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(Err, ErrorRefQualifiers)
{
    Err<std::string> err("hello");

    // lvalue
    EXPECT_EQ(err.Error(), "hello");

    // const lvalue
    const auto& cref = err;
    EXPECT_EQ(cref.Error(), "hello");

    // rvalue
    auto moved = VIOLET_MOVE(err).Error();
    EXPECT_EQ(moved, "hello");
}

TEST(Err, ToString)
{
    Err<Int32> err(42);
    auto s = err.ToString();
    EXPECT_FALSE(s.empty());
}

TEST(Err, StreamOperator)
{
    Err<Int32> err(42);
    std::ostringstream os;
    os << err;
    EXPECT_FALSE(os.str().empty());
}

TEST(ErrConstexpr, ConstructAndAccess)
{
    constexpr Err<int> err(42);
    static_assert(err.Error() == 42);
}

TEST(ErrConstexpr, Equality)
{
    constexpr Err<int> a(1);
    constexpr Err<int> b(1);
    constexpr Err<int> c(2);

    static_assert(a == b);
    static_assert(a != c);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
