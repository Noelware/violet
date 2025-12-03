// 🌺💜 Violet: Extended standard library for C++26
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
#include <violet/Iterator/Skip.h>
#include <violet/Violet.h>

using namespace violet; // NOLINT(google-build-using-namespace)

TEST(Iterators, SkipNoneReturnsAll)
{
    Vec<UInt32> vi({ 1, 2, 3, 4 });
    auto it = MkIterable(vi).Skip(0);

    ASSERT_EQ(it.Collect<Vec<UInt32>>(), vi);
}

TEST(Iterators, SkipSome)
{
    Vec<UInt32> vi({ 10, 20, 30, 40, 50 });
    auto it = MkIterable(vi).Skip(2);

    Vec<UInt32> expected({ 30, 40, 50 });
    ASSERT_EQ(it.Collect<Vec<UInt32>>(), expected);
}

TEST(Iterators, SkipPastEndYieldsEmpty)
{
    Vec<UInt32> vi({ 10, 20 });
    auto it = MkIterable(vi).Skip(10);

    ASSERT_TRUE(it.Collect<Vec<UInt32>>().empty());
}
