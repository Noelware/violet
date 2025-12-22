// 🌺💜 Violet: Extended C++ standard library
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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
#include <violet/Iterator.h>

using namespace violet; // NOLINT(google-build-using-namespace)

TEST(Iterators, FoldSum) // sounds like folsom
{
    auto vec = Vec<UInt32>({ 0, 1, 2, 3, 4 });
    auto it = MkIterable(vec);

    UInt32 sum = it.Fold(0, [](UInt32 acc, UInt32 value) -> UInt32 { return acc + value; });
    ASSERT_EQ(sum, 10);
}

TEST(Iterators, Position)
{
    auto vec = Vec<UInt32>({ 0, 2, 4 });
    auto it = MkIterable(vec);

    auto pos1 = it.Position([](UInt32 value) -> bool { return value == 2; });
    ASSERT_TRUE(pos1);
    ASSERT_EQ(*pos1, 1);

    auto pos2 = it.Position([](UInt32 value) -> bool { return value == 9; });
    ASSERT_FALSE(pos2);
}

TEST(Iterators, Find)
{
    auto vec = Vec<UInt32>({ 1, 2, 3 });
    auto it = MkIterable(vec);

    auto even = it.Find([](UInt32 value) -> bool { return value % 2 == 0; });
    ASSERT_TRUE(even);
    ASSERT_EQ(*even, 2);
}

TEST(Iterators, FindMap)
{
    Vec<UInt32> vi({ 1, 2, 3, 4 });
    auto found = MkIterable(vi).FindMap([](UInt32 value) -> Optional<UInt32> {
        if (value % 2 == 0) {
            return Some<UInt32>(value * 10);
        }

        return Nothing;
    });

    ASSERT_TRUE(found);
    ASSERT_EQ(found, 20);
}

TEST(Iterators, Count)
{
    Vec<UInt32> vi = { 1, 2, 3, 4, 5 };
    ASSERT_EQ(MkIterable(vi).Count(), 5);
}

TEST(Iterators, AdvanceBy)
{
    Vec<UInt32> vi = { 1, 2, 3, 4, 5 };
    auto it = MkIterable(vi);

    ASSERT_TRUE(it.AdvanceBy(1));
    ASSERT_EQ(it.Next(), Some<UInt32>(2));
}

TEST(Iterators, Nth)
{
    Vec<UInt32> vi{ 1, 2, 3 };
    auto it = MkIterable(vi);

    ASSERT_EQ(it.Nth(1), Some<UInt32>(2));
}

TEST(Iterators, Any)
{
    Vec<UInt32> vi{ 0, 1, 2 };
    auto it = MkIterable(vi);

    ASSERT_TRUE(it.Any([](int num) { return num == 2; }));
    ASSERT_FALSE(it.Any([](int num) { return num == 5; }));
}

TEST(Iterators, CollectVec)
{
    Vec<UInt32> vi({ 0, 1, 2 });

    auto vec = MkIterable(vi).Collect<Vec<UInt32>>();
    ASSERT_EQ(vi, vec);
}

TEST(Iterators, CollectArray)
{
    Vec<UInt32> vi({ 0, 1, 2 });

    auto vec = MkIterable(vi).Collect<Array<UInt32, 3>>();
    ASSERT_EQ(vi[0], vec[0]);
    ASSERT_EQ(vi[1], vec[1]);
    ASSERT_EQ(vi[2], vec[2]);
}
