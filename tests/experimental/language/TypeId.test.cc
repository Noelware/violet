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
#include <violet/Experimental/Language/TypeId.h>

using violet::experimental::TypeId;

namespace {

struct Foo final { };
struct Bar final { };

template<typename T>
struct Wrapper final { };

} // namespace

TEST(TypeId, SameTypeProducesSameId)
{
    EXPECT_EQ(TypeId::Of<Foo>(), TypeId::Of<Foo>());
}

TEST(TypeId, DifferentTypesProduceDifferentIds)
{
    EXPECT_NE(TypeId::Of<Foo>(), TypeId::Of<Bar>());
}

TEST(TypeId, TemplateInstantiationsAreDistinct)
{
    EXPECT_NE(TypeId::Of<Wrapper<Foo>>(), TypeId::Of<Wrapper<Bar>>());
}

TEST(TypeId, TemplateInstantiationDiffersFromArgument)
{
    EXPECT_NE(TypeId::Of<Wrapper<Foo>>(), TypeId::Of<Foo>());
}

TEST(TypeId, ConstIsDistinctFromUnqualified)
{
    EXPECT_NE(TypeId::Of<Foo>(), TypeId::Of<const Foo>());
}

TEST(TypeId, LvalueRefIsDistinctFromUnqualified)
{
    EXPECT_NE(TypeId::Of<Foo>(), TypeId::Of<Foo&>());
}

TEST(TypeId, RvalueRefIsDistinctFromUnqualified)
{
    EXPECT_NE(TypeId::Of<Foo>(), TypeId::Of<Foo&&>());
}

TEST(TypeId, ConstRefIsDistinctFromRef)
{
    EXPECT_NE(TypeId::Of<Foo&>(), TypeId::Of<const Foo&>());
}

TEST(TypeId, PointerIsDistinctFromUnqualified)
{
    EXPECT_NE(TypeId::Of<Foo>(), TypeId::Of<Foo*>());
}

TEST(TypeId, ConstPointerIsDistinctFromPointer)
{
    EXPECT_NE(TypeId::Of<Foo*>(), TypeId::Of<const Foo*>());
}

TEST(TypeId, PrimitiveTypesAreDistinct)
{
    EXPECT_NE(TypeId::Of<int>(), TypeId::Of<long>());
    EXPECT_NE(TypeId::Of<int>(), TypeId::Of<unsigned int>());
    EXPECT_NE(TypeId::Of<int>(), TypeId::Of<float>());
    EXPECT_NE(TypeId::Of<float>(), TypeId::Of<double>());
    EXPECT_NE(TypeId::Of<char>(), TypeId::Of<unsigned char>());
    EXPECT_NE(TypeId::Of<char>(), TypeId::Of<signed char>());
}

TEST(TypeId, SamePrimitiveProducesSameId)
{
    EXPECT_EQ(TypeId::Of<int>(), TypeId::Of<int>());
    EXPECT_EQ(TypeId::Of<double>(), TypeId::Of<double>());
    EXPECT_EQ(TypeId::Of<void>(), TypeId::Of<void>());
}

// NOLINTBEGIN(readability-identifier-length)
TEST(TypeId, IdIsStableAcrossMultipleCalls)
{
    auto a = TypeId::Of<Foo>();
    auto b = TypeId::Of<Foo>();
    auto c = TypeId::Of<Foo>();
    EXPECT_EQ(a, b);
    EXPECT_EQ(b, c);
}

TEST(TypeId, IdIsCopyable)
{
    auto a = TypeId::Of<Foo>();
    auto b = a; // NOLINT
    EXPECT_EQ(a, b);
}

TEST(TypeId, HashCodeIsConsistentWithEquality)
{
    EXPECT_EQ(TypeId::Of<Foo>().HashCode(), TypeId::Of<Foo>().HashCode());
}

TEST(TypeId, HashCodeDiffersForDifferentTypes)
{
    // Not guaranteed by the contract but true in practice for distinct statics.
    EXPECT_NE(TypeId::Of<Foo>().HashCode(), TypeId::Of<Bar>().HashCode());
}

TEST(TypeId, UsableAsUnorderedMapKey)
{
    std::unordered_map<TypeId, std::string_view> map;
    map[TypeId::Of<Foo>()] = "Foo";
    map[TypeId::Of<Bar>()] = "Bar";

    EXPECT_EQ(map.at(TypeId::Of<Foo>()), "Foo");
    EXPECT_EQ(map.at(TypeId::Of<Bar>()), "Bar");
    EXPECT_NE(map.find(TypeId::Of<Foo>()), map.find(TypeId::Of<Bar>()));
}

TEST(TypeId, OfIsConstexpr)
{
    constexpr auto id = TypeId::Of<Foo>();

#ifndef VIOLET_GCC
    static_assert(TypeId::Of<Foo>() == TypeId::Of<Foo>());
    static_assert(TypeId::Of<Foo>() != TypeId::Of<Bar>());
#else
    // gcc fucking sucks dude (it is only hit on UBSan, unsure if that is the cause but i dont care, i want to blame gcc
    // anyway for my love of clang because that totally solves my mental problems (probably))
    //
    // error: '(((const void*)(&violet::experimental::TypeId::sentinel_t<{anonymous}::Foo>::kValue)) != ((const
    // void*)(&violet::experimental::TypeId::sentinel_t<{anonymous}::Bar>::kValue)))' is not a constant expression
    EXPECT_NE(TypeId::Of<Foo>(), TypeId::Of<Bar>());
#endif

    (void)id;
}

// NOLINTEND(readability-identifier-length)
