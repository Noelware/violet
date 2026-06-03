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
#include <violet/IO/Read.h>

using namespace violet;
using namespace violet::filesystem;
using namespace violet::filesystem::testing;

namespace {
struct FileTest: public LayoutFixture { };
} // namespace

TEST_F(FileTest, DefaultConstructIsInvalid)
{
    File file;
    EXPECT_FALSE(file.Valid());
    EXPECT_FALSE(static_cast<bool>(file));
}

TEST_F(FileTest, OpenReturnsValidFile)
{
    auto file = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(file) << "failed to open file [" << Layout->A << "]: " << file.Error();
    EXPECT_TRUE(file->Valid());
    EXPECT_NE(file->Descriptor(), -1);
}

TEST_F(FileTest, CloseInvalidatesTheHandle)
{
    auto opened = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(opened) << "failed to open file [" << Layout->A << "]: " << opened.Error();

    auto& file = opened.Value();
    EXPECT_TRUE(file.Valid());

    auto closed = file.Close();
    ASSERT_TRUE(closed) << "failed to close file: " << closed.Error();
    EXPECT_FALSE(file.Valid());
}

TEST_F(FileTest, CloseIsIdempotent)
{
    auto opened = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(opened) << "failed to open file [" << Layout->A << "]: " << opened.Error();
    ASSERT_TRUE(opened->Close()) << "failed to close file";

    auto again = opened->Close();
    EXPECT_TRUE(again) << "second Close() must be a no-op success: " << again.Error();
    EXPECT_FALSE(opened->Valid());
}

TEST_F(FileTest, MoveTransfersOwnership)
{
    auto opened = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(opened) << "failed to open file [" << Layout->A << "]: " << opened.Error();

    auto& source = opened.Value();
    const auto fd = source.Descriptor();
    ASSERT_NE(fd, -1);

    File destination = VIOLET_MOVE(source);
    EXPECT_FALSE(source.Valid()) << "moved-from File must report invalid";
    EXPECT_TRUE(destination.Valid());
    EXPECT_EQ(destination.Descriptor(), fd);
}

TEST_F(FileTest, OpenWithReadReadsContent)
{
    auto opened = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(opened) << "failed to open file [" << Layout->A << "]: " << opened.Error();

    Array<UInt8, 5> buf{ };
    auto bytes = opened->Read(buf);
    ASSERT_TRUE(bytes) << "failed to read file: " << bytes.Error();
    EXPECT_EQ(*bytes, buf.size());
    EXPECT_EQ(::memcmp(buf.data(), "hello", 5), 0);
}

TEST_F(FileTest, OpenWriteOnlyAllowsWritesButNotReads)
{
    auto opened = OpenOptions{ }.Write().Open(Layout->A);
    ASSERT_TRUE(opened) << opened.Error();

    Array<UInt8, 3> payload({ 'r', 'e', 'd' });
    auto wrote = opened->Write(payload);
    ASSERT_TRUE(wrote) << wrote.Error();
    EXPECT_EQ(wrote.Value(), payload.size());

    Array<UInt8, 8> scratch{ };
    auto reread = opened->Read(scratch);
    EXPECT_FALSE(reread) << "reading from a write-only fd must surface EBADF";
}

TEST_F(FileTest, CreateNewWritesFreshFile)
{
    const Path fresh = Layout->Root.Path().Join("fresh.bin");
    {
        auto opened = OpenOptions{ }.CreateNew().Write().Open(fresh);
        ASSERT_TRUE(opened) << opened.Error();

        Array<UInt8, 4> bytes({ 0xDE, 0xAD, 0xBE, 0xEF });
        ASSERT_TRUE(opened->Write(bytes));
        ASSERT_TRUE(opened->Flush());
    }

    auto reopened = OpenOptions{ }.Read().Open(fresh);
    ASSERT_TRUE(reopened);

    auto bytes = io::ReadToBytes(*reopened);
    ASSERT_TRUE(bytes) << bytes.Error();
    ASSERT_EQ(bytes->size(), 4U);
    EXPECT_EQ((*bytes)[0], 0xDE);
    EXPECT_EQ((*bytes)[3], 0xEF);
}

TEST_F(FileTest, TruncateClearsExistingContent)
{
    auto opened = OpenOptions{ }.Write().Truncate().Open(Layout->A);
    ASSERT_TRUE(opened) << "failed to open file [" << Layout->A << "]: " << opened.Error();
    ASSERT_TRUE(opened->Close());

    auto md = Metadata::For(Layout->A);
    ASSERT_TRUE(md) << "failed to grab metadata: " << md.Error();
    EXPECT_EQ(md->Size, 0U) << "Truncate must zero the file";
}

TEST_F(FileTest, AppendWritesAtEnd)
{
    auto opened = OpenOptions{ }.Write().Append().Open(Layout->A);
    ASSERT_TRUE(opened) << opened.Error();

    Array<UInt8, 6> suffix({ ' ', 'w', 'o', 'r', 'l', 'd' });
    ASSERT_TRUE(opened->Write(suffix));
    ASSERT_TRUE(opened->Flush());
    ASSERT_TRUE(opened->Close());

    auto reopened = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(reopened);

    auto bytes = io::ReadToBytes(*reopened);
    ASSERT_TRUE(bytes);
    ASSERT_EQ(bytes->size(), 11U) << "Append must preserve the existing 5 bytes";
    EXPECT_EQ(std::memcmp(bytes->data(), "hello world", 11), 0);
}

TEST_F(FileTest, CloneProducesAnIndependentDescriptor)
{
    auto opened = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(opened) << "failed to open file [" << Layout->A << "]: " << opened.Error();

    auto cloned = opened->Clone(false);
    ASSERT_TRUE(cloned) << "failed to clone file: " << cloned.Error();
    ASSERT_TRUE(cloned->Valid());

    EXPECT_NE(opened->Descriptor(), cloned->Descriptor()) << "`Clone' must dup the descriptor";

    // Closing the original must not break the clone.
    ASSERT_TRUE(opened->Close());
    EXPECT_TRUE(cloned->Valid());

    Array<UInt8, 5> buf{ };
    auto bytes = cloned->Read(buf);
    ASSERT_TRUE(bytes) << bytes.Error();
    EXPECT_EQ(*bytes, buf.size());
}

TEST_F(FileTest, MetadataMatchesPathBasedQuery)
{
    auto opened = OpenOptions{ }.Read().Open(Layout->A);
    ASSERT_TRUE(opened) << "failed to open file [" << Layout->A << "]: " << opened.Error();

    auto viaFile = opened->Metadata();
    auto viaPath = Metadata::For(Layout->A);
    ASSERT_TRUE(viaFile) << viaFile.Error();
    ASSERT_TRUE(viaPath);

    EXPECT_EQ(viaFile->Size, viaPath->Size);
#if VIOLET_PLATFORM(UNIX)
    EXPECT_EQ(viaFile->Inode, viaPath->Inode);
    EXPECT_EQ(viaFile->Device, viaPath->Device);
#endif
}
