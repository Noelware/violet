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
#include <violet/Iterator/Skip.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;
using namespace violet::testing::fixtures;

TEST(Iterators, SkipSkipsFirstNElements)
{
    auto iter = FixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Skip(2);
    EXPECT_EQ(*iter.Next(), 3);
    EXPECT_EQ(*iter.Next(), 4);
    EXPECT_EQ(*iter.Next(), 5);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, SkipSkipZeroYieldsAll)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Skip(0);
    EXPECT_EQ(*iter.Next(), 1);
    EXPECT_EQ(*iter.Next(), 2);
    EXPECT_EQ(*iter.Next(), 3);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, SkipSkipExactLength)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Skip(3);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, SkipSkipMoreThanLength)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Skip(100);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, SkipSkipOnEmptyIterator)
{
    struct Empty final: public Iterator<Empty> {
        using Item = Int32;

        auto Next() noexcept -> Optional<Item>
        {
            return Nothing;
        }
    };

    auto iter = Empty().Skip(5);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, SkipSkipSingleElement)
{
    auto iter = FixedSizeIterator<Int32, 2>({ 10, 20 }).Skip(1);
    auto elem = iter.Next();
    ASSERT_TRUE(elem);
    EXPECT_EQ(*elem, 20);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, SkipExhaustedIteratorRemainsExhausted)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Skip(2);
    ASSERT_TRUE(iter.Next());
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, SkipSizeHintReducedBySkipCount)
{
    auto iter = FixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Skip(2);
    auto hint = iter.SizeHint();
    EXPECT_EQ(hint.Low, 3U);
    ASSERT_TRUE(hint.High);
    EXPECT_EQ(*hint.High, 3U);
}

TEST(Iterators, SkipSizeHintClampedToZero)
{
    auto iter = FixedSizeIterator<Int32, 2>({ 1, 2 }).Skip(10);
    auto hint = iter.SizeHint();
    EXPECT_EQ(hint.Low, 0U);
    ASSERT_TRUE(hint.High);
    EXPECT_EQ(*hint.High, 0U);
}

TEST(Iterators, SkipSizeHintWithSkipZero)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2, 3 }).Skip(0);
    auto hint = iter.SizeHint();
    EXPECT_EQ(hint.Low, 3U);
    ASSERT_TRUE(hint.High);
    EXPECT_EQ(*hint.High, 3U);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
