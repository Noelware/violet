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
#include <violet/Experimental/Any.h>

// NOLINTBEGIN(readability-identifier-length,google-build-using-namespace,performance-unnecessary-copy-initialization)
using namespace violet::experimental;
using violet::Int32;
using violet::String;

TEST(Any, NewIntCanDowncast)
{
    Any value = Any::New<Int32>(42);
    auto downcasted = value.Downcast<Int32>();

    ASSERT_TRUE(downcasted);
    EXPECT_EQ(downcasted.Value(), 42);
}

TEST(Any, NewStringCanDowncast)
{
    Any value = Any::New<String>("hello, world!");
    auto downcasted = value.Downcast<String>();

    ASSERT_TRUE(downcasted);
    EXPECT_EQ(downcasted.Value(), "hello, world!");
}

TEST(Any, NewWithMultipleArgs)
{
    Any value = Any::New<String>(5, 'x');
    auto downcasted = value.Downcast<String>();

    ASSERT_TRUE(downcasted);
    EXPECT_EQ(downcasted.Value(), "xxxxx");
}

TEST(Any, DowncastWithWrongTypeReturnsNothing)
{
    Any value = Any::New<Int32>(69);
    EXPECT_FALSE(value.Downcast<double>());
}

TEST(Any, CopyConstructProducesIndependentCopy)
{
    auto original = Any::New<String>("original");
    auto copy = original;

    // Both should hold the same value.
    auto orig_val = original.Downcast<String>();
    auto copy_val = copy.Downcast<String>();

    ASSERT_TRUE(orig_val);
    ASSERT_TRUE(copy_val);
    EXPECT_EQ(*orig_val, "original");
    EXPECT_EQ(*copy_val, "original");
}

TEST(Any, CopyConstructMutatingOriginalDoesNotAffectCopy)
{
    auto original = Any::New<std::vector<int>>(std::vector<int>{ 1, 2, 3 });
    auto copy = original;

    // Overwrite original.
    original = Any::New<std::vector<int>>(std::vector<int>{ 99 });

    auto copy_val = copy.Downcast<std::vector<int>>();
    ASSERT_TRUE(copy_val);
    EXPECT_EQ(copy_val->size(), 3U);
    EXPECT_EQ((*copy_val)[0], 1);
}

TEST(Any, CopyAssignOverwritesExistingValue)
{
    auto a = Any::New<int>(1);
    auto b = Any::New<int>(2);

    a = b;

    auto result = a.Downcast<int>();
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 2);
}

TEST(Any, CopyAssignSelfAssignment)
{
    auto value = Any::New<String>("self");

    VIOLET_DIAGNOSTIC_PUSH
#if VIOLET_COMPILER(CLANG) || VIOLET_COMPILER(GCC)
    VIOLET_DIAGNOSTIC_IGNORE("-Wself-assign-overloaded")
#endif

    value = value;

    VIOLET_DIAGNOSTIC_POP

    auto result = value.Downcast<String>();
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "self");
}

TEST(Any, MoveConstructTransfersOwnership)
{
    auto original = Any::New<String>("moved");
    auto moved = std::move(original);

    auto result = moved.Downcast<String>();
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "moved");
}

TEST(Any, MoveAssignTransfersOwnership)
{
    auto a = Any::New<int>(10);
    auto b = Any::New<int>(20);

    a = std::move(b);

    auto result = a.Downcast<int>();
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 20);
}

TEST(Any, ToStringStreamableType)
{
    auto value = Any::New<int>(42);
    auto str = value.ToString();

    EXPECT_FALSE(str.empty());
    // int is streamable, so we expect "42".
    EXPECT_EQ(str, "42");
}

TEST(Any, ToStringStringType)
{
    auto value = Any::New<String>("hello");
    auto str = value.ToString();

    EXPECT_FALSE(str.empty());
    EXPECT_EQ(str, "hello");
}

TEST(Any, OperatorOstream)
{
    auto value = Any::New<int>(7);
    std::ostringstream os;

    os << value;

    EXPECT_EQ(os.str(), "7");
}

#if VIOLET_FEATURE(RTTI)
TEST(Any, TypeNameInt)
{
    auto value = Any::New<int>(0);
    auto name = value.TypeName();

    // Demangled name should contain "int".
    EXPECT_NE(name.find("int"), String::npos);
}

TEST(Any, TypeNameString)
{
    auto value = Any::New<String>("x");
    auto name = value.TypeName();

    // Demangled name should contain "string" in some form.
    EXPECT_NE(name.find("string"), String::npos);
}
#endif

namespace {

struct LifecycleTracker { // NOLINT(cppcoreguidelines-special-member-functions)
    static inline int alive = 0;

    LifecycleTracker()
    {
        ++alive;
    }
    LifecycleTracker(const LifecycleTracker&)
    {
        ++alive;
    }
    ~LifecycleTracker()
    {
        --alive;
    }

    // Make it streamable so Message vtable entry works.
    friend auto operator<<(std::ostream& os, const LifecycleTracker&) -> std::ostream&
    {
        return os << "LifecycleTracker";
    }
};

} // namespace

TEST(Any, DestructorDestroysStoredObject)
{
    LifecycleTracker::alive = 0;

    {
        auto value = Any::New<LifecycleTracker>();
        EXPECT_EQ(LifecycleTracker::alive, 1);
    }

    EXPECT_EQ(LifecycleTracker::alive, 0);
}

TEST(Any, CopyIncrementsLifecycleCount)
{
    LifecycleTracker::alive = 0;

    {
        auto a = Any::New<LifecycleTracker>();
        EXPECT_EQ(LifecycleTracker::alive, 1);

        auto b = a;
        EXPECT_EQ(LifecycleTracker::alive, 2);
    }

    EXPECT_EQ(LifecycleTracker::alive, 0);
}

TEST(Any, MoveDoesNotIncrementLifecycleCount)
{
    LifecycleTracker::alive = 0;

    {
        auto a = Any::New<LifecycleTracker>();
        EXPECT_EQ(LifecycleTracker::alive, 1);

        auto b = std::move(a);
        // Move should not create a new object.
        EXPECT_EQ(LifecycleTracker::alive, 1);
    }

    EXPECT_EQ(LifecycleTracker::alive, 0);
}

// NOLINTEND(readability-identifier-length,google-build-using-namespace,performance-unnecessary-copy-initialization)
