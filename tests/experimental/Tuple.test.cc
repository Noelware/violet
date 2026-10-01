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
#include <violet/Experimental/Tuple.h>

namespace violet::experimental {
namespace {

struct Explicit final {
    Int32 Value;

    constexpr VIOLET_EXPLICIT Explicit(Int32 value)
        : Value(value)
    {
    }
};

struct Base {
    virtual ~Base() = default;
};

struct Derived final: public Base { };

template<typename T, typename... Args>
concept implicitly_constructible_from = requires(Args&&... args) { [](T) -> void { }({VIOLET_FWD(Args, args)...}); };

} // namespace

static_assert(std::tuple_size_v<Tuple<>> == 0);
static_assert(std::tuple_size_v<Tuple<Int32, String, double>> == 3);
static_assert(std::tuple_size_v<const Tuple<Int32, String, double>> == 3);
static_assert(Tuple<Int32, String, double>::Size() == 3);

static_assert(std::is_same_v<std::tuple_element_t<0, Tuple<Int32, String, double>>, Int32>);
static_assert(std::is_same_v<std::tuple_element_t<1, Tuple<Int32, String, double>>, String>);
static_assert(std::is_same_v<std::tuple_element_t<2, Tuple<Int32, String, double>>, double>);
static_assert(std::is_same_v<std::tuple_element_t<0, const Tuple<Int32>>, const Int32>);
static_assert(std::is_same_v<std::tuple_element_t<0, Tuple<Int32&>>, Int32&>);

static_assert(is_tuple_v<Tuple<>>);
static_assert(is_tuple_v<Tuple<Int32, String>>);
static_assert(!is_tuple_v<Int32>);
static_assert(tuple_like<const Tuple<Int32>&>);
static_assert(!tuple_like<Int32>);

static_assert(std::is_trivially_destructible_v<Tuple<Int32, double>>);
static_assert(std::is_trivially_copy_constructible_v<Tuple<Int32, double>>);
static_assert(std::is_trivially_move_constructible_v<Tuple<Int32, double>>);

static_assert(implicitly_constructible_from<Tuple<Int32, String>, Int32, const char*>);
static_assert(std::is_constructible_v<Tuple<Explicit, Int32>, Int32, Int32>);
static_assert(!implicitly_constructible_from<Tuple<Explicit, Int32>, Int32, Int32>);
static_assert(!implicitly_constructible_from<Tuple<Int32, Explicit>, Int32, Int32>);
static_assert(!std::is_constructible_v<Tuple<Int32, Int32>, Int32>);
static_assert(!std::is_constructible_v<Tuple<Int32, Int32>, Int32, Int32, Int32>);

TEST(Tuple, Empty)
{
    constexpr Tuple<> empty;
    static_assert(empty.Size() == 0); // NOLINT(readability-static-accessed-through-instance)
    static_assert(empty == Tuple<>());
}

TEST(Tuple, DefaultConstructorValueInitializes)
{
    // C++20 `std::tuple` value-initializes every element, even with default-initialization.
    constexpr auto tuple = [] consteval -> Tuple<Int32, double, bool> {
        Tuple<Int32, double, bool> tuple;
        return tuple;
    }();

    static_assert(tuple.At<0>() == 0);
    static_assert(tuple.At<1>() == 0.0);
    static_assert(!tuple.At<2>());

    Tuple<String, Int32> strings;
    EXPECT_TRUE(strings.At<0>().empty());
    EXPECT_EQ(strings.At<1>(), 0);

    static_assert(!std::is_default_constructible_v<Tuple<Int32, Explicit>>);
    static_assert(!std::is_default_constructible_v<Tuple<Int32&>>);
}

TEST(Tuple, ConstructFromValues)
{
    constexpr Tuple<Int32, double, char> tuple(1, 2.5, 'c');
    static_assert(tuple.At<0>() == 1);
    static_assert(tuple.At<1>() == 2.5);
    static_assert(tuple.At<2>() == 'c');

    const String hello = "hello";
    Tuple<String, String, Int32> strings(hello, "world", 'a');
    EXPECT_EQ(strings.At<0>(), "hello");
    EXPECT_EQ(strings.At<1>(), "world");
    EXPECT_EQ(strings.At<2>(), 'a');
}

TEST(Tuple, ConstructFromMoveOnlyValues)
{
    Tuple<std::unique_ptr<Int32>, Int32> tuple(std::make_unique<Int32>(5), 6);
    ASSERT_NE(tuple.At<0>(), nullptr);
    EXPECT_EQ(*tuple.At<0>(), 5);
    EXPECT_EQ(tuple.At<1>(), 6);
}

TEST(Tuple, CopyAndMove)
{
    Tuple<String, Int32> original("violet", 1);
    Tuple<String, Int32> copy = original; // NOLINT(performance-unnecessary-copy-initialization)
    EXPECT_EQ(copy.At<0>(), "violet");
    EXPECT_EQ(original.At<0>(), "violet");

    Tuple<String, Int32> moved = VIOLET_MOVE(original);
    EXPECT_EQ(moved.At<0>(), "violet");
    EXPECT_EQ(moved.At<1>(), 1);

    Tuple<std::unique_ptr<Int32>> unique(std::make_unique<Int32>(1));
    Tuple<std::unique_ptr<Int32>> stolen = VIOLET_MOVE(unique);
    EXPECT_EQ(unique.At<0>(), nullptr); // NOLINT(bugprone-use-after-move)
    ASSERT_NE(stolen.At<0>(), nullptr);
    EXPECT_EQ(*stolen.At<0>(), 1);

    static_assert(!std::is_copy_constructible_v<Tuple<std::unique_ptr<Int32>>>);
    static_assert(std::is_nothrow_move_constructible_v<Tuple<std::unique_ptr<Int32>>>);
}

TEST(Tuple, SingleElementCopyIsNotHijacked)
{
    Tuple<Int32> inner(5);
    Tuple<Int32> copy(inner);
    EXPECT_EQ(copy.At<0>(), 5);

    Tuple<Tuple<Int32>> nested(inner);
    EXPECT_EQ(nested.At<0>().At<0>(), 5);
}

TEST(Tuple, ConvertingCopyConstructor)
{
    const Tuple<Int32, float> source(1, 2.5F);
    Tuple<Int64, double> widened = source;
    EXPECT_EQ(widened.At<0>(), 1L);
    EXPECT_DOUBLE_EQ(widened.At<1>(), 2.5);

    static_assert(implicitly_constructible_from<Tuple<Int64, double>, const Tuple<Int32, float>&>);
    static_assert(std::is_constructible_v<Tuple<Explicit, Int32>, const Tuple<Int32, Int32>&>);
    static_assert(!implicitly_constructible_from<Tuple<Explicit, Int32>, const Tuple<Int32, Int32>&>);
}

TEST(Tuple, ConvertingMoveConstructor)
{
    Tuple<std::unique_ptr<Derived>, Int32> source(std::make_unique<Derived>(), 3);
    Tuple<std::unique_ptr<Base>, Int32> converted = VIOLET_MOVE(source);

    EXPECT_NE(converted.At<0>(), nullptr);
    EXPECT_EQ(converted.At<1>(), 3);
    EXPECT_EQ(source.At<0>(), nullptr); // NOLINT(bugprone-use-after-move)

    static_assert(std::is_constructible_v<Tuple<Explicit, Int32>, Tuple<Int32, Int32>&&>);
    static_assert(!implicitly_constructible_from<Tuple<Explicit, Int32>, Tuple<Int32, Int32>&&>);
}

TEST(Tuple, ClassTemplateArgumentDeduction)
{
    Tuple numbers(1, 2.0, 'c');
    static_assert(std::is_same_v<decltype(numbers), Tuple<Int32, double, char>>);

    Tuple literal("hello", 1);
    static_assert(std::is_same_v<decltype(literal), Tuple<const char*, Int32>>);

    Tuple copied(numbers);
    static_assert(std::is_same_v<decltype(copied), Tuple<Int32, double, char>>);
}

TEST(Tuple, Assignment)
{
    Tuple<String, Int32> tuple("a", 1);
    const Tuple<String, Int32> other("b", 2);

    tuple = other;
    EXPECT_EQ(tuple.At<0>(), "b");
    EXPECT_EQ(tuple.At<1>(), 2);

    tuple = Tuple<String, Int32>("c", 3);
    EXPECT_EQ(tuple.At<0>(), "c");
    EXPECT_EQ(tuple.At<1>(), 3);
}

TEST(Tuple, ConvertingAssignment)
{
    Tuple<Int64, double> tuple(0L, 0.0);

    const Tuple<Int32, float> source(1, 2.5F);
    tuple = source;
    EXPECT_EQ(tuple.At<0>(), 1L);
    EXPECT_DOUBLE_EQ(tuple.At<1>(), 2.5);

    tuple = Tuple<Int32, float>(3, 4.5F);
    EXPECT_EQ(tuple.At<0>(), 3L);
    EXPECT_DOUBLE_EQ(tuple.At<1>(), 4.5);

    Tuple<std::unique_ptr<Base>> base;
    base = Tuple<std::unique_ptr<Derived>>(std::make_unique<Derived>());
    EXPECT_NE(base.At<0>(), nullptr);
}

TEST(Tuple, ReferenceMembersAssignThrough)
{
    Int32 a = 1;
    Int32 b = 2;

    // Behaves like `std::tie`.
    Tuple<Int32&, Int32&> tied(a, b);
    tied = Tuple<Int32, Int32>(10, 20);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 20);

    Int32 c = 3;
    Int32 d = 4;
    const Tuple<Int32&, Int32&> other(c, d);
    tied = other;
    EXPECT_EQ(a, 3);
    EXPECT_EQ(b, 4);
    EXPECT_EQ(&tied.At<0>(), &a);
}

TEST(Tuple, AtReturnsReferences)
{
    Tuple<Int32, String> tuple(1, "one");

    static_assert(std::is_same_v<decltype(tuple.At<0>()), Int32&>);
    static_assert(std::is_same_v<decltype(std::as_const(tuple).At<0>()), const Int32&>);
    static_assert(std::is_same_v<decltype(VIOLET_MOVE(tuple).At<1>()), String&&>);
    static_assert(std::is_same_v<decltype(VIOLET_MOVE(std::as_const(tuple)).At<1>()), const String&&>);

    tuple.At<0>() = 2;
    EXPECT_EQ(tuple.At<0>(), 2);

    // `At` must refer to the stored element, not a copy of it.
    EXPECT_EQ(&tuple.At<1>(), &std::as_const(tuple).At<1>());

    String stolen = VIOLET_MOVE(tuple).At<1>();
    EXPECT_EQ(stolen, "one");
    EXPECT_TRUE(tuple.At<1>().empty()); // NOLINT(bugprone-use-after-move)
}

TEST(Tuple, GetByIndex)
{
    Tuple<Int32, String, double> tuple(1, "one", 1.5);

    EXPECT_EQ(get<0>(tuple), 1);
    EXPECT_EQ(get<1>(tuple), "one");
    EXPECT_EQ(get<2>(tuple), 1.5);

    get<0>(tuple) = 2;
    EXPECT_EQ(tuple.At<0>(), 2);

    static_assert(std::is_same_v<decltype(get<0>(tuple)), Int32&>);
    static_assert(std::is_same_v<decltype(get<0>(std::as_const(tuple))), const Int32&>);
    static_assert(std::is_same_v<decltype(get<1>(VIOLET_MOVE(tuple))), String&&>);
}

TEST(Tuple, GetByType)
{
    constexpr Tuple<Int32, double, char> tuple(1, 2.5, 'c');
    static_assert(get<Int32>(tuple) == 1);
    static_assert(get<double>(tuple) == 2.5);
    static_assert(get<char>(tuple) == 'c');
}

TEST(Tuple, StructuredBindings)
{
    Tuple<Int32, String, double> tuple(7, "seven", 7.5);

    auto [number, name, real] = tuple;
    EXPECT_EQ(number, 7);
    EXPECT_EQ(name, "seven");
    EXPECT_EQ(real, 7.5);

    auto& [numberRef, nameRef, realRef] = tuple;
    numberRef = 8;
    nameRef = "eight";
    EXPECT_EQ(tuple.At<0>(), 8);
    EXPECT_EQ(tuple.At<1>(), "eight");

    constexpr auto sum = [] consteval -> Int32 {
        auto [a, b, c] = Tuple<Int32, Int32, Int32>(20, 20, 2);
        return a + b + c;
    }();

    static_assert(sum == 42);
}

TEST(Tuple, ReferenceElements)
{
    Int32 value = 1;
    Tuple<Int32&> ref(value);

    ref.At<0>() = 5;
    EXPECT_EQ(value, 5);
    EXPECT_EQ(&ref.At<0>(), &value);
}

TEST(Tuple, NewDecaysLikeMakeTuple)
{
    Int32 value = 1;
    const String str = "s";

    auto tuple = Tuple<>::New(value, str, "literal", std::ref(value));
    static_assert(std::is_same_v<decltype(tuple), Tuple<Int32, String, const char*, Int32&>>);

    EXPECT_EQ(tuple.At<0>(), 1);
    EXPECT_EQ(tuple.At<1>(), "s");

    // `std::reference_wrapper` is unwrapped into a real reference.
    tuple.At<3>() = 42;
    EXPECT_EQ(value, 42);

    // Plain values are copies.
    tuple.At<0>() = 9;
    EXPECT_EQ(value, 42);
}

TEST(Tuple, FwdAsLikeForwardAsTuple)
{
    Int32 value = 1;
    String str = "s";

    auto forwarded = Tuple<>::FwdAs(value, VIOLET_MOVE(str));
    static_assert(std::is_same_v<decltype(forwarded), Tuple<Int32&, String&&>>);

    forwarded.At<0>() = 2;
    EXPECT_EQ(value, 2);

    String stolen = VIOLET_MOVE(forwarded).At<1>();
    EXPECT_EQ(stolen, "s");
}

TEST(Tuple, Concat)
{
    auto joined = Tuple<Int32, String>(1, "a").Concat(Tuple<double, char>(2.5, 'b'));
    static_assert(std::is_same_v<decltype(joined), Tuple<Int32, String, double, char>>);

    EXPECT_EQ(joined.At<0>(), 1);
    EXPECT_EQ(joined.At<1>(), "a");
    EXPECT_EQ(joined.At<2>(), 2.5);
    EXPECT_EQ(joined.At<3>(), 'b');

    auto unique = Tuple<std::unique_ptr<Int32>>(std::make_unique<Int32>(1))
                      .Concat(Tuple<std::unique_ptr<Int32>>(std::make_unique<Int32>(2)));

    ASSERT_NE(unique.At<0>(), nullptr);
    ASSERT_NE(unique.At<1>(), nullptr);
    EXPECT_EQ(*unique.At<0>(), 1);
    EXPECT_EQ(*unique.At<1>(), 2);
}

TEST(Tuple, Apply)
{
    Tuple<Int32, Int32, Int32> tuple(1, 2, 3);
    EXPECT_EQ(tuple.Apply([](Int32 a, Int32 b, Int32 c) -> Int32 { return a + b + c; }), 6);

    // Elements are passed by reference.
    tuple.Apply([](Int32& a, Int32&, Int32&) -> void { a = 100; });
    EXPECT_EQ(tuple.At<0>(), 100);

    const Tuple<String, Int32> constTuple("x", 3);
    auto repeated = constTuple.Apply([](const String& s, Int32 n) -> String {
        String out;
        for (Int32 i = 0; i < n; i++) {
            out += s;
        }

        return out;
    });

    EXPECT_EQ(repeated, "xxx");

    constexpr auto product = Tuple<Int32, Int32>(6, 7).Apply([](Int32 a, Int32 b) -> Int32 { return a * b; });
    static_assert(product == 42);

    EXPECT_EQ(Tuple<>().Apply([]() -> Int32 { return 1; }), 1);
}

TEST(Tuple, Equality)
{
    constexpr Tuple<Int32, Int32> a(1, 2);
    constexpr Tuple<Int32, Int32> b(1, 2);
    constexpr Tuple<Int32, Int32> c(1, 3);

    static_assert(a == b);
    static_assert(a != c);
    static_assert(Tuple<Int32, double>(1, 2.0) == Tuple<Int64, float>(1L, 2.0F));

    EXPECT_EQ((Tuple<String, Int32>("x", 1)), (Tuple<String, Int32>("x", 1)));
    EXPECT_NE((Tuple<String, Int32>("x", 1)), (Tuple<String, Int32>("y", 1)));
}

TEST(Tuple, ThreeWayComparisonIsLexicographic)
{
    constexpr Tuple<Int32, Int32, Int32> a(1, 9, 9);
    constexpr Tuple<Int32, Int32, Int32> b(2, 0, 0);
    constexpr Tuple<Int32, Int32, Int32> c(2, 0, 1);

    static_assert((a <=> b) == std::strong_ordering::less);
    static_assert((c <=> b) == std::strong_ordering::greater);
    static_assert((b <=> b) == std::strong_ordering::equal);
    static_assert(a < b && b < c && c > a && a <= a && c >= b);

    // The comparison category is the common category of every element.
    static_assert(std::is_same_v<decltype(Tuple<Int32, double>() <=> Tuple<Int32, double>()), std::partial_ordering>);
    static_assert(std::is_same_v<decltype(Tuple<Int32, String>() <=> Tuple<Int32, String>()), std::strong_ordering>);
    static_assert((Tuple<>() <=> Tuple<>()) == std::strong_ordering::equal);

    EXPECT_LT((Tuple<String, Int32>("a", 5)), (Tuple<String, Int32>("b", 0)));
    EXPECT_LT((Tuple<String, Int32>("a", 0)), (Tuple<String, Int32>("a", 1)));
}

TEST(Tuple, ComparisonsAcrossManySpecializations)
{
    // Every `Tuple<...>` specialization in a translation unit must be able to coexist
    // with its comparison operators.
    EXPECT_TRUE((Tuple<Int32>(1) == Tuple<Int32>(1)));
    EXPECT_TRUE((Tuple<double>(1.0) == Tuple<double>(1.0)));
    EXPECT_TRUE((Tuple<char, char>('a', 'b') < Tuple<char, char>('a', 'c')));
}

TEST(Tuple, Swap)
{
    Tuple<String, Int32> a("a", 1);
    Tuple<String, Int32> b("b", 2);

    a.swap(b);
    EXPECT_EQ(a.At<0>(), "b");
    EXPECT_EQ(a.At<1>(), 2);
    EXPECT_EQ(b.At<0>(), "a");
    EXPECT_EQ(b.At<1>(), 1);

    swap(a, b);
    EXPECT_EQ(a.At<0>(), "a");
    EXPECT_EQ(b.At<0>(), "b");

    std::ranges::swap(a, b);
    EXPECT_EQ(a.At<0>(), "b");
    EXPECT_EQ(b.At<0>(), "a");

    static_assert(std::is_nothrow_swappable_v<Tuple<Int32, String>>);
}

} // namespace violet::experimental
