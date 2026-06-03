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
#error "must be compiled on a Unix-based system (Linux, macOS)"
#endif

#include <violet/Filesystem/Metadata.h>
#include <violet/IO/Read.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet;
using namespace violet::filesystem;
using namespace violet::filesystem::testing;

namespace {
struct FileUnixTest: public LayoutFixture { };
} // namespace

TEST_F(FileUnixTest, OpenMissingPathReturnsENOENT)
{
    auto file = OpenOptions{ }.Read().Open(Layout->Root.Path().Join("does-not-exist"));
    ASSERT_FALSE(file);

    auto raw = file.Error().RawOSError();
    ASSERT_TRUE(raw);
    EXPECT_EQ(*raw, ENOENT);
}

TEST_F(FileUnixTest, ModeIsAppliedOnCreate)
{
    const Path fresh = Layout->Root.Path().Join("moded.bin");
    auto opened = OpenOptions{ }.CreateNew().Write().Mode(0640).Open(fresh);
    ASSERT_TRUE(opened) << "unable to open file [" << fresh << "]: " << opened.Error();
    ASSERT_TRUE(opened->Close());

    auto md = Metadata::For(fresh);
    ASSERT_TRUE(md);

    const auto mode = md->Permissions.Mode();
    EXPECT_TRUE(mode.OwnerCanRead());
    EXPECT_TRUE(mode.OwnerCanWrite());
    EXPECT_TRUE(mode.GroupCanRead());
    EXPECT_FALSE(mode.GroupCanWrite());
    EXPECT_FALSE(mode.OtherCanRead());
}

TEST_F(FileUnixTest, ExclusiveLockBlocksAnotherHolder)
{
    auto holder = OpenOptions{ }.Read().Write().Open(Layout->A);
    ASSERT_TRUE(holder) << "unable to open file [" << Layout->A << "]: " << holder.Error();
    ASSERT_TRUE(holder->Lock()) << "initial Lock() must succeed";

    auto observed = holder->Locked();
    ASSERT_TRUE(observed) << observed.Error();
    EXPECT_TRUE(*observed);

    auto challenger = OpenOptions{ }.Read().Write().Open(Layout->A);
    ASSERT_TRUE(challenger);

    auto challengerLocked = challenger->Locked();
    ASSERT_TRUE(challengerLocked);
    EXPECT_TRUE(*challengerLocked) << "a second handle must observe the lock";
    ASSERT_TRUE(holder->Unlock());
}

TEST_F(FileUnixTest, ScopedLockReleasesOnDestruction)
{
    auto file = OpenOptions{ }.Read().Write().Open(Layout->A);
    ASSERT_TRUE(file) << "unable to open file [" << Layout->A << "]: " << file.Error();
    {
        auto guard = file->MkScopedLock();
        ASSERT_TRUE(guard) << guard.Error();

        auto locked = file->Locked();
        ASSERT_TRUE(locked);
        EXPECT_TRUE(*locked);
    }

    auto locked = file->Locked();
    ASSERT_TRUE(locked);
    EXPECT_FALSE(locked.Value()) << "Lock must be released once the guard is destroyed";
}

TEST_F(FileUnixTest, SharedLockAllowsMultipleHolders)
{
    auto first = OpenOptions{ }.Read().Open(Layout->A);
    auto second = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    auto firstLock = first->MkSharedScopedLock();
    ASSERT_TRUE(firstLock) << firstLock.Error();

    auto secondLock = second->MkSharedScopedLock();
    EXPECT_TRUE(secondLock) << "shared locks must be reentrant across handles: " << secondLock.Error();
}

// NOLINTEND(google-build-using-namespace)
