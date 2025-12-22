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
#include <violet/Filesystem/Path.h>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::io; // NOLINT(google-build-using-namespace)
using namespace violet::filesystem; // NOLINT(google-build-using-namespace)

TEST(Paths, Empty)
{
    Path path;

    EXPECT_FALSE(path);
    EXPECT_FALSE(path.Absolute());
    EXPECT_TRUE(path.Relative());
}

#ifdef VIOLET_WINDOWS
TEST(Paths, Absolute)
{
    PathRef system32 = "C:\\Windows\\System32";
    EXPECT_TRUE(system32.Absolute());
    EXPECT_FALSE(system32.Relative());

    PathRef unc = "\\\\server\share";
    EXPECT_TRUE(unc.Absolute());
}
#else
TEST(Paths, Absolute)
{
    PathRef bin = "/usr/local/bin";

    EXPECT_TRUE(bin.Absolute());
    EXPECT_FALSE(bin.Relative());
}
#endif

TEST(Paths, FilenameExt)
{
    PathRef file = "/home/noeltowa/file.txt";
    EXPECT_EQ(file.Filename(), "file.txt");

    auto ext = file.Extension();
    ASSERT_TRUE(ext);
    EXPECT_EQ(*ext, "txt");

    PathRef gitignore = "/Workspaces/Noelware/Libraries/violet/.gitignore";
    EXPECT_EQ(gitignore.Filename(), ".gitignore");
    EXPECT_FALSE(gitignore.Extension());
}

TEST(Paths, Stem)
{
    PathRef file = "/home/noeltowa/file.txt";
    EXPECT_EQ(file.Stem(), "file");

    PathRef gitignore = "/Workspaces/Noelware/Libraries/violet/.gitignore";
    EXPECT_EQ(gitignore.Stem(), ".gitignore");
}

TEST(Paths, Parent)
{
    PathRef file = "/home/noeltowa/file.txt";
    auto parent = file.Parent();
    ASSERT_TRUE(parent);

#ifndef VIOLET_WINDOWS
    EXPECT_EQ(*parent.Value(), Path{ "/home/noeltowa" });
#endif
}

TEST(Paths, Join)
{
    Path base("/home/noeltowa");
    Path joined = base.Join("docs/file.txt");
    EXPECT_EQ(joined.Filename(), "file.txt");

    Path absolute("/etc/passwd");
    joined = base.Join("/etc/passwd"); // absolute overrides base
    EXPECT_EQ(joined, absolute);
}

TEST(Paths, WithFilename)
{
    Path path("/home/noeltowa/file.txt");
    Path replaced = path.WithFilename("other.txt");

    EXPECT_EQ(replaced.Filename(), "other.txt");
    EXPECT_EQ(replaced.Stem(), "other");
}

TEST(Paths, WithExtension)
{
    Path path("/home/noeltowa/file.txt");
    Path changed = path.WithExtension("md");
    EXPECT_EQ(changed.Extension(), Some<String>("md"));

    Path dotfile("/home/noeltowa/.gitignore");
    Path unchanged = dotfile.WithExtension("txt");
    EXPECT_EQ(unchanged.Filename(), ".gitignore.txt");
}

TEST(Paths, Canonicalize)
{
    PathRef clang = "/usr/./local/../bin/clang";
    Path canon = clang.Canonicalize();

#ifndef VIOLET_WINDOWS
    EXPECT_EQ(canon, Path("/usr/bin/clang"));
#endif
}

TEST(Paths, JoinAndParent)
{
    Path path("/usr/local");
    Path joined = path.Join("bin/clang");
    EXPECT_EQ(joined.Filename(), "clang");

    auto parent = joined.Parent();
    ASSERT_TRUE(parent);
    EXPECT_EQ(*parent, Path("/usr/local/bin"));
}

TEST(Paths, Operators)
{
    PathRef a("/home/user/file.txt"); // NOLINT(readability-identifier-length)
    PathRef b("/home/user/file.txt"); // NOLINT(readability-identifier-length)
    PathRef c("/home/user/other.txt"); // NOLINT(readability-identifier-length)

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a == c);
}
