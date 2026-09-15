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
#include <violet/Experimental/Unique.h>

using namespace violet::experimental::ptr;
using namespace violet;

namespace {

struct Counter {
    static std::atomic<Int32> Constructed;
    static std::atomic<Int32> Destructed;

    Int32 Value;

    VIOLET_EXPLICIT Counter(Int32 v)
        : Value(v)
    {
        Constructed.fetch_add(1, std::memory_order_relaxed);
    }

    ~Counter()
    {
        Destructed.fetch_add(1, std::memory_order_relaxed);
    }

    Counter(const Counter&) = delete;
    auto operator=(const Counter&) -> Counter& = delete;
    Counter(Counter&&) = delete;
    auto operator=(Counter&&) -> Counter& = delete;

    static void Reset()
    {
        Constructed.store(0, std::memory_order_relaxed);
        Destructed.store(0, std::memory_order_relaxed);
    }

    static auto Alive() -> Int32
    {
        return Constructed.load(std::memory_order_relaxed) - Destructed.load(std::memory_order_relaxed);
    }
};

std::atomic<Int32> Counter::Constructed{0};
std::atomic<Int32> Counter::Destructed{0};

struct Animal {
    VIOLET_DISALLOW_COPY_AND_MOVE(Animal);

    Int32 Legs;

    VIOLET_EXPLICIT Animal(Int32 legs)
        : Legs(legs)
    {
    }

    virtual ~Animal() = default;
};

struct Widget {
    static std::atomic<Int32> Destructed;

    Int32 Value = 0;
    virtual ~Widget() = default;
};

struct BaseWidget {
    Int32 Value;
    VIOLET_EXPLICIT BaseWidget(Int32 v)
        : Value(v)
    {
    }
};

struct TerminalWidget final: public BaseWidget {
    VIOLET_DISALLOW_COPY_AND_MOVE(TerminalWidget);

    static std::atomic<Int32> Destructed;
    String Name;

    VIOLET_IMPLICIT TerminalWidget(Str name, Int32 v)
        : BaseWidget(v)
        , Name(name)
    {
    }

    ~TerminalWidget()
    {
        TerminalWidget::Destructed.fetch_add(1, std::memory_order_relaxed);
    }
};

std::atomic<Int32> TerminalWidget::Destructed{0};

} // namespace

TEST(Unique, ConstructsNullFromDefault)
{
    Unique<Int32> p;
    EXPECT_FALSE(p.Valid());
    EXPECT_FALSE(p);
    EXPECT_EQ(p.Get(), nullptr);
    EXPECT_TRUE(p == nullptr);
}

/*

TEST(Unique, ConstructsNullFromDefault)
{
    Unique<Int32> p;
    EXPECT_FALSE(p.Valid());
    EXPECT_FALSE(p);
    EXPECT_EQ(p.Get(), nullptr);
    EXPECT_TRUE(p == nullptr);
}

TEST(Unique, ConstructsNullFromNullptr)
{
    Unique<Int32> p(nullptr);
    EXPECT_FALSE(p.Valid());
    EXPECT_TRUE(p == nullptr);
    EXPECT_FALSE(p != nullptr);
}

TEST(Unique, ConstructsFromRawPointer)
{
    Unique<Int32> p(new Int32(7));
    ASSERT_TRUE(p.Valid());
    EXPECT_EQ(*p, 7);
    EXPECT_TRUE(p);
}

TEST(Unique, DestructorRunsOnScopeExit)
{
    Counter::Reset();
    {
        Unique<Counter> p(new Counter(1));
        EXPECT_EQ(Counter::Alive(), 1);
    }
    EXPECT_EQ(Counter::Alive(), 0);
    EXPECT_EQ(Counter::Destructed.load(), 1);
}

TEST(Unique, CustomDeleterIsInvoked)
{
    Int32 stack = 99;
    bool called = false;

    {
        Unique<Int32, std::function<void(Int32*)>> p(&stack, [&called](Int32*) { called = true; });
        EXPECT_EQ(*p, 99);
    }

    EXPECT_TRUE(called);
}

TEST(Unique, MoveTransfersOwnership)
{
    Unique<Int32> a(new Int32(11));
    Unique<Int32> b(VIOLET_MOVE(a));

    EXPECT_EQ(*b, 11);
    EXPECT_FALSE(a.Valid());
    EXPECT_EQ(a.Get(), nullptr);
}

TEST(Unique, MoveAssignmentDestroysPrevious)
{
    Counter::Reset();
    Unique<Counter> a(new Counter(1));
    Unique<Counter> b(new Counter(2));
    EXPECT_EQ(Counter::Alive(), 2);

    b = VIOLET_MOVE(a);
    EXPECT_EQ(Counter::Alive(), 1);
    EXPECT_EQ(b->Value, 1);
    EXPECT_FALSE(a.Valid());
}

TEST(Unique, SelfMoveAssignmentIsSafe)
{
    Unique<Int32> a(new Int32(42));
    auto& alias = a;
    a = VIOLET_MOVE(alias);

    ASSERT_TRUE(a.Valid());
    EXPECT_EQ(*a, 42);
}

TEST(Unique, ResetDestroysObject)
{
    Counter::Reset();
    Unique<Counter> p(new Counter(1));
    EXPECT_EQ(Counter::Alive(), 1);

    p.Reset();
    EXPECT_EQ(Counter::Alive(), 0);
    EXPECT_FALSE(p.Valid());
}

TEST(Unique, ResetWithRawPointerReplacesObject)
{
    Counter::Reset();
    Unique<Counter> p(new Counter(1));
    p.Reset(new Counter(2));

    EXPECT_EQ(Counter::Alive(), 1);
    EXPECT_EQ(p->Value, 2);
}

TEST(Unique, ArrowAndStarOperators)
{
    Unique<Counter> p(new Counter(10));
    EXPECT_EQ(p->Value, 10);
    EXPECT_EQ((*p).Value, 10);

    p->Value = 20;
    EXPECT_EQ(p->Value, 20);
}

TEST(Unique, ConstAccessorsReturnConstPointer)
{
    const Unique<Int32> p(new Int32(33));
    const Int32* raw = p.Get();
    ASSERT_NE(raw, nullptr);
    EXPECT_EQ(*raw, 33);
    EXPECT_EQ(*p, 33);
}

TEST(Unique, LeakDisarmsDestructorAndReturnsPointer)
{
    Counter::Reset();
    Unique<Counter> p(new Counter(1));

    auto leaked = p.Leak();
    ASSERT_TRUE(leaked.HasValue());
    EXPECT_FALSE(p.Valid());
    EXPECT_EQ(Counter::Alive(), 1); // destructor was disarmed, object still alive

    delete leaked->Get();
    EXPECT_EQ(Counter::Alive(), 0);
}

TEST(Unique, LeakOnEmptyUniqueYieldsNothing)
{
    Unique<Int32> p;
    auto leaked = p.Leak();
    EXPECT_FALSE(leaked.HasValue());
}

TEST(Unique, NewConstructsInPlace)
{
    auto p = Unique<Int32>::New(42);
    ASSERT_TRUE(p.Valid());
    EXPECT_EQ(*p, 42);
}

TEST(Unique, NewDestroysThroughConcreteTypeWithoutVirtualDestructor)
{
    WidgetDerived::Destructed.store(0);
    {
        Unique<WidgetBase> w = Unique<WidgetBase>::New<WidgetDerived>("gadget", 4);
        ASSERT_TRUE(w.Valid());
        EXPECT_EQ(w->Value, 4);
    }
    // If `~WidgetDerived()` weren't called, this would still be 0 — `WidgetBase` has no
    // virtual destructor, so a naive `delete` through the base pointer wouldn't reach it.
    EXPECT_EQ(WidgetDerived::Destructed.load(), 1);
}

TEST(Unique, OrdersAgainstNullptr)
{
    Unique<Int32> empty;
    Unique<Int32> present(new Int32(1));

    EXPECT_TRUE(empty <=> nullptr == std::strong_ordering::equal);
    EXPECT_TRUE(present <=> nullptr != std::strong_ordering::equal);
}

TEST(Unique, ComparesByPointeeValueNotAddress)
{
    Unique<Int32> a(new Int32(5));
    Unique<Int32> b(new Int32(5));
    Unique<Int32> c(new Int32(9));

    // Different allocations, same value: address comparison would disagree, value
    // comparison says equal.
    EXPECT_TRUE((a <=> b) == std::strong_ordering::equal);
    EXPECT_TRUE((a <=> c) == std::strong_ordering::less);
}

TEST(Unique, NullComparesLessThanNonNullInCrossComparison)
{
    Unique<Int32> empty;
    Unique<Int32> present(new Int32(1));

    EXPECT_TRUE((empty <=> present) == std::strong_ordering::less);
}

*/
