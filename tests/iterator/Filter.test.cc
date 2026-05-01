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
#include <violet/Iterator/Filter.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;
using namespace violet::testing::fixtures;

namespace {

constexpr auto isEven(Int32 num) -> bool
{
    return num % 2 == 0;
}

} // namespace

TEST(Iterators, FilterCanBeUsedWithMkIterable)
{
    Vec<UInt32> vi({ 1, 2, 3, 4, 5, 6 });

    auto filtered = MkIterable(vi).Filter([](UInt32 value) -> bool { return value % 2 == 0; });
    Vec<UInt32> expected({ 2, 4, 6 });

    ASSERT_EQ(filtered.Collect<Vec<UInt32>>(), expected);
}

TEST(Iterators, FilterFiltersEvenNumbers)
{
    auto iter = FixedSizeIterator<Int32, 6>({ 1, 2, 3, 4, 5, 6 }).Filter(isEven);

    auto a = iter.Next();
    ASSERT_TRUE(a);
    EXPECT_EQ(*a, 2);

    auto b = iter.Next();
    ASSERT_TRUE(b);
    EXPECT_EQ(*b, 4);

    auto c = iter.Next();
    ASSERT_TRUE(c);
    EXPECT_EQ(*c, 6);

    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, FilterNoMatchesYieldsNothing)
{
    auto iter = FixedSizeIterator<Int32, 4>({ 1, 3, 5, 7 }).Filter(isEven);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, FilterAllMatchesYieldsEverything)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 2, 4, 6 }).Filter(isEven);
    EXPECT_EQ(*iter.Next(), 2);
    EXPECT_EQ(*iter.Next(), 4);
    EXPECT_EQ(*iter.Next(), 6);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, FilterEmptyIterator)
{
    struct Empty final: public Iterator<Empty> {
        using Item = Int32;

        auto Next() noexcept -> Optional<Item>
        {
            return Nothing;
        }
    };

    auto iter = Empty().Filter(std::identity{ });
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, FilterSingleMatchingElement)
{
    auto iter = FixedSizeIterator<Int32, 1>({ 42 }).Filter([](Int32) -> bool { return true; });
    auto elem = iter.Next();
    ASSERT_TRUE(elem);
    EXPECT_EQ(*elem, 42);
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, FilterSingleNonMatchingElement)
{
    auto iter = FixedSizeIterator<Int32, 1>({ 42 }).Filter([](Int32) -> bool { return false; });
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, FilterPredicateReceivesCorrectValues)
{
    Vec<Int32> seen;
    auto iter = FixedSizeIterator<Int32, 3>({ 10, 20, 30 }).Filter([&seen](Int32 n) -> bool {
        seen.push_back(n);
        return n == 20;
    });

    auto result = iter.Next();
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 20);

    // The predicate should have been called with 10 (rejected) then 20 (accepted).
    ASSERT_EQ(seen.size(), 2U);
    EXPECT_EQ(seen[0], 10);
    EXPECT_EQ(seen[1], 20);
}

TEST(Iterators, FilterExhaustedIteratorRemainsExhausted)
{
    auto iter = FixedSizeIterator<Int32, 3>({ 1, 2 }).Filter([](Int32 num) -> bool { return num == 1; });

    ASSERT_TRUE(iter.Next());
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
    EXPECT_FALSE(iter.Next());
}

TEST(Iterators, FilterNextBackFiltersFromEnd)
{
    auto iter = DoubleEndedFixedSizeIterator<Int32, 6>({ 1, 2, 3, 4, 5, 6 }).Filter(isEven);

    auto a = iter.NextBack();
    ASSERT_TRUE(a);
    EXPECT_EQ(*a, 6);

    auto b = iter.NextBack();
    ASSERT_TRUE(b);
    EXPECT_EQ(*b, 4);

    auto c = iter.NextBack();
    ASSERT_TRUE(c);
    EXPECT_EQ(*c, 2);

    EXPECT_FALSE(iter.NextBack());
}

TEST(Iterators, FilterNextBackNoMatchesYieldsNothing)
{
    auto iter = DoubleEndedFixedSizeIterator<Int32, 3>({ 1, 3, 5 }).Filter(isEven);
    EXPECT_FALSE(iter.NextBack());
}

TEST(Iterators, FilterSizeHintLowerBoundIsZero)
{
    auto iter = FixedSizeIterator<Int32, 5>({ 1, 2, 3, 4, 5 }).Filter([](Int32 num) -> bool { return num > 3; });
    auto hint = iter.SizeHint();

    EXPECT_EQ(hint.Low, 0U);
    EXPECT_EQ(hint.High, 5U);
}

TEST(Iterators, FilterSizeHintAfterPartialConsumption)
{
    auto iter = DoubleEndedFixedSizeIterator<Int32, 4>({ 1, 2, 3, 4 }).Filter(isEven);

    // Consume one element from the front (skips 1, yields 2).
    (void)iter.Next();

    auto hint = iter.SizeHint();
    EXPECT_EQ(hint.Low, 0U);

    // Upper bound reflects the underlying iterator's remaining count.
    EXPECT_LE(hint.High, 4U);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
