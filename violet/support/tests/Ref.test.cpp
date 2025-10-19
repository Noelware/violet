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

#include "violet/support/Ref.h"
#include <gtest/gtest.h>

using Noelware::Violet::int32;
using Noelware::Violet::Ref;

TEST(RefTests, BasicFunctionality)
{
    Ref<int32> ref(42);
    ASSERT_TRUE(ref);
    EXPECT_TRUE(*ref == 42);
    EXPECT_EQ(ref.StrongCount(), 1);
    EXPECT_EQ(ref.WeakCount(), 0);

    auto b = ref.Clone(); // NOLINT(readability-identifier-length)
    ASSERT_TRUE(b);
    EXPECT_EQ(ref.StrongCount(), 2);
    EXPECT_EQ(b.StrongCount(), 2);
    EXPECT_TRUE(*b == 42);

    b.Release();
    EXPECT_EQ(ref.StrongCount(), 1);
}

TEST(RefTests, MoveConstructionAndAssignment)
{
    Ref<int32> ref(42);
    auto b = VIOLET_MOVE(ref); // NOLINT(readability-identifier-length)
    ASSERT_FALSE(ref);
    ASSERT_TRUE(b);
    EXPECT_TRUE(*b == 42);

    Ref<int32> c; // NOLINT(readability-identifier-length)
    c = VIOLET_MOVE(b);
    ASSERT_FALSE(b);
    ASSERT_TRUE(c);
    EXPECT_TRUE(*c == 42);
}

TEST(RefTests, WeakRefUpgrade)
{
    Ref<int32> ref(42);
    auto weak = ref.Downgrade();
    ASSERT_TRUE(weak);
    EXPECT_EQ(weak.StrongCount(), 1);

    auto upgraded = weak.Upgrade();
    ASSERT_TRUE(upgraded);
    EXPECT_TRUE(*upgraded->Value() == 42);
    EXPECT_EQ(upgraded->StrongCount(), 2);

    ref.Release();
    EXPECT_EQ(upgraded->StrongCount(), 1);

    upgraded->Release();
    ASSERT_FALSE(weak.Upgrade());
}

struct TestStruct {
    int32 Value = 42;

    TestStruct() = default;
};

TEST(ARefTests, UniquePtrConstructor)
{
    Noelware::Violet::UniquePtr<TestStruct> test = std::make_unique<TestStruct>();
    Ref<TestStruct> ref(VIOLET_MOVE(test));

    ASSERT_TRUE(test == nullptr);
    ASSERT_TRUE(ref);

    ASSERT_NE(ref.Value(), nullptr);
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

TEST(RefTests, AbstractClasses)
{
    Ref<AbstractCls> ref(new ConcreteCls(42));
    ASSERT_EQ(ref->Foo(), 42);
    ASSERT_EQ((*ref).Foo(), 42);

    {
        Ref<AbstractCls> ref2(new ConcreteCls(100));
        ASSERT_EQ(ref2->Foo(), 100);
        ASSERT_EQ((*ref2).Foo(), 100);
    } // RAII will guarantee that `ref2` = destroyed
}
