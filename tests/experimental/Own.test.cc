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
#include <violet/Experimental/Own.h>

#include <atomic>
#include <thread>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length,cppcoreguidelines-owning-memory,performance-unnecessary-copy-initialization)
using namespace violet::experimental;
using violet::Int32;
using violet::Str;
using violet::String;
using violet::UInt;

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

std::atomic<Int32> Counter::Constructed{ 0 };
std::atomic<Int32> Counter::Destructed{ 0 };

struct Animal {
    Int32 Legs;

    VIOLET_EXPLICIT Animal(Int32 legs)
        : Legs(legs)
    {
    }
    virtual ~Animal() = default;

    Animal(const Animal&) = delete;
    auto operator=(const Animal&) -> Animal& = delete;
    Animal(Animal&&) = delete;
    auto operator=(Animal&&) -> Animal& = delete;
};

struct Cat final: public Animal { // NOLINT(cppcoreguidelines-special-member-functions)
    static std::atomic<Int32> Destructed;

    String Name;

    Cat(Str name, Int32 legs)
        : Animal(legs)
        , Name(name)
    {
    }

    ~Cat() override
    {
        Destructed.fetch_add(1, std::memory_order_relaxed);
    }
};

std::atomic<Int32> Cat::Destructed{ 0 };

struct Dog final: public Animal { // NOLINT(cppcoreguidelines-special-member-functions)
    VIOLET_EXPLICIT Dog(Int32 legs)
        : Animal(legs)
    {
    }

    ~Dog() override = default;
};

} // namespace

TEST(Own, ConstructsNullFromNullptr)
{
    Own<Int32> p(nullptr);
    EXPECT_EQ(p.Get(), nullptr);
    EXPECT_EQ(p.StrongRefs(), 0U);
    EXPECT_EQ(p.WeakRefs(), 0U);
    EXPECT_FALSE(p);
}

TEST(Own, NewConstructsInPlace)
{
    auto p = Own<Int32>::New(42);
    ASSERT_TRUE(p);
    EXPECT_EQ(*p, 42);
    EXPECT_EQ(p.StrongRefs(), 1U);
    EXPECT_EQ(p.WeakRefs(), 0U);
    EXPECT_TRUE(p.Unique());
}

TEST(Own, ConstructsFromRawPointer)
{
    auto* raw = new Int32(7);
    Own<Int32> p(raw);
    EXPECT_EQ(p.Get(), raw);
    EXPECT_EQ(*p, 7);
    EXPECT_EQ(p.StrongRefs(), 1U);
}

TEST(Own, CustomDeleterIsInvoked)
{
    Int32 stack = 99;
    bool called = false;
    {
        Own<Int32> p(&stack, [&called](Int32*) -> void { called = true; });
        EXPECT_EQ(*p, 99);
    }

    EXPECT_TRUE(called);
}

TEST(Own, DestructorRunsWhenLastReferenceDrops)
{
    Counter::Reset();
    {
        auto p = Own<Counter>::New(1);
        EXPECT_EQ(Counter::Alive(), 1);
    }

    EXPECT_EQ(Counter::Alive(), 0);
    EXPECT_EQ(Counter::Destructed.load(), 1);
}

TEST(Own, CopyIncrementsStrongRefs)
{
    auto a = Own<Int32>::New(5);
    EXPECT_EQ(a.StrongRefs(), 1U);

    auto b = a;
    EXPECT_EQ(a.StrongRefs(), 2U);
    EXPECT_EQ(b.StrongRefs(), 2U);
    EXPECT_EQ(*a, 5);
    EXPECT_EQ(*b, 5);
    EXPECT_FALSE(a.Unique());
}

TEST(Own, CopyAssignmentReleasesPrevious)
{
    Counter::Reset();
    auto a = Own<Counter>::New(1);
    {
        auto b = Own<Counter>::New(2);
        EXPECT_EQ(Counter::Alive(), 2);
        b = a;
        EXPECT_EQ(Counter::Alive(), 1);
        EXPECT_EQ(b->Value, 1);
        EXPECT_EQ(a.StrongRefs(), 2U);
    }
    EXPECT_EQ(Counter::Alive(), 1);
}

TEST(Own, CopyAssignmentToSelfIsSafe)
{
    auto a = Own<Int32>::New(42);
    auto& alias = a;
    a = alias;
    EXPECT_EQ(a.StrongRefs(), 1U);
    EXPECT_EQ(*a, 42);
}

TEST(Own, MoveTransfersOwnershipWithoutBumpingRefs)
{
    auto a = Own<Int32>::New(11);
    EXPECT_EQ(a.StrongRefs(), 1U);

    Own<Int32> b(VIOLET_MOVE(a));
    EXPECT_EQ(b.StrongRefs(), 1U);
    EXPECT_EQ(*b, 11);
    EXPECT_EQ(a.Get(), nullptr);
    EXPECT_FALSE(a);
}

TEST(Own, MoveAssignmentReleasesPrevious)
{
    Counter::Reset();
    auto a = Own<Counter>::New(1);
    auto b = Own<Counter>::New(2);
    EXPECT_EQ(Counter::Alive(), 2);

    b = VIOLET_MOVE(a);
    EXPECT_EQ(Counter::Alive(), 1);
    EXPECT_EQ(b->Value, 1);
    EXPECT_EQ(a.Get(), nullptr);
}

TEST(Own, ResetReleasesObject)
{
    Counter::Reset();
    auto p = Own<Counter>::New(1);
    EXPECT_EQ(Counter::Alive(), 1);

    p.Reset();
    EXPECT_EQ(Counter::Alive(), 0);
    EXPECT_EQ(p.Get(), nullptr);
    EXPECT_FALSE(p);
}

TEST(Own, ResetWithRawPointerReplacesObject)
{
    Counter::Reset();
    auto p = Own<Counter>::New(1);
    EXPECT_EQ(Counter::Alive(), 1);

    p.Reset(new Counter(2));
    EXPECT_EQ(Counter::Alive(), 1);
    EXPECT_EQ(p->Value, 2);
    EXPECT_EQ(p.StrongRefs(), 1U);
}

TEST(Own, ArrowAndStarOperators)
{
    auto p = Own<Counter>::New(10);
    EXPECT_EQ(p->Value, 10);
    EXPECT_EQ((*p).Value, 10);

    p->Value = 20;
    EXPECT_EQ(p->Value, 20);
}

TEST(Own, ConstAccessorsReturnConstPointer)
{
    const auto p = Own<Int32>::New(33);
    const Int32* raw = p.Get();
    ASSERT_NE(raw, nullptr);
    EXPECT_EQ(*raw, 33);
    EXPECT_EQ(*p, 33);
}

TEST(Own, BoolOperatorReflectsOwnership)
{
    Own<Int32> empty(nullptr);
    EXPECT_FALSE(empty);

    auto p = Own<Int32>::New(1);
    EXPECT_TRUE(p);

    p.Reset();
    EXPECT_FALSE(p);
}

TEST(Own, InPlaceTypePolymorphicConstruction)
{
    Cat::Destructed.store(0);
    {
        Own<Animal> pet(std::in_place_type<Cat>, "Mochi", 4);
        ASSERT_TRUE(pet);
        EXPECT_EQ(pet->Legs, 4);

#if VIOLET_FEATURE(RTTI)
        auto* cat = dynamic_cast<Cat*>(pet.Get());
        ASSERT_NE(cat, nullptr);
        EXPECT_EQ(cat->Name, "Mochi");
#endif
    }
    EXPECT_EQ(Cat::Destructed.load(), 1);
}

TEST(Own, NewPolymorphicReturnsBase)
{
    Cat::Destructed.store(0);
    {
        Own<Animal> pet = Own<Animal>::New<Cat>("Luna", 4);
        ASSERT_TRUE(pet);
        EXPECT_EQ(pet->Legs, 4);
    }
    EXPECT_EQ(Cat::Destructed.load(), 1);
}

TEST(Own, UpcastCopyConstructorSharesControlBlock)
{
    auto cat = Own<Cat>::New<Cat>("Tama", 4);
    EXPECT_EQ(cat.StrongRefs(), 1U);

    Own<Animal> as_animal = cat;
    EXPECT_EQ(cat.StrongRefs(), 2U);
    EXPECT_EQ(as_animal.StrongRefs(), 2U);
    EXPECT_EQ(as_animal->Legs, 4);
}

TEST(Own, UpcastMoveConstructorTransfersOwnership)
{
    auto cat = Own<Cat>::New<Cat>("Tama", 4);
    Own<Animal> as_animal = VIOLET_MOVE(cat);

    EXPECT_EQ(as_animal.StrongRefs(), 1U);
    EXPECT_EQ(cat.Get(), nullptr);
    EXPECT_EQ(as_animal->Legs, 4);
}

TEST(Own, DowngradeProducesWeak)
{
    auto strong = Own<Int32>::New(42);
    auto weak = strong.Downgrade();

    EXPECT_EQ(strong.StrongRefs(), 1U);
    EXPECT_EQ(strong.WeakRefs(), 1U);
    EXPECT_EQ(weak.StrongRefs(), 1U);
    EXPECT_EQ(weak.WeakRefs(), 1U);
}

TEST(Own, WeakUpgradeSucceedsWhileStrongAlive)
{
    auto strong = Own<Int32>::New(99);
    auto weak = strong.Downgrade();

    auto upgraded = weak.Upgrade();
    ASSERT_TRUE(upgraded.HasValue());
    EXPECT_EQ(**upgraded, 99);
    EXPECT_EQ(strong.StrongRefs(), 2U);
}

TEST(Own, WeakUpgradeFailsAfterStrongReleased)
{
    auto strong = Own<Int32>::New(99);
    auto weak = strong.Downgrade();
    strong.Reset();

    auto upgraded = weak.Upgrade();
    EXPECT_FALSE(upgraded.HasValue());
    EXPECT_EQ(weak.StrongRefs(), 0U);
}

TEST(Own, WeakDoesNotKeepObjectAlive)
{
    Counter::Reset();
    Weak<Counter> weak;
    {
        auto strong = Own<Counter>::New(1);
        weak = strong.Downgrade();
        EXPECT_EQ(Counter::Alive(), 1);
    }

    EXPECT_EQ(Counter::Alive(), 0);
    EXPECT_FALSE(weak.Upgrade().HasValue());
}

TEST(Own, WeakCopyIncrementsWeakRefs)
{
    auto strong = Own<Int32>::New(1);
    auto weak = strong.Downgrade();
    EXPECT_EQ(strong.WeakRefs(), 1U);

    auto weak2 = weak;
    EXPECT_EQ(strong.WeakRefs(), 2U);
    EXPECT_EQ(weak.WeakRefs(), 2U);
    EXPECT_EQ(weak2.WeakRefs(), 2U);
}

TEST(Own, WeakMoveTransfersWithoutBumpingRefs)
{
    auto strong = Own<Int32>::New(1);
    auto weak = strong.Downgrade();
    EXPECT_EQ(strong.WeakRefs(), 1U);

    Weak<Int32> moved(VIOLET_MOVE(weak));
    EXPECT_EQ(strong.WeakRefs(), 1U);
    EXPECT_EQ(moved.WeakRefs(), 1U);
}

TEST(Own, ConcurrentCopiesPreserveRefcount)
{
    constexpr Int32 kThreads = 8;
    constexpr Int32 kIterations = 10000;

    auto base = Own<Int32>::New(0);
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (Int32 i = 0; i < kThreads; ++i) {
        threads.emplace_back([base] -> void {
            for (Int32 j = 0; j < kIterations; ++j) {
                auto copy = base;
                EXPECT_GE(copy.StrongRefs(), 2U);
                EXPECT_EQ(*copy, 0);
            }
        });
    }

    for (auto& t: threads) {
        t.join();
    }

    EXPECT_EQ(base.StrongRefs(), 1U);
}

TEST(Own, ConcurrentWeakUpgradeRace)
{
    constexpr Int32 kThreads = 8;
    constexpr Int32 kIterations = 5000;

    auto strong = Own<Counter>::New(42);
    auto weak = strong.Downgrade();

    std::atomic<Int32> upgrade_successes{ 0 };
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (Int32 i = 0; i < kThreads; ++i) {
        threads.emplace_back([&weak, &upgrade_successes] -> void {
            for (Int32 j = 0; j < kIterations; ++j) {
                if (auto upgraded = weak.Upgrade(); upgraded.HasValue()) {
                    EXPECT_EQ((*upgraded)->Value, 42);
                    upgrade_successes.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    for (auto& t: threads) {
        t.join();
    }

    EXPECT_GT(upgrade_successes.load(), 0);
    EXPECT_EQ(strong.StrongRefs(), 1U);
}

TEST(Own, StaticCastUpcastSharesControlBlock)
{
    auto cat = Own<Cat>::New<Cat>("Mochi", 4);
    EXPECT_EQ(cat.StrongRefs(), 1U);

    auto animal = cat.StaticCast<Animal>();
    ASSERT_TRUE(animal);
    EXPECT_EQ(cat.StrongRefs(), 2U);
    EXPECT_EQ(animal.StrongRefs(), 2U);
    EXPECT_EQ(animal->Legs, 4);
}

TEST(Own, StaticCastDowncastRecoversDerived)
{
    Own<Animal> pet = Own<Animal>::New<Cat>("Luna", 4);

    auto cat = pet.StaticCast<Cat>();
    ASSERT_TRUE(cat);
    EXPECT_EQ(cat->Name, "Luna");
    EXPECT_EQ(pet.StrongRefs(), 2U);
}

TEST(Own, StaticCastOnEmptyYieldsEmpty)
{
    Own<Cat> empty(nullptr);

    auto animal = empty.StaticCast<Animal>();
    EXPECT_FALSE(animal);
    EXPECT_EQ(animal.Get(), nullptr);
    EXPECT_EQ(animal.StrongRefs(), 0U);
}

TEST(Own, StaticCastKeepsObjectAliveUntilAllDrop)
{
    Cat::Destructed.store(0);
    {
        auto cat = Own<Cat>::New<Cat>("Tama", 4);
        auto animal = cat.StaticCast<Animal>();

        cat.Reset();
        EXPECT_EQ(Cat::Destructed.load(), 0); // animal still holds the block
        EXPECT_EQ(animal.StrongRefs(), 1U);
    }

    EXPECT_EQ(Cat::Destructed.load(), 1);
}

TEST(Own, ConstCastAddsAndRemovesConst)
{
    auto mut = Own<Int32>::New(7);
    EXPECT_EQ(mut.StrongRefs(), 1U);

    Own<const Int32> immut = mut.ConstCast<const Int32>();
    ASSERT_TRUE(immut);
    EXPECT_EQ(*immut, 7);
    EXPECT_EQ(mut.StrongRefs(), 2U);

    auto back = immut.ConstCast<Int32>();
    ASSERT_TRUE(back);
    *back = 21;
    EXPECT_EQ(*mut, 21); // same underlying object
    EXPECT_EQ(mut.StrongRefs(), 3U);
}

TEST(Own, ConstCastOnEmptyYieldsEmpty)
{
    Own<Int32> empty(nullptr);

    auto immut = empty.ConstCast<const Int32>();
    EXPECT_FALSE(immut);
    EXPECT_EQ(immut.StrongRefs(), 0U);
}

#if VIOLET_FEATURE(RTTI)
TEST(Own, DynamicCastDowncastSucceedsForActualType)
{
    Own<Animal> pet = Own<Animal>::New<Cat>("Mochi", 4);
    EXPECT_EQ(pet.StrongRefs(), 1U);

    auto cat = pet.DynamicCast<Cat>();
    ASSERT_TRUE(cat);
    EXPECT_EQ(cat->Name, "Mochi");
    EXPECT_EQ(cat->Legs, 4);
    EXPECT_EQ(pet.StrongRefs(), 2U);
    EXPECT_EQ(cat.StrongRefs(), 2U);
}

TEST(Own, DynamicCastFailsForWrongTypeWithoutBumpingRefs)
{
    Own<Animal> pet = Own<Animal>::New<Dog>(4);
    EXPECT_EQ(pet.StrongRefs(), 1U);

    auto cat = pet.DynamicCast<Cat>();
    EXPECT_FALSE(cat);
    EXPECT_EQ(cat.Get(), nullptr);
    EXPECT_EQ(pet.StrongRefs(), 1U); // failed cast must not share ownership
}

TEST(Own, DynamicCastOnEmptyYieldsEmpty)
{
    Own<Animal> empty(nullptr);

    auto cat = empty.DynamicCast<Cat>();
    EXPECT_FALSE(cat);
    EXPECT_EQ(cat.StrongRefs(), 0U);
}

TEST(Own, DynamicCastKeepsObjectAliveUntilAllDrop)
{
    Cat::Destructed.store(0);
    {
        Own<Animal> pet = Own<Animal>::New<Cat>("Sora", 4);
        auto cat = pet.DynamicCast<Cat>();
        ASSERT_TRUE(cat);

        pet.Reset();
        EXPECT_EQ(Cat::Destructed.load(), 0);
        EXPECT_EQ(cat.StrongRefs(), 1U);
    }

    EXPECT_EQ(Cat::Destructed.load(), 1);
}

TEST(Own, RvalueDynamicCastSuccessConsumesSource)
{
    Own<Animal> pet = Own<Animal>::New<Cat>("Mochi", 4);

    auto cat = VIOLET_MOVE(pet).DynamicCast<Cat>();
    ASSERT_TRUE(cat);
    EXPECT_EQ(cat->Name, "Mochi");
    EXPECT_EQ(cat.StrongRefs(), 1U); // moved, not shared
    EXPECT_FALSE(pet); // source consumed
    EXPECT_EQ(pet.Get(), nullptr);
}

TEST(Own, RvalueDynamicCastConsumesSourceOnFailure)
{
    Cat::Destructed.store(0);
    {
        Own<Animal> pet = Own<Animal>::New<Dog>(4);

        auto cat = VIOLET_MOVE(pet).DynamicCast<Cat>();
        EXPECT_FALSE(cat); // cast failed
        EXPECT_FALSE(pet); // source still consumed
        EXPECT_EQ(pet.Get(), nullptr);
    }

    EXPECT_EQ(Cat::Destructed.load(), 0); // the Dog was managed, not a Cat
}
#endif

TEST(Own, RvalueStaticCastConsumesSource)
{
    auto cat = Own<Cat>::New<Cat>("Luna", 4);

    auto animal = VIOLET_MOVE(cat).StaticCast<Animal>();
    ASSERT_TRUE(animal);
    EXPECT_EQ(animal->Legs, 4);
    EXPECT_EQ(animal.StrongRefs(), 1U); // moved, not shared
    EXPECT_FALSE(cat); // source consumed
}

TEST(Own, RvalueConstCastConsumesSource)
{
    auto mut = Own<Int32>::New(7);

    Own<const Int32> immut = VIOLET_MOVE(mut).ConstCast<const Int32>();
    ASSERT_TRUE(immut);
    EXPECT_EQ(*immut, 7);
    EXPECT_EQ(immut.StrongRefs(), 1U); // moved, not shared
    EXPECT_FALSE(mut); // source consumed
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length,cppcoreguidelines-owning-memory,performance-unnecessary-copy-initialization)
