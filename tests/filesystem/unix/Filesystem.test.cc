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

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet;
using namespace violet::filesystem;
using namespace violet::filesystem::testing;

namespace {
struct FilesystemUnixTest: public LayoutFixture { };
} // namespace

TEST_F(FilesystemUnixTest, CopyMissingSourceErrors)
{
    auto result = Copy(Layout->Root.Path().Join("ghost"), Layout->Root.Path().Join("ghost.copy"));
    ASSERT_FALSE(result);

    auto raw = result.Error().RawOSError();
    ASSERT_TRUE(raw);
    EXPECT_EQ(raw.Value(), ENOENT);
}

TEST_F(FilesystemUnixTest, CreateDirectoryOnExistingPathReturnsEEXIST)
{
    auto result = CreateDirectory(Layout->Empty);
    ASSERT_FALSE(result);

    auto raw = result.Error().RawOSError();
    ASSERT_TRUE(raw);
    EXPECT_EQ(raw.Value(), EEXIST);
}

TEST_F(FilesystemUnixTest, ReadDirOnFileReturnsENOTDIR)
{
    auto dirs = ReadDir(Layout->A);
    ASSERT_FALSE(dirs);

    auto raw = dirs.Error().RawOSError();
    ASSERT_TRUE(raw);
    EXPECT_EQ(raw.Value(), ENOTDIR);
}

TEST_F(FilesystemUnixTest, WalkDirSurfacesPermissionErrorAsItem)
{
    // Make `nested/deeper/` unreadable. The walk must still yield `deeper` itself
    // (we read its parent) but the recursion into it must surface as an error
    // rather than being silently swallowed.
    Layout->Nested.Deeper.Path.WithCStr([](CStr path) -> void { ::chmod(path, 0); });

    auto walk = WalkDir(Layout->Nested.Path);
    ASSERT_TRUE(walk) << walk.Error();

    bool sawError = false;
    for (auto ent: *walk) {
        if (ent.Err()) {
            sawError = true;
            break;
        }
    }

    // Restore so the TempDir cleanup can rmdir it.
    Layout->Nested.Deeper.Path.WithCStr([](CStr path) -> void { ::chmod(path, 0700); });
    EXPECT_TRUE(sawError) << "WalkDir must report unreadable subdirs as Err items";
}

// NOLINTEND(google-build-using-namespace)
