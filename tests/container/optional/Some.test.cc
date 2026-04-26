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
#include <violet/Container/Optional.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;

TEST(Some, ConstructFromLvalue)
{
    int val = 42;
    Some<Int32> s(val);
    EXPECT_EQ(s.Value(), 42);
}

TEST(Some, ConstructFromRvalue)
{
    Some<std::string> s(std::string("hello"));
    EXPECT_EQ(s.Value(), "hello");
}

TEST(Some, InPlaceConstruction)
{
    Some<std::string> s(5, 'x');
    EXPECT_EQ(s.Value(), "xxxxx");
}

TEST(Some, ConvertingMoveConstruct)
{
    Some<Int32> inner(42);
    Some<Int64> outer(VIOLET_MOVE(inner));
    EXPECT_EQ(outer.Value(), 42L);
}

TEST(Some, ToString)
{
    Some<Int32> s(42);
    EXPECT_FALSE(s.ToString().empty());
}

TEST(Some, StreamOperator)
{
    Some<Int32> s(42);
    std::ostringstream os;
    os << s;
    EXPECT_FALSE(os.str().empty());
}

TEST(SomeConstexpr, Construct)
{
    constexpr Some<Int32> s(42);
    static_assert(s.Value() == 42);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
