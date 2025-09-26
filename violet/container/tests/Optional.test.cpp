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

#include "violet/container/Optional.h"

#include <gtest/gtest.h>
#include <optional>

using namespace Noelware::Violet; // NOLINT(google-build-using-namespace)

TEST(Optionals, DefaultConstruction)
{
    Optional<uint32> opt;
    EXPECT_FALSE(opt.HasValue());
    EXPECT_FALSE(static_cast<bool>(opt));
}

TEST(Optionals, NullOptValue)
{
    Optional<uint32> opt = Nothing;
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optionals, InPlaceConstruction)
{
    Optional<String> opt1 = Optional<String>(std::in_place, "32");
    EXPECT_TRUE(opt1.HasValue());
    EXPECT_EQ(static_cast<std::optional<String>>(opt1), static_cast<std::optional<String>>("32"));
}

TEST(Optionals, CopyConstruction)
{
    Optional<uint32> aa(42);
    Optional<uint32> bb = Optional(aa);

    EXPECT_TRUE(bb.HasValue());
    EXPECT_EQ(bb, std::optional{ 42 });
}

TEST(Optionals, MoveConstruction)
{
    Optional<uint32> aa(42);
    Optional<uint32> bb(VIOLET_MOVE(aa));

    EXPECT_TRUE(bb.HasValue());
    EXPECT_EQ(*bb, 42);
    EXPECT_FALSE(aa.HasValue());
}

TEST(Optionals, StdOptionalCopy)
{
    std::optional<uint32> so(10);
    Optional<uint32> vo(so);

    EXPECT_TRUE(vo.HasValue());
    EXPECT_EQ(*vo, 10);
}

TEST(Optionals, StdOptionalMove)
{
    std::optional<uint32> so(10);
    Optional<uint32> vo(VIOLET_MOVE(so));

    EXPECT_TRUE(vo.HasValue());
    EXPECT_EQ(*vo, 10);
}

TEST(Optionals, AssignmentFromNullOpt)
{
    Optional<uint32> opt(42);
    opt = Nothing;

    EXPECT_FALSE(opt.HasValue());
}

TEST(Optionals, CopyAssignment)
{
    Optional<uint32> aa(42);
    Optional<uint32> bb;

    bb = aa;
    EXPECT_TRUE(bb.HasValue());
    EXPECT_EQ(*bb, 42);
}

TEST(Optionals, MoveAssignment)
{
    Optional<uint32> aa(42);
    Optional<uint32> bb;

    bb = VIOLET_MOVE(aa);
    EXPECT_TRUE(bb.HasValue());
    EXPECT_EQ(*bb, 42);
}

TEST(Optionals, AssignmentFromValue)
{
    Optional<uint32> aa(42);
    Optional<uint32> bb;

    bb = 69;
    EXPECT_TRUE(bb.HasValue());
    EXPECT_EQ(*bb, 69);
}

TEST(Optionals, AssignmentFromStdOptional)
{
    std::optional<uint32> aa(42);
    Optional<uint32> bb;

    bb = aa;
    EXPECT_TRUE(bb.HasValue());
    EXPECT_EQ(*bb, 42);
}

TEST(Optionals, ResetShouldWork)
{
    Optional<uint32> opt(123);
    opt.Reset();

    EXPECT_FALSE(opt.HasValue());
}

TEST(Optionals, ReplaceShouldWork)
{
    Optional<uint32> opt(10);
    auto& ref = opt.Replace(20);

    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(ref, 20);
    EXPECT_EQ(*opt, 20);
}

TEST(Optionals, TakeShouldWork)
{
    Optional<String> opt("hi");
    auto taken = opt.Take();

    EXPECT_TRUE(taken.HasValue());
    EXPECT_EQ(*taken, "hi");
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optionals, MapTransformsValue)
{
    Optional<uint32> opt(5);
    auto result = opt.Map([](uint32 xy) { return xy * 2; });
    EXPECT_TRUE(result.HasValue());
    EXPECT_EQ(*result, 10);

    Optional<uint32> empty;
    auto mapped = empty.Map([](uint32 xy) { return xy * 2; });
    EXPECT_FALSE(mapped.HasValue());
}

TEST(Optionals, InspectRunsSideEffects)
{
    Optional<uint32> opt(5);
    uint32 seen = 0;

    opt.Inspect([&](uint32 vv) { seen = vv; });
    EXPECT_EQ(seen, 5);
}
