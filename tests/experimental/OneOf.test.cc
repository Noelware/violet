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
    OneOf<int, String> a = String{ "hello" };
    auto b = a;
    ASSERT_TRUE(b.Holds<String>());
    EXPECT_EQ(*b.Get<String>(), "hello");
}

TEST(OneOf, CopyDoesNotAliasStorage)
{
    OneOf<String, int> a = String{ "abc" };
    auto b = a;
    *b.Get<String>() = "xyz";
    EXPECT_EQ(*a.Get<String>(), "abc"); // a is unaffected
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
    OneOf<int, String> v = 7;
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
    OneOf<int, float, String> v = String("hi");
    String result = v.Match([](int) -> String { return String{ "int" }; },
        [](float) -> String { return String{ "float" }; }, [](String& s) -> String { return s; });

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
    OneOf<throw_on_copy_t, int> b = throw_on_copy_t{ };

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

TEST(OneOfVisit, LvalueVoidReturn)
{
    OneOf<Mono, int, double> o;
    o = 42;

    int seen_int = 0;
    double seen_double = 0.0;
    o.Visit([&](auto& x) -> void {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, int>) {
            seen_int = x;
        } else if constexpr (std::is_same_v<T, double>) {
            seen_double = x;
        }
    });

    EXPECT_EQ(seen_int, 42);
    EXPECT_EQ(seen_double, 0.0);
}

TEST(OneOfVisit, LvalueValueReturn)
{
    OneOf<int, double> o;
    o = 42;

    auto doubled = o.Visit([](auto& x) -> int { return static_cast<int>(x) * 2; });

    EXPECT_EQ(doubled, 84);
    static_assert(std::is_same_v<decltype(doubled), int>);
}

TEST(OneOfVisit, LvalueAllowsMutation)
{
    OneOf<Mono, int, double> o;
    o = 10;

    o.Visit([](auto& x) -> void {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, int>) {
            x += 5;
        }
    });

    auto stored = o.template Get<int>();
    ASSERT_TRUE(stored.HasValue());
    EXPECT_EQ(*stored, 15);
}

TEST(OneOfVisit, ConstLvalueVoidReturn)
{
    OneOf<Mono, int, double> o;
    o = 3.14;
    const auto& co = o;

    int seen_int = 0;
    double seen_double = 0.0;
    co.Visit([&](const auto& x) -> void {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, int>) {
            seen_int = x;
        } else if constexpr (std::is_same_v<T, double>) {
            seen_double = x;
        }
    });

    EXPECT_EQ(seen_int, 0);
    EXPECT_DOUBLE_EQ(seen_double, 3.14);
}

TEST(OneOfVisit, ConstLvalueRejectsMutation)
{
    // Primarily a compile-time check: visitor takes `const auto&`,
    // so attempting to mutate should be a compile error. We verify at
    // runtime that the visitor runs and sees the expected type.
    OneOf<Mono, int> o;
    o = 42;
    const auto& co = o;

    bool const_ref_observed = false;
    co.Visit([&](const auto& x) -> void {
        using Ref = decltype(x);
        const_ref_observed = std::is_const_v<std::remove_reference_t<Ref>>;
    });

    EXPECT_TRUE(const_ref_observed);
}

TEST(OneOfVisit, RvalueVoidReturn)
{
    OneOf<Mono, int, String> o;
    o = String{ "hello" };

    String seen;
    VIOLET_MOVE(o).Visit([&](auto&& x) -> void {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, String>) {
            seen = VIOLET_MOVE(x);
        }
    });

    EXPECT_EQ(seen, "hello");
}

TEST(OneOfVisit, RvalueValueReturn)
{
    OneOf<int, double> o;
    o = 21;

    auto doubled = VIOLET_MOVE(o).Visit([](auto&& x) -> int { return static_cast<int>(x) * 2; });

    EXPECT_EQ(doubled, 42);
}

TEST(OneOfVisit, RvalueMovesMoveOnlyValue)
{
    // This test would've been broken by using `std::common_type_t` if any branch
    // returned a `std::unique_ptr`
    OneOf<std::unique_ptr<int>, String> o(std::make_unique<int>(42));
    auto ptr = VIOLET_MOVE(o).Visit([](auto&& x) -> std::unique_ptr<int> {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::unique_ptr<int>>) {
            return VIOLET_MOVE(x);
        } else {
            return nullptr;
        }
    });

    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);
}

TEST(OneOfVisit, DecayedReturnTypeInt)
{
    OneOf<int, double> o;
    o = 10;

    int sum = o.Visit([](auto& x) -> int { return static_cast<int>(x) + 1; });
    EXPECT_EQ(sum, 11);
}

TEST(OneOfVisit, DecayedReturnTypeString)
{
    OneOf<int, double> o;
    o = 42;

    String s = o.Visit([](auto& x) -> String { return std::format("value={}", x); });
    EXPECT_EQ(s, "value=42");
}

TEST(OneOfVisit, DecayedReturnTypeBool)
{
    OneOf<int, double> o;
    o = 3.14;

    bool is_double = o.Visit([](auto& x) -> bool { return std::is_same_v<std::decay_t<decltype(x)>, double>; });

    EXPECT_TRUE(is_double);
}

TEST(OneOfVisit, DecayedReturnTypeMoveOnlyValue)
{
    OneOf<int, double> o;
    o = 7;

    auto ptr = o.Visit([](auto& x) -> std::unique_ptr<int> { return std::make_unique<int>(static_cast<int>(x)); });

    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 7);
}

TEST(OneOfVisit, DispatchesToCorrectAlternativeInt)
{
    OneOf<Mono, int, double, String> o;
    o = 42;

    auto tag = o.Visit([](auto& x) -> int {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, int>)
            return 1;
        else if constexpr (std::is_same_v<T, double>)
            return 2;
        else if constexpr (std::is_same_v<T, String>)
            return 3;
        else
            return 0;
    });

    EXPECT_EQ(tag, 1);
}

TEST(OneOfVisit, DispatchesToCorrectAlternativeDouble)
{
    OneOf<Mono, int, double, String> o;
    o = 3.14;

    auto tag = o.Visit([](auto& x) -> int {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, int>)
            return 1;
        else if constexpr (std::is_same_v<T, double>)
            return 2;
        else if constexpr (std::is_same_v<T, String>)
            return 3;
        else
            return 0;
    });

    EXPECT_EQ(tag, 2);
}

TEST(OneOfVisit, DispatchesToCorrectAlternativeString)
{
    OneOf<Mono, int, double, String> o;
    o = String{ "hi" };

    auto tag = o.Visit([](auto& x) -> int {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, int>)
            return 1;
        else if constexpr (std::is_same_v<T, double>)
            return 2;
        else if constexpr (std::is_same_v<T, String>)
            return 3;
        else
            return 0;
    });

    EXPECT_EQ(tag, 3);
}

TEST(OneOfVisit, StatefulFunctorIsInvoked)
{
    struct Counter {
        int calls = 0;
        auto operator()(int&) -> int
        {
            return ++calls;
        }
        auto operator()(double&) -> int
        {
            return ++calls;
        }
    };

    OneOf<int, double> o;
    o = 1;

    Counter c;
    auto r1 = o.Visit(std::ref(c));
    EXPECT_EQ(r1, 1);

    auto r2 = o.Visit(std::ref(c));
    EXPECT_EQ(r2, 2);
}

TEST(OneOfVisit, TypeTaggedDispatch)
{
    // Simulate a message-dispatch pattern — each alternative has its
    // own handler, and the visitor picks the right one at runtime.
    OneOf<Mono, int, String, double> msg;
    msg = String{ "hello" };

    String out = msg.Visit([](auto& m) -> String {
        using T = std::decay_t<decltype(m)>;
        if constexpr (std::is_same_v<T, int>) {
            return std::format("int({})", m);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::format("double({})", m);
        } else if constexpr (std::is_same_v<T, String>) {
            return std::format("str(\"{}\")", m);
        } else {
            return "mono";
        }
    });

    EXPECT_EQ(out, "str(\"hello\")");
}

// NOLINTEND(readability-identifier-length,google-build-using-namespace)
