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

#if !VIOLET_PLATFORM(UNIX)
#error "must be built on Linux/macOS"
#endif

#include <violet/Filesystem/Metadata.h>

#include <fcntl.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::filesystem::testing;
using namespace violet::filesystem;
using namespace violet::io;
using namespace violet;

namespace {

struct MetadataUnixTest: public LayoutFixture {
protected:
    static auto openDirFd(PathRef path) -> io::Result<io::FileDescriptor>
    {
        const Int32 fd
            = path.WithCStr([&](CStr path) -> Int32 { return ::open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC); });

        if (fd < 0) {
            return Err(io::Error::OSError());
        }

        return io::FileDescriptor(fd);
    }
};

} // namespace

TEST_F(MetadataUnixTest, MissingPathReturnsENOENT)
{
    auto md = Metadata::For(Layout->Root.Path().Join("does-not-exist"));
    ASSERT_FALSE(md);

    auto raw = md.Error().RawOSError();
    ASSERT_TRUE(raw) << "expected an OS-backed error";
    EXPECT_EQ(*raw, ENOENT);
}

TEST_F(MetadataUnixTest, DanglingSymlinkFollowReturnsENOENT)
{
    auto md = Metadata::For(Layout->Dangling, SymlinkResolution::Follow);
    ASSERT_FALSE(md);

    auto raw = md.Error().RawOSError();
    ASSERT_TRUE(raw);
    EXPECT_EQ(raw.Value(), ENOENT);
}

TEST_F(MetadataUnixTest, HardlinkSharesDeviceAndInode)
{
    auto original = Metadata::For(Layout->A);
    auto linked = Metadata::For(Layout->HardlinkToA);
    ASSERT_TRUE(original) << "failed to retrieve metadata for [" << Layout->A << "]: " << original.Error();
    ASSERT_TRUE(linked) << "failed to retrieve metadata for [" << Layout->HardlinkToA << "]: " << linked.Error();

    EXPECT_EQ(original->Device, linked->Device);
    EXPECT_EQ(original->Inode, linked->Inode);
    EXPECT_GE(original->HardLinks, 2U) << "expected `link` to have raised the count";
    EXPECT_EQ(original->HardLinks, linked->HardLinks);
    EXPECT_FALSE(original->Rdev);
}

TEST_F(MetadataUnixTest, OwnershipMatchesProcess)
{
    auto md = Metadata::For(Layout->A);
    ASSERT_TRUE(md) << "failed to retrieve metadata for [" << Layout->A << "]: " << md.Error();

    EXPECT_EQ(md->UserID, ::geteuid());
    EXPECT_EQ(md->GroupID, ::getegid());
}

TEST_F(MetadataUnixTest, ChmodReflectsInModeAccessors)
{
    const Int32 ret = Layout->A.WithCStr([&](CStr path) -> Int32 { return ::chmod(path, 0640); });
    ASSERT_EQ(ret, 0) << "`chmod(0640)` on path [" << Layout->A << "] failed: " << io::Error::FromOSError(ret);

    auto md = Metadata::For(Layout->A);
    ASSERT_TRUE(md) << "failed to retrieve metadata for [" << Layout->A << "]: " << md.Error();

    const auto mode = md->Permissions.Mode();
    EXPECT_TRUE(mode.OwnerCanRead());
    EXPECT_TRUE(mode.OwnerCanWrite());
    EXPECT_FALSE(mode.OwnerCanExecute());

    EXPECT_TRUE(mode.GroupCanRead());
    EXPECT_FALSE(mode.GroupCanWrite());
    EXPECT_FALSE(mode.GroupCanExecute());

    EXPECT_FALSE(mode.OtherCanRead());
    EXPECT_FALSE(mode.OtherCanWrite());
    EXPECT_FALSE(mode.OtherCanExecute());

    EXPECT_FALSE(mode.HasSetUID());
    EXPECT_FALSE(mode.HasSetGID());
    EXPECT_FALSE(mode.Sticky());
}

TEST_F(MetadataUnixTest, FromDirFdResolvesRelativeToDescriptor)
{
    auto dirfd = openDirFd(Layout->Nested.Path);
    ASSERT_TRUE(dirfd) << dirfd.Error();

    auto viaDirfd = Metadata::For(dirfd->Get(), "c.txt");
    auto viaPath = Metadata::For(Layout->Nested.C);
    ASSERT_TRUE(viaDirfd) << viaDirfd.Error();
    ASSERT_TRUE(viaPath);

    EXPECT_TRUE(viaDirfd->Type.File());
    EXPECT_EQ(viaDirfd->Size, 3U);
    EXPECT_EQ(viaDirfd->Inode, viaPath->Inode);
    EXPECT_EQ(viaDirfd->Device, viaPath->Device);
}

TEST_F(MetadataUnixTest, FromDirFdHonoursNoFollow)
{
    auto dirfd = openDirFd(Layout->Root.Path());
    ASSERT_TRUE(dirfd) << dirfd.Error();

    auto follow = Metadata::For(dirfd->Get(), "link-to-a", SymlinkResolution::Follow);
    ASSERT_TRUE(follow) << follow.Error();
    EXPECT_TRUE(follow->Type.File());
    EXPECT_EQ(follow->Size, 5U);

    auto noFollow = Metadata::For(dirfd->Get(), "link-to-a", SymlinkResolution::NoFollow);
    ASSERT_TRUE(noFollow) << noFollow.Error();
    EXPECT_TRUE(noFollow->Type.Symlink());
}

TEST_F(MetadataUnixTest, FromDirFdWithFileFdReturnsENOTDIR)
{
    auto file = OpenOptions().Read().Open(Layout->A);
    ASSERT_TRUE(file);

    auto md = Metadata::For(file->Descriptor(), "anything");
    ASSERT_FALSE(md);

    auto raw = md.Error().RawOSError();
    ASSERT_TRUE(raw);
    EXPECT_EQ(raw.Value(), ENOTDIR);
}

TEST(MetadataUnix, CharDeviceCarriesRdev)
{
    auto md = Metadata::For("/dev/null");
    ASSERT_TRUE(md) << "failed to assume that `/dev/null` existed: " << md.Error();

    EXPECT_TRUE(md->Type.CharDevice());
    EXPECT_FALSE(md->Type.File());
    EXPECT_FALSE(md->Type.Dir());

    ASSERT_TRUE(md->Rdev) << "/dev/null must report an Rdev";
    EXPECT_TRUE(md->RdevMajor);
    EXPECT_TRUE(md->RdevMinor);
}

// NOLINTEND(google-build-using-namespace)
