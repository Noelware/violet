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
#include <violet/Experimental/Pair.h>

#include <compare>
#include <memory>

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

static_assert(std::is_same_v<Pair<Int32, String>::first_type, Int32>);
static_assert(std::is_same_v<Pair<Int32, String>::second_type, String>);

static_assert(std::tuple_size_v<Pair<Int32, String>> == 2);
static_assert(std::tuple_size_v<const Pair<Int32, String>> == 2);
static_assert(std::is_same_v<std::tuple_element_t<0, Pair<Int32, String>>, Int32>);
static_assert(std::is_same_v<std::tuple_element_t<1, Pair<Int32, String>>, String>);
static_assert(std::is_same_v<std::tuple_element_t<1, const Pair<Int32, String>>, const String>);

// `std::pair` is trivially copyable/destructible when its members are.
static_assert(std::is_trivially_destructible_v<Pair<Int32, double>>);
static_assert(std::is_trivially_copy_constructible_v<Pair<Int32, double>>);
static_assert(std::is_trivially_move_constructible_v<Pair<Int32, double>>);

TEST(Pair, DefaultConstructorValueInitializes)
{
    // C++20 `std::pair` value-initializes both members, even with default-initialization.
    constexpr auto pair = [] consteval -> Pair<Int32, double> {
        Pair<Int32, double> pair;
        return pair;
    }();

    static_assert(pair.First == 0);
    static_assert(pair.Second == 0.0);

    Pair<String, Int32> strings;
    EXPECT_TRUE(strings.First.empty());
    EXPECT_EQ(strings.Second, 0);

    static_assert(std::is_nothrow_default_constructible_v<Pair<Int32, Int32>>);
    static_assert(!std::is_default_constructible_v<Pair<Explicit, Int32>>);
}

TEST(Pair, ConstructFromValues)
{
    constexpr Pair<Int32, double> pair(1, 2.5);
    static_assert(pair.First == 1);
    static_assert(pair.Second == 2.5);

    const String hello = "hello";
    Pair<String, Int32> fromLvalues(hello, 42);
    EXPECT_EQ(fromLvalues.First, "hello");
    EXPECT_EQ(fromLvalues.Second, 42);

    // Heterogeneous arguments are forwarded and converted.
    Pair<String, Int32> fromLiterals("world", 'a');
    EXPECT_EQ(fromLiterals.First, "world");
    EXPECT_EQ(fromLiterals.Second, 'a');
}

TEST(Pair, ConstructFromMoveOnlyValues)
{
    Pair<std::unique_ptr<Int32>, Int32> pair(std::make_unique<Int32>(5), 6);
    ASSERT_NE(pair.First, nullptr);
    EXPECT_EQ(*pair.First, 5);
    EXPECT_EQ(pair.Second, 6);
}

TEST(Pair, ValueConstructorIsConditionallyExplicit)
{
    static_assert(implicitly_constructible_from<Pair<Int32, String>, Int32, const char*>);

    static_assert(std::is_constructible_v<Pair<Explicit, Int32>, Int32, Int32>);
    static_assert(!implicitly_constructible_from<Pair<Explicit, Int32>, Int32, Int32>);
    static_assert(!implicitly_constructible_from<Pair<Int32, Explicit>, Int32, Int32>);
}

TEST(Pair, CopyAndMove)
{
    Pair<String, Int32> original("violet", 1);

    Pair<String, Int32> copy = original; // NOLINT(performance-unnecessary-copy-initialization)
    EXPECT_EQ(copy.First, "violet");
    EXPECT_EQ(original.First, "violet");

    Pair<String, Int32> moved = VIOLET_MOVE(original);
    EXPECT_EQ(moved.First, "violet");
    EXPECT_EQ(moved.Second, 1);

    Pair<std::unique_ptr<Int32>, Int32> unique(std::make_unique<Int32>(1), 2);
    Pair<std::unique_ptr<Int32>, Int32> stolen = VIOLET_MOVE(unique);
    EXPECT_EQ(unique.First, nullptr); // NOLINT(bugprone-use-after-move)
    ASSERT_NE(stolen.First, nullptr);
    EXPECT_EQ(*stolen.First, 1);

    static_assert(!std::is_copy_constructible_v<Pair<std::unique_ptr<Int32>, Int32>>);
    static_assert(std::is_nothrow_move_constructible_v<Pair<std::unique_ptr<Int32>, Int32>>);
}

TEST(Pair, ConvertingCopyConstructor)
{
    const Pair<Int32, float> source(1, 2.5F);
    Pair<Int64, double> widened = source;
    EXPECT_EQ(widened.First, 1L);
    EXPECT_DOUBLE_EQ(widened.Second, 2.5);

    static_assert(implicitly_constructible_from<Pair<Int64, double>, const Pair<Int32, float>&>);
    static_assert(std::is_constructible_v<Pair<Explicit, Int32>, const Pair<Int32, Int32>&>);
    static_assert(!implicitly_constructible_from<Pair<Explicit, Int32>, const Pair<Int32, Int32>&>);
    static_assert(!implicitly_constructible_from<Pair<Int32, Explicit>, const Pair<Int32, Int32>&>);
}

TEST(Pair, ConvertingMoveConstructor)
{
    Pair<std::unique_ptr<Derived>, Int32> source(std::make_unique<Derived>(), 3);
    Pair<std::unique_ptr<Base>, Int32> converted = VIOLET_MOVE(source);

    EXPECT_NE(converted.First, nullptr);
    EXPECT_EQ(converted.Second, 3);
    EXPECT_EQ(source.First, nullptr); // NOLINT(bugprone-use-after-move)

    static_assert(
        implicitly_constructible_from<Pair<std::unique_ptr<Base>, Int32>, Pair<std::unique_ptr<Derived>, Int32>>);

    // Explicit if *either* member conversion is explicit.
    static_assert(std::is_constructible_v<Pair<Explicit, Int32>, Pair<Int32, Int32>&&>);
    static_assert(!implicitly_constructible_from<Pair<Explicit, Int32>, Pair<Int32, Int32>&&>);
    static_assert(!implicitly_constructible_from<Pair<Int32, Explicit>, Pair<Int32, Int32>&&>);
}

TEST(Pair, ClassTemplateArgumentDeduction)
{
    Pair numbers(1, 2.0);
    static_assert(std::is_same_v<decltype(numbers), Pair<Int32, double>>);

    Pair literal("hello", 1);
    static_assert(std::is_same_v<decltype(literal), Pair<const char*, Int32>>);

    const String str = "x";
    Pair fromConst(str, 1);
    static_assert(std::is_same_v<decltype(fromConst), Pair<String, Int32>>);

    Pair copied(numbers);
    static_assert(std::is_same_v<decltype(copied), Pair<Int32, double>>);
}

TEST(Pair, Assignment)
{
    Pair<String, Int32> pair("a", 1);
    const Pair<String, Int32> other("b", 2);

    pair = other;
    EXPECT_EQ(pair.First, "b");
    EXPECT_EQ(pair.Second, 2);

    pair = Pair<String, Int32>("c", 3);
    EXPECT_EQ(pair.First, "c");
    EXPECT_EQ(pair.Second, 3);

    Pair<std::unique_ptr<Int32>, Int32> unique;
    unique = Pair<std::unique_ptr<Int32>, Int32>(std::make_unique<Int32>(9), 1);
    ASSERT_NE(unique.First, nullptr);
    EXPECT_EQ(*unique.First, 9);
}

TEST(Pair, Equality)
{
    constexpr Pair<Int32, Int32> a(1, 2);
    constexpr Pair<Int32, Int32> b(1, 2);
    constexpr Pair<Int32, Int32> c(1, 3);

    static_assert(a == b);
    static_assert(a != c);

    EXPECT_EQ((Pair<String, Int32>("x", 1)), (Pair<String, Int32>("x", 1)));
    EXPECT_NE((Pair<String, Int32>("x", 1)), (Pair<String, Int32>("y", 1)));
}

TEST(Pair, ThreeWayComparisonIsLexicographic)
{
    constexpr Pair<Int32, Int32> a(1, 9);
    constexpr Pair<Int32, Int32> b(2, 0);
    constexpr Pair<Int32, Int32> c(2, 1);

    static_assert((a <=> b) == std::strong_ordering::less);
    static_assert((c <=> b) == std::strong_ordering::greater);
    static_assert((b <=> b) == std::strong_ordering::equal);
    static_assert(a < b && b < c && c > a && a <= a && c >= b);

    // The comparison category is the common category of both members.
    static_assert(std::is_same_v<decltype(Pair<Int32, double>() <=> Pair<Int32, double>()), std::partial_ordering>);
    static_assert(std::is_same_v<decltype(Pair<Int32, String>() <=> Pair<Int32, String>()), std::strong_ordering>);

    EXPECT_LT((Pair<String, Int32>("a", 5)), (Pair<String, Int32>("b", 0)));
    EXPECT_LT((Pair<String, Int32>("a", 0)), (Pair<String, Int32>("a", 1)));
}

TEST(Pair, Swap)
{
    Pair<String, Int32> a("a", 1);
    Pair<String, Int32> b("b", 2);

    a.swap(b);
    EXPECT_EQ(a.First, "b");
    EXPECT_EQ(a.Second, 2);
    EXPECT_EQ(b.First, "a");
    EXPECT_EQ(b.Second, 1);

    swap(a, b);
    EXPECT_EQ(a.First, "a");
    EXPECT_EQ(b.First, "b");

    std::ranges::swap(a, b);
    EXPECT_EQ(a.First, "b");
    EXPECT_EQ(b.First, "a");

    static_assert(std::is_nothrow_swappable_v<Pair<Int32, String>>);
}

TEST(Pair, GetByIndex)
{
    Pair<Int32, String> pair(1, "one");

    EXPECT_EQ(get<0>(pair), 1);
    EXPECT_EQ(get<1>(pair), "one");

    get<0>(pair) = 2;
    EXPECT_EQ(pair.First, 2);

    static_assert(std::is_same_v<decltype(get<0>(pair)), Int32&>);
    static_assert(std::is_same_v<decltype(get<0>(std::as_const(pair))), const Int32&>);
    static_assert(std::is_same_v<decltype(get<1>(VIOLET_MOVE(pair))), String&&>);

    String stolen = get<1>(VIOLET_MOVE(pair));
    EXPECT_EQ(stolen, "one");
}

TEST(Pair, GetByType)
{
    constexpr Pair<Int32, double> pair(1, 2.5);
    static_assert(get<Int32>(pair) == 1);
    static_assert(get<double>(pair) == 2.5);
}

TEST(Pair, StructuredBindings)
{
    Pair<Int32, String> pair(7, "seven");

    auto [number, name] = pair;
    EXPECT_EQ(number, 7);
    EXPECT_EQ(name, "seven");

    auto& [numberRef, nameRef] = pair;
    numberRef = 8;
    nameRef = "eight";
    EXPECT_EQ(pair.First, 8);
    EXPECT_EQ(pair.Second, "eight");

    constexpr auto sum = [] consteval -> Int32 {
        auto [a, b] = Pair<Int32, Int32>(20, 22);
        return a + b;
    }();

    static_assert(sum == 42);
}

TEST(Pair, New)
{
    auto pair = Pair<Int32, String>::New(1, "a");
    static_assert(std::is_same_v<decltype(pair), Pair<Int32, String>>);

    EXPECT_EQ(pair.First, 1);
    EXPECT_EQ(pair.Second, "a");
}

TEST(Pair, MapFirst)
{
    Pair<Int32, String> pair(2, "two");

    auto doubled = pair.MapFirst([](Int32& value) -> Int32 { return value * 2; });
    static_assert(std::is_same_v<decltype(doubled), Pair<Int32, String>>);
    EXPECT_EQ(doubled.First, 4);
    EXPECT_EQ(doubled.Second, "two");

    const auto& constPair = pair;
    auto stringified = constPair.MapFirst([](const Int32& value) -> String { return violet::ToString(value); });
    static_assert(std::is_same_v<decltype(stringified), Pair<String, String>>);
    EXPECT_EQ(stringified.First, "2");

    Pair<std::unique_ptr<Int32>, std::unique_ptr<Int32>> unique(std::make_unique<Int32>(1), std::make_unique<Int32>(2));
    auto unwrapped
        = VIOLET_MOVE(unique).MapFirst([](std::unique_ptr<Int32>&& ptr) -> Int32 { return *VIOLET_MOVE(ptr); });

    EXPECT_EQ(unwrapped.First, 1);
    ASSERT_NE(unwrapped.Second, nullptr);
    EXPECT_EQ(*unwrapped.Second, 2);
}

TEST(Pair, MapSecond)
{
    Pair<Int32, String> pair(2, "two");

    auto length = pair.MapSecond([](String& value) -> UInt { return value.size(); });
    static_assert(std::is_same_v<decltype(length), Pair<Int32, UInt>>);
    EXPECT_EQ(length.First, 2);
    EXPECT_EQ(length.Second, 3U);

    const auto& constPair = pair;
    auto upper = constPair.MapSecond([](const String& value) -> String { return value + "!"; });
    EXPECT_EQ(upper.Second, "two!");
    EXPECT_EQ(pair.Second, "two");

    Pair<std::unique_ptr<Int32>, std::unique_ptr<Int32>> unique(std::make_unique<Int32>(1), std::make_unique<Int32>(2));
    auto unwrapped
        = VIOLET_MOVE(unique).MapSecond([](std::unique_ptr<Int32>&& ptr) -> Int32 { return *VIOLET_MOVE(ptr); });

    ASSERT_NE(unwrapped.First, nullptr);
    EXPECT_EQ(*unwrapped.First, 1);
    EXPECT_EQ(unwrapped.Second, 2);
}

TEST(Pair, Format)
{
    EXPECT_EQ(std::format("{}", Pair<Int32, String>(1, "one")), "{1, one}");
    EXPECT_EQ(std::format("{}", Pair<Int32, double>(1, 2.5)), "{1, 2.5}");
}

} // namespace violet::experimental
