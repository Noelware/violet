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
#include <violet/Experimental/Collections/HashMap.h>
#include <violet/Experimental/Own.h>

namespace violet::experimental {

TEST(HashMap, DefaultConstructsEmpty)
{
    HashMap<Int32, Int32> m;
    EXPECT_TRUE(m.Empty());
    EXPECT_EQ(m.Size(), 0U);
}

TEST(HashMap, InitializerListPopulates)
{
    HashMap<Int32, Int32> m{{1, 10}, {2, 20}, {3, 30}};
    EXPECT_EQ(m.Size(), 3U);
    EXPECT_TRUE(m.Contains(1));
    EXPECT_TRUE(m.Contains(2));
    EXPECT_TRUE(m.Contains(3));
}

TEST(HashMap, SizeReflectsInsertions)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);
    (void)m.Insert(2, 20);
    EXPECT_EQ(m.Size(), 2U);
}

TEST(HashMap, InsertNewKeyReturnsNothing)
{
    HashMap<Int32, Int32> m;
    auto result = m.Insert(1, 10);
    EXPECT_FALSE(result) << "key `1` shouldn't be inserted";
    EXPECT_EQ(*m.Get(1), 10);
}

TEST(HashMap, InsertMoveOnlyValueType)
{
    HashMap<Int32, std::shared_ptr<Int32>> m;
    (void)m.Insert(1, std::make_shared<Int32>(42));
    ASSERT_TRUE(m.Contains(1));
    EXPECT_EQ(**m.Get(1), 42);

    HashMap<Int32, Own<Int32>> h;
    (void)h.Insert(1, Own<Int32>::New(5));
    ASSERT_TRUE(h.Contains(1));
    EXPECT_EQ(**h.Get(1), 5);
}

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
TEST(HashMap, CapacityConstructorReservesSpace)
{
    HashMap<Int32, Int32> m(64);
    EXPECT_GE(m.Capacity(), 64U);
    EXPECT_TRUE(m.Empty());
}

TEST(HashMap, ShrinkToFitRehashesDown)
{
    HashMap<Int32, Int32> m(1024);
    (void)m.Insert(1, 10);

    const auto before = m.Capacity();
    m.ShrinkToFit();
    EXPECT_LT(m.Capacity(), before);
    EXPECT_EQ(m.Size(), 1U);
}
#endif

TEST(HashMap, GetReturnsNothingForAbsentKey)
{
    HashMap<Int32, Int32> m;
    EXPECT_FALSE(m.Get(1));
}

TEST(HashMap, GetReturnsValueForPresentKey)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 42);

    auto v = m.Get(1);
    ASSERT_TRUE(v.HasValue());
    EXPECT_EQ(*v, 42);
}

TEST(HashMap, ContainsMatchesGet)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);

    EXPECT_TRUE(m.Contains(1));
    EXPECT_FALSE(m.Contains(2));
}

TEST(HashMap, TransparentLookupWithStringLiteral)
{
    HashMap<String, Int32> m;
    (void)m.Insert("hello", 1);

    // Lookup via const char* avoids constructing String.
    EXPECT_TRUE(m.Contains("hello"));
    EXPECT_FALSE(m.Contains("world"));
}

TEST(HashMap, RemoveReturnsValueAndShrinks)
{
    HashMap<Int32, Int32> m{{1, 10}, {2, 20}};

    auto v = m.Remove(1);
    ASSERT_TRUE(v.HasValue());
    EXPECT_EQ(*v, 10);
    EXPECT_EQ(m.Size(), 1U);
    EXPECT_FALSE(m.Contains(1));
}

TEST(HashMap, RemoveAbsentKeyReturnsNothing)
{
    HashMap<Int32, Int32> m;
    EXPECT_FALSE(m.Remove(1));
}

TEST(HashMap, RemoveEntryReturnsKeyAndValue)
{
    HashMap<Int32, Int32> m{{1, 10}};
    auto entry = m.RemoveEntry(1);
    ASSERT_TRUE(entry.HasValue());
    EXPECT_EQ(entry->first, 1);
    EXPECT_EQ(entry->second, 10);
    EXPECT_TRUE(m.Empty());
}

TEST(HashMap, RetainKeepsMatchingEntries)
{
    HashMap<Int32, Int32> m{{1, 10}, {2, 20}, {3, 30}, {4, 40}};
    m.Retain([](const Int32& k, Int32&) -> bool { return k % 2 == 0; });
    EXPECT_EQ(m.Size(), 2U);
    EXPECT_TRUE(m.Contains(2));
    EXPECT_TRUE(m.Contains(4));
    EXPECT_FALSE(m.Contains(1));
}

TEST(HashMap, RetainCanMutateValues)
{
    HashMap<Int32, Int32> m{{1, 10}, {2, 20}, {3, 30}};
    m.Retain([](const Int32&, Int32& v) -> bool {
        v *= 2;
        return true;
    });

    EXPECT_EQ(*m.Get(1), 20);
    EXPECT_EQ(*m.Get(2), 40);
    EXPECT_EQ(*m.Get(3), 60);
}

TEST(HashMap, ExtractIfPassesRemovedToSink)
{
    HashMap<Int32, Int32> m{{1, 10}, {2, 20}, {3, 30}};
    Int32 sum_keys = 0;
    Int32 sum_values = 0;
    m.ExtractIf([](const Int32& k, Int32&) -> bool { return k >= 2; },
        [&](Int32&& k, Int32&& v) -> void {
            sum_keys += VIOLET_MOVE(k);
            sum_values += VIOLET_MOVE(v);
        });

    EXPECT_EQ(sum_keys, 5); // 2 + 3
    EXPECT_EQ(sum_values, 50); // 20 + 30
    EXPECT_EQ(m.Size(), 1U);
    EXPECT_TRUE(m.Contains(1));
}

TEST(HashMap, EntryOrInsertOnVacantAddsValue)
{
    HashMap<Int32, Int32> m;
    auto& v = m.Entry(1).OrInsert(10);
    EXPECT_EQ(v, 10);
    EXPECT_EQ(*m.Get(1), 10);
}

TEST(HashMap, EntryOrInsertOnOccupiedReturnsExisting)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);
    auto& v = m.Entry(1).OrInsert(99);
    EXPECT_EQ(v, 10);
    EXPECT_EQ(*m.Get(1), 10);
}

TEST(HashMap, EntryOrInsertWithSkipsFactoryOnHit)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);

    bool factory_ran = false;
    auto& v = m.Entry(1).OrInsertWith([&] -> Int32 {
        factory_ran = true;
        return 99;
    });

    EXPECT_EQ(v, 10);
    EXPECT_FALSE(factory_ran);
}

TEST(HashMap, EntryOrInsertWithInvokesFactoryOnMiss)
{
    HashMap<Int32, Int32> m;

    bool factory_ran = false;
    auto& v = m.Entry(1).OrInsertWith([&] -> Int32 {
        factory_ran = true;
        return 42;
    });

    EXPECT_EQ(v, 42);
    EXPECT_TRUE(factory_ran);
}

TEST(HashMap, EntryAndModifyAppliesToOccupied)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);
    auto& v = m.Entry(1).AndModify([](Int32& n) -> void { n += 5; }).OrInsert(0);
    EXPECT_EQ(v, 15);
}

TEST(HashMap, EntryAndModifyIsNoOpOnVacant)
{
    HashMap<Int32, Int32> m;
    Int32 modify_count = 0;
    auto& v = m.Entry(1).AndModify([&](Int32&) -> void { ++modify_count; }).OrInsert(42);
    EXPECT_EQ(v, 42);
    EXPECT_EQ(modify_count, 0);
}

TEST(HashMap, EntryIncrementOrInitializePattern)
{
    HashMap<String, Int32> counts;
    for (const auto& word: {"a", "b", "a", "c", "b", "a"}) {
        counts.Entry(word).AndModify([](Int32& n) -> void { ++n; }).OrInsert(1);
    }

    EXPECT_EQ(*counts.Get("a"), 3);
    EXPECT_EQ(*counts.Get("b"), 2);
    EXPECT_EQ(*counts.Get("c"), 1);
}

TEST(HashMap, EntryOrDefaultUsesDefaultConstructor)
{
    HashMap<Int32, Int32> m;
    auto& v = m.Entry(1).OrDefault();
    EXPECT_EQ(v, 0);
    EXPECT_TRUE(m.Contains(1));
}

TEST(HashMap, EntryOccupiedReportsTrueWhenKeyPresent)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);
    auto e = m.Entry(1);
    EXPECT_TRUE(e.Occupied());
    EXPECT_FALSE(e.Vacant());
}

TEST(HashMap, EntryVacantReportsTrueWhenKeyAbsent)
{
    HashMap<Int32, Int32> m;
    auto e = m.Entry(1);
    EXPECT_TRUE(e.Vacant());
    EXPECT_FALSE(e.Occupied());
}

TEST(HashMap, EntryAsOccupiedYieldsAccessor)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);

    auto e = m.Entry(1);
    auto occupied = e.AsOccupied();
    ASSERT_TRUE(occupied.HasValue());
    EXPECT_EQ(occupied->Value(), 10);
    EXPECT_EQ(occupied->Key(), 1);
}

TEST(HashMap, EntryAsVacantYieldsKey)
{
    HashMap<Int32, Int32> m;
    auto e = m.Entry(5);
    auto vacant = e.AsVacant();
    ASSERT_TRUE(vacant.HasValue());
    EXPECT_EQ(vacant->Key(), 5);
}

TEST(HashMap, EntryKeyReturnsStoredKey)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);
    EXPECT_EQ(m.Entry(1).Key(), 1);
    EXPECT_EQ(m.Entry(2).Key(), 2); // vacant path still has key
}

TEST(HashMap, OccupiedEntryInsertReplacesAndReturnsOld)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);

    auto e = m.Entry(1);
    auto occupied = e.AsOccupied();
    ASSERT_TRUE(occupied.HasValue());
    auto old = occupied->Insert(42);
    EXPECT_EQ(old, 10);
    EXPECT_EQ(*m.Get(1), 42);
}

TEST(HashMap, OccupiedEntryRemoveConsumesEntry)
{
    HashMap<Int32, Int32> m;
    (void)m.Insert(1, 10);

    auto occupied = VIOLET_MOVE(m.Entry(1)).IntoOccupied();
    auto v = VIOLET_MOVE(occupied).Remove();
    EXPECT_EQ(v, 10);
    EXPECT_FALSE(m.Contains(1));
}

TEST(HashMap, VacantEntryInsertAddsValue)
{
    HashMap<Int32, Int32> m;

    auto vacant = VIOLET_MOVE(m.Entry(1)).IntoVacant();
    auto& v = VIOLET_MOVE(vacant).Insert(42);
    EXPECT_EQ(v, 42);
    EXPECT_EQ(*m.Get(1), 42);
}

TEST(HashMap, VacantEntryIntoKeyReturnsKey)
{
    HashMap<Int32, Int32> m;
    auto vacant = VIOLET_MOVE(m.Entry(99)).IntoVacant();
    auto key = VIOLET_MOVE(vacant).IntoKey();
    EXPECT_EQ(key, 99);
    EXPECT_FALSE(m.Contains(99)); // never got inserted
}

TEST(HashMap, RangeBasedForIteratesAllEntries)
{
    HashMap<Int32, Int32> m{{1, 10}, {2, 20}, {3, 30}};
    Int32 key_sum = 0;
    Int32 value_sum = 0;
    for (const auto& [k, v]: m) {
        key_sum += k;
        value_sum += v;
    }

    EXPECT_EQ(key_sum, 6);
    EXPECT_EQ(value_sum, 60);
}

TEST(HashMap, IterationAllowsValueMutation)
{
    HashMap<Int32, Int32> m{{1, 10}, {2, 20}};
    for (auto& [k, v]: m) {
        v *= 10;
    }
    EXPECT_EQ(*m.Get(1), 100);
    EXPECT_EQ(*m.Get(2), 200);
}

TEST(HashMap, WordFrequencyCount)
{
    HashMap<String, Int32> freq;
    const String words[] = {"the", "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog", "the"};
    for (const auto& w: words) {
        freq.Entry(w).AndModify([](Int32& n) -> void { ++n; }).OrInsert(1);
    }

    EXPECT_EQ(*freq.Get("the"), 3);
    EXPECT_EQ(*freq.Get("fox"), 1);
    EXPECT_EQ(freq.Size(), 8U); // 10 words, 8 unique
}

TEST(HashMap, BuildAndQueryAgainstIndex)
{
    HashMap<Int32, String> index;
    for (Int32 i = 0; i < 1000; ++i) {
        (void)index.Insert(i, std::format("value{}", i));
    }

    EXPECT_EQ(index.Size(), 1000U);
    EXPECT_EQ(*index.Get(500), "value500");
    EXPECT_FALSE(index.Get(1000));
}

} // namespace violet::experimental
