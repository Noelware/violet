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
#include <violet/Support/Terminal.h>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::terminal; // NOLINT(google-build-using-namespace)

TEST(RGB, UInt8)
{
    constexpr RGB rgb(255, 128, 0);

    ASSERT_EQ(rgb.Red, 255);
    ASSERT_EQ(rgb.Green, 128);
    ASSERT_EQ(rgb.Blue, 0);
}

TEST(RGB, Paint)
{
    constexpr RGB rgb(255, 128, 0);

    auto painted = rgb.Paint();
    ASSERT_FALSE(painted.empty());
    ASSERT_TRUE(painted.find("\x1b[") != String::npos);
}

TEST(Style, NamedColorForeground)
{
    constexpr Style red = Style::Red();

    auto painted = red.Paint();
    ASSERT_FALSE(painted.empty());
    ASSERT_TRUE(painted.find("\x1b[") != String::npos);
    ASSERT_TRUE(painted.find("31m") != String::npos); // ANSI foreground red
}

TEST(Style, NamedColorBackground)
{
    constexpr Style blue = Style::Blue(/*foreground=*/false);

    auto painted = blue.Paint();
    ASSERT_FALSE(painted.empty());
    ASSERT_TRUE(painted.find("\x1b[") != String::npos);
    ASSERT_TRUE(painted.find("44m") != String::npos); // ANSI background blue
}

TEST(Style, RGBPaint)
{
    constexpr Style rgb = Style::RGB<0, 255, 0>();

    auto painted = rgb.Paint();
    ASSERT_FALSE(painted.empty());
    ASSERT_TRUE(painted.find("38;2;0;255;0") != String::npos);
}

TEST(Style, Tags)
{
    Style style = Style().Bold().Italic().Underline();

    auto painted = style.Paint();
    EXPECT_TRUE(painted.find("1m") != std::string::npos); // bold
    EXPECT_TRUE(painted.find("3m") != std::string::npos); // italic
    EXPECT_TRUE(painted.find("4m") != std::string::npos); // underline
}
