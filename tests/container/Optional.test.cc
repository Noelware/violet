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
#include <violet/Container/Optional.h>

using namespace violet; // NOLINT(google-build-using-namespace)

TEST(Optionals, Basic)
{
    const Optional<UInt32> opt{};
    ASSERT_FALSE(opt.HasValue());
}

TEST(Optionals, Nothing)
{
    const Optional<UInt32> opt = Nothing;
    ASSERT_FALSE(opt.HasValue());
}

TEST(Optionals, InPlaceConstructor)
{
    const Optional<String> opt(std::in_place, "hello, world!");
    const Optional<String> opt2 = Some<String>("hello, world!");

    ASSERT_TRUE(opt.HasValue());
    ASSERT_TRUE(opt2.HasValue());

    ASSERT_EQ(opt.Value(), opt2.Value());
}

TEST(Optionals, CopyConstructor)
{
    const auto opt1 = Some<String>("hello");
    const auto opt2 = opt1; // NOLINT(performance-unnecessary-copy-initialization)

    ASSERT_TRUE(opt1.HasValue());
    ASSERT_TRUE(opt2.HasValue());
    ASSERT_EQ(opt1.Value(), "hello");
    ASSERT_EQ(opt2.Value(), "hello");
}

TEST(Optionals, MoveConstructor)
{
    auto opt1 = Some<String>("hello");
    auto opt2 = VIOLET_MOVE(opt1);

    ASSERT_FALSE(opt1.HasValue());
    ASSERT_TRUE(opt2.HasValue());
    ASSERT_EQ(opt2.Value(), "hello");
}

TEST(Optionals, CopyAssignment)
{
    const auto opt1 = Some<String>("hello");
    Optional<String> opt2;

    opt2 = opt1;

    ASSERT_TRUE(opt1.HasValue());
    ASSERT_TRUE(opt2.HasValue());
    ASSERT_EQ(opt1.Value(), "hello");
    ASSERT_EQ(opt2.Value(), "hello");
}

TEST(Optionals, MoveAssignment)
{
    auto opt1 = Some<String>("hello");
    Optional<String> opt2;

    opt2 = VIOLET_MOVE(opt1);

    ASSERT_FALSE(opt1.HasValue());
    ASSERT_TRUE(opt2.HasValue());
    ASSERT_EQ(opt2.Value(), "hello");
}

TEST(Optionals, HasValue)
{
    const Optional<UInt32> opt1{};
    const auto opt2 = Some<UInt32>(1);

    ASSERT_FALSE(opt1.HasValue());
    ASSERT_TRUE(opt2.HasValue());
}

TEST(Optionals, Value)
{
    const auto opt = Some<String>("world");
    ASSERT_EQ(opt.Value(), "world");
}

TEST(Optionals, Unwrap)
{
    auto opt = Some<String>("world");
    ASSERT_EQ(VIOLET_MOVE(opt).Unwrap(), "world");
}

TEST(Optionals, UnwrapOr)
{
    auto opt1 = Some<String>("world");
    Optional<String> opt2;

    ASSERT_EQ(VIOLET_MOVE(opt1).UnwrapOr("hello"), "world");
    ASSERT_EQ(VIOLET_MOVE(opt2).UnwrapOr("hello"), "hello");
}

TEST(Optionals, Map)
{
    const auto opt1 = Some<String>("hello");
    const auto opt2 = Optional<String>();

    const auto res1 = opt1.Map([](const String& value) -> UInt { return value.length(); });
    const auto res2 = opt2.Map([](const String& value) -> UInt { return value.length(); });

    ASSERT_TRUE(res1.HasValue());
    ASSERT_EQ(res1.Value(), 5);
    ASSERT_FALSE(res2.HasValue());
}

TEST(Optionals, MapOr)
{
    const auto opt1 = Some<String>("hello");
    const auto opt2 = Optional<String>();

    const auto res1 = opt1.MapOr(0, [](const String& value) -> UInt { return value.length(); });
    const auto res2 = opt2.MapOr(0, [](const String& value) -> UInt { return value.length(); });

    ASSERT_EQ(res1, 5);
    ASSERT_EQ(res2, 0);
}

TEST(Optionals, HasValueAnd)
{
    const auto opt1 = Some<UInt32>(2);
    const auto opt2 = Some<UInt32>(3);
    const auto opt3 = Optional<UInt32>();

    ASSERT_TRUE(opt1.HasValueAnd([](UInt32 value) -> bool { return value % 2 == 0; }));
    ASSERT_FALSE(opt2.HasValueAnd([](UInt32 value) -> bool { return value % 2 == 0; }));
    ASSERT_FALSE(opt3.HasValueAnd([](UInt32 value) -> bool { return value % 2 == 0; }));
}

TEST(Optionals, Take)
{
    auto opt1 = Some<String>("hello");
    const auto opt2 = opt1.Take();

    ASSERT_FALSE(opt1.HasValue());
    ASSERT_TRUE(opt2.HasValue());
    ASSERT_EQ(opt2.Value(), "hello");
}

TEST(Optionals, Reset)
{
    auto opt = Some<String>("hello");
    ASSERT_TRUE(opt.HasValue());

    opt.Reset();
    ASSERT_FALSE(opt.HasValue());
}

TEST(Optionals, SizeAndAlignmentRequirementsMustMatchRust)
{
    struct AlignTest final {
        alignas(16) char __dummy_data[16];
    };

    EXPECT_GE(sizeof(Optional<int>), sizeof(int));
    EXPECT_EQ(alignof(Optional<int>), alignof(int));

    EXPECT_EQ(alignof(Optional<AlignTest>), alignof(AlignTest));
    EXPECT_GE(sizeof(Optional<AlignTest>), sizeof(AlignTest) + 1);
}
