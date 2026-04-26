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

VIOLET_DIAGNOSTIC_PUSH

#if VIOLET_COMPILER(CLANG) || VIOLET_COMPILER(GCC)
VIOLET_DIAGNOSTIC_IGNORE("-Wself-move")
VIOLET_DIAGNOSTIC_IGNORE("-Wself-assign-overloaded")
#endif

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length,performance-unnecessary-copy-initialization,cppcoreguidelines-special-member-functions)
using namespace violet;

TEST(Optional, DefaultIsDisengaged)
{
    Optional<Int32> opt;
    EXPECT_FALSE(opt.HasValue());
    EXPECT_FALSE(static_cast<bool>(opt));
}

TEST(Optional, NothingIsDisengaged)
{
    Optional<Int32> opt = Nothing;
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, ConstructFromValue)
{
    Optional<Int32> opt = 42;
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromRvalue)
{
    Optional<String> opt = std::string("hello");
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), "hello");
}

TEST(Optional, ConstructInPlace)
{
    Optional<String> opt(std::in_place, 5, 'x');
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), "xxxxx");
}

TEST(Optional, ConstructFromSomeLvalue)
{
    Some<Int32> s(42);
    Optional<Int32> opt(s);
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromSomeRvalue)
{
    Optional<Int32> opt(Some<Int32>(42));
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromStdOptionalEngaged)
{
    std::optional<Int32> std_opt = 42;
    Optional<Int32> opt(std_opt);
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromStdOptionalEmpty)
{
    std::optional<Int32> std_opt;
    Optional<Int32> opt(std_opt);
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, ConstructFromStdOptionalRvalue)
{
    Optional<String> opt(std::optional<String>("hello"));
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), "hello");
}

TEST(Optional, CopyConstructEngaged)
{
    Optional<Int32> a = 42;
    Optional<Int32> b(a);
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), 42);
}

TEST(Optional, CopyConstructDisengaged)
{
    Optional<Int32> a;
    Optional<Int32> b(a);
    EXPECT_FALSE(b.HasValue());
}

TEST(Optional, CopyAssignEngagedToEngaged)
{
    Optional<Int32> a = 1;
    Optional<Int32> b = 2;
    b = a;
    EXPECT_EQ(b.Value(), 1);
}

TEST(Optional, CopyAssignEngagedToDisengaged)
{
    Optional<Int32> a = 42;
    Optional<Int32> b;
    b = a;
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), 42);
}

TEST(Optional, CopyAssignDisengagedToEngaged)
{
    Optional<Int32> a;
    Optional<Int32> b = 42;
    b = a;
    EXPECT_FALSE(b.HasValue());
}

TEST(Optional, CopyAssignSelf)
{
    Optional<Int32> a = 42;
    a = a;
    EXPECT_TRUE(a.HasValue());
    EXPECT_EQ(a.Value(), 42);
}

TEST(Optional, MoveConstructEngaged)
{
    Optional<String> a = std::string("hello");
    Optional<String> b(VIOLET_MOVE(a));
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), "hello");
    // a should be disengaged after move
    EXPECT_FALSE(a.HasValue()); // NOLINT(bugprone-use-after-move)
}

TEST(Optional, MoveConstructDisengaged)
{
    Optional<Int32> a;
    Optional<Int32> b(VIOLET_MOVE(a));
    EXPECT_FALSE(b.HasValue());
}

TEST(Optional, MoveAssignEngagedToEngaged)
{
    Optional<String> a = std::string("hello");
    Optional<String> b = std::string("world");
    b = VIOLET_MOVE(a);
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Optional, MoveAssignEngagedToDisengaged)
{
    Optional<String> a = std::string("hello");
    Optional<String> b;
    b = VIOLET_MOVE(a);
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Optional, MoveAssignDisengagedToEngaged)
{
    Optional<String> a;
    Optional<String> b = std::string("hello");
    b = VIOLET_MOVE(a);
    EXPECT_FALSE(b.HasValue());
}

TEST(Optional, MoveAssignSelf)
{
    Optional<Int32> a = 42;
    a = VIOLET_MOVE(a);
    // Should not crash; value may be in moved-from state
    EXPECT_TRUE(a.HasValue());
}

TEST(Optional, AssignNothingToEngaged)
{
    Optional<Int32> opt = 42;
    opt = Nothing;
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, AssignNothingToDisengaged)
{
    Optional<Int32> opt;
    opt = Nothing;
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, AssignValueLvalue)
{
    Optional<Int32> opt;
    int val = 42;
    opt = val;
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, AssignValueRvalue)
{
    Optional<String> opt;
    opt = std::string("hello");
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), "hello");
}

TEST(Optional, ReassignValue)
{
    Optional<Int32> opt = 1;
    opt = 42;
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ValueRefQualifiers)
{
    Optional<String> opt = std::string("hello");

    // lvalue
    EXPECT_EQ(opt.Value(), "hello");

    // const lvalue
    const auto& cref = opt;
    EXPECT_EQ(cref.Value(), "hello");
}

TEST(Optional, DerefOperator)
{
    Optional<Int32> opt = 42;
    EXPECT_EQ(*opt, 42);
}

TEST(Optional, ArrowOperator)
{
    Optional<String> opt = std::string("hello");
    EXPECT_EQ(opt->size(), 5U);
}

TEST(Optional, BoolConversion)
{
    Optional<Int32> engaged = 42;
    Optional<Int32> empty;

    EXPECT_TRUE(static_cast<bool>(engaged));
    EXPECT_FALSE(static_cast<bool>(empty));
}

TEST(Optional, UnwrapEngaged)
{
    Optional<Int32> opt = 42;
    EXPECT_EQ(opt.Unwrap(), 42);
}

TEST(Optional, UnwrapOrEngaged)
{
    Optional<Int32> opt = 42;
    EXPECT_EQ(opt.UnwrapOr(0), 42);
}

TEST(Optional, UnwrapOrDisengaged)
{
    Optional<Int32> opt;
    EXPECT_EQ(opt.UnwrapOr(99), 99);
}

TEST(Optional, UnwrapOrDefaultEngaged)
{
    Optional<Int32> opt = 42;
    EXPECT_EQ(opt.UnwrapOrDefault(), 42);
}

TEST(Optional, UnwrapOrDefaultDisengaged)
{
    Optional<Int32> opt;
    EXPECT_EQ(opt.UnwrapOrDefault(), 0);
}

TEST(Optional, UnwrapUnchecked)
{
    Optional<Int32> opt = 42;
    EXPECT_EQ(opt.UnwrapUnchecked(Unsafe("in test code; this will not fail")), 42);
}

TEST(Optional, ExceptEngaged)
{
    Optional<Int32> opt = 42;
    EXPECT_EQ(opt.Except("should not panic"), 42);
}

TEST(Optional, MapEngaged)
{
    Optional<Int32> opt = 21;
    auto mapped = opt.Map([](int v) { return v * 2; });
    EXPECT_TRUE(mapped.HasValue());
    EXPECT_EQ(mapped.Value(), 42);
}

TEST(Optional, MapDisengaged)
{
    Optional<Int32> opt;
    auto mapped = opt.Map([](int v) { return v * 2; });
    EXPECT_FALSE(mapped.HasValue());
}

TEST(Optional, MapChangesType)
{
    Optional<Int32> opt = 42;
    auto mapped = opt.Map([](int v) { return std::to_string(v); });
    EXPECT_TRUE(mapped.HasValue());
    EXPECT_EQ(mapped.Value(), "42");
}

TEST(Optional, MapRvalue)
{
    auto mapped = Optional<Int32>(21).Map([](int v) { return v * 2; });
    EXPECT_TRUE(mapped.HasValue());
    EXPECT_EQ(mapped.Value(), 42);
}

TEST(Optional, MapOrEngaged)
{
    Optional<Int32> opt = 21;
    auto val = opt.MapOr(0, [](int v) { return v * 2; });
    EXPECT_EQ(val, 42);
}

TEST(Optional, MapOrDisengaged)
{
    Optional<Int32> opt;
    auto val = opt.MapOr(99, [](int v) { return v * 2; });
    EXPECT_EQ(val, 99);
}

TEST(Optional, HasValueAndTrue)
{
    Optional<Int32> opt = 42;
    EXPECT_TRUE(opt.HasValueAnd([](int v) { return v > 0; }));
}

TEST(Optional, HasValueAndFalse)
{
    Optional<Int32> opt = 42;
    EXPECT_FALSE(opt.HasValueAnd([](int v) { return v < 0; }));
}

TEST(Optional, HasValueAndDisengaged)
{
    Optional<Int32> opt;
    EXPECT_FALSE(opt.HasValueAnd([](int) { return true; }));
}

TEST(Optional, TakeEngaged)
{
    Optional<Int32> opt = 42;
    auto taken = opt.Take();
    EXPECT_TRUE(taken.HasValue());
    EXPECT_EQ(taken.Value(), 42);
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, TakeDisengaged)
{
    Optional<Int32> opt;
    auto taken = opt.Take();
    EXPECT_FALSE(taken.HasValue());
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, ReplaceEngaged)
{
    Optional<Int32> opt = 1;
    auto& ref = opt.Replace(42);
    EXPECT_EQ(ref, 42);
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ReplaceDisengaged)
{
    Optional<Int32> opt;
    auto& ref = opt.Replace(42);
    EXPECT_EQ(ref, 42);
    EXPECT_TRUE(opt.HasValue());
}

TEST(Optional, Reset)
{
    Optional<Int32> opt = 42;
    opt.Reset();
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, ResetAlreadyDisengaged)
{
    Optional<Int32> opt;
    opt.Reset();
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, EqualityBothEngaged)
{
    Optional<Int32> a = 42;
    Optional<Int32> b = 42;
    EXPECT_EQ(a, b);
}

TEST(Optional, InequalityDifferentValues)
{
    Optional<Int32> a = 1;
    Optional<Int32> b = 2;
    EXPECT_NE(a, b);
}

TEST(Optional, EqualityBothDisengaged)
{
    Optional<Int32> a;
    Optional<Int32> b;
    // Both are Nothing — compare via nullopt
    EXPECT_EQ(a, Nothing);
    EXPECT_EQ(b, Nothing);
}

TEST(Optional, EqualityWithNothing)
{
    Optional<Int32> engaged = 42;
    Optional<Int32> empty;

    EXPECT_NE(engaged, Nothing);
    EXPECT_EQ(empty, Nothing);
}

TEST(Optional, EqualityWithValue)
{
    Optional<Int32> opt = 42;
    EXPECT_EQ(opt, 42);
    EXPECT_NE(opt, 0);
}

TEST(Optional, EqualityDisengagedWithValue)
{
    Optional<Int32> opt;
    EXPECT_NE(opt, 42);
}

TEST(Optional, EqualityWithStdOptional)
{
    Optional<Int32> opt = 42;
    std::optional<Int32> std_opt = 42;
    EXPECT_EQ(opt, std_opt);
}

TEST(Optional, InequalityWithStdOptional)
{
    Optional<Int32> opt = 42;
    std::optional<Int32> std_opt = 99;
    EXPECT_NE(opt, std_opt);
}

TEST(Optional, ConvertToStdOptionalEngaged)
{
    Optional<Int32> opt = 42;
    auto std_opt = static_cast<std::optional<Int32>>(opt);
    EXPECT_TRUE(std_opt);
    EXPECT_EQ(*std_opt, 42);
}

TEST(Optional, ConvertToStdOptionalDisengaged)
{
    Optional<Int32> opt;
    auto std_opt = static_cast<std::optional<Int32>>(opt);
    EXPECT_FALSE(std_opt);
}

TEST(Optional, MoveOnlyType)
{
    Optional<std::unique_ptr<Int32>> opt = std::make_unique<Int32>(42);
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(**opt, 42);

    auto moved = VIOLET_MOVE(opt);
    EXPECT_TRUE(moved.HasValue());
    EXPECT_EQ(*moved.Value(), 42);
}

TEST(Optional, VectorValue)
{
    std::vector<Int32> v = { 1, 2, 3 };
    Optional<std::vector<Int32>> opt = VIOLET_MOVE(v);
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value().size(), 3U);
}

TEST(Optional, DestructorCalledOnReset)
{
    static int dtor_count = 0;
    struct Counted final {
        ~Counted()
        {
            ++dtor_count;
        }
    };

    dtor_count = 0;
    {
        Optional<Counted> opt(std::in_place);
        EXPECT_EQ(dtor_count, 0);
        opt.Reset();
        EXPECT_EQ(dtor_count, 1);
    }
    // Destructor of disengaged optional should not call dtor again
    EXPECT_EQ(dtor_count, 1);
}

TEST(Optional, DestructorCalledOnScopeExit)
{
    static int dtor_count = 0;
    struct Counted {
        ~Counted()
        {
            ++dtor_count;
        }
    };

    dtor_count = 0;
    {
        Optional<Counted> opt(std::in_place);
    }
    EXPECT_EQ(dtor_count, 1);
}

TEST(Optional, ToStringEngaged)
{
    Optional<Int32> opt = 42;
    EXPECT_FALSE(opt.ToString().empty());
}

TEST(Optional, ToStringDisengaged)
{
    Optional<Int32> opt;
    auto s = opt.ToString();
    EXPECT_FALSE(s.empty());
}

TEST(Optional, StreamOperator)
{
    Optional<Int32> opt = 42;
    std::ostringstream os;
    os << opt;
    EXPECT_FALSE(os.str().empty());
}

TEST(OptionalRef, ConstructWithRef)
{
    int value = 42;
    Optional<std::reference_wrapper<Int32>> opt(std::ref(value));

    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, 42);
}

TEST(OptionalRef, MutatesThroughRef)
{
    int value = 10;

    Optional<std::reference_wrapper<Int32>> opt(std::ref(value));
    *opt = 99;

    EXPECT_EQ(value, 99);
}

TEST(OptionalRef, NoneState)
{
    Optional<std::reference_wrapper<Int32>> opt;
    EXPECT_FALSE(opt);
}

TEST(OptionalRef, ConstRef)
{
    const std::string s = "hello";
    Optional<std::reference_wrapper<const std::string>> opt(std::cref(s));

    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "hello");
}

TEST(OptionalRef, ResetFromValue)
{
    int value = 42;
    Optional<std::reference_wrapper<Int32>> opt(std::ref(value));
    ASSERT_TRUE(opt);

    opt.Reset();
    EXPECT_FALSE(opt);
}

TEST(OptionalRef, MapThroughRef)
{
    int value = 5;
    Optional<std::reference_wrapper<Int32>> opt(std::ref(value));
    auto mapped = opt.Map([](int ref) -> int { return ref * 2; });

    ASSERT_TRUE(mapped);
    EXPECT_EQ(*mapped, 10);

    // Original unchanged
    EXPECT_EQ(value, 5);
}

TEST(OptionalRef, MapOnNone)
{
    Optional<std::reference_wrapper<Int32>> opt;

    bool called = false;
    auto mapped = opt.Map([&](int ref) -> int {
        called = true;
        return ref;
    });

    EXPECT_FALSE(mapped);
    EXPECT_FALSE(called);
}

TEST(OptionalRef, EqualityBetweenRefs)
{
    int a = 42;
    int b = 42;

    Optional<std::reference_wrapper<Int32>> opt_a(std::ref(a));
    Optional<std::reference_wrapper<Int32>> opt_b(std::ref(b));
    Optional<std::reference_wrapper<Int32>> none;

    // Both wrap value 42, but different objects — depends on your == semantics.
    // std::reference_wrapper::operator== compares the underlying values.
    EXPECT_EQ(*opt_a, *opt_b);
    EXPECT_NE(none, opt_a);
}

TEST(OptionalRef, CopySemantics)
{
    int value = 7;
    Optional<std::reference_wrapper<Int32>> a(std::ref(value));
    auto b = a;

    ASSERT_TRUE(b);
    *b = 77;

    // Both refs point to the same int
    EXPECT_EQ(*a, 77);
    EXPECT_EQ(value, 77);
}

TEST(OptionalRef, MoveSemantics)
{
    int value = 3;
    Optional<std::reference_wrapper<Int32>> a(std::ref(value));
    auto b = VIOLET_MOVE(a);

    ASSERT_TRUE(b);
    EXPECT_EQ(*b, 3);
}

TEST(OptionalFromStd, ConstructFromEngagedStdOptional)
{
    std::optional<Int32> std_opt = 42;
    Optional<Int32> opt(std_opt);

    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, 42);
}

TEST(OptionalFromStd, ConstructFromEmptyStdOptional)
{
    std::optional<Int32> std_opt;
    Optional<Int32> opt(std_opt);

    EXPECT_FALSE(opt);
}

TEST(OptionalFromStd, ConstructFromStdOptionalRvalue)
{
    std::optional<String> std_opt = "hello";
    Optional<String> opt(VIOLET_MOVE(std_opt));

    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "hello");
}

TEST(OptionalFromStd, ConstructFromEmptyStdOptionalRvalue)
{
    std::optional<String> std_opt;
    Optional<String> opt(VIOLET_MOVE(std_opt));

    EXPECT_FALSE(opt);
}

TEST(OptionalFromStd, AssignEngagedStdOptionalToNone)
{
    Optional<Int32> opt;
    std::optional<Int32> std_opt = 10;

    opt = std_opt;

    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, 10);
}

TEST(OptionalFromStd, AssignEmptyStdOptionalToEngaged)
{
    Optional<Int32> opt = 42;
    std::optional<Int32> std_opt;

    opt = std_opt;
    EXPECT_FALSE(opt);
}

TEST(OptionalFromStd, AssignEngagedStdOptionalToEngaged)
{
    Optional<Int32> opt = 1;
    std::optional<Int32> std_opt = 2;

    opt = std_opt;
    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, 2);
}

TEST(OptionalFromStd, AssignEmptyStdOptionalToEmpty)
{
    Optional<Int32> opt;
    std::optional<Int32> std_opt;

    opt = std_opt;
    EXPECT_FALSE(opt);
}

TEST(OptionalFromStd, MoveAssignStdOptional)
{
    Optional<String> opt;
    std::optional<String> std_opt = "moved";

    opt = VIOLET_MOVE(std_opt);
    ASSERT_TRUE(opt);
    EXPECT_EQ(*opt, "moved");
}

TEST(OptionalFromStd, MoveAssignEmptyStdOptional)
{
    Optional<String> opt = "existing";
    std::optional<String> std_opt;

    opt = VIOLET_MOVE(std_opt);
    EXPECT_FALSE(opt);
}

TEST(OptionalFromStd, WithMoveOnlyType)
{
    std::optional<std::unique_ptr<Int32>> std_opt = std::make_unique<Int32>(99);
    Optional<std::unique_ptr<Int32>> opt(VIOLET_MOVE(std_opt));

    ASSERT_TRUE(opt);
    ASSERT_NE(*opt, nullptr);
    EXPECT_EQ(**opt, 99);
}

TEST(OptionalFromStd, WithStruct)
{
    struct Point {
        int x;
        int y;
    };

    std::optional<Point> std_opt = Point{ .x = 3, .y = 4 };
    Optional<Point> opt(std_opt);

    ASSERT_TRUE(opt);
    EXPECT_EQ(opt->x, 3);
    EXPECT_EQ(opt->y, 4);
}

TEST(OptionalFromStd, PreservesValueSemantics)
{
    std::optional<String> std_opt = "original";
    Optional<String> opt(std_opt);

    // Mutating the violet optional shouldn't affect the std one
    *opt = "changed";
    EXPECT_EQ(*std_opt, "original");
    EXPECT_EQ(*opt, "changed");
}

TEST(OptionalToStd, ConvertEngagedToStdOptional)
{
    Optional<Int32> opt = 42;
    std::optional<Int32> std_opt(opt);

    ASSERT_TRUE(std_opt.has_value());
    EXPECT_EQ(*std_opt, 42);
}

TEST(OptionalToStd, ConvertEmptyToStdOptional)
{
    Optional<Int32> opt;
    std::optional<Int32> std_opt(opt);

    EXPECT_FALSE(std_opt.has_value());
}

TEST(OptionalTraits, IsOptional)
{
    static_assert(is_optional_v<Optional<Int32>>);
    static_assert(is_optional_v<std::optional<Int32>>);
    static_assert(!is_optional_v<Int32>);
    static_assert(!is_optional_v<Some<Int32>>);
}

TEST(OptionalTraits, OptionalType)
{
    static_assert(std::same_as<optional_type_t<Optional<Int32>>, int>);
    static_assert(std::same_as<optional_type_t<std::optional<String>>, std::string>);
}

TEST(OptionalConstexpr, DefaultIsDisengaged)
{
    constexpr Optional<Int32> opt;
    static_assert(!opt.HasValue());
    static_assert(!static_cast<bool>(opt));
}

TEST(OptionalConstexpr, NothingIsDisengaged)
{
    constexpr Optional<Int32> opt = Nothing;
    static_assert(!opt.HasValue());
}

TEST(OptionalConstexpr, ConstructFromValue)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(opt.HasValue());
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, ConstructInPlace)
{
    constexpr Optional<Int32> opt(std::in_place, 42);
    static_assert(opt.HasValue());
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, ConstructFromSome)
{
    constexpr Optional<Int32> opt(Some<Int32>(42));
    static_assert(opt.HasValue());
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, BoolConversion)
{
    constexpr Optional<Int32> engaged = 42;
    constexpr Optional<Int32> empty;

    static_assert(static_cast<bool>(engaged));
    static_assert(!static_cast<bool>(empty));
}

TEST(OptionalConstexpr, DerefOperator)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(*opt == 42);
}

TEST(OptionalConstexpr, ValueAccess)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, UnwrapOrEngaged)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(opt.UnwrapOr(0) == 42);
}

TEST(OptionalConstexpr, UnwrapOrDisengaged)
{
    constexpr Optional<Int32> opt;
    static_assert(opt.UnwrapOr(99) == 99);
}

TEST(OptionalConstexpr, UnwrapOrDefaultEngaged)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(opt.UnwrapOrDefault() == 42);
}

TEST(OptionalConstexpr, UnwrapOrDefaultDisengaged)
{
    constexpr Optional<Int32> opt;
    static_assert(opt.UnwrapOrDefault() == 0);
}

TEST(OptionalConstexpr, UnwrapUnchecked)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(opt.UnwrapUnchecked(Unsafe("in test code; don't care about safety")) == 42);
}

TEST(OptionalConstexpr, MapEngaged)
{
    constexpr auto mapped = Optional<Int32>(21).Map([](int v) { return v * 2; });
    static_assert(mapped.HasValue());
    static_assert(mapped.Value() == 42);
}

TEST(OptionalConstexpr, MapDisengaged)
{
    constexpr auto mapped = Optional<Int32>().Map([](int v) { return v * 2; });
    static_assert(!mapped.HasValue());
}

TEST(OptionalConstexpr, MapOrEngaged)
{
    constexpr Optional<Int32> opt = 21;
    constexpr auto val = opt.MapOr(0, [](int v) { return v * 2; });
    static_assert(val == 42);
}

TEST(OptionalConstexpr, MapOrDisengaged)
{
    constexpr Optional<Int32> opt;
    constexpr auto val = opt.MapOr(99, [](int v) { return v * 2; });
    static_assert(val == 99);
}

TEST(OptionalConstexpr, HasValueAndTrue)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(opt.HasValueAnd([](int v) { return v > 0; }));
}

TEST(OptionalConstexpr, HasValueAndFalse)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(!opt.HasValueAnd([](int v) { return v < 0; }));
}

TEST(OptionalConstexpr, HasValueAndDisengaged)
{
    constexpr Optional<Int32> opt;
    static_assert(!opt.HasValueAnd([](int) { return true; }));
}

TEST(OptionalConstexpr, EqualityWithNothing)
{
    constexpr Optional<Int32> engaged = 42;
    constexpr Optional<Int32> empty;

    static_assert(engaged != Nothing);
    static_assert(empty == Nothing);
}

TEST(OptionalConstexpr, EqualityWithValue)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(opt == 42);
    static_assert(opt != 0);
}

TEST(OptionalConstexpr, EqualityBothEngaged)
{
    constexpr Optional<Int32> a = 42;
    constexpr Optional<Int32> b = 42;
    constexpr Optional<Int32> c = 99;

    static_assert(a == b);
    static_assert(a != c);
}

TEST(OptionalConstexpr, CopyConstruct)
{
    constexpr Optional<Int32> a = 42;
    constexpr Optional<Int32> b(a);
    static_assert(b.HasValue());
    static_assert(b.Value() == 42);
}

TEST(OptionalConstexpr, CopyConstructDisengaged)
{
    constexpr Optional<Int32> a;
    constexpr Optional<Int32> b(a);
    static_assert(!b.HasValue());
}

namespace {

consteval auto ConstexprMapChain() -> int
{
    return Optional<Int32>(10).Map([](int v) { return v + 5; }).Map([](int v) { return v * 2; }).UnwrapOr(0);
}

consteval auto ConstexprTakeAndReplace() -> int
{
    Optional<Int32> opt = 42;
    auto taken = opt.Take();
    if (opt.HasValue()) {
        return -1; // should be disengaged
    }

    (void)opt.Replace(99);
    return opt.Value() + taken.Value(); // 99 + 42
}

consteval auto ConstexprMoveConstruct() -> int
{
    Optional<Int32> a = 42;
    Optional<Int32> b(static_cast<Optional<Int32>&&>(a));
    if (a.HasValue()) {
        return -1; // should be disengaged
    }
    return b.Value();
}

} // namespace

TEST(OptionalConsteval, MapChain)
{
    static_assert(ConstexprMapChain() == 30);
}

TEST(OptionalConsteval, TakeAndReplace)
{
    static_assert(ConstexprTakeAndReplace() == 141);
}

TEST(OptionalConsteval, MoveConstruct)
{
    static_assert(ConstexprMoveConstruct() == 42);
}

TEST(OptionalTraitsConstexpr, IsOptional)
{
    static_assert(is_optional_v<Optional<Int32>>);
    static_assert(is_optional_v<std::optional<Int32>>);
    static_assert(!is_optional_v<Int32>);
    static_assert(!is_optional_v<Some<Int32>>);
}

TEST(OptionalTraitsConstexpr, OptionalType)
{
    static_assert(std::same_as<optional_type_t<Optional<Int32>>, int>);
    static_assert(std::same_as<optional_type_t<Optional<double>>, double>);
    static_assert(std::same_as<optional_type_t<std::optional<Int32>>, int>);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length,performance-unnecessary-copy-initialization,cppcoreguidelines-special-member-functions)

VIOLET_DIAGNOSTIC_POP
