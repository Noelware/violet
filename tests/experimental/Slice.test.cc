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
#include <violet/Experimental/Slice.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length,cppcoreguidelines-special-member-functions)
using namespace violet::experimental;
using namespace violet;

namespace {
struct MoveOnly {
    VIOLET_DISALLOW_CONSTRUCTOR(MoveOnly);
    VIOLET_DISALLOW_COPY(MoveOnly);

    Int32 Value;
    bool Moved = false;

    VIOLET_EXPLICIT MoveOnly(Int32 v)
        : Value(v)
    {
    }

    VIOLET_IMPLICIT MoveOnly(MoveOnly&& other) noexcept
        : Value(other.Value)
    {
        other.Moved = true;
    }

    auto operator=(MoveOnly&& other) noexcept -> MoveOnly&
    {
        this->Value = other.Value;
        other.Moved = true;
        return *this;
    }

    auto operator==(const MoveOnly& other) const -> bool
    {
        return this->Value == other.Value;
    }

    auto operator<=>(const MoveOnly& other) const = default;
};

struct LifetimeTracker {
    static Int32 Alive;
    Int32 Value;

    VIOLET_EXPLICIT LifetimeTracker(Int32 v)
        : Value(v)
    {
        Alive++;
    }

    VIOLET_IMPLICIT LifetimeTracker(const LifetimeTracker& other)
        : Value(other.Value)
    {
        Alive++;
    }

    auto operator=(const LifetimeTracker&) -> LifetimeTracker& = delete;

    VIOLET_IMPLICIT LifetimeTracker(LifetimeTracker&& other) noexcept
        : Value(std::exchange(other.Value, 0))
    {
        Alive++;
    }

    auto operator=(LifetimeTracker&&) -> LifetimeTracker& = delete;

    ~LifetimeTracker()
    {
        Alive--;
    }

    auto operator==(const LifetimeTracker& other) const -> bool
    {
        return this->Value == other.Value;
    }

    auto operator<=>(const LifetimeTracker& other) const = default;
};

Int32 LifetimeTracker::Alive = 0;

} // namespace

TEST(Construction, DefaultConstructsEmpty)
{
    Slice<Int32, 4> s;
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Elements(), 0U);
    EXPECT_EQ(s.Size(), 4U);
    EXPECT_FALSE(s.Full());
}

TEST(Construction, InitializerList)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    EXPECT_EQ(s.Elements(), 3U);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[1], 2);
    EXPECT_EQ(s[2], 3);
}

TEST(Construction, MoveConstruct)
{
    Slice<Int32, 4> a({ 10, 20, 30 });
    Slice<Int32, 4> b(VIOLET_MOVE(a));

    EXPECT_EQ(b.Elements(), 3U);
    EXPECT_EQ(b[0], 10);
    EXPECT_EQ(b[1], 20);
    EXPECT_EQ(b[2], 30);
    EXPECT_TRUE(a.Empty());
}

TEST(Construction, MoveAssign)
{
    Slice<Int32, 4> a = { 1, 2 };
    Slice<Int32, 4> b = { 10, 20, 30 };

    b = VIOLET_MOVE(a);

    EXPECT_EQ(b.Elements(), 2U);
    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b[1], 2);
    EXPECT_TRUE(a.Empty());
}

TEST(Construction, MoveAssignSelf)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    auto* addr = std::addressof(s);
    s = VIOLET_MOVE(*addr);

    EXPECT_EQ(s.Elements(), 3U);
    EXPECT_EQ(s[0], 1);
}

TEST(Construction, MoveOnlyType)
{
    Slice<MoveOnly, 4> s;
    s.Emplace(42);
    s.Emplace(99);

    Slice<MoveOnly, 4> other(VIOLET_MOVE(s));
    EXPECT_EQ(other.Elements(), 2U);
    EXPECT_EQ(other[0].Value, 42);
    EXPECT_EQ(other[1].Value, 99);
    EXPECT_TRUE(s.Empty());
}

TEST(Lifetime, DestructorDestroysAllElements)
{
    LifetimeTracker::Alive = 0;
    {
        Slice<LifetimeTracker, 4> s;
        s.Emplace(1);
        s.Emplace(2);
        s.Emplace(3);
        EXPECT_EQ(LifetimeTracker::Alive, 3);
    }

    EXPECT_EQ(LifetimeTracker::Alive, 0);
}

TEST(Lifetime, ClearDestroysAllElements)
{
    LifetimeTracker::Alive = 0;
    Slice<LifetimeTracker, 4> s;
    s.Emplace(1);
    s.Emplace(2);
    EXPECT_EQ(LifetimeTracker::Alive, 2);

    s.Clear();
    EXPECT_EQ(LifetimeTracker::Alive, 0);
    EXPECT_TRUE(s.Empty());
}

TEST(Lifetime, PopDestroysOneElement)
{
    LifetimeTracker::Alive = 0;
    Slice<LifetimeTracker, 4> s;
    s.Emplace(1);
    s.Emplace(2);
    s.Emplace(3);
    EXPECT_EQ(LifetimeTracker::Alive, 3);

    s.Pop();
    EXPECT_EQ(LifetimeTracker::Alive, 2);
    EXPECT_EQ(s.Elements(), 2U);
}

TEST(Lifetime, MoveConstructDoesNotLeak)
{
    LifetimeTracker::Alive = 0;
    {
        Slice<LifetimeTracker, 4> a;
        a.Emplace(1);
        a.Emplace(2);

        Slice<LifetimeTracker, 4> b(VIOLET_MOVE(a));
        // `a` is cleared, `b` has 2 moved elements
    }

    EXPECT_EQ(LifetimeTracker::Alive, 0);
}

TEST(Lifetime, MoveAssignDoesNotLeak)
{
    LifetimeTracker::Alive = 0;
    {
        Slice<LifetimeTracker, 4> a;
        a.Emplace(1);

        Slice<LifetimeTracker, 4> b;
        b.Emplace(10);
        b.Emplace(20);

        b = VIOLET_MOVE(a);
        // b's old elements destroyed, a cleared
    }

    EXPECT_EQ(LifetimeTracker::Alive, 0);
}

TEST(Access, OperatorBracket)
{
    Slice<Int32, 4> s({ 10, 20, 30 });
    EXPECT_EQ(s[0], 10);
    EXPECT_EQ(s[1], 20);
    EXPECT_EQ(s[2], 30);

    s[1] = 99;
    EXPECT_EQ(s[1], 99);
}

TEST(Access, ConstOperatorBracket)
{
    const Slice<Int32, 4> s({ 10, 20, 30 });
    EXPECT_EQ(s[0], 10);
    EXPECT_EQ(s[1], 20);
    EXPECT_EQ(s[2], 30);
}

TEST(Access, FrontReturnsFirstElement)
{
    Slice<Int32, 4> s({ 42, 99 });
    auto front = s.Front();
    ASSERT_TRUE(front);
    EXPECT_EQ(*front, 42);
}

TEST(Access, FrontEmptyReturnsNothing)
{
    Slice<Int32, 4> s;
    EXPECT_FALSE(s.Front());
}

TEST(Access, BackReturnsLastElement)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    auto back = s.Back();
    ASSERT_TRUE(back);
    EXPECT_EQ(*back, 3);
}

TEST(Access, BackEmptyReturnsNothing)
{
    Slice<Int32, 4> s;
    EXPECT_FALSE(s.Back());
}

TEST(Access, DataPointsToFirstElement)
{
    Slice<Int32, 4> s({ 5, 10, 15 });
    EXPECT_EQ(*s.Data(), 5);
    EXPECT_EQ(s.Data()[1], 10);
    EXPECT_EQ(s.Data()[2], 15);
}

TEST(Capacity, SizeReturnsN)
{
    Slice<Int32, 8> s;
    EXPECT_EQ(s.Size(), 8U);

    s.Push(1);
    EXPECT_EQ(s.Size(), 8U);
}

TEST(Capacity, ElementsTracksCount)
{
    Slice<Int32, 4> s;
    EXPECT_EQ(s.Elements(), 0U);

    s.Push(1);
    EXPECT_EQ(s.Elements(), 1U);

    s.Push(2);
    EXPECT_EQ(s.Elements(), 2U);

    s.Pop();
    EXPECT_EQ(s.Elements(), 1U);
}

TEST(Capacity, EmptyAndFull)
{
    Slice<Int32, 2> s;
    EXPECT_TRUE(s.Empty());
    EXPECT_FALSE(s.Full());

    s.Push(1);
    EXPECT_FALSE(s.Empty());
    EXPECT_FALSE(s.Full());

    s.Push(2);
    EXPECT_FALSE(s.Empty());
    EXPECT_TRUE(s.Full());
}

TEST(Modifiers, EmplaceConstructsInPlace)
{
    Slice<String, 4> s;
    auto& ref = s.Emplace(5, 'x');

    EXPECT_EQ(ref, "xxxxx");
    EXPECT_EQ(s.Elements(), 1U);
}

TEST(Modifiers, PushCopy)
{
    Slice<String, 4> s;
    String val = "hello";
    s.Push(val);

    EXPECT_EQ(s[0], "hello");
    EXPECT_EQ(val, "hello"); // original unchanged
}

TEST(Modifiers, PushMove)
{
    Slice<String, 4> s;
    String val = "hello";
    s.Push(VIOLET_MOVE(val));

    EXPECT_EQ(s[0], "hello");
    EXPECT_TRUE(val.empty()); // moved from
}

TEST(Modifiers, PopRemovesLast)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    s.Pop();

    EXPECT_EQ(s.Elements(), 2U);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[1], 2);
}

TEST(Modifiers, PopBackReturnsAndRemoves)
{
    Slice<Int32, 4> s({ 10, 20, 30 });
    auto val = s.PopBack();

    EXPECT_EQ(val, 30);
    EXPECT_EQ(s.Elements(), 2U);
}

TEST(Modifiers, PopBackMoveOnly)
{
    Slice<MoveOnly, 4> s;
    s.Emplace(42);
    s.Emplace(99);

    auto val = s.PopBack();
    EXPECT_EQ(val.Value, 99);
    EXPECT_EQ(s.Elements(), 1U);
}

TEST(Modifiers, ClearEmptiesSlice)
{
    Slice<Int32, 4> s = { 1, 2, 3, 4 };
    EXPECT_TRUE(s.Full());

    s.Clear();
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Elements(), 0U);
}

TEST(Modifiers, PushAfterClear)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    s.Clear();
    s.Push(42);

    EXPECT_EQ(s.Elements(), 1U);
    EXPECT_EQ(s[0], 42);
}

TEST(Modifiers, FillToCapacity)
{
    Slice<Int32, 3> s;
    s.Push(1);
    s.Push(2);
    s.Push(3);

    EXPECT_TRUE(s.Full());
    EXPECT_EQ(s.Elements(), 3U);
}

TEST(Iterators, RangeFor)
{
    Slice<Int32, 4> s({ 10, 20, 30 });
    Int32 sum = 0;
    for (auto& val: s) {
        sum += val;
    }

    EXPECT_EQ(sum, 60);
}

TEST(Iterators, ConstRangeFor)
{
    const Slice<Int32, 4> s({ 1, 2, 3, 4 });
    Int32 sum = 0;
    for (const auto& val: s) {
        sum += val;
    }

    EXPECT_EQ(sum, 10);
}

TEST(Iterators, MutateViaRangeFor)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    for (auto& val: s) {
        val *= 10;
    }

    EXPECT_EQ(s[0], 10);
    EXPECT_EQ(s[1], 20);
    EXPECT_EQ(s[2], 30);
}

TEST(Iterators, EmptyRangeFor)
{
    Slice<Int32, 4> s;
    Int32 count = 0;
    for ([[maybe_unused]] auto& val: s) {
        count++;
    }

    EXPECT_EQ(count, 0);
}

TEST(Iterators, ReverseIteration)
{
    Slice<Int32, 4> s = { 1, 2, 3 };
    Vec<Int32> reversed;
    for (auto it = s.rbegin(); it != s.rend(); ++it) { // NOLINT(modernize-loop-convert)
        reversed.push_back(*it);
    }

    EXPECT_EQ(reversed, (Vec<Int32>{ 3, 2, 1 }));
}

TEST(VioletIter, NextYieldsAllElements)
{
    Slice<Int32, 4> s({ 10, 20, 30 });
    auto it = s.Iter();

    auto a = it.Next();
    ASSERT_TRUE(a);
    EXPECT_EQ(*a, 10);

    auto b = it.Next();
    ASSERT_TRUE(b);
    EXPECT_EQ(*b, 20);

    auto c = it.Next();
    ASSERT_TRUE(c);
    EXPECT_EQ(*c, 30);

    auto d = it.Next();
    EXPECT_FALSE(d);
}

TEST(VioletIter, NextBackYieldsReverse)
{
    Slice<Int32, 4> s({ 10, 20, 30 });
    auto it = s.Iter();

    auto a = it.NextBack();
    ASSERT_TRUE(a);
    EXPECT_EQ(*a, 30);

    auto b = it.NextBack();
    ASSERT_TRUE(b);
    EXPECT_EQ(*b, 20);

    auto c = it.NextBack();
    ASSERT_TRUE(c);
    EXPECT_EQ(*c, 10);

    auto d = it.NextBack();
    EXPECT_FALSE(d);
}

TEST(VioletIter, InterleavedNextAndNextBack)
{
    Slice<Int32, 4> s({ 1, 2, 3, 4 });
    auto it = s.Iter();

    auto a = it.Next();
    ASSERT_TRUE(a);
    EXPECT_EQ(*a, 1);

    auto b = it.NextBack();
    ASSERT_TRUE(b);
    EXPECT_EQ(*b, 4);

    auto c = it.Next();
    ASSERT_TRUE(c);
    EXPECT_EQ(*c, 2);

    auto d = it.NextBack();
    ASSERT_TRUE(d);
    EXPECT_EQ(*d, 3);

    EXPECT_FALSE(it.Next());
    EXPECT_FALSE(it.NextBack());
}

TEST(VioletIter, EmptySliceYieldsNothing)
{
    Slice<Int32, 4> s;
    auto it = s.Iter();
    EXPECT_FALSE(it.Next());
    EXPECT_FALSE(it.NextBack());
}

TEST(VioletIter, ConstIter)
{
    const Slice<Int32, 4> s({ 5, 10, 15 });
    auto it = s.Iter();

    auto a = it.Next();
    ASSERT_TRUE(a);
    EXPECT_EQ(*a, 5);
}

TEST(VioletIter, MutateViaIter)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    auto it = s.Iter();
    while (auto item = it.Next()) {
        *item *= 100;
    }

    EXPECT_EQ(s[0], 100);
    EXPECT_EQ(s[1], 200);
    EXPECT_EQ(s[2], 300);
}

TEST(VioletIter, SizeHint)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    auto it = s.Iter();

    auto [lower, upper] = it.SizeHint();
    EXPECT_EQ(lower, 3U);

    ASSERT_TRUE(upper);
    EXPECT_EQ(*upper, 3U);

    (void)it.Next();
    auto [lower2, upper2] = it.SizeHint();
    EXPECT_EQ(lower2, 2U);

    (void)it.NextBack();
    auto [lower3, upper3] = it.SizeHint();
    EXPECT_EQ(lower3, 1U);
}

TEST(Operators, BoolConversion)
{
    Slice<Int32, 4> s;
    EXPECT_FALSE(static_cast<bool>(s));

    s.Push(1);
    EXPECT_TRUE(static_cast<bool>(s));

    s.Pop();
    EXPECT_FALSE(static_cast<bool>(s));
}

TEST(Operators, EqualityEmpty)
{
    Slice<Int32, 4> a;
    Slice<Int32, 4> b;
    EXPECT_EQ(a, b);
}

TEST(Operators, EqualitySameElements)
{
    Slice<Int32, 4> a({ 1, 2, 3 });
    Slice<Int32, 4> b({ 1, 2, 3 });
    EXPECT_EQ(a, b);
}

TEST(Operators, EqualityDifferentElements)
{
    Slice<Int32, 4> a({ 1, 2, 3 });
    Slice<Int32, 4> b({ 1, 2, 4 });
    EXPECT_NE(a, b);
}

TEST(Operators, EqualityDifferentSizes)
{
    Slice<Int32, 4> a({ 1, 2 });
    Slice<Int32, 4> b = { 1, 2, 3 };
    EXPECT_NE(a, b);
}

TEST(Operators, ThreeWayLessThan)
{
    Slice<Int32, 4> a({ 1, 2, 3 });
    Slice<Int32, 4> b({ 1, 2, 4 });
    EXPECT_TRUE(a < b);
    EXPECT_FALSE(b < a);
}

TEST(Operators, ThreeWayShorterIsLess)
{
    Slice<Int32, 4> a({ 1, 2 });
    Slice<Int32, 4> b({ 1, 2, 3 });
    EXPECT_TRUE(a < b);
}

TEST(Operators, ThreeWayEqual)
{
    Slice<Int32, 4> a({ 1, 2, 3 });
    Slice<Int32, 4> b({ 1, 2, 3 });
    EXPECT_TRUE((a <=> b) == 0);
}

TEST(Span, DynamicExtentSpan)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    Span<Int32> span = s;

    EXPECT_EQ(span.size(), 3U);
    EXPECT_EQ(span[0], 1);
    EXPECT_EQ(span[1], 2);
    EXPECT_EQ(span[2], 3);
}

TEST(Span, ConstDynamicExtentSpan)
{
    const Slice<Int32, 4> s({ 10, 20 });
    Span<const Int32> span = s;
    EXPECT_EQ(span.size(), 2U);
    EXPECT_EQ(span[0], 10);
}

TEST(Span, FixedExtentSpan)
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    Span<Int32, 3> span = s;

    EXPECT_EQ(span.size(), 3U);
    EXPECT_EQ(span[0], 1);
    EXPECT_EQ(span[2], 3);
}

namespace {

constexpr auto MakeAndCount() -> UInt
{
    Slice<Int32, 4> s;
    s.Push(1);
    s.Push(2);
    s.Push(3);

    return s.Elements();
}

constexpr auto MakeAndSum() -> Int32
{
    Slice<int, 4> s({ 10, 20, 30 });
    Int32 sum = 0;
    for (UInt i = 0; i < s.Elements(); i++) {
        sum += s[i];
    }

    return sum;
}

constexpr auto MakeAndPopBack() -> Int32
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    return s.PopBack();
}

constexpr auto MakeAndCompare() -> bool
{
    Slice<Int32, 4> a({ 1, 2, 3 });
    Slice<Int32, 4> b({ 1, 2, 3 });
    return a == b;
}

constexpr auto MakeMovedSlice() -> UInt
{
    Slice<Int32, 4> a({ 1, 2, 3 });
    Slice<Int32, 4> b(VIOLET_MOVE(a));
    return b.Elements();
}

constexpr auto MakeClearAndReuse() -> Int32
{
    Slice<Int32, 4> s({ 1, 2, 3 });
    s.Clear();
    s.Push(42);

    return s[0];
}

} // namespace

static_assert(MakeAndCount() == 3);
static_assert(MakeAndSum() == 60);
static_assert(MakeAndPopBack() == 3);
static_assert(MakeAndCompare());
static_assert(MakeMovedSlice() == 3);
static_assert(MakeClearAndReuse() == 42);

// NOLINTEND(google-build-using-namespace,readability-identifier-length,cppcoreguidelines-special-member-functions)
