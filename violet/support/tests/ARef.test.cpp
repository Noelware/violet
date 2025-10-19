// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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

#include "violet/support/ARef.h"
#include "violet/violet.h"

#include <gtest/gtest.h>

using Noelware::Violet::ARef;
using Noelware::Violet::AWeak;
using Noelware::Violet::int32;

TEST(ARefTests, BasicFunctionality)
{
    ARef<int32> ref(42);
    EXPECT_EQ(*ref, 42);
    EXPECT_EQ(ref.StrongCount(), 1);

    auto cloned = ref.Clone();
    EXPECT_EQ(ref.StrongCount(), 2);
    EXPECT_EQ(*cloned, 42);

    cloned.Release(); // releases the cloned `ARef`.
    EXPECT_EQ(ref.StrongCount(), 1);
}

TEST(ARefTests, Move)
{
    ARef<int32> refa(42);
    auto refb = VIOLET_MOVE(refa);
    EXPECT_FALSE(refa);
    EXPECT_EQ(*refb, 42);
}

TEST(ARefTests, NoThrowTraits)
{
    static_assert(noexcept(ARef<int32>(42)));
    static_assert(noexcept(std::declval<ARef<int32>>().Clone()));
    static_assert(noexcept(std::declval<ARef<int32>>().Release()));
}

TEST(AWeakTests, DowngradeAndUpgrade)
{
    ARef<int32> ref(42);
    AWeak<int32> weak = ref.Downgrade();

    EXPECT_EQ(ref.StrongCount(), 1);
    EXPECT_EQ(ref.WeakCount(), 1);

    auto upgraded = weak.Upgrade();
    ASSERT_TRUE(upgraded);
    EXPECT_EQ(*upgraded.Value(), 42);
    EXPECT_EQ(upgraded->StrongCount(), 2);
}

TEST(AWeakTests, ExpiredWeakPtrReturnsNothing)
{
    AWeak<int32> weak;
    {
        ARef<int32> strong(42);
        weak = strong.Downgrade();
    }

    auto result = weak.Upgrade();
    EXPECT_FALSE(result);
}

TEST(ARefTests, WeakCountDropsAfterDestroy)
{
    ARef<int32> strong(42);
    AWeak<int32> weak = strong.Downgrade();
    EXPECT_EQ(strong.WeakCount(), 1);

    strong.Release();
    EXPECT_EQ(weak.StrongCount(), 0);
    EXPECT_FALSE(weak.Upgrade());
}

struct TestStruct {
    int32 Value = 42;

    TestStruct() = default;
};

TEST(ARefTests, UniquePtrConstructor)
{
    Noelware::Violet::UniquePtr<TestStruct> test = std::make_unique<TestStruct>();
    ARef<TestStruct> ref(VIOLET_MOVE(test));

    EXPECT_TRUE(test == nullptr);
    EXPECT_TRUE(ref);

    EXPECT_NE(ref.Value(), nullptr);
}

struct AbstractCls {
    virtual ~AbstractCls() = default;

    [[nodiscard]] virtual auto Foo() const noexcept -> int32 = 0;
};

struct ConcreteCls: public AbstractCls {
    explicit ConcreteCls(int32 value)
        : x(value)
    {
    }

    [[nodiscard]] auto Foo() const noexcept -> int32 override
    {
        return x;
    }

private:
    int32 x;
};

TEST(ARefTests, AbstractClasses)
{
    ARef<AbstractCls> ref(new ConcreteCls(42));
    ASSERT_EQ(ref->Foo(), 42);
    ASSERT_EQ((*ref).Foo(), 42);

    {
        ARef<AbstractCls> ref2(new ConcreteCls(100));
        ASSERT_EQ(ref2->Foo(), 100);
        ASSERT_EQ((*ref2).Foo(), 100);
    } // RAII will guarantee that `ref2` = destroyed
}
