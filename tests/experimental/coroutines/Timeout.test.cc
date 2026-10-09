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
#include <violet/Experimental/Coroutines/Runtime.h>
#include <violet/Experimental/Coroutines/Timeout.h>

namespace violet::experimental::coro {
namespace {

using chrono::Duration;

auto SleepsThenReturns(Int32 millis, bool* finished) -> Task<Int32>
{
    co_await Sleep(Duration::Milliseconds(millis));
    *finished = true;
    co_return 7;
}

auto WithTimeout(Int32 work, Int32 limit, bool* finished) -> Task<Optional<Int32>>
{
    co_return co_await Timeout(Duration::Milliseconds(limit), SleepsThenReturns(work, finished));
}

} // namespace

TEST(Timeout, FinishesInTime)
{
    bool finished = false;
    auto rt = Runtime::Builder::CurrentThread().EnableTimers().Build();

    Optional<Int32> result = rt->BlockOn(WithTimeout(5, 100, &finished));
    ASSERT_EQ(result.Value(), 7);
    ASSERT_TRUE(finished);
}

TEST(Timeout, CancelsSlowTask)
{
    bool finished = false;
    auto rt = Runtime::Builder::CurrentThread().EnableTimers().Build();

    Optional<Int32> result = rt->BlockOn(WithTimeout(60'000, 10, &finished));
    ASSERT_FALSE(result.HasValue());
    ASSERT_FALSE(finished);
}

} // namespace violet::experimental::coro
