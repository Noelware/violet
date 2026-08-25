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

namespace violet {
// NOLINTBEGIN(readability-identifier-length)

TEST(ResultErr, ConstructFromLValue)
{
    Err<String> error("failed");
    EXPECT_EQ(error.Error(), "failed");
}

TEST(ResultErr, ConstructFromRValue)
{
    Err error(String("failed"));
    EXPECT_EQ(error.Error(), "failed");
}

TEST(ResultErr, InPlaceConstruction)
{
    Err<String> error(5, 'x');
    EXPECT_EQ(error.Error(), "xxxxx");
}

TEST(ResultErr, ConvertingMoveConstruct)
{
    Err inner(42);
    Err<Int64> outer(VIOLET_MOVE(inner));
    EXPECT_EQ(outer.Error(), 42L);
}

TEST(ResultErr, Equality)
{
    Err a(1);
    Err b(1);
    Err c(2);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST(ResultErr, RefQualifiers)
{
    Err<String> error("hello");
    EXPECT_EQ(error.Error(), "hello"); // lvalue

    // const lvalue
    const auto& cref = error;
    EXPECT_EQ(cref.Error(), "hello");

    // rvalue
    auto moved = VIOLET_MOVE(error).Error();
    EXPECT_EQ(moved, "hello");
}

namespace {
consteval auto constructAndAccess() noexcept -> bool
{
    constexpr Err error(42);
    return error.Error() == 42;
}

static_assert(constructAndAccess());

consteval auto equality() noexcept -> bool
{
    constexpr Err a(42);
    constexpr Err b(42);
    constexpr Err c(420);

    return a == b && a != c;
}

static_assert(equality());
} // namespace

// NOLINTEND(readability-identifier-length)
} // namespace violet
