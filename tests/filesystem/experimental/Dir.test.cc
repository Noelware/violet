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
#include <violet/Filesystem/Experimental/Dir.h>
#include <violet/Filesystem/Metadata.h>
#include <violet/IO/Read.h>

#include <unordered_set>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet;
using namespace violet::filesystem;
using namespace violet::filesystem::experimental;
using namespace violet::filesystem::testing;

namespace {

struct DirTest: public LayoutFixture { };

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

TEST_F(DirTest, OpenReturnsAnAliveHandle)
{
    auto dir = Dir::Open(Layout->Root.Path());
    ASSERT_TRUE(dir) << "failed to open directory: " << dir.Error();
    EXPECT_TRUE(dir->Alive());
    EXPECT_TRUE(static_cast<bool>(*dir));
    EXPECT_NE(dir->Descriptor(), -1);
}

TEST_F(DirTest, OpenFailsForMissingDirectory)
{
    auto dir = Dir::Open(Layout->Root.Path().Join("does-not-exist"));
    EXPECT_FALSE(dir) << "opening a non-existent directory must fail";
}

TEST_F(DirTest, OpenFailsForRegularFile)
{
    auto dir = Dir::Open(Layout->A);
    EXPECT_FALSE(dir) << "opening a regular file as a directory must fail (ENOTDIR)";
}

TEST_F(DirTest, OpenFileResolvesRelativeToTheDirectory)
{
    auto dir = Dir::Open(Layout->Root.Path());
    ASSERT_TRUE(dir) << dir.Error();

    // `a.txt` is resolved relative to the open directory, not the process CWD.
    auto file = dir->OpenFile("a.txt");
    ASSERT_TRUE(file) << "failed to open a.txt relative to the dir: " << file.Error();

    auto bytes = io::ReadToBytes(*file);
    ASSERT_TRUE(bytes) << bytes.Error();
    ASSERT_EQ(bytes->size(), 5U);
    EXPECT_EQ(std::memcmp(bytes->data(), "hello", 5), 0);
}

TEST_F(DirTest, OpenFileFailsForMissingEntry)
{
    auto dir = Dir::Open(Layout->Root.Path());
    ASSERT_TRUE(dir) << dir.Error();

    auto file = dir->OpenFile("nope.txt");
    EXPECT_FALSE(file) << "opening a missing entry must fail";
}

TEST_F(DirTest, MetadataReportsDirectory)
{
    auto dir = Dir::Open(Layout->Root.Path());
    ASSERT_TRUE(dir) << dir.Error();

    auto md = dir->Metadata();
    ASSERT_TRUE(md) << md.Error();
    EXPECT_TRUE(md->Type.Dir());
}

TEST_F(DirTest, IterYieldsImmediateChildrenOnly)
{
    auto dir = Dir::Open(Layout->Nested.Path);
    ASSERT_TRUE(dir) << dir.Error();

    auto iter = dir->Iter();
    ASSERT_TRUE(iter) << iter.Error();

    auto names = collectNames(*iter);
    EXPECT_NE(names.find("c.txt"), names.end());
    EXPECT_NE(names.find("loveletter.txt"), names.end());
    EXPECT_NE(names.find("deeper"), names.end());

    // Crucially, `Iter` must NOT descend into `deeper/`.
    EXPECT_EQ(names.find("d.txt"), names.end());
}

TEST_F(DirTest, WalkRecursesIntoSubdirectories)
{
    auto dir = Dir::Open(Layout->Nested.Path);
    ASSERT_TRUE(dir) << dir.Error();

    auto walk = dir->Walk();
    ASSERT_TRUE(walk) << walk.Error();

    auto names = collectNames(*walk);
    EXPECT_NE(names.find("c.txt"), names.end());
    EXPECT_NE(names.find("deeper"), names.end());
    EXPECT_NE(names.find("d.txt"), names.end()) << "Walk must descend into deeper/";
}

TEST_F(DirTest, CloseMakesTheHandleUnusable)
{
    auto dir = Dir::Open(Layout->Root.Path());
    ASSERT_TRUE(dir) << dir.Error();
    ASSERT_TRUE(dir->Alive());

    dir->Close();
    EXPECT_FALSE(dir->Alive());
    EXPECT_FALSE(static_cast<bool>(*dir));

    auto file = dir->OpenFile("a.txt");
    EXPECT_FALSE(file) << "OpenFile on a closed Dir must fail";
}

TEST_F(DirTest, ReleaseTransfersDescriptorOwnership)
{
    auto dir = Dir::Open(Layout->Root.Path());
    ASSERT_TRUE(dir) << dir.Error();

    const auto raw = dir->Descriptor();
    auto fd = VIOLET_MOVE(dir.Value()).Release();

    EXPECT_TRUE(fd.Valid()) << "Release must hand back the live descriptor";
    EXPECT_EQ(fd.Get(), raw);
    EXPECT_FALSE(dir->Alive()) << "the released Dir no longer owns a descriptor";

    // The descriptor is still open and usable; querying it must succeed.
    auto md = Metadata::For(fd.Get());
    ASSERT_TRUE(md) << md.Error();
    EXPECT_TRUE(md->Type.Dir());
}

TEST_F(DirTest, MoveTransfersOwnership)
{
    auto dir = Dir::Open(Layout->Root.Path());
    ASSERT_TRUE(dir) << dir.Error();

    const auto raw = dir->Descriptor();
    Dir moved = VIOLET_MOVE(dir.Value());

    EXPECT_FALSE(dir->Alive()) << "moved-from Dir must no longer be alive";
    EXPECT_TRUE(moved.Alive());
    EXPECT_EQ(moved.Descriptor(), raw);
}

// NOLINTEND(google-build-using-namespace)
