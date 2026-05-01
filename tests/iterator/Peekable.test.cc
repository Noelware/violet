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
#include <violet/Iterator/Peekable.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;
using namespace violet::testing::fixtures;

TEST(Iterators, PeekablePeekReturnsNextWithoutConsuming)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 10, 20, 30 }).Peekable();
    auto peeked = iter.Peek();
    ASSERT_TRUE(peeked);
    EXPECT_EQ(*peeked, 10);

    // Peek again; same value, not advanced.
    auto peeked_again = iter.Peek();
    ASSERT_TRUE(peeked_again);
    EXPECT_EQ(*peeked_again, 10);
}

TEST(Iterators, PeekableNextReturnsPeekedValue)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 10, 20, 30 }).Peekable();

    (void)iter.Peek();

    auto first = iter.Next();
    ASSERT_TRUE(first);
    EXPECT_EQ(*first, 10);

    // Next element is now 20.
    auto second = iter.Next();
    ASSERT_TRUE(second);
    EXPECT_EQ(*second, 20);
}

TEST(Iterators, PeekableNextWithoutPeek)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Peekable();
    EXPECT_EQ(*iter.Next(), 1);
    EXPECT_EQ(*iter.Next(), 2);
    EXPECT_EQ(*iter.Next(), 3);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, PeekablePeekThenNextAlternating)
{
    auto iter = FixedSizeIterator<Int32, 4>({ 1, 2, 3, 4 }).Peekable();

    // Peek at 1, consume 1.
    EXPECT_EQ(*iter.Peek(), 1);
    EXPECT_EQ(*iter.Next(), 1);

    // Consume 2 directly.
    EXPECT_EQ(*iter.Next(), 2);

    // Peek at 3, consume 3.
    EXPECT_EQ(*iter.Peek(), 3);
    EXPECT_EQ(*iter.Next(), 3);

    // Peek at 4, consume 4.
    EXPECT_EQ(*iter.Peek(), 4);
    EXPECT_EQ(*iter.Next(), 4);

    EXPECT_FALSE(iter.Peek());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, PeekableEmptyIteratorPeekReturnsNothing)
{
    struct Empty final: public Iterator<Empty> {
        using Item = Int32;

        auto Next() noexcept -> Optional<Item>
        {
            return Nothing;
        }
    };

    auto iter = Empty().Peekable();

    EXPECT_FALSE(iter.Peek());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, PeekablePeekOnExhaustedIteratorReturnsNothing)
{
    auto iter = FixedSizeIterator<Int32, 1>({ 42 }).Peekable();
    EXPECT_EQ(*iter.Next(), 42);
    EXPECT_FALSE(iter.Peek());
    EXPECT_FALSE(iter.Peek());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, PeekableMultiplePeeksDoNotAdvanceUnderlying)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 5, 6, 7 }).Peekable();
    for (int i = 0; i < 10; ++i) {
        auto peeked = iter.Peek();
        ASSERT_TRUE(peeked);
        EXPECT_EQ(*peeked, 5);
    }

    // All elements are still available.
    EXPECT_EQ(*iter.Next(), 5);
    EXPECT_EQ(*iter.Next(), 6);
    EXPECT_EQ(*iter.Next(), 7);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, PeekablePeekAfterPartialConsumption)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Peekable();
    EXPECT_EQ(*iter.Next(), 1);

    auto peeked = iter.Peek();
    ASSERT_TRUE(peeked);
    EXPECT_EQ(*peeked, 2);

    EXPECT_EQ(*iter.Next(), 2);
    EXPECT_EQ(*iter.Next(), 3);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, PeekableSingleElement)
{
    auto iter = FixedSizeIterator<Int32, 1>({ 99 }).Peekable();
    EXPECT_EQ(*iter.Peek(), 99);
    EXPECT_EQ(*iter.Peek(), 99);
    EXPECT_EQ(*iter.Next(), 99);
    EXPECT_FALSE(iter.Peek());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, PeekablePeekDoesNotSkipElements)
{
    auto iter = FixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Peekable();
    Vec<Int32> collected;

    // Peek before every Next to ensure nothing is lost.
    while (iter.Peek()) {
        collected.push_back(*iter.Next());
    }

    ASSERT_EQ(collected.size(), 5U);
    EXPECT_EQ(collected[0], 1);
    EXPECT_EQ(collected[1], 2);
    EXPECT_EQ(collected[2], 3);
    EXPECT_EQ(collected[3], 4);
    EXPECT_EQ(collected[4], 5);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
