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
#include <violet/Experimental/Threading/Parallelism.h>
#include <violet/Print.h>
#include <violet/System.h>

#include <cstdlib>

namespace violet::experimental::threading {
namespace {
inline constexpr auto kExpectedParallelism = "VIOLET_EXPECTED_PARALLELISM";
}

TEST(Parallelism, IsNeverZero)
{
    EXPECT_GE(AvailableParallelism(), 1U);
}

TEST(Parallelism, MatchesExpectedWhenConstrained)
{
    const auto count = AvailableParallelism();
    Println("`violet::experimental::threading::AvailableParallelism()` = {}", count);

    auto expected = sys::GetEnv(kExpectedParallelism);
    if (!expected.HasValue()) {
        GTEST_SKIP() << "`$" << kExpectedParallelism << "` was not set";
    }

    EXPECT_EQ(count, ::strtoull(expected->c_str(), nullptr, 10));
}

} // namespace violet::experimental::threading
