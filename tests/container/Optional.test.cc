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
    Optional<int> opt;
    EXPECT_FALSE(opt.HasValue());
    EXPECT_FALSE(static_cast<bool>(opt));
}

TEST(Optional, NothingIsDisengaged)
{
    Optional<int> opt = Nothing;
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, ConstructFromValue)
{
    Optional<int> opt = 42;
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromRvalue)
{
    Optional<std::string> opt = std::string("hello");
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), "hello");
}

TEST(Optional, ConstructInPlace)
{
    Optional<std::string> opt(std::in_place, 5, 'x');
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), "xxxxx");
}

TEST(Optional, ConstructFromSomeLvalue)
{
    Some<int> s(42);
    Optional<int> opt(s);
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromSomeRvalue)
{
    Optional<int> opt(Some<int>(42));
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromStdOptionalEngaged)
{
    std::optional<int> std_opt = 42;
    Optional<int> opt(std_opt);
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromStdOptionalEmpty)
{
    std::optional<int> std_opt;
    Optional<int> opt(std_opt);
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, ConstructFromStdOptionalRvalue)
{
    Optional<std::string> opt(std::optional<std::string>("hello"));
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), "hello");
}

TEST(Optional, CopyConstructEngaged)
{
    Optional<int> a = 42;
    Optional<int> b(a);
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), 42);
}

TEST(Optional, CopyConstructDisengaged)
{
    Optional<int> a;
    Optional<int> b(a);
    EXPECT_FALSE(b.HasValue());
}

TEST(Optional, CopyAssignEngagedToEngaged)
{
    Optional<int> a = 1;
    Optional<int> b = 2;
    b = a;
    EXPECT_EQ(b.Value(), 1);
}

TEST(Optional, CopyAssignEngagedToDisengaged)
{
    Optional<int> a = 42;
    Optional<int> b;
    b = a;
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), 42);
}

TEST(Optional, CopyAssignDisengagedToEngaged)
{
    Optional<int> a;
    Optional<int> b = 42;
    b = a;
    EXPECT_FALSE(b.HasValue());
}

TEST(Optional, CopyAssignSelf)
{
    Optional<int> a = 42;
    a = a;
    EXPECT_TRUE(a.HasValue());
    EXPECT_EQ(a.Value(), 42);
}

TEST(Optional, MoveConstructEngaged)
{
    Optional<std::string> a = std::string("hello");
    Optional<std::string> b(VIOLET_MOVE(a));
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), "hello");
    // a should be disengaged after move
    EXPECT_FALSE(a.HasValue()); // NOLINT(bugprone-use-after-move)
}

TEST(Optional, MoveConstructDisengaged)
{
    Optional<int> a;
    Optional<int> b(VIOLET_MOVE(a));
    EXPECT_FALSE(b.HasValue());
}

TEST(Optional, MoveAssignEngagedToEngaged)
{
    Optional<std::string> a = std::string("hello");
    Optional<std::string> b = std::string("world");
    b = VIOLET_MOVE(a);
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Optional, MoveAssignEngagedToDisengaged)
{
    Optional<std::string> a = std::string("hello");
    Optional<std::string> b;
    b = VIOLET_MOVE(a);
    EXPECT_TRUE(b.HasValue());
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Optional, MoveAssignDisengagedToEngaged)
{
    Optional<std::string> a;
    Optional<std::string> b = std::string("hello");
    b = VIOLET_MOVE(a);
    EXPECT_FALSE(b.HasValue());
}

TEST(Optional, MoveAssignSelf)
{
    Optional<int> a = 42;
    a = VIOLET_MOVE(a);
    // Should not crash; value may be in moved-from state
    EXPECT_TRUE(a.HasValue());
}

TEST(Optional, AssignNothingToEngaged)
{
    Optional<int> opt = 42;
    opt = Nothing;
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, AssignNothingToDisengaged)
{
    Optional<int> opt;
    opt = Nothing;
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, AssignValueLvalue)
{
    Optional<int> opt;
    int val = 42;
    opt = val;
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, AssignValueRvalue)
{
    Optional<std::string> opt;
    opt = std::string("hello");
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(opt.Value(), "hello");
}

TEST(Optional, ReassignValue)
{
    Optional<int> opt = 1;
    opt = 42;
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ValueRefQualifiers)
{
    Optional<std::string> opt = std::string("hello");

    // lvalue
    EXPECT_EQ(opt.Value(), "hello");

    // const lvalue
    const auto& cref = opt;
    EXPECT_EQ(cref.Value(), "hello");
}

TEST(Optional, DerefOperator)
{
    Optional<int> opt = 42;
    EXPECT_EQ(*opt, 42);
}

TEST(Optional, ArrowOperator)
{
    Optional<std::string> opt = std::string("hello");
    EXPECT_EQ(opt->size(), 5U);
}

TEST(Optional, BoolConversion)
{
    Optional<int> engaged = 42;
    Optional<int> empty;

    EXPECT_TRUE(static_cast<bool>(engaged));
    EXPECT_FALSE(static_cast<bool>(empty));
}

TEST(Optional, UnwrapEngaged)
{
    Optional<int> opt = 42;
    EXPECT_EQ(opt.Unwrap(), 42);
}

TEST(Optional, UnwrapOrEngaged)
{
    Optional<int> opt = 42;
    EXPECT_EQ(opt.UnwrapOr(0), 42);
}

TEST(Optional, UnwrapOrDisengaged)
{
    Optional<int> opt;
    EXPECT_EQ(opt.UnwrapOr(99), 99);
}

TEST(Optional, UnwrapOrDefaultEngaged)
{
    Optional<int> opt = 42;
    EXPECT_EQ(opt.UnwrapOrDefault(), 42);
}

TEST(Optional, UnwrapOrDefaultDisengaged)
{
    Optional<int> opt;
    EXPECT_EQ(opt.UnwrapOrDefault(), 0);
}

TEST(Optional, UnwrapUnchecked)
{
    Optional<int> opt = 42;
    EXPECT_EQ(opt.UnwrapUnchecked(Unsafe("in test code; this will not fail")), 42);
}

TEST(Optional, ExceptEngaged)
{
    Optional<int> opt = 42;
    EXPECT_EQ(opt.Except("should not panic"), 42);
}

TEST(Optional, MapEngaged)
{
    Optional<int> opt = 21;
    auto mapped = opt.Map([](int v) { return v * 2; });
    EXPECT_TRUE(mapped.HasValue());
    EXPECT_EQ(mapped.Value(), 42);
}

TEST(Optional, MapDisengaged)
{
    Optional<int> opt;
    auto mapped = opt.Map([](int v) { return v * 2; });
    EXPECT_FALSE(mapped.HasValue());
}

TEST(Optional, MapChangesType)
{
    Optional<int> opt = 42;
    auto mapped = opt.Map([](int v) { return std::to_string(v); });
    EXPECT_TRUE(mapped.HasValue());
    EXPECT_EQ(mapped.Value(), "42");
}

TEST(Optional, MapRvalue)
{
    auto mapped = Optional<int>(21).Map([](int v) { return v * 2; });
    EXPECT_TRUE(mapped.HasValue());
    EXPECT_EQ(mapped.Value(), 42);
}

TEST(Optional, MapOrEngaged)
{
    Optional<int> opt = 21;
    auto val = opt.MapOr(0, [](int v) { return v * 2; });
    EXPECT_EQ(val, 42);
}

TEST(Optional, MapOrDisengaged)
{
    Optional<int> opt;
    auto val = opt.MapOr(99, [](int v) { return v * 2; });
    EXPECT_EQ(val, 99);
}

TEST(Optional, HasValueAndTrue)
{
    Optional<int> opt = 42;
    EXPECT_TRUE(opt.HasValueAnd([](int v) { return v > 0; }));
}

TEST(Optional, HasValueAndFalse)
{
    Optional<int> opt = 42;
    EXPECT_FALSE(opt.HasValueAnd([](int v) { return v < 0; }));
}

TEST(Optional, HasValueAndDisengaged)
{
    Optional<int> opt;
    EXPECT_FALSE(opt.HasValueAnd([](int) { return true; }));
}

TEST(Optional, TakeEngaged)
{
    Optional<int> opt = 42;
    auto taken = opt.Take();
    EXPECT_TRUE(taken.HasValue());
    EXPECT_EQ(taken.Value(), 42);
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, TakeDisengaged)
{
    Optional<int> opt;
    auto taken = opt.Take();
    EXPECT_FALSE(taken.HasValue());
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, ReplaceEngaged)
{
    Optional<int> opt = 1;
    auto& ref = opt.Replace(42);
    EXPECT_EQ(ref, 42);
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ReplaceDisengaged)
{
    Optional<int> opt;
    auto& ref = opt.Replace(42);
    EXPECT_EQ(ref, 42);
    EXPECT_TRUE(opt.HasValue());
}

TEST(Optional, Reset)
{
    Optional<int> opt = 42;
    opt.Reset();
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, ResetAlreadyDisengaged)
{
    Optional<int> opt;
    opt.Reset();
    EXPECT_FALSE(opt.HasValue());
}

TEST(Optional, EqualityBothEngaged)
{
    Optional<int> a = 42;
    Optional<int> b = 42;
    EXPECT_EQ(a, b);
}

TEST(Optional, InequalityDifferentValues)
{
    Optional<int> a = 1;
    Optional<int> b = 2;
    EXPECT_NE(a, b);
}

TEST(Optional, EqualityBothDisengaged)
{
    Optional<int> a;
    Optional<int> b;
    // Both are Nothing — compare via nullopt
    EXPECT_EQ(a, Nothing);
    EXPECT_EQ(b, Nothing);
}

TEST(Optional, EqualityWithNothing)
{
    Optional<int> engaged = 42;
    Optional<int> empty;

    EXPECT_NE(engaged, Nothing);
    EXPECT_EQ(empty, Nothing);
}

TEST(Optional, EqualityWithValue)
{
    Optional<int> opt = 42;
    EXPECT_EQ(opt, 42);
    EXPECT_NE(opt, 0);
}

TEST(Optional, EqualityDisengagedWithValue)
{
    Optional<int> opt;
    EXPECT_NE(opt, 42);
}

TEST(Optional, EqualityWithStdOptional)
{
    Optional<int> opt = 42;
    std::optional<int> std_opt = 42;
    EXPECT_EQ(opt, std_opt);
}

TEST(Optional, InequalityWithStdOptional)
{
    Optional<int> opt = 42;
    std::optional<int> std_opt = 99;
    EXPECT_NE(opt, std_opt);
}

TEST(Optional, ConvertToStdOptionalEngaged)
{
    Optional<int> opt = 42;
    auto std_opt = static_cast<std::optional<int>>(opt);
    EXPECT_TRUE(std_opt.has_value());
    EXPECT_EQ(*std_opt, 42);
}

TEST(Optional, ConvertToStdOptionalDisengaged)
{
    Optional<int> opt;
    auto std_opt = static_cast<std::optional<int>>(opt);
    EXPECT_FALSE(std_opt.has_value());
}

TEST(Optional, MoveOnlyType)
{
    Optional<std::unique_ptr<int>> opt = std::make_unique<int>(42);
    EXPECT_TRUE(opt.HasValue());
    EXPECT_EQ(**opt, 42);

    auto moved = VIOLET_MOVE(opt);
    EXPECT_TRUE(moved.HasValue());
    EXPECT_EQ(*moved.Value(), 42);
}

TEST(Optional, VectorValue)
{
    std::vector<int> v = { 1, 2, 3 };
    Optional<std::vector<int>> opt = VIOLET_MOVE(v);
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
    Optional<int> opt = 42;
    EXPECT_FALSE(opt.ToString().empty());
}

TEST(Optional, ToStringDisengaged)
{
    Optional<int> opt;
    auto s = opt.ToString();
    EXPECT_FALSE(s.empty());
}

TEST(Optional, StreamOperator)
{
    Optional<int> opt = 42;
    std::ostringstream os;
    os << opt;
    EXPECT_FALSE(os.str().empty());
}

TEST(OptionalTraits, IsOptional)
{
    static_assert(is_optional_v<Optional<int>>);
    static_assert(is_optional_v<std::optional<int>>);
    static_assert(!is_optional_v<int>);
    static_assert(!is_optional_v<Some<int>>);
}

TEST(OptionalTraits, OptionalType)
{
    static_assert(std::same_as<optional_type_t<Optional<int>>, int>);
    static_assert(std::same_as<optional_type_t<std::optional<std::string>>, std::string>);
}

TEST(OptionalConstexpr, DefaultIsDisengaged)
{
    constexpr Optional<int> opt;
    static_assert(!opt.HasValue());
    static_assert(!static_cast<bool>(opt));
}

TEST(OptionalConstexpr, NothingIsDisengaged)
{
    constexpr Optional<int> opt = Nothing;
    static_assert(!opt.HasValue());
}

TEST(OptionalConstexpr, ConstructFromValue)
{
    constexpr Optional<int> opt = 42;
    static_assert(opt.HasValue());
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, ConstructInPlace)
{
    constexpr Optional<int> opt(std::in_place, 42);
    static_assert(opt.HasValue());
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, ConstructFromSome)
{
    constexpr Optional<int> opt(Some<int>(42));
    static_assert(opt.HasValue());
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, BoolConversion)
{
    constexpr Optional<int> engaged = 42;
    constexpr Optional<int> empty;

    static_assert(static_cast<bool>(engaged));
    static_assert(!static_cast<bool>(empty));
}

TEST(OptionalConstexpr, DerefOperator)
{
    constexpr Optional<int> opt = 42;
    static_assert(*opt == 42);
}

TEST(OptionalConstexpr, ValueAccess)
{
    constexpr Optional<int> opt = 42;
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, UnwrapOrEngaged)
{
    constexpr Optional<int> opt = 42;
    static_assert(opt.UnwrapOr(0) == 42);
}

TEST(OptionalConstexpr, UnwrapOrDisengaged)
{
    constexpr Optional<int> opt;
    static_assert(opt.UnwrapOr(99) == 99);
}

TEST(OptionalConstexpr, UnwrapOrDefaultEngaged)
{
    constexpr Optional<int> opt = 42;
    static_assert(opt.UnwrapOrDefault() == 42);
}

TEST(OptionalConstexpr, UnwrapOrDefaultDisengaged)
{
    constexpr Optional<int> opt;
    static_assert(opt.UnwrapOrDefault() == 0);
}

TEST(OptionalConstexpr, UnwrapUnchecked)
{
    constexpr Optional<int> opt = 42;
    static_assert(opt.UnwrapUnchecked(Unsafe("in test code; don't care about safety")) == 42);
}

TEST(OptionalConstexpr, MapEngaged)
{
    constexpr auto mapped = Optional<int>(21).Map([](int v) { return v * 2; });
    static_assert(mapped.HasValue());
    static_assert(mapped.Value() == 42);
}

TEST(OptionalConstexpr, MapDisengaged)
{
    constexpr auto mapped = Optional<int>().Map([](int v) { return v * 2; });
    static_assert(!mapped.HasValue());
}

TEST(OptionalConstexpr, MapOrEngaged)
{
    constexpr Optional<int> opt = 21;
    constexpr auto val = opt.MapOr(0, [](int v) { return v * 2; });
    static_assert(val == 42);
}

TEST(OptionalConstexpr, MapOrDisengaged)
{
    constexpr Optional<int> opt;
    constexpr auto val = opt.MapOr(99, [](int v) { return v * 2; });
    static_assert(val == 99);
}

TEST(OptionalConstexpr, HasValueAndTrue)
{
    constexpr Optional<int> opt = 42;
    static_assert(opt.HasValueAnd([](int v) { return v > 0; }));
}

TEST(OptionalConstexpr, HasValueAndFalse)
{
    constexpr Optional<int> opt = 42;
    static_assert(!opt.HasValueAnd([](int v) { return v < 0; }));
}

TEST(OptionalConstexpr, HasValueAndDisengaged)
{
    constexpr Optional<int> opt;
    static_assert(!opt.HasValueAnd([](int) { return true; }));
}

TEST(OptionalConstexpr, EqualityWithNothing)
{
    constexpr Optional<int> engaged = 42;
    constexpr Optional<int> empty;

    static_assert(engaged != Nothing);
    static_assert(empty == Nothing);
}

TEST(OptionalConstexpr, EqualityWithValue)
{
    constexpr Optional<int> opt = 42;
    static_assert(opt == 42);
    static_assert(opt != 0);
}

TEST(OptionalConstexpr, EqualityBothEngaged)
{
    constexpr Optional<int> a = 42;
    constexpr Optional<int> b = 42;
    constexpr Optional<int> c = 99;

    static_assert(a == b);
    static_assert(a != c);
}

TEST(OptionalConstexpr, CopyConstruct)
{
    constexpr Optional<int> a = 42;
    constexpr Optional<int> b(a);
    static_assert(b.HasValue());
    static_assert(b.Value() == 42);
}

TEST(OptionalConstexpr, CopyConstructDisengaged)
{
    constexpr Optional<int> a;
    constexpr Optional<int> b(a);
    static_assert(!b.HasValue());
}

namespace {

consteval auto ConstexprMapChain() -> int
{
    return Optional<int>(10).Map([](int v) { return v + 5; }).Map([](int v) { return v * 2; }).UnwrapOr(0);
}

consteval auto ConstexprTakeAndReplace() -> int
{
    Optional<int> opt = 42;
    auto taken = opt.Take();
    if (opt.HasValue()) {
        return -1; // should be disengaged
    }

    (void)opt.Replace(99);
    return opt.Value() + taken.Value(); // 99 + 42
}

consteval auto ConstexprMoveConstruct() -> int
{
    Optional<int> a = 42;
    Optional<int> b(static_cast<Optional<int>&&>(a));
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
    static_assert(is_optional_v<Optional<int>>);
    static_assert(is_optional_v<std::optional<int>>);
    static_assert(!is_optional_v<int>);
    static_assert(!is_optional_v<Some<int>>);
}

TEST(OptionalTraitsConstexpr, OptionalType)
{
    static_assert(std::same_as<optional_type_t<Optional<int>>, int>);
    static_assert(std::same_as<optional_type_t<Optional<double>>, double>);
    static_assert(std::same_as<optional_type_t<std::optional<int>>, int>);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length,performance-unnecessary-copy-initialization,cppcoreguidelines-special-member-functions)

VIOLET_DIAGNOSTIC_POP
