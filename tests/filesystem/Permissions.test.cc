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
#include <violet/Filesystem/Permissions.h>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::filesystem; // NOLINT(google-build-using-namespace)

TEST(Permissions, BasicReadonly)
{
    Permissions perm(Mode(S_IRUSR | S_IRGRP | S_IROTH)); // r--r--r--
    EXPECT_TRUE(perm.Readonly());
    perm.SetReadonly(false);
    EXPECT_FALSE(perm.Readonly());
}

#ifdef VIOLET_UNIX

#include <sys/stat.h>

TEST(Mode, PermissionAccessors)
{
    Mode mode(S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);

    EXPECT_TRUE(mode.OwnerCanRead());
    EXPECT_TRUE(mode.OwnerCanWrite());
    EXPECT_TRUE(mode.OwnerCanExecute());
    EXPECT_TRUE(mode.OtherCanRead());
    EXPECT_FALSE(mode.OtherCanWrite());
    EXPECT_FALSE(mode.OtherCanExecute());
}

TEST(Mode, BitwiseOperators)
{
    Mode mode1(S_IRUSR | S_IWUSR);
    Mode mode2(S_IXUSR);

    Mode mode3 = mode1 | mode2;
    EXPECT_TRUE(mode3.OwnerCanRead());
    EXPECT_TRUE(mode3.OwnerCanWrite());
    EXPECT_TRUE(mode3.OwnerCanExecute());

    mode3 &= S_IRUSR;
    EXPECT_TRUE(mode3.OwnerCanRead());
    EXPECT_FALSE(mode3.OwnerCanWrite());
    EXPECT_FALSE(mode3.OwnerCanExecute());
}

TEST(Mode, ToStringFormatting)
{
    Mode mode(S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    auto str = mode.ToString();
    EXPECT_EQ(str.size(), 10);
    EXPECT_EQ(str[0], '-');
    EXPECT_EQ(str.substr(1, 9), "rw-r--r--");
}

TEST(Mode, ComparisonOperators)
{
    Mode mode1(S_IRUSR | S_IWUSR);
    Mode mode2(S_IRUSR | S_IWUSR);
    Mode mode3(S_IRUSR);

    EXPECT_TRUE(mode1 == mode2);
    EXPECT_TRUE(mode1 != mode3);
    EXPECT_FALSE(mode1 < mode3);
}

TEST(Permissions, ToStringDelegatesToMode)
{
    Permissions perm(Mode(S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    EXPECT_EQ(perm.ToString(), "Permissions(readonly=false, mode=\"-rw-r--r--\")");
}

#endif
