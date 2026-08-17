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
#include <violet/Experimental/Memory/NonNull.h>
#include <violet/Experimental/Slice.h>

// NOLINTBEGIN(readability-identifier-length)
using namespace violet::experimental::ptr;
using namespace violet::experimental;
using namespace violet;

namespace {

struct Incomplete;

// NOLINTBEGIN(cppcoreguidelines-special-member-functions)
struct MoveOnly final {
    MoveOnly(MoveOnly&&) noexcept;
    ~MoveOnly();
};

#if VIOLET_COMPILER(CLANG)
VIOLET_DIAGNOSTIC_PUSH
VIOLET_DIAGNOSTIC_IGNORE("-Wundefined-internal")
#endif

struct NonConstexprDtor final {
    ~NonConstexprDtor();
};

struct NoDefaultCtor final {
    ~NoDefaultCtor();
};

#if VIOLET_COMPILER(CLANG)
VIOLET_DIAGNOSTIC_POP
#endif

#if VIOLET_FEATURE(EXCEPTIONS)
struct ThrowingConstructor final {
    VIOLET_EXPLICIT ThrowingConstructor(Int32)
    {
        throw 42;
    }
};
#endif
// NOLINTEND(cppcoreguidelines-special-member-functions)

static_assert(sizeof(NonNull<Int32>) == sizeof(Int32*));
static_assert(alignof(NonNull<Int32>) == alignof(Int32*));
static_assert(std::is_standard_layout_v<NonNull<Int32>>);

// usable with incomplete types; intrusive lists, PIMPL, btree nodes
static_assert(sizeof(NonNull<Incomplete>) == sizeof(void*));

static_assert(std::is_trivially_copyable_v<NonNull<Int32>>);
static_assert(std::is_trivially_destructible_v<NonNull<Int32>>);
static_assert(std::is_trivially_copy_constructible_v<NonNull<Int32>>);
static_assert(std::is_trivially_move_constructible_v<NonNull<Int32>>);
static_assert(std::is_trivially_copy_assignable_v<NonNull<Int32>>);
static_assert(std::is_trivially_move_assignable_v<NonNull<Int32>>);

// `T`'s own semantics should never leak into the pointer.
static_assert(std::is_trivially_copyable_v<NonNull<MoveOnly>>);
static_assert(std::is_trivially_destructible_v<NonNull<MoveOnly>>);
static_assert(std::is_trivially_copyable_v<NonNull<NonConstexprDtor>>);
static_assert(std::is_trivially_destructible_v<NonNull<NoDefaultCtor>>);

static_assert(violet::is_trivially_relocatable_v<NonNull<Int32>>);
static_assert(violet::is_trivially_relocatable_v<NonNull<MoveOnly>>);

static_assert(!std::is_default_constructible_v<NonNull<Int32>>);
static_assert(!std::is_constructible_v<NonNull<Int32>, std::nullptr_t>);
static_assert(!std::is_convertible_v<Int32*, NonNull<Int32>>);

static_assert(std::is_convertible_v<NonNull<Int32>, NonNull<const Int32>>);
static_assert(!std::is_convertible_v<NonNull<const Int32>, NonNull<Int32>>);
static_assert(!std::is_convertible_v<NonNull<volatile Int32>, NonNull<Int32>>);

static_assert(!ptr_internal::sentinel_constructible<Incomplete>);
static_assert(!ptr_internal::sentinel_constructible<NoDefaultCtor>);
static_assert(!ptr_internal::sentinel_constructible<NonConstexprDtor>);

static_assert(!std::is_convertible_v<NonNull<Int32>, NonNull<float>>);
static_assert(!std::is_convertible_v<NonNull<Int32>, NonNull<void>>);

static_assert(std::copyable<NonNull<Int32>>);
static_assert(std::equality_comparable<NonNull<Int32>>);
static_assert(std::totally_ordered<NonNull<Int32>>);

static_assert(!std::default_initializable<NonNull<Int32>>);
static_assert(!std::semiregular<NonNull<Int32>>);
static_assert(!std::regular<NonNull<Int32>>);

static_assert(std::copyable<NonNull<MoveOnly>>);
static_assert(std::copyable<NonNull<Incomplete>>);

static_assert(!ptr_internal::sentinel_constructible<Incomplete>);
static_assert(!ptr_internal::sentinel_constructible<NoDefaultCtor>);
static_assert(!ptr_internal::sentinel_constructible<NonConstexprDtor>);

static_assert(NonNull<Int32>::Dangling().Get() != nullptr);

constexpr inline auto kDangling = NonNull<Int32>::Dangling();
static_assert(kDangling == NonNull<Int32>::Dangling());

template<typename T>
concept has_dangling = requires { NonNull<T>::Dangling(); };

static_assert(has_dangling<Int32>);
static_assert(has_dangling<int[]>); // alignof(int[]) is valid, sizeof is not
static_assert(has_dangling<NoDefaultCtor>); // falls to the runtime branch, still exists
static_assert(!has_dangling<void>);
static_assert(!has_dangling<void()>);

static_assert(sizeof(NonNull<void>) == sizeof(void*));

struct node final {
    NonNull<node> Next;
    Int32 Value;
};

static_assert(std::is_trivially_copyable_v<node>);

} // namespace

TEST(NonNull, CTAD)
{
    Int32 x = 0;
    NonNull p1(x);
    static_assert(std::same_as<decltype(p1), NonNull<Int32>>);

    NonNull p2(p1);
    static_assert(std::same_as<decltype(p2), NonNull<Int32>>);

    const Int32 y = 0;
    NonNull p3(y);
    static_assert(std::same_as<decltype(p3), NonNull<const Int32>>);

    ASSERT_EQ(p1.Get(), &x);
    ASSERT_TRUE(p2 == p1);
    ASSERT_EQ(p3.Get(), &y);
}

static_assert(std::same_as<decltype(std::declval<const NonNull<Int32>&>().Get()), Int32*>);
static_assert(std::same_as<decltype(*std::declval<const NonNull<Int32>&>()), Int32&>);
static_assert(std::same_as<decltype(std::declval<const NonNull<Int32>&>().operator->()), Int32*>);
static_assert(std::same_as<decltype(std::declval<NonNull<const Int32>&>().Get()), const Int32*>);

TEST(NonNull, ConstPointerStillWrites)
{
    Int32 x = 1;
    const auto ptr = NonNull<Int32>::NewUnchecked(Unsafe("stack address"), &x);
    *ptr = 2;
    ASSERT_EQ(x, 2);
}

TEST(NonNull, From)
{
    Int32 x = 7;
    ASSERT_TRUE(NonNull<Int32>::New(&x));
    ASSERT_FALSE(NonNull<Int32>::New(nullptr));
    ASSERT_EQ(NonNull<Int32>::New(&x)->Get(), &x);
}

TEST(NonNull, CopyIsIndependentAndMoveDoesNotNull)
{
    Int32 x = 0;
    auto base = NonNull<Int32>::NewUnchecked(Unsafe("stack address"), &x);

    auto copy = base;
    auto moved = VIOLET_MOVE(copy);

    ASSERT_EQ(copy, base);
    ASSERT_EQ(moved, base);
}

TEST(NonNull, DanglingIdentityAcrossEvaluationBoundary)
{
    ASSERT_TRUE(NonNull<Int32>::Dangling() == kDangling);
}

TEST(NonNull, CastRoundTrip)
{
    Int32 x = 0;
    const auto base = NonNull<Int32>::NewUnchecked(Unsafe("stack address"), &x);
    ASSERT_EQ(base.Cast<UInt8>(Unsafe("casting between integers is ok"))
                  .Cast<Int32>(Unsafe("casting between integers is ok")),
        base);
}

// NOLINTEND(readability-identifier-length)
