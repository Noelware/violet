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

namespace violet {

VIOLET_DIAGNOSTIC_PUSH

#if VIOLET_COMPILER(CLANG) || VIOLET_COMPILER(GCC)
VIOLET_DIAGNOSTIC_IGNORE("-Wself-move")
VIOLET_DIAGNOSTIC_IGNORE("-Wself-assign-overloaded")
#endif

TEST(Optional, DefaultIsDisengaged)
{
    Optional<Int32> opt;
    EXPECT_FALSE(opt);
    EXPECT_FALSE(static_cast<bool>(opt));
}

TEST(Optional, NothingIsDisengaged)
{
    Optional<Int32> opt = Nothing;
    EXPECT_FALSE(opt);
}

TEST(Optional, ConstructFromValue)
{
    Optional<Int32> opt = 42;
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromRvalue)
{
    Optional<String> opt = String("hello");
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.Value(), "hello");
}

TEST(Optional, ConstructInPlace)
{
    Optional<String> opt(std::in_place, 5, 'x');
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.Value(), "xxxxx");
}

TEST(Optional, ConstructFromSomeLvalue)
{
    Some<Int32> s(42);
    Optional<Int32> opt(s);
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromSomeRvalue)
{
    Optional<Int32> opt(Some<Int32>(42));
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromStdOptionalEngaged)
{
    std::optional<Int32> std_opt = 42;
    Optional<Int32> opt(std_opt);
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, ConstructFromStdOptionalEmpty)
{
    std::optional<Int32> std_opt;
    Optional<Int32> opt(std_opt);
    EXPECT_FALSE(opt);
}

TEST(Optional, ConstructFromStdOptionalRvalue)
{
    Optional<String> opt(std::optional<String>("hello"));
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.Value(), "hello");
}

TEST(Optional, CopyConstructEngaged)
{
    Optional<Int32> a = 42;
    Optional<Int32> b(a);
    EXPECT_TRUE(b);
    EXPECT_EQ(b.Value(), 42);
}

TEST(Optional, CopyConstructDisengaged)
{
    Optional<Int32> a;
    Optional<Int32> b(a);
    EXPECT_FALSE(b);
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
    EXPECT_TRUE(b);
    EXPECT_EQ(b.Value(), 42);
}

TEST(Optional, CopyAssignDisengagedToEngaged)
{
    Optional<Int32> a;
    Optional<Int32> b = 42;
    b = a;
    EXPECT_FALSE(b);
}

TEST(Optional, CopyAssignSelf)
{
    Optional<Int32> a = 42;
    a = a;
    EXPECT_TRUE(a);
    EXPECT_EQ(a.Value(), 42);
}

TEST(Optional, MoveConstructEngaged)
{
    Optional<String> a = String("hello");
    Optional<String> b(VIOLET_MOVE(a));
    EXPECT_TRUE(b);
    EXPECT_EQ(b.Value(), "hello");
    EXPECT_TRUE(a); // NOLINT(bugprone-use-after-move): moved-from but still engaged, like std::optional
}

TEST(Optional, MoveConstructDisengaged)
{
    Optional<Int32> a;
    Optional<Int32> b(VIOLET_MOVE(a));
    EXPECT_FALSE(b);
}

TEST(Optional, MoveAssignEngagedToEngaged)
{
    Optional<String> a = String("hello");
    Optional<String> b = String("world");
    b = VIOLET_MOVE(a);
    EXPECT_TRUE(b);
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Optional, MoveAssignEngagedToDisengaged)
{
    Optional<String> a = String("hello");
    Optional<String> b;
    b = VIOLET_MOVE(a);
    EXPECT_TRUE(b);
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Optional, MoveAssignDisengagedToEngaged)
{
    Optional<String> a;
    Optional<String> b = String("hello");
    b = VIOLET_MOVE(a);
    EXPECT_FALSE(b);
}

TEST(Optional, MoveAssignSelf)
{
    Optional<Int32> a = 42;
    a = VIOLET_MOVE(a);
    // Should not crash; value may be in moved-from state
    EXPECT_TRUE(a);
}

TEST(Optional, AssignNothingToEngaged)
{
    Optional<Int32> opt = 42;
    opt = Nothing;
    EXPECT_FALSE(opt);
}

TEST(Optional, AssignNothingToDisengaged)
{
    Optional<Int32> opt;
    opt = Nothing;
    EXPECT_FALSE(opt);
}

TEST(Optional, AssignValueLvalue)
{
    Optional<Int32> opt;
    int val = 42;
    opt = val;
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt.Value(), 42);
}

TEST(Optional, AssignValueRvalue)
{
    Optional<String> opt;
    opt = String("hello");
    EXPECT_TRUE(opt);
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
    Optional<String> opt = String("hello");

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
    Optional<String> opt = String("hello");
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

TEST(Optional, ExpectEngaged)
{
    Optional<Int32> opt = 42;
    EXPECT_EQ(opt.Expect("should not panic"), 42);
}

TEST(Optional, MapEngaged)
{
    Optional<Int32> opt = 21;
    auto mapped = opt.Map([](int v) { return v * 2; });
    EXPECT_TRUE(mapped);
    EXPECT_EQ(mapped.Value(), 42);
}

TEST(Optional, MapDisengaged)
{
    Optional<Int32> opt;
    auto mapped = opt.Map([](int v) { return v * 2; });
    EXPECT_FALSE(mapped);
}

TEST(Optional, MapChangesType)
{
    Optional<Int32> opt = 42;
    auto mapped = opt.Map([](int v) { return std::to_string(v); });
    EXPECT_TRUE(mapped);
    EXPECT_EQ(mapped.Value(), "42");
}

TEST(Optional, MapRvalue)
{
    auto mapped = Optional<Int32>(21).Map([](int v) { return v * 2; });
    EXPECT_TRUE(mapped);
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
    EXPECT_TRUE(taken);
    EXPECT_EQ(taken.Value(), 42);
    EXPECT_FALSE(opt);
}

TEST(Optional, TakeDisengaged)
{
    Optional<Int32> opt;
    auto taken = opt.Take();
    EXPECT_FALSE(taken);
    EXPECT_FALSE(opt);
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
    EXPECT_TRUE(opt);
}

TEST(Optional, Reset)
{
    Optional<Int32> opt = 42;
    opt.Reset();
    EXPECT_FALSE(opt);
}

TEST(Optional, ResetAlreadyDisengaged)
{
    Optional<Int32> opt;
    opt.Reset();
    EXPECT_FALSE(opt);
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
    EXPECT_TRUE(opt);
    EXPECT_EQ(**opt, 42);

    auto moved = VIOLET_MOVE(opt);
    EXPECT_TRUE(moved);
    EXPECT_EQ(*moved.Value(), 42);
}

TEST(Optional, VectorValue)
{
    std::vector<Int32> v = {1, 2, 3};
    Optional<std::vector<Int32>> opt = VIOLET_MOVE(v);
    EXPECT_TRUE(opt);
    EXPECT_EQ(opt->size(), 3U);
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

namespace {

template<typename Opt, typename Arg>
concept CanReplace = requires(Opt opt, Arg&& arg) { opt.Replace(VIOLET_FWD(Arg, arg)); };

template<typename Opt, typename Arg>
concept CanUnwrapOr = requires(const Opt& opt, Arg&& arg) { opt.UnwrapOr(VIOLET_FWD(Arg, arg)); };

} // namespace

TEST(OptionalRef, Layout)
{
    static_assert(sizeof(Optional<Int32&>) == sizeof(Int32*));
    static_assert(std::is_trivially_copyable_v<Optional<Int32&>>);
    static_assert(std::is_trivially_copyable_v<Optional<const String&>>);
}

TEST(OptionalRef, RejectsTemporaries)
{
    static_assert(!std::is_constructible_v<Optional<const Int32&>, Int32>);
    static_assert(!std::is_constructible_v<Optional<const Int32&>, Int32&&>);
    static_assert(!std::is_constructible_v<Optional<const Int32&>, std::in_place_t, Int32>);
    static_assert(!std::is_assignable_v<Optional<const Int32&>&, Int32>);
    static_assert(!CanReplace<Optional<const Int32&>, Int32>);

    static_assert(std::is_constructible_v<Optional<const Int32&>, const Int32&>);
    static_assert(std::is_assignable_v<Optional<const Int32&>&, const Int32&>);
    static_assert(CanReplace<Optional<const Int32&>, const Int32&>);
}

TEST(OptionalRef, RespectsConstness)
{
    static_assert(!std::is_constructible_v<Optional<Int32&>, const Int32&>);
    static_assert(std::is_constructible_v<Optional<const Int32&>, Int32&>);
}

TEST(OptionalRef, DefaultIsEmpty)
{
    Optional<Int32&> opt;
    EXPECT_FALSE(opt);
}

TEST(OptionalRef, NothingIsEmpty)
{
    Optional<Int32&> opt = Nothing;
    EXPECT_FALSE(opt);
    EXPECT_EQ(opt, Nothing);
}

TEST(OptionalRef, ConstructFromLvalue)
{
    Int32 value = 42;
    Optional<Int32&> opt = value;

    ASSERT_TRUE(opt);
    EXPECT_EQ(&opt.Value(), &value);
}

TEST(OptionalRef, ConstructInPlace)
{
    Int32 value = 42;
    Optional<Int32&> opt(std::in_place, value);

    ASSERT_TRUE(opt);
    EXPECT_EQ(&opt.Value(), &value);
}

TEST(OptionalRef, MutatesThroughValue)
{
    Int32 value = 10;
    Optional<Int32&> opt = value;

    opt.Value() = 99;
    EXPECT_EQ(value, 99);

    *opt = 100;
    EXPECT_EQ(value, 100);
}

TEST(OptionalRef, ConstOptionalGivesMutableReference)
{
    Int32 value = 1;
    const Optional<Int32&> opt = value;

    static_assert(std::same_as<decltype(opt.Value()), Int32&>);
    static_assert(std::same_as<decltype(*opt), Int32&>);

    opt.Value() = 2;
    EXPECT_EQ(value, 2);
}

TEST(OptionalRef, ConstReference)
{
    const String s = "hello";
    Optional<const String&> opt = s;

    static_assert(std::same_as<decltype(opt.Value()), const String&>);

    ASSERT_TRUE(opt);
    EXPECT_EQ(opt.Value(), "hello");
    EXPECT_EQ(&opt.Value(), &s);
}

TEST(OptionalRef, ValueOnTemporaryOptional)
{
    Int32 value = 5;
    Int32& ref = Optional<Int32&>(value).Value();

    EXPECT_EQ(&ref, &value);
}

TEST(OptionalRef, ArrowOperator)
{
    String s = "hello";
    Optional<String&> opt = s;

    EXPECT_EQ(opt->size(), 5U);

    opt->append(" world");
    EXPECT_EQ(s, "hello world");
}

TEST(OptionalRef, AssignmentRebinds)
{
    Int32 a = 1;
    Int32 b = 2;

    Optional<Int32&> opt = a;
    opt = b;

    EXPECT_EQ(a, 1);
    EXPECT_EQ(&opt.Value(), &b);
}

TEST(OptionalRef, CopyAssignmentRebinds)
{
    Int32 a = 1;
    Int32 b = 2;

    Optional<Int32&> opt_a = a;
    Optional<Int32&> opt_b = b;

    opt_a = opt_b;

    EXPECT_EQ(a, 1);
    EXPECT_EQ(&opt_a.Value(), &b);
}

TEST(OptionalRef, CopySharesReferent)
{
    Int32 value = 7;
    Optional<Int32&> a = value;
    auto b = a;

    ASSERT_TRUE(b);
    *b = 77;

    EXPECT_EQ(*a, 77);
    EXPECT_EQ(value, 77);
}

TEST(OptionalRef, MoveLeavesReferentUntouched)
{
    String value = "hello";
    Optional<String&> a = value;
    auto b = VIOLET_MOVE(a);

    ASSERT_TRUE(b);
    EXPECT_EQ(&b.Value(), &value);
    EXPECT_EQ(value, "hello");
}

TEST(OptionalRef, AssignNothing)
{
    Int32 value = 42;
    Optional<Int32&> opt = value;

    opt = Nothing;

    EXPECT_FALSE(opt);
    EXPECT_EQ(value, 42);
}

TEST(OptionalRef, ResetLeavesReferentUntouched)
{
    String value = "hello";
    Optional<String&> opt = value;

    opt.Reset();

    EXPECT_FALSE(opt);
    EXPECT_EQ(value, "hello");
}

TEST(OptionalRef, UnwrapReturnsReferent)
{
    Int32 value = 42;
    Optional<Int32&> opt = value;

    static_assert(std::same_as<decltype(opt.Unwrap()), Int32&>);
    EXPECT_EQ(&opt.Unwrap(), &value);
}

TEST(OptionalRef, ExpectReturnsReferent)
{
    Int32 value = 42;
    Optional<Int32&> opt = value;

    static_assert(std::same_as<decltype(opt.Expect("should not panic")), Int32&>);
    EXPECT_EQ(&opt.Expect("should not panic"), &value);
}

TEST(OptionalRef, UnwrapOrCopyableReturnsCopy)
{
    Int32 value = 42;
    Optional<const Int32&> engaged = value;
    Optional<const Int32&> empty;

    static_assert(std::same_as<decltype(engaged.UnwrapOr(0)), Int32>);

    EXPECT_EQ(engaged.UnwrapOr(0), 42);
    EXPECT_EQ(empty.UnwrapOr(5), 5);
}

TEST(OptionalRef, UnwrapOrNonCopyableReturnsReference)
{
    auto value = std::make_unique<Int32>(1);
    auto fallback = std::make_unique<Int32>(2);

    Optional<std::unique_ptr<Int32>&> engaged = value;
    Optional<std::unique_ptr<Int32>&> empty;

    static_assert(std::same_as<decltype(engaged.UnwrapOr(fallback)), std::unique_ptr<Int32>&>);
    static_assert(!CanUnwrapOr<Optional<std::unique_ptr<Int32>&>, std::unique_ptr<Int32>>);

    EXPECT_EQ(&engaged.UnwrapOr(fallback), &value);
    EXPECT_EQ(&empty.UnwrapOr(fallback), &fallback);
}

TEST(OptionalRef, UnwrapOrDefault)
{
    Int32 value = 42;
    Optional<Int32&> engaged = value;
    Optional<Int32&> empty;

    static_assert(std::same_as<decltype(engaged.UnwrapOrDefault()), Int32>);

    EXPECT_EQ(engaged.UnwrapOrDefault(), 42);
    EXPECT_EQ(empty.UnwrapOrDefault(), 0);
}

TEST(OptionalRef, UnwrapUnchecked)
{
    Int32 value = 42;
    Optional<Int32&> opt = value;

    EXPECT_EQ(&opt.UnwrapUnchecked(Unsafe("in test code; this will not fail")), &value);
}

TEST(OptionalRef, MapByValue)
{
    Int32 value = 5;
    Optional<Int32&> opt = value;

    auto mapped = opt.Map([](Int32& v) -> Int32 { return v * 2; });

    static_assert(std::same_as<decltype(mapped), Optional<Int32>>);
    ASSERT_TRUE(mapped);
    EXPECT_EQ(*mapped, 10);
    EXPECT_EQ(value, 5);
}

TEST(OptionalRef, MapToMemberReference)
{
    struct User {
        String Name;
    };

    User user{.Name = "Noel"};
    Optional<User&> opt = user;

    auto name = opt.Map([](User& u) -> String& { return u.Name; });

    static_assert(std::same_as<decltype(name), Optional<String&>>);
    ASSERT_TRUE(name);
    EXPECT_EQ(&name.Value(), &user.Name);
}

TEST(OptionalRef, MapOnEmpty)
{
    Optional<Int32&> opt;

    bool called = false;
    auto mapped = opt.Map([&](Int32& v) -> Int32 {
        called = true;
        return v;
    });

    EXPECT_FALSE(mapped);
    EXPECT_FALSE(called);
}

TEST(OptionalRef, MapOr)
{
    Int32 value = 21;
    Optional<Int32&> engaged = value;
    Optional<Int32&> empty;

    EXPECT_EQ(engaged.MapOr(0, [](Int32& v) { return v * 2; }), 42);
    EXPECT_EQ(empty.MapOr(99, [](Int32& v) { return v * 2; }), 99);
}

TEST(OptionalRef, HasValueAnd)
{
    Int32 value = 42;
    Optional<Int32&> engaged = value;
    Optional<Int32&> empty;

    EXPECT_TRUE(engaged.HasValueAnd([](Int32& v) { return v > 0; }));
    EXPECT_FALSE(engaged.HasValueAnd([](Int32& v) { return v < 0; }));
    EXPECT_FALSE(empty.HasValueAnd([](Int32&) { return true; }));
}

TEST(OptionalRef, TakeMovesReferenceNotReferent)
{
    String value = "hello";
    Optional<String&> opt = value;

    auto taken = opt.Take();

    static_assert(std::same_as<decltype(taken), Optional<String&>>);
    EXPECT_FALSE(opt);
    ASSERT_TRUE(taken);
    EXPECT_EQ(&taken.Value(), &value);
    EXPECT_EQ(value, "hello");
}

TEST(OptionalRef, ReplaceRebinds)
{
    Int32 a = 1;
    Int32 b = 2;
    Optional<Int32&> opt = a;

    Int32& ref = opt.Replace(b);

    EXPECT_EQ(&ref, &b);
    EXPECT_EQ(&opt.Value(), &b);
    EXPECT_EQ(a, 1);
}

TEST(OptionalRef, EqualityComparesValuesNotAddresses)
{
    Int32 a = 42;
    Int32 b = 42;
    Int32 c = 7;

    Optional<Int32&> opt_a = a;
    Optional<Int32&> opt_b = b;
    Optional<Int32&> opt_c = c;
    Optional<Int32&> none;

    EXPECT_EQ(opt_a, opt_b);
    EXPECT_NE(opt_a, opt_c);
    EXPECT_NE(opt_a, none);
    EXPECT_EQ(none, Nothing);
}

TEST(OptionalRef, EqualityWithValueAndSome)
{
    Int32 value = 42;
    Optional<Int32&> opt = value;

    EXPECT_EQ(opt, 42);
    EXPECT_NE(opt, 0);
    EXPECT_TRUE(opt == Some<Int32>(42));
}

TEST(OptionalRef, Ordering)
{
    Int32 one = 1;
    Int32 two = 2;

    Optional<Int32&> opt_one = one;
    Optional<Int32&> opt_two = two;
    Optional<Int32&> none;

    EXPECT_LT(opt_one, opt_two);
    EXPECT_LT(none, opt_one);
    EXPECT_GT(opt_two, 1);
}

TEST(OptionalRef, ToString)
{
    Int32 value = 42;
    Optional<Int32&> engaged = value;
    Optional<Int32&> empty;

    EXPECT_FALSE(engaged.ToString().empty());
    EXPECT_FALSE(empty.ToString().empty());
}

namespace {

constexpr Int32 kStaticValue = 42;

consteval auto ConstexprRefRebind() -> Int32
{
    Int32 a = 1;
    Int32 b = 2;

    Optional<Int32&> opt = a;
    opt.Value() = 10;

    opt = b;
    opt.Value() += 5;

    return a + b; // 10 + 7
}

consteval auto ConstexprRefTake() -> bool
{
    Int32 value = 3;
    Optional<Int32&> opt = value;
    auto taken = opt.Take();

    return !opt && taken && &taken.Value() == &value;
}

} // namespace

TEST(OptionalRefConstexpr, StaticReferent)
{
    constexpr Optional<const Int32&> opt = kStaticValue;
    static_assert(opt);
    static_assert(opt.Value() == 42);
    static_assert(opt == 42);
}

TEST(OptionalRefConstexpr, EmptyIsDisengaged)
{
    constexpr Optional<const Int32&> opt;
    static_assert(!opt);
    static_assert(opt.UnwrapOr(5) == 5);
}

TEST(OptionalRefConsteval, Rebind)
{
    static_assert(ConstexprRefRebind() == 17);
}

TEST(OptionalRefConsteval, Take)
{
    static_assert(ConstexprRefTake());
}

TEST(OptionalTraits, OptionalTypeOfReference)
{
    static_assert(std::same_as<optional_type_t<Optional<Int32&>>, Int32&>);
    static_assert(std::same_as<Optional<Int32&>::value_type, Int32>);
    static_assert(std::same_as<Optional<const Int32&>::value_type, const Int32>);
}

TEST(Optional, MovedFromStaysEngaged)
{
    Optional<String> a = String("hello");
    Optional<String> b = VIOLET_MOVE(a);

    EXPECT_TRUE(a); // NOLINT(bugprone-use-after-move): matches std::optional; use Take() to empty it
    ASSERT_TRUE(b);
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Optional, TriviallyCopyable)
{
    static_assert(std::is_trivially_copyable_v<Optional<Int32>>);
    static_assert(!std::is_copy_constructible_v<Optional<std::unique_ptr<Int32>>>);
    static_assert(std::is_move_constructible_v<Optional<std::unique_ptr<Int32>>>);
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

    std::optional<Point> std_opt = Point{.x = 3, .y = 4};
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
    static_assert(std::same_as<optional_type_t<std::optional<String>>, String>);
}

TEST(OptionalConstexpr, DefaultIsDisengaged)
{
    constexpr Optional<Int32> opt;
    static_assert(!opt);
    static_assert(!static_cast<bool>(opt));
}

TEST(OptionalConstexpr, NothingIsDisengaged)
{
    constexpr Optional<Int32> opt = Nothing;
    static_assert(!opt);
}

TEST(OptionalConstexpr, ConstructFromValue)
{
    constexpr Optional<Int32> opt = 42;
    static_assert(opt);
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, ConstructInPlace)
{
    constexpr Optional<Int32> opt(std::in_place, 42);
    static_assert(opt);
    static_assert(opt.Value() == 42);
}

TEST(OptionalConstexpr, ConstructFromSome)
{
    constexpr Optional<Int32> opt(Some<Int32>(42));
    static_assert(opt);
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
    static_assert(mapped);
    static_assert(mapped.Value() == 42);
}

TEST(OptionalConstexpr, MapDisengaged)
{
    constexpr auto mapped = Optional<Int32>().Map([](int v) { return v * 2; });
    static_assert(!mapped);
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
    static_assert(b);
    static_assert(b.Value() == 42);
}

TEST(OptionalConstexpr, CopyConstructDisengaged)
{
    constexpr Optional<Int32> a;
    constexpr Optional<Int32> b(a);
    static_assert(!b);
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
    if (opt) {
        return -1; // should be disengaged
    }

    (void)opt.Replace(99);
    return opt.Value() + taken.Value(); // 99 + 42
}

consteval auto ConstexprMoveConstruct() -> int
{
    Optional<Int32> a = 42;
    Optional<Int32> b(static_cast<Optional<Int32>&&>(a));
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

namespace {

struct NonAssignable final {
    ~NonAssignable() = default;

    const String Key; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    String Value;

    VIOLET_IMPLICIT NonAssignable(String key, String value)
        : Key(VIOLET_MOVE(key))
        , Value(VIOLET_MOVE(value))
    {
    }

    VIOLET_IMPLICIT NonAssignable(const NonAssignable&) = default;
    VIOLET_IMPLICIT NonAssignable(NonAssignable&&) = default;

    auto operator=(const NonAssignable&) -> NonAssignable& = delete;
    auto operator=(NonAssignable&&) -> NonAssignable& = delete;

    auto operator==(const NonAssignable& other) const -> bool
    {
        return this->Key == other.Key && this->Value == other.Value;
    }
};

static_assert(!std::is_copy_assignable_v<NonAssignable>);
static_assert(!std::is_move_assignable_v<NonAssignable>);
static_assert(std::is_copy_constructible_v<NonAssignable>);
static_assert(std::is_move_constructible_v<NonAssignable>);

struct MoveOnlyNonAssignable final {
    const Int32 ID; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    UniquePtr<Int32> Data;

    VIOLET_IMPLICIT MoveOnlyNonAssignable(Int32 id, Int32 data)
        : ID(id)
        , Data(std::make_unique<Int32>(data))
    {
    }

    MoveOnlyNonAssignable(MoveOnlyNonAssignable&&) = default;
    MoveOnlyNonAssignable(const MoveOnlyNonAssignable&) = delete;

    auto operator=(MoveOnlyNonAssignable&&) -> MoveOnlyNonAssignable& = delete;
    auto operator=(const MoveOnlyNonAssignable&) -> MoveOnlyNonAssignable& = delete;
};

static_assert(!std::is_move_assignable_v<MoveOnlyNonAssignable>);
static_assert(std::is_move_constructible_v<MoveOnlyNonAssignable>);

} // namespace

TEST(OptionalNonAssignable, CopyAssignBothEngaged)
{
    Optional<NonAssignable> a(NonAssignable{"key1", "val1"});
    Optional<NonAssignable> b(NonAssignable{"key2", "val2"});

    a = b;

    ASSERT_TRUE(a);
    EXPECT_EQ(a->Key, "key2");
    EXPECT_EQ(a->Value, "val2");
}

TEST(OptionalNonAssignable, CopyAssignFromEngagedToEmpty)
{
    Optional<NonAssignable> a;
    Optional<NonAssignable> b{NonAssignable{"key", "val"}};

    a = b;

    ASSERT_TRUE(a);
    EXPECT_EQ(a->Key, "key");
    EXPECT_EQ(a->Value, "val");
}

TEST(OptionalNonAssignable, CopyAssignFromEmptyToEngaged)
{
    Optional<NonAssignable> a{NonAssignable{"key", "val"}};
    Optional<NonAssignable> b;

    a = b;
    EXPECT_FALSE(a);
}

TEST(OptionalNonAssignable, CopyAssignSelf)
{
    Optional<NonAssignable> a{NonAssignable{"key", "val"}};

    a = a;

    ASSERT_TRUE(a);
    EXPECT_EQ(a->Key, "key");
}

TEST(OptionalNonAssignable, CopyAssignBothEmpty)
{
    Optional<NonAssignable> a;
    Optional<NonAssignable> b;

    a = b;

    EXPECT_FALSE(a);
}

TEST(OptionalNonAssignable, MoveAssignBothEngaged)
{
    Optional<NonAssignable> a{NonAssignable{"key1", "val1"}};
    Optional<NonAssignable> b{NonAssignable{"key2", "val2"}};

    a = VIOLET_MOVE(b);

    ASSERT_TRUE(a);
    EXPECT_EQ(a->Key, "key2");
    EXPECT_EQ(a->Value, "val2");
}

TEST(OptionalNonAssignable, MoveAssignFromEngagedToEmpty)
{
    Optional<NonAssignable> a;
    Optional<NonAssignable> b{NonAssignable{"key", "val"}};

    a = VIOLET_MOVE(b);

    ASSERT_TRUE(a);
    EXPECT_EQ(a->Key, "key");
}

TEST(OptionalNonAssignable, MoveAssignFromEmptyToEngaged)
{
    Optional<NonAssignable> a{NonAssignable{"key", "val"}};
    Optional<NonAssignable> b;

    a = VIOLET_MOVE(b);

    EXPECT_FALSE(a);
}

TEST(OptionalMoveOnlyNonAssignable, MoveAssignBothEngaged)
{
    Optional<MoveOnlyNonAssignable> a{MoveOnlyNonAssignable{1, 100}};
    Optional<MoveOnlyNonAssignable> b{MoveOnlyNonAssignable{2, 200}};

    a = VIOLET_MOVE(b);

    ASSERT_TRUE(a);
    EXPECT_EQ(a->ID, 2);
    EXPECT_EQ(*a->Data, 200);
}

TEST(OptionalMoveOnlyNonAssignable, MoveAssignFromEngagedToEmpty)
{
    Optional<MoveOnlyNonAssignable> a;
    Optional<MoveOnlyNonAssignable> b{MoveOnlyNonAssignable{3, 300}};

    a = VIOLET_MOVE(b);

    ASSERT_TRUE(a);
    EXPECT_EQ(a->ID, 3);
    EXPECT_EQ(*a->Data, 300);
}

TEST(OptionalMoveOnlyNonAssignable, MoveAssignFromEmptyToEngaged)
{
    Optional<MoveOnlyNonAssignable> a{MoveOnlyNonAssignable{4, 400}};
    Optional<MoveOnlyNonAssignable> b;

    a = VIOLET_MOVE(b);
    EXPECT_FALSE(a);
}

TEST(OptionalSTLConstTemplate, CopyAssignBothEngaged)
{
    using P = std::pair<const String, String>;

    Optional<P> a{P{"host", "localhost"}};
    Optional<P> b{P{"host", "0.0.0.0"}};

    a = b;

    ASSERT_TRUE(a);
    EXPECT_EQ(a->first, "host");
    EXPECT_EQ(a->second, "0.0.0.0");
}

TEST(OptionalSTLConstTemplate, MoveAssignBothEngaged)
{
    using P = std::pair<const String, String>;

    Optional<P> a{P{"key1", "val1"}};
    Optional<P> b{P{"key2", "val2"}};

    a = VIOLET_MOVE(b);

    ASSERT_TRUE(a);
    EXPECT_EQ(a->first, "key2");
    EXPECT_EQ(a->second, "val2");
}

TEST(OptionalSTLConstTemplate, ChainedAssignment)
{
    using P = std::pair<const String, String>;

    Optional<P> a{P{"a", "1"}};
    Optional<P> b{P{"b", "2"}};
    Optional<P> c{P{"c", "3"}};

    a = b;
    EXPECT_EQ(a->first, "b");

    a = VIOLET_MOVE(c);
    EXPECT_EQ(a->first, "c");

    a = Optional<P>{};
    EXPECT_FALSE(a);
}

TEST(OptionalAssignable, CopyAssignBothEngaged)
{
    Optional<String> a{"hello"};
    Optional<String> b{"world"};

    a = b;

    ASSERT_TRUE(a);
    EXPECT_EQ(a.Value(), "world");
    ASSERT_TRUE(b);
    EXPECT_EQ(b.Value(), "world");
}

TEST(OptionalAssignable, MoveAssignBothEngaged)
{
    Optional<String> a{"hello"};
    Optional<String> b{"world"};

    a = VIOLET_MOVE(b);

    ASSERT_TRUE(a);
    EXPECT_EQ(a.Value(), "world");
}

TEST(OptionalAssignable, IntCopyAssign)
{
    Optional<Int32> a{42};
    Optional<Int32> b{99};

    a = b;

    ASSERT_TRUE(a);
    EXPECT_EQ(a.Value(), 99);
}

VIOLET_DIAGNOSTIC_POP

} // namespace violet
