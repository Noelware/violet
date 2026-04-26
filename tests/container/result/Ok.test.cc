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

TEST(Ok, ConstructFromLvalue)
{
    Int32 val = 42;
    Ok<Int32> ok(val);
    EXPECT_EQ(ok.Value(), 42);
}

TEST(Ok, ConstructFromRvalue)
{
    Ok<std::string> ok(std::string("hello"));
    EXPECT_EQ(ok.Value(), "hello");
}

TEST(Ok, InPlaceConstruction)
{
    Ok<std::string> ok(3, 'a');
    EXPECT_EQ(ok.Value(), "aaa");
}

TEST(Ok, ConvertingMoveConstruct)
{
    Ok<Int32> inner(42);
    Ok<Int64> outer(VIOLET_MOVE(inner));
    EXPECT_EQ(outer.Value(), 42L);
}

TEST(Ok, Equality)
{
    Ok<Int32> a(1);
    Ok<Int32> b(1);
    Ok<Int32> c(2);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(Ok, ValueRefQualifiers)
{
    Ok<std::string> ok("hello");

    EXPECT_EQ(ok.Value(), "hello");

    const auto& cref = ok;
    EXPECT_EQ(cref.Value(), "hello");

    auto moved = VIOLET_MOVE(ok).Value();
    EXPECT_EQ(moved, "hello");
}

TEST(OkConstexpr, ConstructAndAccess)
{
    constexpr Ok<Int32> ok(42);
    static_assert(ok.Value() == 42);
}

TEST(OkConstexpr, Equality)
{
    constexpr Ok<Int32> a(1);
    constexpr Ok<Int32> b(1);
    constexpr Ok<Int32> c(2);

    static_assert(a == b);
    static_assert(a != c);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
