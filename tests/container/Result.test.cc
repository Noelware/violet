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
#include <violet/Container/Result.h>

VIOLET_DIAGNOSTIC_PUSH

#if VIOLET_COMPILER(CLANG) || VIOLET_COMPILER(GCC)
VIOLET_DIAGNOSTIC_IGNORE("-Wself-move")
VIOLET_DIAGNOSTIC_IGNORE("-Wself-assign-overloaded")
#endif

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length,performance-unnecessary-copy-initialization)
using namespace violet;

TEST(Result, ConstructOkImplicit)
{
    Result<Int32, String> r = 42;
    EXPECT_TRUE(r.Ok());
    EXPECT_EQ(r.Value(), 42);
}

TEST(Result, ConstructOkFromOkTag)
{
    Result<Int32, String> r = Ok<Int32>(42);
    EXPECT_TRUE(r.Ok());
    EXPECT_EQ(r.Value(), 42);
}

TEST(Result, ConstructErrImplicit)
{
    Result<Int32, String> r = Err<String>("bad");
    EXPECT_TRUE(r.Err());
    EXPECT_EQ(r.Error(), "bad");
}

TEST(Result, ConstructOkInPlace)
{
    Result<String, Int32> r(std::in_place_index<0>, 5, 'x');
    EXPECT_TRUE(r.Ok());
    EXPECT_EQ(r.Value(), "xxxxx");
}

TEST(Result, ConstructErrInPlace)
{
    Result<Int32, String> r(std::in_place_index<1>, 3, 'e');
    EXPECT_TRUE(r.Err());
    EXPECT_EQ(r.Error(), "eee");
}

// ── Converting Err construction ────────────────────────────

TEST(Result, ConstructFromConvertibleErrLvalue)
{
    Err<Int32> inner(42);
    Result<String, Int64> r(inner);
    EXPECT_TRUE(r.Err());
    EXPECT_EQ(r.Error(), 42L);
}

TEST(Result, ConstructFromConvertibleErrRvalue)
{
    Result<String, Int64> r(Err<Int32>(42));
    EXPECT_TRUE(r.Err());
    EXPECT_EQ(r.Error(), 42L);
}

TEST(Result, CopyConstructOk)
{
    Result<Int32, String> a = 42;
    Result<Int32, String> b(a);
    EXPECT_TRUE(b.Ok());
    EXPECT_EQ(b.Value(), 42);
}

TEST(Result, CopyConstructErr)
{
    Result<Int32, String> a = Err<String>("fail");
    Result<Int32, String> b(a);
    EXPECT_TRUE(b.Err());
    EXPECT_EQ(b.Error(), "fail");
}

TEST(Result, CopyAssignOkToOk)
{
    Result<Int32, String> a = 1;
    Result<Int32, String> b = 2;
    b = a;
    EXPECT_EQ(b.Value(), 1);
}

TEST(Result, CopyAssignErrToOk)
{
    Result<Int32, String> a = Err<String>("fail");
    Result<Int32, String> b = 42;
    b = a;
    EXPECT_TRUE(b.Err());
    EXPECT_EQ(b.Error(), "fail");
}

TEST(Result, CopyAssignOkToErr)
{
    Result<Int32, String> a = 42;
    Result<Int32, String> b = Err<String>("fail");
    b = a;
    EXPECT_TRUE(b.Ok());
    EXPECT_EQ(b.Value(), 42);
}

TEST(Result, MoveConstructOk)
{
    Result<String, Int32> a = String("hello");
    Result<String, Int32> b(VIOLET_MOVE(a));
    EXPECT_TRUE(b.Ok());
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Result, MoveConstructErr)
{
    Result<Int32, String> a = Err<String>("fail");
    Result<Int32, String> b(VIOLET_MOVE(a));
    EXPECT_TRUE(b.Err());
    EXPECT_EQ(b.Error(), "fail");
}

TEST(Result, MoveAssignSameVariant)
{
    Result<String, Int32> a = String("hello");
    Result<String, Int32> b = String("world");
    b = VIOLET_MOVE(a);
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Result, MoveAssignDifferentVariant)
{
    Result<String, Int32> a = String("hello");
    Result<String, Int32> b = Err<Int32>(1);
    b = VIOLET_MOVE(a);
    EXPECT_TRUE(b.Ok());
    EXPECT_EQ(b.Value(), "hello");
}

TEST(Result, SelfAssign)
{
    Result<Int32, String> r = 42;
    r = r;
    EXPECT_TRUE(r.Ok());
    EXPECT_EQ(r.Value(), 42);
}

TEST(Result, OkAndErrPredicates)
{
    Result<Int32, String> ok = 42;
    Result<Int32, String> err = Err<String>("bad");

    EXPECT_TRUE(ok.Ok());
    EXPECT_FALSE(ok.Err());
    EXPECT_FALSE(err.Ok());
    EXPECT_TRUE(err.Err());
}

TEST(Result, BoolConversion)
{
    Result<Int32, String> ok = 42;
    Result<Int32, String> err = Err<String>("bad");

    EXPECT_TRUE(ok);
    EXPECT_FALSE(err);
}

TEST(Result, ValueRefQualifiers)
{
    Result<String, Int32> r = String("hello");
    EXPECT_EQ(r.Value(), "hello");

    const auto& cref = r;
    EXPECT_EQ(cref.Value(), "hello");
}

TEST(Result, ErrorRefQualifiers)
{
    Result<Int32, String> r = Err<String>("fail");

    EXPECT_EQ(r.Error(), "fail");

    const auto& cref = r;
    EXPECT_EQ(cref.Error(), "fail");
}

TEST(Result, DerefOperator)
{
    Result<Int32, String> r = 42;
    EXPECT_EQ(*r, 42);
}

TEST(Result, ArrowOperator)
{
    Result<String, Int32> r = String("hello");
    EXPECT_EQ(r->size(), 5U);
}

TEST(Result, UnwrapOk)
{
    Result<Int32, String> r = 42;
    EXPECT_EQ(r.Unwrap(), 42);
}

TEST(Result, UnwrapOrOnOk)
{
    Result<Int32, String> r = 42;
    EXPECT_EQ(r.UnwrapOr(0), 42);
}

TEST(Result, UnwrapOrOnErr)
{
    Result<Int32, String> r = Err<String>("fail");
    EXPECT_EQ(r.UnwrapOr(99), 99);
}

TEST(Result, UnwrapOrDefaultOnOk)
{
    Result<Int32, String> r = 42;
    EXPECT_EQ(r.UnwrapOrDefault(), 42);
}

TEST(Result, UnwrapOrDefaultOnErr)
{
    Result<Int32, String> r = Err<String>("fail");
    EXPECT_EQ(r.UnwrapOrDefault(), 0);
}

TEST(Result, UnwrapErrOnErr)
{
    Result<Int32, String> r = Err<String>("fail");
    EXPECT_EQ(r.UnwrapErr(), "fail");
}

TEST(Result, UnwrapUnchecked)
{
    Result<Int32, String> r = 42;
    EXPECT_EQ(r.UnwrapUnchecked(Unsafe("for testing")), 42);
}

TEST(Result, UnwrapErrUnchecked)
{
    Result<Int32, String> r = Err<String>("fail");
    EXPECT_EQ(r.UnwrapErrUnchecked(Unsafe("for testing")), "fail");
}

TEST(Result, ExceptOnOk)
{
    Result<Int32, String> r = 42;
    EXPECT_EQ(r.Except("should not panic"), 42);
}

TEST(Result, MapOnOk)
{
    Result<Int32, String> r = 21;
    auto mapped = VIOLET_MOVE(r).Map([](Int32 v) -> Int32 { return v * 2; });
    EXPECT_TRUE(mapped.Ok());
    EXPECT_EQ(mapped.Value(), 42);
}

TEST(Result, MapOnErr)
{
    Result<Int32, String> r = Err<String>("fail");
    auto mapped = VIOLET_MOVE(r).Map([](Int32 v) -> Int32 { return v * 2; });
    EXPECT_TRUE(mapped.Err());
}

TEST(Result, MapErrOnErr)
{
    Result<Int32, String> r = Err<String>("fail");
    auto mapped = VIOLET_MOVE(r).MapErr([](String&& s) -> String { return VIOLET_MOVE(s) + "!"; });
    EXPECT_TRUE(mapped.Err());
}

TEST(Result, MapOrOnOk)
{
    Result<Int32, String> r = 21;
    auto val = r.MapOr(0, [](Int32 v) -> Int32 { return v * 2; });
    EXPECT_EQ(val, 42);
}

TEST(Result, MapOrOnErr)
{
    Result<Int32, String> r = Err<String>("fail");
    auto val = r.MapOr(99, [](Int32 v) -> Int32 { return v * 2; });
    EXPECT_EQ(val, 99);
}

TEST(Result, OkAndPredicate)
{
    Result<Int32, String> r = 42;
    EXPECT_TRUE(r.OkAnd([](Int32 v) -> bool { return v > 0; }));
    EXPECT_FALSE(r.OkAnd([](Int32 v) -> bool { return v < 0; }));
}

TEST(Result, OkAndOnErr)
{
    Result<Int32, String> r = Err<String>("fail");
    EXPECT_FALSE(r.OkAnd([](Int32) -> bool { return true; }));
}

TEST(Result, ErrAndPredicate)
{
    Result<Int32, String> r = Err<String>("fail");
    EXPECT_TRUE(r.ErrAnd([](const String& s) -> bool { return !s.empty(); }));
    EXPECT_FALSE(r.ErrAnd([](const String& s) -> bool { return s.empty(); }));
}

TEST(Result, ErrAndOnOk)
{
    Result<Int32, String> r = 42;
    EXPECT_FALSE(r.ErrAnd([](const String&) -> bool { return true; }));
}

TEST(Result, InspectOnOk)
{
    Result<Int32, String> r = 42;
    Int32 captured = 0;
    (void)r.Inspect([&captured](const Int32& v) -> void { captured = v; });
    EXPECT_EQ(captured, 42);
}

TEST(Result, InspectOnErr)
{
    Result<Int32, String> r = Err<String>("fail");
    bool called = false;
    (void)r.Inspect([&called](const Int32&) -> void { called = true; });
    EXPECT_FALSE(called);
}

TEST(Result, MoveOnlyType)
{
    Result<std::unique_ptr<Int32>, String> r = std::make_unique<Int32>(42);
    EXPECT_TRUE(r.Ok());
    EXPECT_EQ(**r, 42);

    auto moved = VIOLET_MOVE(r);
    EXPECT_TRUE(moved.Ok());
    EXPECT_EQ(*moved.Value(), 42);
}

TEST(Result, MoveOnlyUnwrap)
{
    Result<std::unique_ptr<Int32>, String> r = std::make_unique<Int32>(42);
    auto ptr = VIOLET_MOVE(r).Unwrap();
    EXPECT_EQ(*ptr, 42);
}

TEST(Result, VectorValue)
{
    std::vector<Int32> v = { 1, 2, 3 };
    Result<std::vector<Int32>, String> r = VIOLET_MOVE(v);
    EXPECT_TRUE(r.Ok());
    EXPECT_EQ(r.Value().size(), 3U);
    EXPECT_EQ(r.Value()[0], 1);
}

TEST(Result, ToStringOk)
{
    Result<Int32, String> r = 42;
    EXPECT_FALSE(r.ToString().empty());
}

TEST(Result, ToStringErr)
{
    Result<Int32, String> r = Err<String>("fail");
    EXPECT_FALSE(r.ToString().empty());
}

TEST(Result, StreamOperator)
{
    Result<Int32, String> r = 42;
    std::ostringstream os;
    os << r;
    EXPECT_FALSE(os.str().empty());
}

TEST(ResultRef, OkWithRef)
{
    int value = 42;
    Result<std::reference_wrapper<Int32>, String> res = Ok(std::ref(value));

    ASSERT_TRUE(res);
    EXPECT_EQ(res.Value(), 42);
}

TEST(ResultRef, ErrWithRefValue)
{
    Result<std::reference_wrapper<Int32>, String> res = Err(String("failed"));

    ASSERT_FALSE(res);
    EXPECT_EQ(res.Error(), "failed");
}

TEST(ResultRef, ConstRefInOk)
{
    const String s = "immutable";
    Result<std::reference_wrapper<const String>, Int32> res = Ok(std::cref(s));

    ASSERT_TRUE(res);
    EXPECT_EQ(res.Value(), "immutable");
}

TEST(ResultRef, MapPreservesRef)
{
    int value = 8;
    Result<std::reference_wrapper<Int32>, String> res = Ok(std::ref(value));

    auto mapped = res.Map([](int ref) -> int { return ref * 3; });

    ASSERT_TRUE(mapped);
    EXPECT_EQ(*mapped, 24);
    EXPECT_EQ(value, 8); // Original unchanged
}

TEST(ResultRef, MapOnErr)
{
    Result<std::reference_wrapper<Int32>, String> res = Err(String("nope"));

    bool called = false;
    auto mapped = res.Map([&](int ref) -> int {
        called = true;
        return ref;
    });

    EXPECT_FALSE(mapped);
    EXPECT_FALSE(called);
}

TEST(ResultRef, AndThenWithRef)
{
    int value = 4;
    Result<std::reference_wrapper<Int32>, String> res = Ok(std::ref(value));

    auto chained = VIOLET_MOVE(res).AndThen([](int ref) -> Result<Int32, String> {
        if (ref > 0) {
            return Ok(ref * 10);
        }

        return Err(String("non-positive"));
    });

    ASSERT_TRUE(chained);
    EXPECT_EQ(VIOLET_MOVE(chained).Transpose().Value(), 40);
}

TEST(ResultRef, MoveSemantics)
{
    int value = 33;
    Result<std::reference_wrapper<Int32>, String> a = Ok(std::ref(value));

    auto b = VIOLET_MOVE(a);
    ASSERT_TRUE(b);
    EXPECT_EQ(b.Value(), 33);
}

#if VIOLET_REQUIRE_STL(202302L)
TEST(ResultFromStd, ConstructFromExpectedWithValue)
{
    std::expected<Int32, String> exp = 42;
    Result<Int32, String> res(exp);

    ASSERT_TRUE(res);
    EXPECT_EQ(res.Value(), 42);
}

TEST(ResultFromStd, ConstructFromExpectedWithError)
{
    std::expected<Int32, String> exp = std::unexpected("fail");
    Result<Int32, String> res(exp);

    ASSERT_FALSE(res);
    EXPECT_EQ(res.Error(), "fail");
}

TEST(ResultFromStd, ConstructFromExpectedRvalueWithValue)
{
    std::expected<String, Int32> exp = "hello";
    Result<String, Int32> res(VIOLET_MOVE(exp));

    ASSERT_TRUE(res);
    EXPECT_EQ(res.Value(), "hello");
}

TEST(ResultFromStd, ConstructFromExpectedRvalueWithError)
{
    std::expected<String, Int32> exp = std::unexpected(404);
    Result<String, Int32> res(VIOLET_MOVE(exp));

    ASSERT_FALSE(res);
    EXPECT_EQ(res.Error(), 404);
}

TEST(ResultFromStd, AssignExpectedValueToOk)
{
    Result<Int32, String> res = Ok(1);
    std::expected<Int32, String> exp = 2;

    res = exp;

    ASSERT_TRUE(res);
    EXPECT_EQ(res.Value(), 2);
}

TEST(ResultFromStd, AssignExpectedErrorToOk)
{
    Result<Int32, String> res = Ok(1);
    std::expected<Int32, String> exp = std::unexpected(String("oops"));

    res = exp;

    ASSERT_FALSE(res);
    EXPECT_EQ(res.Error(), "oops");
}

TEST(ResultFromStd, AssignExpectedValueToErr)
{
    Result<Int32, String> res = Err(String("old"));
    std::expected<Int32, String> exp = 99;

    res = exp;

    ASSERT_TRUE(res);
    EXPECT_EQ(res.Value(), 99);
}

TEST(ResultFromStd, AssignExpectedErrorToErr)
{
    Result<Int32, String> res = Err(String("old"));
    std::expected<Int32, String> exp = std::unexpected(String("new"));

    res = exp;

    ASSERT_FALSE(res);
    EXPECT_EQ(res.Error(), "new");
}

TEST(ResultFromStd, MoveAssignExpectedValue)
{
    Result<String, Int32> res = Err(0);
    std::expected<String, Int32> exp = "moved";

    res = VIOLET_MOVE(exp);

    ASSERT_TRUE(res);
    EXPECT_EQ(res.Value(), "moved");
}

TEST(ResultFromStd, MoveAssignExpectedError)
{
    Result<Int32, String> res = Ok(0);
    std::expected<Int32, String> exp = std::unexpected(String("moved err"));

    res = VIOLET_MOVE(exp);

    ASSERT_FALSE(res);
    EXPECT_EQ(res.Error(), "moved err");
}

TEST(ResultFromStd, WithMoveOnlyValue)
{
    std::expected<std::unique_ptr<Int32>, Int32> exp = std::make_unique<Int32>(77);
    Result<std::unique_ptr<Int32>, Int32> res(VIOLET_MOVE(exp));

    ASSERT_TRUE(res);
    ASSERT_NE(res.Value(), nullptr);
    EXPECT_EQ(*(res->get()), 77);
}

TEST(ResultFromStd, WithVoidValue)
{
    std::expected<void, String> exp;
    Result<void, String> res(exp);
    EXPECT_TRUE(res);
}

TEST(ResultFromStd, PreservesValueSemantics)
{
    std::expected<String, Int32> exp = "original";
    Result<String, Int32> res(exp);

    res = Ok("changed");
    EXPECT_EQ(*exp, "original");
    EXPECT_EQ(res.Value(), "changed");
}
#endif

TEST(ResultVoid, DefaultIsOk)
{
    Result<void, String> r;
    EXPECT_TRUE(r.Ok());
    EXPECT_FALSE(r.Err());
}

TEST(ResultVoid, ConstructErr)
{
    Result<void, String> r = Err<String>("fail");
    EXPECT_TRUE(r.Err());
    EXPECT_EQ(r.Error(), "fail");
}

TEST(ResultVoid, CopyConstructOk)
{
    Result<void, String> a;
    Result<void, String> b(a);
    EXPECT_TRUE(b.Ok());
}

TEST(ResultVoid, CopyConstructErr)
{
    Result<void, String> a = Err<String>("fail");
    Result<void, String> b(a);
    EXPECT_TRUE(b.Err());
    EXPECT_EQ(b.Error(), "fail");
}

TEST(ResultVoid, MoveConstructOk)
{
    Result<void, String> a;
    Result<void, String> b(VIOLET_MOVE(a));
    EXPECT_TRUE(b.Ok());
}

TEST(ResultVoid, MoveConstructErr)
{
    Result<void, String> a = Err<String>("fail");
    Result<void, String> b(VIOLET_MOVE(a));
    EXPECT_TRUE(b.Err());
    EXPECT_EQ(b.Error(), "fail");
}

TEST(ResultVoid, CopyAssign)
{
    Result<void, String> a = Err<String>("fail");
    Result<void, String> b;
    b = a;
    EXPECT_TRUE(b.Err());
    EXPECT_EQ(b.Error(), "fail");
}

TEST(ResultVoid, MoveAssign)
{
    Result<void, String> a = Err<String>("fail");
    Result<void, String> b;
    b = VIOLET_MOVE(a);
    EXPECT_TRUE(b.Err());
    EXPECT_EQ(b.Error(), "fail");
}

TEST(ResultVoid, BoolConversion)
{
    Result<void, String> ok;
    Result<void, String> err = Err<String>("bad");

    EXPECT_TRUE(static_cast<bool>(ok));
    EXPECT_FALSE(static_cast<bool>(err));
}

TEST(ResultVoid, OkAndPredicate)
{
    Result<void, String> ok;
    EXPECT_TRUE(ok.OkAnd([] { return true; }));
    EXPECT_FALSE(ok.OkAnd([] { return false; }));
}

TEST(ResultVoid, ErrAndPredicate)
{
    Result<void, String> err = Err<String>("fail");
    EXPECT_TRUE(err.ErrAnd([](const String& s) -> bool { return !s.empty(); }));
}

TEST(ResultVoid, ToString)
{
    Result<void, String> ok;
    Result<void, String> err = Err<String>("fail");

    EXPECT_FALSE(ok.ToString().empty());
    EXPECT_FALSE(err.ToString().empty());
}

namespace {

auto ReturnsOk() -> Result<Int32, String>
{
    return 42;
}

auto ReturnsErr() -> Result<Int32, String>
{
    return Err<String>("fail");
}

auto ReturnsVoidOk() -> Result<void, String>
{
    return { };
}

auto ReturnsVoidErr() -> Result<void, String>
{
    return Err<String>("fail");
}

auto TryOkChain() -> Result<Int32, String>
{
    auto val = VIOLET_TRY(ReturnsOk());
    return val + 1;
}

auto TryErrChain() -> Result<Int32, String>
{
    auto val = VIOLET_TRY(ReturnsErr());
    return val + 1;
}

auto TryVoidOkChain() -> Result<void, String>
{
    VIOLET_TRY_VOID(ReturnsVoidOk());
    return { };
}

auto TryVoidErrChain() -> Result<void, String>
{
    VIOLET_TRY_VOID(ReturnsVoidErr());
    return { };
}

// Error conversion: Int32 -> Int64
auto ReturnsInt32Err() -> Result<Int32, Int32>
{
    return Err<Int32>(42);
}

auto TryWithConversion() -> Result<Int32, Int64>
{
    auto val = VIOLET_TRY(ReturnsInt32Err());
    return val;
}

} // namespace

TEST(VioletTry, PropagatesOk)
{
    auto r = TryOkChain();
    EXPECT_TRUE(r.Ok());
    EXPECT_EQ(r.Value(), 43);
}

TEST(VioletTry, PropagatesErr)
{
    auto r = TryErrChain();
    EXPECT_TRUE(r.Err());
    EXPECT_EQ(r.Error(), "fail");
}

TEST(VioletTryVoid, PropagatesOk)
{
    auto r = TryVoidOkChain();
    EXPECT_TRUE(r.Ok());
}

TEST(VioletTryVoid, PropagatesErr)
{
    auto r = TryVoidErrChain();
    EXPECT_TRUE(r.Err());
    EXPECT_EQ(r.Error(), "fail");
}

TEST(VioletTry, ErrorConversion)
{
    auto r = TryWithConversion();
    EXPECT_TRUE(r.Err());
    EXPECT_EQ(r.Error(), 42L);
}

TEST(ResultTraits, IsResult)
{
    static_assert(is_result_v<Result<Int32, String>>);
    static_assert(is_result_v<Result<void, Int32>>);
    static_assert(!is_result_v<Int32>);
    static_assert(!is_result_v<String>);
}

TEST(ResultTraits, ResultTypeExtraction)
{
    static_assert(std::same_as<result_value_type_t<Result<Int32, String>>, Int32>);
    static_assert(std::same_as<result_error_type_t<Result<Int32, String>>, String>);
    static_assert(std::same_as<result_value_type_t<Result<void, Int32>>, void>);
}

TEST(ResultTraits, NestedResult)
{
    using Inner = Result<Int32, String>;
    using Outer = Result<Inner, String>;

    static_assert(nested_result<Inner, String>);
    static_assert(!nested_result<Int32, String>);
    (void)sizeof(Outer); // suppress unused
}

TEST(Result, LargeValue)
{
    struct Big {
        Int32 data[256] = { };
    };

    Big b;
    b.data[0] = 42;
    b.data[255] = 99;

    Result<Big, String> r = b;
    EXPECT_TRUE(r.Ok());
    EXPECT_EQ(r.Value().data[0], 42);
    EXPECT_EQ(r.Value().data[255], 99);
}

TEST(Result, SelfMoveAssign)
{
    Result<String, Int32> r = String("hello");
    r = VIOLET_MOVE(r);

    // Just verify no crash; value may be in moved-from state
    EXPECT_TRUE(r);
}

TEST(Result, ChainedMap)
{
    Result<Int32, String> r = 10;
    auto final_val
        = VIOLET_MOVE(r).Map([](Int32 v) -> int { return v * 2; }).Map([](Int32 v) -> int { return v + 2; }).Unwrap();

    EXPECT_EQ(final_val, 22);
}

TEST(ResultConstexpr, ConstructOkImplicit)
{
    constexpr Result<Int32, Int32> r = 42;
    static_assert(r.Ok());
    static_assert(!r.Err());
    static_assert(r.Value() == 42);
}

TEST(ResultConstexpr, ConstructOkFromTag)
{
    constexpr Result<Int32, Int32> r = Ok<Int32>(42);
    static_assert(r.Ok());
    static_assert(r.Value() == 42);
}

TEST(ResultConstexpr, ConstructErrFromTag)
{
    constexpr Result<Int32, Int32> r = Err<Int32>(99);
    static_assert(r.Err());
    static_assert(r.Error() == 99);
}

TEST(ResultConstexpr, ConstructOkInPlace)
{
    constexpr Result<Int32, Int32> r(std::in_place_index<0>, 42);
    static_assert(r.Ok());
    static_assert(r.Value() == 42);
}

TEST(ResultConstexpr, ConstructErrInPlace)
{
    constexpr Result<Int32, Int32> r(std::in_place_index<1>, 99);
    static_assert(r.Err());
    static_assert(r.Error() == 99);
}

TEST(ResultConstexpr, BoolConversion)
{
    constexpr Result<Int32, Int32> ok = 42;
    constexpr Result<Int32, Int32> err = Err<Int32>(1);

    static_assert(static_cast<bool>(ok));
    static_assert(!static_cast<bool>(err));
}

TEST(ResultConstexpr, DerefOperator)
{
    constexpr Result<Int32, Int32> r = 42;
    static_assert(*r == 42);
}

TEST(ResultConstexpr, UnwrapOrOnOk)
{
    constexpr Result<Int32, Int32> r = 42;
    static_assert(r.UnwrapOr(0) == 42);
}

TEST(ResultConstexpr, UnwrapOrOnErr)
{
    constexpr Result<Int32, Int32> r = Err<Int32>(1);
    static_assert(r.UnwrapOr(99) == 99);
}

TEST(ResultConstexpr, UnwrapOrDefaultOnOk)
{
    constexpr Result<Int32, Int32> r = 42;
    static_assert(r.UnwrapOrDefault() == 42);
}

TEST(ResultConstexpr, UnwrapOrDefaultOnErr)
{
    constexpr Result<Int32, Int32> r = Err<Int32>(1);
    static_assert(r.UnwrapOrDefault() == 0);
}

TEST(ResultConstexpr, OkAnd)
{
    constexpr Result<Int32, Int32> r = 42;
    static_assert(r.OkAnd([](int v) { return v > 0; }));
    static_assert(!r.OkAnd([](int v) { return v < 0; }));
}

TEST(ResultConstexpr, OkAndOnErr)
{
    constexpr Result<Int32, Int32> r = Err<Int32>(1);
    static_assert(!r.OkAnd([](int) { return true; }));
}

TEST(ResultConstexpr, ErrAnd)
{
    constexpr Result<Int32, Int32> r = Err<Int32>(42);
    static_assert(r.ErrAnd([](int v) { return v > 0; }));
    static_assert(!r.ErrAnd([](int v) { return v < 0; }));
}

TEST(ResultConstexpr, ErrAndOnOk)
{
    constexpr Result<Int32, Int32> r = 42;
    static_assert(!r.ErrAnd([](int) { return true; }));
}

TEST(ResultConstexpr, MapOnOk)
{
    constexpr auto r = Result<Int32, Int32>(21).Map([](int v) { return v * 2; });
    static_assert(r.Ok());
    static_assert(r.Value() == 42);
}

TEST(ResultConstexpr, MapOnErr)
{
    constexpr auto r = Result<Int32, Int32>(Err<Int32>(1)).Map([](int v) { return v * 2; });
    static_assert(r.Err());
    static_assert(r.Error() == 1);
}

TEST(ResultConstexpr, MapOrOnOk)
{
    constexpr Result<Int32, Int32> r = 21;
    constexpr auto val = r.MapOr(0, [](int v) { return v * 2; });
    static_assert(val == 42);
}

TEST(ResultConstexpr, MapOrOnErr)
{
    constexpr Result<Int32, Int32> r = Err<Int32>(1);
    constexpr auto val = r.MapOr(99, [](int v) { return v * 2; });
    static_assert(val == 99);
}

TEST(ResultConstexpr, CopyConstruct)
{
    constexpr Result<Int32, Int32> a = 42;
    constexpr Result<Int32, Int32> b(a);
    static_assert(b.Ok());
    static_assert(b.Value() == 42);
}

TEST(ResultConstexpr, CopyConstructErr)
{
    constexpr Result<Int32, Int32> a = Err<Int32>(99);
    constexpr Result<Int32, Int32> b(a);
    static_assert(b.Err());
    static_assert(b.Error() == 99);
}

namespace {

consteval auto ConstexprTryChain() -> Result<Int32, Int32>
{
    constexpr Result<Int32, Int32> inner = 21;
    // Can't use VIOLET_TRY in consteval (statement expressions),
    // so test the manual pattern
    if (inner.Err()) {
        return Err<Int32>(inner.Error());
    }
    return inner.Value() * 2;
}

consteval auto ConstexprErrChain() -> Result<Int32, Int32>
{
    constexpr Result<Int32, Int32> inner = Err<Int32>(7);
    if (inner.Err()) {
        return Err<Int32>(inner.Error());
    }
    return inner.Value() * 2;
}

consteval auto ConstexprMapChain() -> int
{
    return Result<Int32, Int32>(10).Map([](int v) { return v + 5; }).Map([](int v) { return v * 2; }).UnwrapOr(0);
}

} // namespace

TEST(ResultConsteval, TryChainOk)
{
    constexpr auto r = ConstexprTryChain();
    static_assert(r.Ok());
    static_assert(r.Value() == 42);
}

TEST(ResultConsteval, TryChainErr)
{
    constexpr auto r = ConstexprErrChain();
    static_assert(r.Err());
    static_assert(r.Error() == 7);
}

TEST(ResultConsteval, MapChain)
{
    static_assert(ConstexprMapChain() == 30);
}

#if __cpp_constexpr_dynamic_alloc >= 201907L

TEST(ResultVoidConstexpr, DefaultIsOk)
{
    constexpr Result<void, Int32> r;
    static_assert(r.Ok());
    static_assert(!r.Err());
    static_assert(static_cast<bool>(r));
}

#endif

TEST(ResultTraitsConstexpr, IsResult)
{
    static_assert(is_result_v<Result<Int32, Int32>>);
    static_assert(is_result_v<Result<void, Int32>>);
    static_assert(!is_result_v<Int32>);
    static_assert(!is_result_v<Ok<Int32>>);
    static_assert(!is_result_v<Err<Int32>>);
}

TEST(ResultTraitsConstexpr, NestedResult)
{
    using Inner = Result<Int32, Int32>;
    static_assert(nested_result<Inner, Int32>);
    static_assert(!nested_result<Int32, Int32>);
}

TEST(ResultTraitsConstexpr, ValueAndErrorTypes)
{
    static_assert(std::same_as<result_value_type_t<Result<Int32, Int64>>, Int32>);
    static_assert(std::same_as<result_error_type_t<Result<Int32, Int64>>, Int64>);
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length,performance-unnecessary-copy-initialization)

VIOLET_DIAGNOSTIC_POP
