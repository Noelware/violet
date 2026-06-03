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
#include <violet/Filesystem.h>
#include <violet/Filesystem/Temporary.h>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::filesystem; // NOLINT(google-build-using-namespace)

TEST(TempDirs, CreationAndAutoDelete)
{
    Path path;
    {
        auto tmpdir = TempBuilder().WithPrefix("gtest-dir-").MkDir();
        ASSERT_TRUE(tmpdir);
        ASSERT_TRUE(filesystem::Exists(tmpdir->Path()));

        path = tmpdir->Path();
    }

    ASSERT_FALSE(filesystem::Exists(path));
}

TEST(TempDirs, ReleasePreventsDeletion)
{
    Path path;
    {
        auto tmpdir = TempBuilder().WithPrefix("gtest-dir-").MkDir();
        ASSERT_TRUE(tmpdir);

        path = tmpdir->Release();
        ASSERT_TRUE(filesystem::Exists(path));
    }

    ASSERT_TRUE(filesystem::Exists(path));
}

TEST(TempDirs, MoveSemantics)
{
    auto tmpdir = TempBuilder().MkDir();
    ASSERT_TRUE(tmpdir);

    Path path = tmpdir->Path();
    auto tmp2 = VIOLET_MOVE(tmpdir.Value());

    ASSERT_TRUE(filesystem::Exists(path));
    EXPECT_TRUE(tmp2.Path().Data() == path.Data());
    EXPECT_TRUE(tmpdir->Path().Empty());
}

TEST(TempFiles, CreationAndAutoDelete)
{
    Path path;
    {
        auto tmpfile = TempBuilder{ }.WithSuffix(".txt").MkFile();
        ASSERT_TRUE(tmpfile) << "failed to build temporary file: " << tmpfile.Error();

        auto file = tmpfile->Path();
        ASSERT_TRUE(file) << "no path was registered";

        path = file.Unwrap();
        auto exists = filesystem::TryExists(path);
        ASSERT_TRUE(exists) << "failed to check existence of path [" << path << "]: " << exists.Error();
        ASSERT_TRUE(exists.Value()) << "path [" << path << "] doesn't exist?!";

        // Try to write to the file
        Array<UInt8, 5> hello({ 'h', 'e', 'l', 'l', 'o' });
        auto writeResult = tmpfile->File().Write(hello);
        ASSERT_TRUE(writeResult) << "failed to write `hello' to path [" << path << "]: " << writeResult.Error();
        EXPECT_EQ(writeResult.Value(), hello.size());
        ASSERT_TRUE(tmpfile->File().Flush()) << "failed to flush changes";
    }

    auto exists = filesystem::TryExists(path);
    ASSERT_TRUE(exists) << "failed to check existence of path [" << path << "]: " << exists.Error();
    ASSERT_FALSE(exists.Value()) << "file persisted out of scope?!";
}

TEST(TempFiles, PersistMovesFileAndTransfersDescriptor)
{
    auto tmpdir = TempBuilder().WithPrefix("gtest-persist-").MkDir();
    ASSERT_TRUE(tmpdir) << "failed to build temporary directory: " << tmpdir.Error();

    const Path dst = tmpdir->Path().Join("persisted.txt");
    Path srcPath;
    {
        auto tmpfile = TempBuilder{ }.WithSuffix(".txt").MkFile();
        ASSERT_TRUE(tmpfile) << "failed to build temporary file: " << tmpfile.Error();

        ASSERT_TRUE(tmpfile->Path()) << "no path was registered";
        srcPath = tmpfile->Path().Value();

        Array<UInt8, 5> hello({ 'h', 'e', 'l', 'l', 'o' });
        auto wrote = tmpfile->File().Write(hello);
        ASSERT_TRUE(wrote) << "failed to write `hello': " << wrote.Error();
        ASSERT_TRUE(tmpfile->File().Flush());

        auto persisted = tmpfile->Persist(dst);
        ASSERT_TRUE(persisted) << "failed to persist: " << persisted.Error();
        EXPECT_FALSE(tmpfile->Path()) << "TempFile still tracks a path after Persist";

        // If Persist did not transfer ownership of the descriptor, the moved-from
        // TempFile would still hold a valid fd aliased with `persisted`. Writing
        // through it would either succeed (corrupting `persisted`) or, after the
        // returned File is destroyed, would double-close.
        Array<UInt8, 3> bad({ 'b', 'a', 'd' });
        auto leak = tmpfile->File().Write(bad);
        EXPECT_FALSE(leak) << "TempFile retained the descriptor after Persist (double-close hazard)";
    }

    auto srcExists = filesystem::TryExists(srcPath);
    ASSERT_TRUE(srcExists) << "failed to stat source path [" << srcPath << "]: " << srcExists.Error();
    EXPECT_FALSE(srcExists.Value()) << "source path still exists after Persist";

    auto dstExists = filesystem::TryExists(dst);
    ASSERT_TRUE(dstExists) << "failed to stat destination [" << dst << "]: " << dstExists.Error();
    ASSERT_TRUE(dstExists.Value()) << "destination [" << dst << "] does not exist";

    auto reader = OpenOptions().Read().Open(dst);
    ASSERT_TRUE(reader) << "failed to open persisted file: " << reader.Error();

    Array<UInt8, 5> buf{ };
    auto num = reader->Read(buf);
    ASSERT_TRUE(num) << "failed to read back: " << num.Error();
    EXPECT_EQ(num.Value(), buf.size());
    EXPECT_EQ(::memcmp(buf.data(), "hello", 5), 0);
}

TEST(TempFiles, PersistFailsOnAlreadyPersistedHandle)
{
    auto tmpdir = TempBuilder().WithPrefix("gtest-persist2-").MkDir();
    ASSERT_TRUE(tmpdir);

    const Path first = tmpdir->Path().Join("first.txt");
    const Path second = tmpdir->Path().Join("second.txt");

    auto tmpfile = TempBuilder{ }.MkFile();
    ASSERT_TRUE(tmpfile);

    ASSERT_TRUE(tmpfile->Persist(first));
    auto again = tmpfile->Persist(second);
    EXPECT_FALSE(again) << "second Persist should have failed";
}
