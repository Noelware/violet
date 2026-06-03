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

#include "tests/filesystem/support/Layout.h"

#include <violet/Filesystem.h>
#include <violet/Filesystem/Metadata.h>

#include <unordered_set>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet;
using namespace violet::filesystem;
using namespace violet::filesystem::testing;

namespace {

struct FilesystemTest: public LayoutFixture { };

template<typename Iter>
    requires(Iterable<Iter> && std::same_as<iter::TypeOf<Iter>, io::Result<DirEntry>>)
auto collectNames(Iter& it) -> std::unordered_set<String>
{
    std::unordered_set<String> names;
    for (io::Result<DirEntry> ret: it) {
        if (ret.Err()) {
            continue;
        }

        names.insert(ret->Path.Filename());
    }

    return names;
}

} // namespace

TEST_F(FilesystemTest, ExistsReturnsTrueForRegularFile)
{
    EXPECT_TRUE(Exists(Layout->A));
}

TEST_F(FilesystemTest, ExistsReturnsTrueForDirectory)
{
    EXPECT_TRUE(Exists(Layout->Empty));
}

TEST_F(FilesystemTest, ExistsReturnsFalseForMissing)
{
    EXPECT_FALSE(Exists(Layout->Root.Path().Join("nope")));
}

TEST_F(FilesystemTest, ExistsFollowsSymlinkToTarget)
{
    EXPECT_TRUE(Exists(Layout->LinkToA)) << "`Exists' must follow into a live symlink target";
}

TEST_F(FilesystemTest, ExistsReportsFalseForDanglingSymlink)
{
    EXPECT_FALSE(Exists(Layout->Dangling)) << "`Exists' must return false when the target is missing";
}

TEST_F(FilesystemTest, TryExistsDistinguishesMissingFromError)
{
    auto alive = TryExists(Layout->A);
    ASSERT_TRUE(alive) << "failed to check the existence of file [" << Layout->A << "]: " << alive.Error();
    EXPECT_TRUE(*alive);

    auto path = Layout->Root.Path().Join("nope");
    auto missing = TryExists(path);
    ASSERT_TRUE(missing) << "failed to check the existence of file [" << path << "]: " << alive.Error();
    EXPECT_FALSE(*missing);
}

TEST_F(FilesystemTest, CopyDuplicatesContentAndSize)
{
    const Path destination = Layout->Root.Path().Join("a.copy");

    auto bytes = Copy(Layout->A, destination);
    ASSERT_TRUE(bytes) << "failed to copy file [" << Layout->A << "] to [" << destination << "]: " << bytes.Error();
    EXPECT_EQ(*bytes, 5U);

    auto md = Metadata::For(destination);
    ASSERT_TRUE(md);
    EXPECT_EQ(md->Size, 5U);
    EXPECT_TRUE(md->Type.File());
}

TEST_F(FilesystemTest, RenameMovesPath)
{
    const Path destination = Layout->Root.Path().Join("a-renamed.txt");
    auto op = Rename(Layout->A, destination);
    ASSERT_TRUE(op) << "failed to rename path [" << Layout->A << "] to [" << destination << "]: " << op.Error();

    EXPECT_FALSE(Exists(Layout->A));
    EXPECT_TRUE(Exists(destination));
}

TEST_F(FilesystemTest, RemoveFileDeletesARegularFile)
{
    ASSERT_TRUE(Exists(Layout->A));
    ASSERT_TRUE(RemoveFile(Layout->A));
    EXPECT_FALSE(Exists(Layout->A));
}

TEST_F(FilesystemTest, RemoveAllDirsRecursivelyDeletes)
{
    ASSERT_TRUE(Exists(Layout->Nested.Path));

    auto ret = RemoveAllDirs(Layout->Nested.Path);
    ASSERT_TRUE(ret) << ret.Error();

    EXPECT_FALSE(Exists(Layout->Nested.Path));
    EXPECT_FALSE(Exists(Layout->Nested.Deeper.Path));
    EXPECT_FALSE(Exists(Layout->Nested.Deeper.D));
}

TEST_F(FilesystemTest, CreateDirectoryCreatesANewDirectory)
{
    const Path fresh = Layout->Root.Path().Join("brand-new");
    ASSERT_TRUE(CreateDirectory(fresh));

    auto md = Metadata::For(fresh);
    ASSERT_TRUE(md);
    EXPECT_TRUE(md->Type.Dir());
}

TEST_F(FilesystemTest, CreateDirectoriesBuildsTheFullChain)
{
    const Path leaf = Layout->Root.Path().Join("alpha/beta/gamma");
    ASSERT_TRUE(CreateDirectories(leaf));

    EXPECT_TRUE(Exists(Layout->Root.Path().Join("alpha")));
    EXPECT_TRUE(Exists(Layout->Root.Path().Join("alpha/beta")));
    EXPECT_TRUE(Exists(leaf));
}

TEST_F(FilesystemTest, ReadDirOnEmptyDirYieldsNoEntries)
{
    auto dirs = ReadDir(Layout->Empty);
    ASSERT_TRUE(dirs) << dirs.Error();
    EXPECT_FALSE(dirs->Next());
}

TEST_F(FilesystemTest, ReadDirYieldsImmediateChildrenOnly)
{
    auto dirs = ReadDir(Layout->Nested.Path);
    ASSERT_TRUE(dirs) << dirs.Error();

    auto names = collectNames(*dirs);
    EXPECT_EQ(names.size(), 3U) << "expected c.txt, loveletter.txt, deeper/";
    EXPECT_NE(names.find("c.txt"), names.end());
    EXPECT_NE(names.find("loveletter.txt"), names.end());
    EXPECT_NE(names.find("deeper"), names.end());

    // Crucially, it must NOT descend into `deeper/`.
    EXPECT_EQ(names.find("d.txt"), names.end());
}

TEST_F(FilesystemTest, DirsStopTraversingHalts)
{
    auto dirs = ReadDir(Layout->Nested.Path);
    ASSERT_TRUE(dirs);
    ASSERT_TRUE(dirs->Next()) << "expected at least one entry before StopTraversing";

    dirs->StopTraversing();
    EXPECT_FALSE(dirs->Next()) << "StopTraversing must drop the rest";
}

TEST_F(FilesystemTest, WalkDirRecursesIntoSubdirectories)
{
    auto walk = WalkDir(Layout->Nested.Path);
    ASSERT_TRUE(walk) << walk.Error();

    auto names = collectNames(*walk);
    EXPECT_NE(names.find("c.txt"), names.end());
    EXPECT_NE(names.find("loveletter.txt"), names.end());
    EXPECT_NE(names.find("deeper"), names.end());
    EXPECT_NE(names.find("d.txt"), names.end()) << "WalkDir must descend into deeper/";
}

TEST_F(FilesystemTest, WalkDirOnEmptyDirYieldsNothing)
{
    auto walk = WalkDir(Layout->Empty);
    ASSERT_TRUE(walk);
    EXPECT_FALSE(walk->Next());
}

// NOLINTEND(google-build-using-namespace)
