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

#include "tests/iterator/fixtures/FixedSizeIterator.h"

#include <gtest/gtest.h>
#include <violet/Iterator/Map.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;
using namespace violet::testing::fixtures;

TEST(Iterators, Map)
{
    Vec<UInt32> vi({ 1, 2, 3, 4 });

    auto pow2 = MkIterable(vi).Map([](UInt32 value) -> UInt32 { return value * 2; });

    Vec<UInt32> expected({ 2, 4, 6, 8 });
    ASSERT_EQ(pow2.Collect<Vec<UInt32>>(), expected);
}

TEST(Iterators, MapTransformsElements)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Map([](Int32 num) -> Int32 { return num * 10; });
    EXPECT_EQ(*iter.Next(), 10);
    EXPECT_EQ(*iter.Next(), 20);
    EXPECT_EQ(*iter.Next(), 30);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, MapChangesElementType)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Map([](Int32 num) -> String { return violet::ToString(num); });

    EXPECT_EQ(*iter.Next(), "1");
    EXPECT_EQ(*iter.Next(), "2");
    EXPECT_EQ(*iter.Next(), "3");
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, MapEmptyIteratorReturnsNothing)
{
    struct Empty final: public Iterator<Empty> {
        using Item = Int32;

        auto Next() noexcept -> Optional<Item>
        {
            return Nothing;
        }
    };

    auto iter = Empty().Map([](auto) -> bool { return true; });
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, MapSingleElement)
{
    auto iter = FixedSizeIterator<Int32, 1>({ 42 }).Map([](Int32 num) -> Int32 { return num + 1; });
    auto elem = iter.Next();
    ASSERT_TRUE(elem);
    EXPECT_EQ(*elem, 43);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, MapFunctionCalledExactlyOncePerElement)
{
    Int32 calls = 0;
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Map([&calls](Int32 num) -> Int32 {
        ++calls;
        return num;
    });

    (void)iter.Next();
    EXPECT_EQ(calls, 1);

    (void)iter.Next();
    EXPECT_EQ(calls, 2);

    (void)iter.Next();
    EXPECT_EQ(calls, 3);

    // No additional calls after exhaustion.
    (void)iter.Next();
    EXPECT_EQ(calls, 3);
}

TEST(Iterators, MapFunctionReceivesCorrectValues)
{
    Vec<Int32> seen;
    auto iter = FixedSizeIterator<Int32, 3>({ 10, 20, 30 }).Map([&seen](Int32 n) -> bool {
        seen.push_back(n);
        return n;
    });

    while (iter.Next()) { }

    ASSERT_EQ(seen.size(), 3U);
    EXPECT_EQ(seen[0], 10);
    EXPECT_EQ(seen[1], 20);
    EXPECT_EQ(seen[2], 30);
}

TEST(Iterators, MapExhaustedIteratorRemainsExhausted)
{
    auto iter = FixedSizeIterator<Int32, 1>({ 1 }).Map([](Int32 num) -> Int32 { return num; });

    ASSERT_TRUE(iter.Next());
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, MapToBoolean)
{
    auto iter = FixedSizeIterator<Int32, 4>({ 1, 2, 3, 4 }).Map([](Int32 num) -> bool { return num % 2 == 0; });

    EXPECT_EQ(*iter.Next(), false);
    EXPECT_EQ(*iter.Next(), true);
    EXPECT_EQ(*iter.Next(), false);
    EXPECT_EQ(*iter.Next(), true);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, MapWithComplexTransform)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 3, 1, 4 }).Map([](Int32 num) -> Pair<Int32, Int32> {
        return std::make_pair(num, num * num);
    });

    auto a = iter.Next();
    ASSERT_TRUE(a);
    EXPECT_EQ(a->first, 3);
    EXPECT_EQ(a->second, 9);

    auto b = iter.Next();
    ASSERT_TRUE(b);
    EXPECT_EQ(b->first, 1);
    EXPECT_EQ(b->second, 1);

    auto c = iter.Next();
    ASSERT_TRUE(c);
    EXPECT_EQ(c->first, 4);
    EXPECT_EQ(c->second, 16);

    EXPECT_FALSE(iter.Next());
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
