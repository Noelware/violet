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

// NOLINTBEGIN(readability-identifier-length,google-build-using-namespace)

#include <gtest/gtest.h>
#include <violet/Experimental/OneOf.h>

using namespace violet;
using namespace experimental;

namespace {

template<typename T>
struct move_only final {
    constexpr VIOLET_DISALLOW_CONSTRUCTOR(move_only);
    VIOLET_DISALLOW_COPY(move_only);
    VIOLET_IMPLICIT_MOVE(move_only);
    ~move_only() = default;

    VIOLET_EXPLICIT move_only(T value)
        : n_value(value)
    {
    }

    auto operator==(const move_only& other) const noexcept -> bool
        requires std::equality_comparable<T>
    {
        return this->n_value == other.n_value;
    }

    [[nodiscard]] auto Value() const noexcept -> T
    {
        return this->n_value;
    }

private:
    T n_value;
};

template<typename T>
struct tracked_object final { // NOLINT(cppcoreguidelines-special-member-functions)
    static void Reset() noexcept
    {
        tracked_object::constructions = 0;
        tracked_object::destructions = 0;
    }

    VIOLET_EXPLICIT tracked_object(T value)
        : n_value(value)
    {
        ++constructions;
    }

    VIOLET_IMPLICIT tracked_object(const tracked_object& other)
        : n_value(other.n_value)
    {
        ++constructions;
    }

    VIOLET_IMPLICIT tracked_object(tracked_object&& other) noexcept
        : n_value(other.n_value)
    {
        ++constructions;
    }

    ~tracked_object() noexcept
    {
        ++destructions;
    }

    [[nodiscard]] auto Value() const noexcept -> T
    {
        return this->n_value;
    }

    [[nodiscard]] constexpr static auto Constructions() noexcept -> Int32
    {
        return tracked_object::constructions;
    }

    [[nodiscard]] constexpr static auto Destructions() noexcept -> Int32
    {
        return tracked_object::destructions;
    }

private:
    static inline Int32 constructions = 0;
    static inline Int32 destructions = 0;

    T n_value;
};

} // namespace

TEST(OneOfs, ConvertingConstructorInt)
{
    OneOf<Int32, float, double> value = 42;

    EXPECT_TRUE(value.Holds<Int32>());
    EXPECT_EQ(value.Index(), 0);
    ASSERT_TRUE(value.Get<Int32>());
    EXPECT_EQ(*value.Get<Int32>(), 42);
}

TEST(OneOfs, ConvertingConstructorFloat)
{
    OneOf<Int32, float, double> value = 3.14F;

    EXPECT_TRUE(value.Holds<float>());
    EXPECT_EQ(value.Index(), 1);
}

TEST(OneOfs, InPlaceConstruction)
{
    auto value = OneOf<Int32, String>::New<String>(5, 'x');

    ASSERT_TRUE(value.Holds<String>());
    EXPECT_EQ(*value.Get<String>(), "xxxxx");
}

TEST(OneOfs, MoveOnlyConstruction)
{
    OneOf<move_only<Int32>, Int32> value = move_only(7);
    ASSERT_TRUE(value.Holds<move_only<Int32>>());
    EXPECT_EQ(value.Get<move_only<Int32>>()->Value(), 7);
}

TEST(OneOfs, GetReturnsNothingOnMismatch)
{
    OneOf<Int32, float> value = 1;
    EXPECT_EQ(value.Get<float>(), Nothing);
}

TEST(OneOf, GetReturnsPointerOnMatch)
{
    OneOf<Int32, float> v = 2.5F;

    ASSERT_NE(v.Get<float>(), Nothing);
    EXPECT_FLOAT_EQ(*v.Get<float>(), 2.5F);
}

TEST(OneOf, ConstGetReturnsPointerOnMatch)
{
    const OneOf<int, float> v = 99;
    ASSERT_NE(v.Get<int>(), Nothing);
    EXPECT_EQ(*v.Get<int>(), 99);
}

TEST(OneOf, ConstGetReturnsNullptrOnMismatch)
{
    const OneOf<int, float> v = 99;
    EXPECT_EQ(v.Get<float>(), Nothing);
}

TEST(OneOf, CopyConstructor)
{
    OneOf<int, std::string> a = std::string{ "hello" };
    auto b = a;
    ASSERT_TRUE(b.Holds<std::string>());
    EXPECT_EQ(*b.Get<std::string>(), "hello");
}

TEST(OneOf, CopyDoesNotAliasStorage)
{
    OneOf<std::string, int> a = std::string{ "abc" };
    auto b = a;
    *b.Get<std::string>() = "xyz";
    EXPECT_EQ(*a.Get<std::string>(), "abc"); // a is unaffected
}

TEST(OneOf, MoveConstructor)
{
    OneOf<move_only<Int32>, int> a = move_only<Int32>{ 42 };
    auto b = std::move(a);
    ASSERT_TRUE(b.Holds<move_only<Int32>>());
    EXPECT_EQ(b.Get<move_only<Int32>>()->Value(), 42);
}

TEST(OneOf, CopyAssignment)
{
    OneOf<int, float> a = 10;
    OneOf<int, float> b = 2.5F;
    b = a;

    EXPECT_TRUE(b.Holds<int>());
    EXPECT_EQ(*b.Get<int>(), 10);
}

TEST(OneOf, MoveAssignment)
{
    OneOf<move_only<Int32>, int> a = move_only(9);
    OneOf<move_only<Int32>, int> b = 0;
    b = VIOLET_MOVE(a);

    ASSERT_TRUE(b.Holds<move_only<Int32>>());
    EXPECT_EQ(b.Get<move_only<Int32>>()->Value(), 9);
}

TEST(OneOf, ConvertingAssignment)
{
    OneOf<int, float> v = 1;
    v = 3.14F;

    EXPECT_TRUE(v.Holds<float>());
    EXPECT_FLOAT_EQ(*v.Get<float>(), 3.14F);
}

TEST(OneOf, DestructorCalledOnDestroy)
{
    tracked_object<Int32>::Reset();
    {
        OneOf<tracked_object<Int32>, int> v = tracked_object(1);
        EXPECT_EQ(tracked_object<Int32>::Constructions(), 2); // one for the temp, one in storage
    }

    EXPECT_EQ(tracked_object<Int32>::Destructions(), 2);
}

TEST(OneOf, DestructorCalledOnReassignment)
{
    tracked_object<Int32>::Reset();
    OneOf<tracked_object<Int32>, int> v = tracked_object(1);
    int before = tracked_object<Int32>::Destructions();
    v = 42; // tracked_object<Int32> should be destroyed during swap-based assignment

    EXPECT_GT(tracked_object<Int32>::Destructions(), before);
    EXPECT_TRUE(v.Holds<int>());
}

TEST(OneOf, VisitLvalue)
{
    OneOf<int, std::string> v = 7;
    int result = v.Visit([](auto& val) -> int {
        if constexpr (std::is_same_v<std::decay_t<decltype(val)>, int>)
            return val * 2;
        else
            return -1;
    });

    EXPECT_EQ(result, 14);
}

TEST(OneOf, VisitConst)
{
    const OneOf<int, float> v = 3.0F;
    float result = v.Visit([](const auto& val) -> float { return static_cast<float>(val); });
    EXPECT_FLOAT_EQ(result, 3.0F);
}

TEST(OneOf, VisitRvalue)
{
    OneOf<move_only<Int32>, int> v = move_only<Int32>(5);
    int result = std::move(v).Visit([](auto&& val) -> int {
        if constexpr (std::is_same_v<std::decay_t<decltype(val)>, move_only<Int32>>)
            return val.Value();
        else
            return -1;
    });

    EXPECT_EQ(result, 5);
}

TEST(OneOf, MatchExhaustive)
{
    OneOf<int, float, std::string> v = std::string("hi");
    std::string result = v.Match([](int) -> std::string { return std::string{ "int" }; },
        [](float) -> std::string { return std::string{ "float" }; }, [](std::string& s) -> std::string { return s; });

    EXPECT_EQ(result, "hi");
}

TEST(OneOf, MatchOnConst)
{
    const OneOf<int, double> v = 3.14982722832938932;
    int result = v.Match([](int i) -> int { return i; }, [](double d) -> int { return static_cast<int>(d); });
    EXPECT_EQ(result, 3);
}

TEST(OneOf, SwapSameType)
{
    OneOf<int, float> a = 1;
    OneOf<int, float> b = 2;
    swap(a, b);
    EXPECT_EQ(*a.Get<int>(), 2);
    EXPECT_EQ(*b.Get<int>(), 1);
}

TEST(OneOf, SwapDifferentTypes)
{
    OneOf<int, float> a = 42;
    OneOf<int, float> b = 1.5F;
    swap(a, b);
    EXPECT_TRUE(a.Holds<float>());
    EXPECT_TRUE(b.Holds<int>());
    EXPECT_FLOAT_EQ(*a.Get<float>(), 1.5F);
    EXPECT_EQ(*b.Get<int>(), 42);
}

TEST(OneOf, EqualitySameTypeAndValue)
{
    OneOf<int, float> a = 5;
    OneOf<int, float> b = 5;
    EXPECT_EQ(a, b);
}

TEST(OneOf, InequalitySameTypeDifferentValue)
{
    OneOf<int, float> a = 5;
    OneOf<int, float> b = 6;
    EXPECT_NE(a, b);
}

TEST(OneOf, InequalityDifferentTypes)
{
    OneOf<int, float> a = 5;
    OneOf<int, float> b = 5.0F;
    EXPECT_NE(a, b);
}

TEST(OneOf, HoldsCorrectType)
{
    OneOf<int, float, double> v = 1.0;
    EXPECT_FALSE(v.Holds<int>());
    EXPECT_FALSE(v.Holds<float>());
    EXPECT_TRUE(v.Holds<double>());
    EXPECT_EQ(v.Index(), 2U);
}

#ifdef VIOLET_HAS_EXCEPTIONS
namespace {

struct throw_on_copy_t final {
    VIOLET_IMPLICIT_MOVE(throw_on_copy_t);
    ~throw_on_copy_t() = default;

    VIOLET_IMPLICIT throw_on_copy_t() = default;
    VIOLET_IMPLICIT throw_on_copy_t(const throw_on_copy_t&)
    {
        throw std::runtime_error("copy failed?!");
    }

    auto operator=(const throw_on_copy_t&) noexcept -> throw_on_copy_t& = delete;

    auto operator==(const throw_on_copy_t&) const -> bool
    {
        return true;
    }
};

} // namespace

TEST(OneOfs, CopyAssignmentLeavesLhsIntactOnThrow)
{
    OneOf<throw_on_copy_t, int> a = 42;
    OneOf<throw_on_copy_t, int> b = throw_on_copy_t{};

    EXPECT_THROW({ a = b; }, std::runtime_error);

    // a must still hold its original value
    ASSERT_TRUE(a.Holds<int>());
    EXPECT_EQ(*a.Get<int>(), 42);
}

#endif

TEST(OneOf, SingleType)
{
    OneOf<int> v = 7;
    EXPECT_TRUE(v.Holds<int>());
    EXPECT_EQ(*v.Get<int>(), 7);
    EXPECT_EQ(v.Index(), 0U);
}

// NOLINTEND(readability-identifier-length,google-build-using-namespace)
