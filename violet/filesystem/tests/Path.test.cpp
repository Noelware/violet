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

#include "violet/filesystem/Path.h"

#include <gtest/gtest.h>

using namespace Noelware::Violet::Filesystem; // NOLINT
using namespace Noelware::Violet; // NOLINT

TEST(Paths, Filename)
{
    EXPECT_EQ(Path("/usr/local/bin/clang").Filename(), "clang");
    EXPECT_EQ(Path("/usr/local/").Filename(), "local");
    EXPECT_EQ(Path("README.md").Filename(), "README.md");
    EXPECT_EQ(Path("./config.json").Filename(), "config.json");

    EXPECT_FALSE(Path().Filename());
}

TEST(Paths, Extension)
{
    EXPECT_EQ(Path("/home/noeltowa/Documents/loveletter.txt").Extension(), Some<StringRef>("txt"));
    EXPECT_EQ(Path("README.md").Extension(), Some<StringRef>("md"));
    EXPECT_EQ(Path("./config.json").Extension(), Some<StringRef>("json"));

    EXPECT_FALSE(Path(".gitignore").Extension());
    EXPECT_FALSE(Path("/usr/local/").Extension());
    EXPECT_FALSE(Path("/usr/local/bin/clang").Extension());
    EXPECT_FALSE(Path().Extension());
}

TEST(Paths, Stem)
{
    EXPECT_EQ(Path("/usr/local/bin/clang++.txt").Stem(), "clang++");
    EXPECT_EQ(Path("archive.tar.gz").Stem(), "archive.tar");
    EXPECT_EQ(Path(".gitignore").Stem(), StringRef(".gitignore"));
    EXPECT_EQ(Path("README").Stem(), "README");

    EXPECT_FALSE(Path().Stem());
}

TEST(Paths, Parent)
{
    EXPECT_EQ(Path("/usr/local/bin/clang++").Parent(), Some<Path>("/usr/local/bin"));
    EXPECT_EQ(Path("/usr/local/").Parent(), Some<Path>("/usr"));

    EXPECT_FALSE(Path("README.md").Parent());
    EXPECT_FALSE(Path("/").Parent());
    EXPECT_FALSE(Path().Parent());
}

TEST(Paths, Join)
{
    EXPECT_EQ(Path("/usr/local").Join("bin").Join("clang++"), "/usr/local/bin/clang++");
    EXPECT_EQ(Path("/usr/local").Join("/etc/passwd"), "/etc/passwd");

#ifdef _WIN32
    EXPECT_EQ(Path("C:\\Windows").Join("System32"), "C:\\Windows\\System32");
#endif
}

TEST(Paths, WithFilename)
{
    Path clangplusplus("/usr/local/bin/clang++");
    EXPECT_EQ(clangplusplus.WithFilename("gcc-14"), "/usr/local/bin/gcc-14");

    Path root("/");
    EXPECT_EQ(root.WithFilename("home/noeltowa"), StringRef("/home/noeltowa"));
}

TEST(Paths, WithExtension)
{
    Path clangplusplus("/usr/local/bin/clang++");
    EXPECT_EQ(clangplusplus.WithExtension(".exe"), "/usr/local/bin/clang++.exe");
    EXPECT_EQ(clangplusplus.WithExtension("out"), "/usr/local/bin/clang++.out");

    Path noext("README");
    EXPECT_EQ(noext.WithExtension("md"), "README.md");
}

TEST(Paths, CheckAbsoluteRel)
{
    EXPECT_TRUE(Path("/usr/local").Absolute());
    EXPECT_FALSE(Path("noeltowa/Documents/loveletter.txt").Absolute());

#ifdef _WIN32
    EXPECT_TRUE(Path("C:\\Users\\NoelTowa").Absolute());
    EXPECT_FALSE(Path("Documents/loveletter.txt").Absolute());
#endif
}

TEST(Paths, Canonicalize)
{
    Path thisfile("/Workspaces/Noelware/Libraries/violet/violet/filesystem/tests/.././Path.test.cpp");
    Path canon = thisfile.Canonicalize();

    EXPECT_EQ(canon, "/Workspaces/Noelware/Libraries/violet/violet/filesystem/tests/Path.test.cpp");
    EXPECT_TRUE(canon.Absolute());

#ifdef _WIN32
    Path thisfileWin32(
        "C:\\Users\\NoelTowa\\Workspaces\\Noelware\\Libraries\\Violet\\violet/filesystem/tests/.././Path.test.cpp");

    Path win32Canon = thisfileWin32.Canonicalize();
    EXPECT_TRUE(win32Canon.Absolute());
    EXPECT_EQ(win32Canon,
        "C:\\Users\\NoelTowa\\Workspaces\\Noelware\\Libraries\\Violet\\violet\\filesystem\\tests\\Path.test.cpp");
#endif
}
