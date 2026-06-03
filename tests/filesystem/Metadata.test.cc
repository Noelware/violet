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

#include <violet/Filesystem/Metadata.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::filesystem::testing;
using namespace violet::filesystem;
using namespace violet;

namespace {
struct MetadataTest: public LayoutFixture { };
} // namespace

TEST_F(MetadataTest, RegularFileReportsTypeAndSize)
{
    auto mt = Metadata::For(Layout->A);
    ASSERT_TRUE(mt) << "failed to retrieve metadata for [" << Layout->A << "]: " << mt.Error();

    EXPECT_TRUE(mt->Type.File());
    EXPECT_FALSE(mt->Type.Dir());
    EXPECT_FALSE(mt->Type.Symlink());
    EXPECT_EQ(mt->Size, 5U);
    EXPECT_FALSE(mt->Rdev.HasValue()) << "regular files must not carry an Rdev";
}

TEST_F(MetadataTest, DirectoryReportsDirType)
{
    auto md = Metadata::For(Layout->Empty);
    ASSERT_TRUE(md) << "failed to retrieve metadata for [" << Layout->Empty << "]: " << md.Error();

    EXPECT_TRUE(md->Type.Dir());
    EXPECT_FALSE(md->Type.File());
    EXPECT_FALSE(md->Type.Symlink());
}

TEST_F(MetadataTest, EmptyFileReportsZeroSize)
{
    auto md = Metadata::For(Layout->Nested.Deeper.D);
    ASSERT_TRUE(md) << "failed to retrieve metadata for [" << Layout->Nested.Deeper.D << "]: " << md.Error();

    EXPECT_TRUE(md->Type.File());
    EXPECT_EQ(md->Size, 0U);
}

TEST_F(MetadataTest, LargeFileReportsExpectedSize)
{
    auto md = Metadata::For(Layout->B);
    ASSERT_TRUE(md) << "failed to retrieve metadata for [" << Layout->B << "]: " << md.Error();

    EXPECT_TRUE(md->Type.File());
    EXPECT_EQ(md->Size, 1024U);
}

TEST_F(MetadataTest, SymlinkFollowResolvesToTarget)
{
    auto md = Metadata::For(Layout->LinkToA, SymlinkResolution::Follow);
    ASSERT_TRUE(md) << "failed to retrieve metadata for [" << Layout->LinkToA << "]: " << md.Error();

    EXPECT_TRUE(md->Type.File());
    EXPECT_FALSE(md->Type.Symlink());
    EXPECT_EQ(md->Size, 5U);
}

TEST_F(MetadataTest, SymlinkNoFollowReportsLinkItself)
{
    auto md = Metadata::For(Layout->LinkToA, SymlinkResolution::NoFollow);
    ASSERT_TRUE(md) << "failed to retrieve metadata for [" << Layout->LinkToA << "]: " << md.Error();

    EXPECT_TRUE(md->Type.Symlink());
    EXPECT_FALSE(md->Type.File());
    EXPECT_FALSE(md->Type.Dir());
}

TEST_F(MetadataTest, DanglingSymlinkNoFollowSucceeds)
{
    auto md = Metadata::For(Layout->Dangling, SymlinkResolution::NoFollow);
    ASSERT_TRUE(md) << "NoFollow must not error on a dangling target: " << md.Error();
    EXPECT_TRUE(md->Type.Symlink());
}

TEST_F(MetadataTest, FromFileDescriptorMatchesPathQuery)
{
    auto file = OpenOptions().Read().Open(Layout->A);
    ASSERT_TRUE(file) << "failed to open file [" << Layout->A << "]: " << file.Error();

    auto viaPath = Metadata::For(Layout->A);
    auto viaFd = Metadata::For(file->Descriptor());
    ASSERT_TRUE(viaPath) << viaPath.Error();
    ASSERT_TRUE(viaFd) << viaFd.Error();

    EXPECT_TRUE(viaFd->Type.File());
    EXPECT_EQ(viaFd->Size, viaPath->Size);

#if VIOLET_PLATFORM(UNIX)
    EXPECT_EQ(viaFd->Inode, viaPath->Inode);
    EXPECT_EQ(viaFd->Device, viaPath->Device);
#endif
}

// NOLINTEND(google-build-using-namespace)
