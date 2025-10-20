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

#include "violet/iter/adapters/Pipes.h"
#include "violet/violet.h"

#include <gtest/gtest.h>

using namespace Noelware::Violet; // NOLINT(google-build-using-namespace)

TEST(PipeSyntax, Position)
{
    Vec<char> chars{ 'h', 'e', 'l', 'l', 'o', '.' };

    auto it = MkIterable(chars) | Position([](char ch) { return ch == '.'; });
    ASSERT_TRUE(it);
    EXPECT_EQ(*it, 5);
}

TEST(PipeSyntax, Count)
{
    Vec<uint32> vi = { 1, 2, 3, 4, 5 };
    ASSERT_EQ(MkIterable(vi) | Count, 5);
}

TEST(PipeSyntax, AdvanceBy)
{
    Vec<uint32> vi = { 1, 2, 3, 4, 5 };
    auto it = MkIterable(vi);

    ASSERT_TRUE(it | AdvanceBy(1));
    ASSERT_EQ(it.Next(), Some<uint32>(2));
}

TEST(PipeSyntax, Nth)
{
    Vec<uint32> vi{ 1, 2, 3 };
    auto it = MkIterable(vi);

    ASSERT_EQ(it | Nth(1), Some<uint32>(2));
}

TEST(PipeSyntax, Any)
{
    Vec<uint32> vi{ 0, 1, 2 };
    ASSERT_TRUE(MkIterable(vi) | Iterators::Any([](uint32 num) { return num == 2; }));
    ASSERT_FALSE(MkIterable(vi) | Iterators::Any([](uint32 num) { return num == 5; }));
}
