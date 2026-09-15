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
#include <violet/Experimental/Collections/HashSet.h>

namespace violet::experimental {

TEST(HashSet, DefaultConstructsEmpty)
{
    HashSet<Int32> s;
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Size(), 0U);
}

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
TEST(HashSet, CapacityConstructorReservesSpace)
{
    HashSet<Int32> s(64);
    EXPECT_GE(s.Capacity(), 64U);
    EXPECT_TRUE(s.Empty());
}
#endif

TEST(HashSet, InitializerListPopulates)
{
    HashSet<Int32> s{1, 2, 3, 4, 5};
    EXPECT_EQ(s.Size(), 5U);
    for (Int32 i = 1; i <= 5; ++i) {
        EXPECT_TRUE(s.Contains(i));
    }
}

TEST(HashSet, InitializerListDeduplicates)
{
    HashSet<Int32> s{1, 2, 2, 3, 3, 3};
    EXPECT_EQ(s.Size(), 3U);
}

TEST(HashSet, SizeReflectsInsertions)
{
    HashSet<Int32> s;
    s.Insert(1);
    s.Insert(2);
    EXPECT_EQ(s.Size(), 2U);
}

TEST(HashSet, ClearEmptiesSet)
{
    HashSet<Int32> s{1, 2, 3};
    s.Clear();
    EXPECT_TRUE(s.Empty());
    EXPECT_FALSE(s.Contains(1));
}

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
TEST(HashSet, ReserveGrowsCapacity)
{
    HashSet<Int32> s;
    const auto initial = s.Capacity();
    s.Reserve(128);
    EXPECT_GE(s.Capacity(), initial + 128);
}
#endif

TEST(HashSet, InsertReturnsTrueForNewValue)
{
    HashSet<Int32> s;
    EXPECT_TRUE(s.Insert(1));
    EXPECT_EQ(s.Size(), 1U);
}

TEST(HashSet, InsertReturnsFalseForExistingValue)
{
    HashSet<Int32> s;
    s.Insert(1);
    EXPECT_FALSE(s.Insert(1));
    EXPECT_EQ(s.Size(), 1U);
}

TEST(HashSet, InsertMoveOnlyValueType)
{
    HashSet<std::unique_ptr<Int32>> s;
    s.Insert(std::make_unique<Int32>(42));
    EXPECT_EQ(s.Size(), 1U);
}

TEST(HashSet, ReplaceOnAbsentReturnsNothing)
{
    HashSet<Int32> s;
    auto old = s.Replace(1);
    EXPECT_FALSE(old);
    EXPECT_TRUE(s.Contains(1));
}

TEST(HashSet, ReplaceOnPresentReturnsOldValue)
{
    HashSet<Int32> s;
    s.Insert(1);
    auto old = s.Replace(1);
    ASSERT_TRUE(old.HasValue());
    EXPECT_EQ(*old, 1);
    EXPECT_EQ(s.Size(), 1U);
}

TEST(HashSet, ContainsReturnsCorrectly)
{
    HashSet<Int32> s{1, 2, 3};
    EXPECT_TRUE(s.Contains(1));
    EXPECT_TRUE(s.Contains(2));
    EXPECT_FALSE(s.Contains(4));
}

TEST(HashSet, GetReturnsReferenceToStoredValue)
{
    HashSet<Int32> s{42};
    auto v = s.Get(42);
    ASSERT_TRUE(v.HasValue());
    EXPECT_EQ(*v, 42);
}

TEST(HashSet, GetReturnsNothingForAbsent)
{
    HashSet<Int32> s{1, 2, 3};
    EXPECT_FALSE(s.Get(99));
}

TEST(HashSet, TransparentLookupWithStringLiteral)
{
    HashSet<String> s;
    s.Insert("hello");
    EXPECT_TRUE(s.Contains("hello"));
    EXPECT_FALSE(s.Contains("world"));
}

TEST(HashSet, RemoveReturnsTrueForPresent)
{
    HashSet<Int32> s{1, 2, 3};
    EXPECT_TRUE(s.Remove(2));
    EXPECT_EQ(s.Size(), 2U);
    EXPECT_FALSE(s.Contains(2));
}

TEST(HashSet, RemoveReturnsFalseForAbsent)
{
    HashSet<Int32> s{1, 2, 3};
    EXPECT_FALSE(s.Remove(99));
    EXPECT_EQ(s.Size(), 3U);
}

TEST(HashSet, TakeReturnsRemovedValue)
{
    HashSet<Int32> s{1, 2, 3};
    auto taken = s.Take(2);
    ASSERT_TRUE(taken);
    EXPECT_EQ(*taken, 2);
    EXPECT_EQ(s.Size(), 2U);
    EXPECT_FALSE(s.Contains(2));
}

TEST(HashSet, TakeAbsentReturnsNothing)
{
    HashSet<Int32> s;
    EXPECT_FALSE(s.Take(1));
}

TEST(HashSet, RetainKeepsMatching)
{
    HashSet<Int32> s{1, 2, 3, 4, 5};
    s.Retain([](const Int32& v) -> bool { return v % 2 == 0; });
    EXPECT_EQ(s.Size(), 2U);
    EXPECT_TRUE(s.Contains(2));
    EXPECT_TRUE(s.Contains(4));
    EXPECT_FALSE(s.Contains(1));
}

TEST(HashSet, RetainEmptyResultWhenNoneMatch)
{
    HashSet<Int32> s{1, 2, 3};
    s.Retain([](const Int32&) -> bool { return false; });
    EXPECT_TRUE(s.Empty());
}

TEST(HashSet, ExtractIfPassesRemovedToSink)
{
    HashSet<Int32> s{1, 2, 3, 4, 5};
    Int32 sum = 0;
    s.ExtractIf([](const Int32& v) -> bool { return v >= 3; }, [&](Int32&& v) -> void { sum += VIOLET_MOVE(v); });

    EXPECT_EQ(sum, 12); // 3 + 4 + 5
    EXPECT_EQ(s.Size(), 2U);
    EXPECT_TRUE(s.Contains(1));
    EXPECT_TRUE(s.Contains(2));
}

TEST(HashSet, DisjointedWithEmptyIsTrue)
{
    HashSet<Int32> a{1, 2, 3};
    HashSet<Int32> b;
    EXPECT_TRUE(a.Disjointed(b));
    EXPECT_TRUE(b.Disjointed(a));
}

TEST(HashSet, DisjointedDetectsSharedElement)
{
    HashSet<Int32> a{1, 2, 3};
    HashSet<Int32> b{3, 4, 5};
    EXPECT_FALSE(a.Disjointed(b));
    EXPECT_FALSE(b.Disjointed(a));
}

TEST(HashSet, DisjointedTrueForNonOverlapping)
{
    HashSet<Int32> a{1, 2, 3};
    HashSet<Int32> b{4, 5, 6};
    EXPECT_TRUE(a.Disjointed(b));
}

TEST(HashSet, SubsetOfReflexive)
{
    HashSet<Int32> s{1, 2, 3};
    EXPECT_TRUE(s.SubsetOf(s));
}

TEST(HashSet, SubsetOfEmptyIsTrue)
{
    HashSet<Int32> empty_;
    HashSet<Int32> nonempty{1, 2, 3};
    EXPECT_TRUE(empty_.SubsetOf(nonempty));
    EXPECT_FALSE(nonempty.SubsetOf(empty_));
}

TEST(HashSet, SubsetOfProperSubset)
{
    HashSet<Int32> small_{1, 2};
    HashSet<Int32> big{1, 2, 3};
    EXPECT_TRUE(small_.SubsetOf(big));
    EXPECT_FALSE(big.SubsetOf(small_));
}

TEST(HashSet, SubsetRejectsPartialOverlap)
{
    HashSet<Int32> a{1, 2, 3};
    HashSet<Int32> b{2, 3, 4};
    EXPECT_FALSE(a.SubsetOf(b));
    EXPECT_FALSE(b.SubsetOf(a));
}

TEST(HashSet, SupersetOfInverseOfSubset)
{
    HashSet<Int32> small_{1, 2};
    HashSet<Int32> big{1, 2, 3};
    EXPECT_TRUE(big.SupersetOf(small_));
    EXPECT_FALSE(small_.SupersetOf(big));
}

TEST(HashSet, RangeBasedForIteratesAllValues)
{
    HashSet<Int32> s{1, 2, 3, 4, 5};
    Int32 sum = 0;
    for (const auto& v: s) {
        sum += v;
    }
    EXPECT_EQ(sum, 15);
}

TEST(HashSet, EmptySetIterationIsNoOp)
{
    HashSet<Int32> s;
    Int32 count = 0;
    for (const auto& v: s) {
        (void)v;
        ++count;
    }
    EXPECT_EQ(count, 0);
}

TEST(HashSet, DeduplicateStream)
{
    HashSet<Int32> seen;
    Vec<Int32> unique;
    const Int32 stream[] = {1, 2, 1, 3, 2, 4, 1, 5, 3};
    for (auto v: stream) {
        if (seen.Insert(v)) {
            unique.push_back(v);
        }
    }

    EXPECT_EQ(unique.size(), 5U);
    EXPECT_EQ(unique[0], 1);
    EXPECT_EQ(unique[1], 2);
    EXPECT_EQ(unique[2], 3);
    EXPECT_EQ(unique[3], 4);
    EXPECT_EQ(unique[4], 5);
}

TEST(HashSet, PermissionCheckUsingSubset)
{
    HashSet<String> user_permissions{"read", "write"};
    HashSet<String> required_read{"read"};
    HashSet<String> required_admin{"read", "write", "admin"};

    EXPECT_TRUE(required_read.SubsetOf(user_permissions));
    EXPECT_FALSE(required_admin.SubsetOf(user_permissions));
}

TEST(HashSet, LargeInsertLookupStress)
{
    HashSet<Int32> s(10'000);
    for (Int32 i = 0; i < 10'000; ++i) {
        EXPECT_TRUE(s.Insert(i));
    }

    EXPECT_EQ(s.Size(), 10'000U);
    for (Int32 i = 0; i < 10'000; ++i) {
        EXPECT_TRUE(s.Contains(i));
    }

    EXPECT_FALSE(s.Contains(10'000));
}

} // namespace violet::experimental
