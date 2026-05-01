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
#include <violet/Iterator/Take.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;
using namespace violet::testing::fixtures;

TEST(Iterators, TakeTakesFirstNElements)
{
    auto iter = FixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Take(3);
    EXPECT_EQ(*iter.Next(), 1);
    EXPECT_EQ(*iter.Next(), 2);
    EXPECT_EQ(*iter.Next(), 3);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, TakeTakesZeroYieldsNothing)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Take(0);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, TakeTakesExactLength)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Take(3);
    EXPECT_EQ(*iter.Next(), 1);
    EXPECT_EQ(*iter.Next(), 2);
    EXPECT_EQ(*iter.Next(), 3);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, TakeTakesMoreThanLength)
{
    auto iter = FixedSizeIterator<Int32, 2>({ 1, 2 }).Take(100);
    EXPECT_EQ(*iter.Next(), 1);
    EXPECT_EQ(*iter.Next(), 2);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, TakeTakesOnEmptyIterator)
{
    struct Empty final: public Iterator<Empty> {
        using Item = Int32;

        auto Next() noexcept -> Optional<Item>
        {
            return Nothing;
        }
    };

    auto iter = Empty().Take(5);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, TakeTakesSingleElement)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 10, 20, 30 }).Take(1);
    auto elem = iter.Next();
    ASSERT_TRUE(elem);
    EXPECT_EQ(*elem, 10);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, TakeExhaustedIteratorRemainsExhausted)
{
    auto iter = FixedSizeIterator<Int32, 2>({ 1, 2 }).Take(1);
    ASSERT_TRUE(iter.Next());
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, TakeNextBackTakesFromEnd)
{
    auto iter = DoubleEndedFixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Take(3);
    auto a = iter.NextBack();
    ASSERT_TRUE(a);
    EXPECT_EQ(*a, 5);

    auto b = iter.NextBack();
    ASSERT_TRUE(b);
    EXPECT_EQ(*b, 4);

    auto c = iter.NextBack();
    ASSERT_TRUE(c);
    EXPECT_EQ(*c, 3);

    EXPECT_FALSE(iter.NextBack());
}

TEST(Iterators, TakeNextAndNextBackShareRemainingCount)
{
    auto iter = DoubleEndedFixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Take(3);

    // Consume from front.
    EXPECT_EQ(*iter.Next(), 1);

    // Consume from back.
    EXPECT_EQ(*iter.NextBack(), 5);

    // One remaining.
    EXPECT_EQ(*iter.Next(), 2);

    // Limit reached.
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.NextBack());
}

TEST(Iterators, TakeNextBackWithTakeZero)
{
    auto iter = DoubleEndedFixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Take(0);
    EXPECT_FALSE(iter.NextBack());
}

TEST(Iterators, TakeSizeHintCappedByRemaining)
{
    auto iter = FixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Take(3);
    auto hint = iter.SizeHint();
    EXPECT_EQ(hint.Low, 3U);
    ASSERT_TRUE(hint.High);
    EXPECT_EQ(*hint.High, 3U);
}

TEST(Iterators, TakeSizeHintWhenTakeExceedsLength)
{
    auto iter = FixedSizeIterator<Int32, 2>({ 1, 2 }).Take(100);
    auto hint = iter.SizeHint();

    // Low is capped at the underlying iterator's size.
    EXPECT_EQ(hint.Low, 2U);
    ASSERT_TRUE(hint.High);

    // High is capped at the underlying iterator's size.
    EXPECT_EQ(*hint.High, 2U);
}

TEST(Iterators, TakeSizeHintAfterPartialConsumption)
{
    auto iter = FixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Take(3);
    (void)iter.Next();

    auto hint = iter.SizeHint();
    EXPECT_LE(hint.Low, 2U);
    ASSERT_TRUE(hint.High);
    EXPECT_LE(*hint.High, 2U);
}

TEST(Iterators, TakeSizeHintWithTakeZero)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Take(0);
    auto hint = iter.SizeHint();
    EXPECT_EQ(hint.Low, 0U);
    ASSERT_TRUE(hint.High);
    EXPECT_EQ(*hint.High, 0U);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
