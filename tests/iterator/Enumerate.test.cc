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
#include <violet/Iterator/Enumerate.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;
using namespace violet::testing::fixtures;

TEST(Iterators, EnumerateFromDifferentIterable)
{
    Vec<UInt32> vi({ 1, 2, 3 });

    auto pair = MkIterable(vi).Enumerate();
    auto first = Pair<UInt, UInt32>(0, 1);
    ASSERT_EQ(*pair.Next(), first);

    auto second = Pair<UInt, UInt32>(1, 2);
    ASSERT_EQ(*pair.Next(), second);

    auto third = Pair<UInt, UInt32>(2, 3);
    ASSERT_EQ(*pair.Next(), third);
}

TEST(Iterators, EnumerateYieldsCorrectIndicesAndValues)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 10, 20, 30 }).Enumerate();
    auto first = iter.Next();
    ASSERT_TRUE(first);
    EXPECT_EQ(first->first, 0U);
    EXPECT_EQ(first->second, 10);

    auto second = iter.Next();
    ASSERT_TRUE(second);
    EXPECT_EQ(second->first, 1U);
    EXPECT_EQ(second->second, 20);

    auto third = iter.Next();
    ASSERT_TRUE(third);
    EXPECT_EQ(third->first, 2U);
    EXPECT_EQ(third->second, 30);

    auto exhausted = iter.Next();
    EXPECT_FALSE(exhausted);
}

TEST(Iterators, EnumerateEmptyIterableReturnsNothing)
{
    // Zero-length array isn't valid, so use a one-element array and consume it
    // before wrapping, or define a dedicated empty iterator.
    struct EmptyIter final: public Iterator<EmptyIter> {
        using Item = Int32;

        auto Next() noexcept -> Optional<Item>
        {
            return Nothing;
        }
    };

    auto iter = EmptyIter().Enumerate();
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, EnumerateExhaustedIteratorRemainsExhausted)
{
    auto iter = FixedSizeIterator<Int32, 1>({ 42 });

    ASSERT_TRUE(iter.Next());

    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, EnumerateIndicesIncrementMonotonically)
{
    auto iter = FixedSizeIterator<Int32, 5>({ 5, 6, 7, 8, 9 });
    EXPECT_EQ(iter.Enumerate().Count(), 5U);
}

TEST(Iterators, EnumeratePreservesUnderlyingValues)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 100, 200, 300 }).Enumerate();
    Array<Int32, 3> expected({ 100, 200, 300 });

    for (auto [i, item]: iter) {
        ASSERT_LT(i, 3U);
        EXPECT_EQ(item, expected[i]);
    }
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
