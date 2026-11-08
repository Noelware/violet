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
#include <violet/Filesystem/Path.h>

#include <compare>

namespace violet::filesystem {

TEST(Paths, Empty)
{
    Path path;

    EXPECT_FALSE(path);
    EXPECT_FALSE(path.Absolute());
    EXPECT_TRUE(path.Relative());
}

#if VIOLET_PLATFORM(WINDOWS)
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
    EXPECT_EQ(parent->Data(), "/home/noeltowa");
#endif
}

TEST(Paths, Join)
{
    Path base("/home/noeltowa");
    Path joined = base.Join("docs/file.txt");
    EXPECT_EQ(joined.Filename(), "file.txt");

    Path absolute("/etc/passwd");
    joined = base.Join("/etc/passwd"); // absolute overrides base
    EXPECT_EQ(joined.Data(), absolute.Data());
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
    EXPECT_EQ(canon.Data(), "/usr/bin/clang");
#endif
}

TEST(Paths, JoinAndParent)
{
    Path path("/usr/local");
    Path joined = path.Join("bin/clang");
    EXPECT_EQ(joined.Filename(), "clang");

    auto parent = joined.Parent();
    ASSERT_TRUE(parent);
    EXPECT_EQ(parent->Data(), "/usr/local/bin");
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

#if !VIOLET_PLATFORM(WINDOWS)
TEST(Paths, SlashOperator)
{
    Path base("/home/noeltowa");

    EXPECT_EQ((base / "docs").Data(), "/home/noeltowa/docs");
    EXPECT_EQ((base / "docs" / "file.txt").Data(), "/home/noeltowa/docs/file.txt");
    EXPECT_EQ((base / String("docs")).Data(), "/home/noeltowa/docs");
    EXPECT_EQ((base / Str("docs")).Data(), "/home/noeltowa/docs");
    EXPECT_EQ((base / Path("docs")).Data(), "/home/noeltowa/docs");

    // an absolute right-hand side replaces the base, like `Join`
    EXPECT_EQ((base / "/etc/passwd").Data(), "/etc/passwd");

    // no doubled separator when the base already ends with one
    Path trailing("/usr/");
    EXPECT_EQ((trailing / "bin").Data(), "/usr/bin");

    // joining onto a borrowed path produces an owned one
    PathRef ref("/usr/local");
    auto joined = ref / "bin";
    static_assert(std::same_as<decltype(joined), Path>);
    EXPECT_EQ(joined.Data(), "/usr/local/bin");
}
#endif

TEST(Paths, ThreeWayComparison)
{
    PathRef a("/home/user/file.txt");
    PathRef b("/home/user/file.txt");
    PathRef c("/home/user/other.txt");

    // path <=> path
    EXPECT_EQ(a <=> b, std::strong_ordering::equal);
    EXPECT_EQ(a <=> c, std::strong_ordering::less);
    EXPECT_EQ(c <=> a, std::strong_ordering::greater);
    EXPECT_TRUE(a < c);
    EXPECT_TRUE(c > a);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a >= b);

    Path owned("/home/user/file.txt");
    Path ownedOther("/home/user/other.txt");
    EXPECT_TRUE(owned < ownedOther);
    EXPECT_EQ(owned <=> Path("/home/user/file.txt"), std::strong_ordering::equal);

    // path <=> string types, including the reversed forms
    EXPECT_EQ(a <=> "/home/user/file.txt", std::strong_ordering::equal);
    EXPECT_TRUE(a < "/home/user/other.txt");
    EXPECT_TRUE("/home/user/other.txt" > a);
    EXPECT_TRUE(a < Str("/home/user/other.txt"));
    EXPECT_TRUE(owned < String("/home/user/other.txt"));

    // Path <=> PathRef, both ways
    EXPECT_EQ(owned <=> a, std::strong_ordering::equal);
    EXPECT_TRUE(owned < c);
    EXPECT_TRUE(c > owned);
}

TEST(Paths, StartsWith)
{
    PathRef path("/usr/lib/libc.so");

    EXPECT_TRUE(path.StartsWith(PathRef("/usr/lib")));
    EXPECT_TRUE(path.StartsWith(PathRef("/usr/lib/")));
    EXPECT_TRUE(path.StartsWith(PathRef("/usr")));
    EXPECT_TRUE(path.StartsWith(PathRef("/")));
    EXPECT_TRUE(path.StartsWith(path));
    EXPECT_TRUE(path.StartsWith(PathRef("")));

    // compares whole components, not raw prefixes
    EXPECT_FALSE(path.StartsWith(PathRef("/usr/li")));
    EXPECT_FALSE(PathRef("/usr/libexec").StartsWith(PathRef("/usr/lib")));
    EXPECT_FALSE(path.StartsWith(PathRef("/usr/lib/libc.so/extra")));
    EXPECT_FALSE(path.StartsWith(PathRef("/opt")));

    // a non-const lvalue still picks the component-wise overload, not the variadic one
    PathRef prefix("/usr/lib");
    EXPECT_FALSE(PathRef("/usr/libexec").StartsWith(prefix));

    Path owned("/home/noeltowa/docs/file.txt");
    Path ownedPrefix("/home/noeltowa");
    EXPECT_TRUE(owned.StartsWith(ownedPrefix));
    EXPECT_FALSE(owned.StartsWith(Path("/home/noel")));
}

TEST(Paths, StartsWithString)
{
    // string arguments forward to the underlying string's `starts_with`, so they're raw prefix checks
    PathRef path("/usr/libexec");

    EXPECT_TRUE(path.StartsWith("/usr/lib"));
    EXPECT_TRUE(path.StartsWith(Str("/usr/lib")));
    EXPECT_TRUE(path.StartsWith('/'));
    EXPECT_FALSE(path.StartsWith("/opt"));

    Path owned("/usr/libexec");
    EXPECT_TRUE(owned.StartsWith("/usr/lib"));
    EXPECT_TRUE(owned.StartsWith(String("/usr")));
}

} // namespace violet::filesystem
