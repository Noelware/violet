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
#include <violet/Iterator/Peekable.h>
#include <violet/Violet.h>

using namespace violet; // NOLINT(google-build-using-namespace)

TEST(Iterators, PeekOnEmptyReturnsNull)
{
    Vec<UInt32> vi{};
    auto it = MkIterable(vi).Peekable();

    ASSERT_EQ(it.Peek(), Nothing);
    ASSERT_EQ(it.Next(), Nothing);
}

TEST(Iterators, PeekDoesntConsume)
{
    Vec<UInt32> vi({ 1, 2, 3 });
    auto it = MkIterable(vi).Peekable();

    auto peeked = it.Peek();
    ASSERT_NE(peeked, Nothing);
    ASSERT_EQ(*peeked.Value(), 1);

    auto value = it.Next();
    ASSERT_TRUE(value);
    ASSERT_EQ(*value.Value(), 1);
}

TEST(Iterators, StableMultiplePeek)
{
    Vec<UInt32> vi({ 10, 20 });
    auto it = MkIterable(vi).Peekable();

    auto p1 = it.Peek();
    auto p2 = it.Peek();
    ASSERT_NE(p1, Nothing);
    ASSERT_NE(p2, Nothing);
    EXPECT_EQ(p1, p2);
    EXPECT_EQ(*p1.Value(), 10);
}

TEST(Iterators, PeekableNextAfterPeekAdvance)
{
    Vec<UInt32> vi({ 10, 20 });
    auto it = MkIterable(vi).Peekable();

    it.Peek();
    auto vi1 = it.Next();
    auto vi2 = it.Next();

    ASSERT_TRUE(vi1);
    EXPECT_EQ(*vi1.Value(), 10);

    ASSERT_TRUE(vi2);
    EXPECT_EQ(*vi2.Value(), 20);
    EXPECT_EQ(it.Next(), Nothing);
}
