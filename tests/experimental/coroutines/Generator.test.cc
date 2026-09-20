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
#include <violet/Experimental/Coroutines/Generator.h>
#include <violet/Iterator/Filter.h>

namespace violet::experimental::coro {
namespace {

auto CountUpTo(Int32 n) -> Generator<Int32>
{
    for (Int32 i = 0; i < n; i++)
        co_yield i;
}

#if VIOLET_FEATURE(EXCEPTIONS)
auto ThrowsAfterTwo() -> Generator<Int32>
{
    co_yield 1;
    co_yield 2;
    throw std::runtime_error("generator go BOOM");
}
#endif

} // namespace

TEST(Generator, YieldsSequence)
{
    auto gen = CountUpTo(3);
    ASSERT_EQ(gen.Next(), 0);
    ASSERT_EQ(gen.Next(), 1);
    ASSERT_EQ(gen.Next(), 2);
    ASSERT_FALSE(gen.Next());
    ASSERT_FALSE(gen.Next());
}

TEST(Generator, LazilyUsed)
{
    bool started = false;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    auto make = [&started] -> Generator<Int32> {
        started = true;
        co_yield 1;
    };

    auto gen = make();
    ASSERT_FALSE(started);

    (void)gen.Next();
    ASSERT_TRUE(started);
}

TEST(Generator, ComposesWithIteratorAdapters)
{
    auto sum = CountUpTo(5).Filter([](auto v) -> bool { return v % 2 == 0; }).Fold(0, [](Int32 acc, Int32 v) -> Int32 {
        return acc + v;
    });

    ASSERT_EQ(sum, 0 + 2 + 4);
}

#if VIOLET_FEATURE(EXCEPTIONS)
TEST(Generator, PropagatesException)
{
    auto gen = ThrowsAfterTwo();
    ASSERT_EQ(gen.Next(), 1);
    ASSERT_EQ(gen.Next(), 2);
    ASSERT_THROW((void)gen.Next(), std::runtime_error);
}
#endif

} // namespace violet::experimental::coro
