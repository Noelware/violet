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
#include <violet/Violet.h>

using namespace violet; // NOLINT(google-build-using-namespace)

TEST(Result, Ok)
{
    auto result = Ok<String, UInt32>("hello");
    ASSERT_TRUE(result.Ok());
    ASSERT_FALSE(result.Err());

    EXPECT_EQ(result.Value(), "hello");
}

TEST(Result, Err)
{
    Result<String, UInt32> result = Err<UInt32>(404);
    ASSERT_FALSE(result.Ok());
    ASSERT_TRUE(result.Err());

    EXPECT_EQ(result.Error(), 404);
}

TEST(Result, IntoOpt)
{
    auto result = Ok<String, UInt32>("world");
    auto opt = result.IntoOpt();
    ASSERT_TRUE(opt.HasValue());
    EXPECT_EQ(*opt.Value(), "world");

    Result<String, UInt32> err = Err<UInt32>(404);
    auto opt_err = err.IntoOpt();
    ASSERT_FALSE(opt_err.HasValue());
}

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202302L) || __cplusplus >= 202302L
TEST(Result, ToStdExpected)
{
    auto ok = Ok<String, UInt32>("hello");
    auto okStd = static_cast<std::expected<String, UInt32>>(ok);
    ASSERT_TRUE(okStd.has_value());
    EXPECT_EQ(okStd.value(), "hello");

    auto err = Err<UInt32>(404);
    auto errStd = static_cast<std::expected<String, UInt32>>(err);
    ASSERT_FALSE(errStd.has_value());
    EXPECT_EQ(errStd.error(), 404);
}
#endif

TEST(Result, VoidOk)
{
    Result<void, UInt32> result;
    ASSERT_TRUE(result.Ok());
    ASSERT_FALSE(result.Err());
}

TEST(Result, VoidErr)
{
    Result<void, UInt32> result = Err<UInt32>(500);
    ASSERT_FALSE(result.Ok());
    ASSERT_TRUE(result.Err());
    EXPECT_EQ(result.Error(), 500);
}

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 202302L) || __cplusplus >= 202302L
TEST(Result, VoidToStdExpected)
{
    Result<void, UInt32> ok;
    auto okStd = static_cast<std::expected<void, UInt32>>(ok);
    ASSERT_TRUE(okStd);

    Result<void, UInt32> err = Err<UInt32>(500);
    auto errStd = static_cast<std::expected<void, UInt32>>(err);
    ASSERT_FALSE(errStd.has_value());
    EXPECT_EQ(errStd.error(), 500);
}
#endif

TEST(Result, CopyConstructOk)
{
    auto r1 = Ok<String, UInt32>("value");
    auto r2 = r1; // NOLINT(performance-unnecessary-copy-initialization)

    ASSERT_TRUE(r1.Ok());
    EXPECT_EQ(r1.Value(), "value");

    ASSERT_TRUE(r2.Ok());
    EXPECT_EQ(r2.Value(), "value");
}

TEST(Result, CopyConstructErr)
{
    Result<String, UInt32> r1 = Err<UInt32>(123);
    auto r2 = r1; // NOLINT(performance-unnecessary-copy-initialization)

    ASSERT_TRUE(r1.Err());
    EXPECT_EQ(r1.Error(), 123);

    ASSERT_TRUE(r2.Err());
    EXPECT_EQ(r2.Error(), 123);
}

TEST(Result, MoveConstructOk)
{
    auto r1 = Ok<String, UInt32>("value");
    auto r2 = VIOLET_MOVE(r1);

    ASSERT_TRUE(r2.Ok());
    EXPECT_EQ(r2.Value(), "value");
}

TEST(Result, MoveConstructErr)
{
    Result<String, UInt32> r1 = Err<UInt32>(123);
    auto r2 = VIOLET_MOVE(r1);

    ASSERT_TRUE(r2.Err());
    EXPECT_EQ(r2.Error(), 123);
}

TEST(Result, CopyAssignOk)
{
    auto r1 = Ok<String, UInt32>("value");
    Result<String, UInt32> r2 = Err<UInt32>(456);
    r2 = r1;

    ASSERT_TRUE(r1.Ok());
    EXPECT_EQ(r1.Value(), "value");
    ASSERT_TRUE(r2.Ok());
    EXPECT_EQ(r2.Value(), "value");
}

TEST(Result, CopyAssignErr)
{
    Result<String, UInt32> r1 = Err<UInt32>(123);
    auto r2 = Ok<String, UInt32>("value");
    r2 = r1;

    ASSERT_TRUE(r1.Err());
    EXPECT_EQ(r1.Error(), 123);

    ASSERT_TRUE(r2.Err());
    EXPECT_EQ(r2.Error(), 123);
}

TEST(Result, MoveAssignOk)
{
    auto r1 = Ok<String, UInt32>("value");
    Result<String, UInt32> r2 = Err<UInt32>(456);
    r2 = std::move(r1);

    ASSERT_TRUE(r2.Ok());
    EXPECT_EQ(r2.Value(), "value");
}

TEST(Result, MoveAssignErr)
{
    Result<String, UInt32> r1 = Err<UInt32>(123);
    auto r2 = Ok<String, UInt32>("value");
    r2 = VIOLET_MOVE(r1);

    ASSERT_TRUE(r2.Err());
    EXPECT_EQ(r2.Error(), 123);
}

#ifdef VIOLET_HAS_EXCEPTIONS
TEST(Result, UnwrapShouldThrow)
{
    Result<String, UInt32> r1 = Err<UInt32>(123);
    EXPECT_THROW(r1.Unwrap(), std::logic_error);
}

TEST(Result, ExpectShouldThrow)
{
    Result<String, UInt32> r1 = Err<UInt32>(123);
    EXPECT_THROW(r1.Expect("my message here :3"), std::logic_error);
}
#else
TEST(Result, UnwrapShouldAbort)
{
    Result<String, UInt32> r1 = Err<UInt32>(123);
    EXPECT_DEATH(r1.Unwrap(), ".*");
}

TEST(Result, ExpectShouldAbort)
{
    Result<String, UInt32> r1 = Err<UInt32>(123);
    EXPECT_DEATH(r1.Expect("my message here :3"), ".*");
}
#endif
