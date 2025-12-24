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

// TEST(TempFiles, CreationAndAutoDelete)
// {
//     Path path;
//     {
//         // auto tmpfile = TempBuilder().WithPrefix("gtest-file-").MkFile();
//         // ASSERT_TRUE(tmpfile);

//         // std::cout << tmpfile->Path().Value() << '\n';

//         // path = *tmpfile->Path().Value();
//         // ASSERT_TRUE(filesystem::Exists(path));

//         // Array<UInt8, 5> hello({ 'h', 'e', 'l', 'l', '0' });
//         // ASSERT_TRUE(tmpfile->File().Write(hello));
//     }

//     // ASSERT_FALSE(filesystem::Exists(path));
// }

// TEST(TempFiles, PersistFile)
// {
//     auto tmpfile = TempBuilder().MkFile();
//     ASSERT_TRUE(tmpfile);

//     Array<UInt8, 5> hello({ 'h', 'e', 'l', 'l', '0' });
//     ASSERT_TRUE(tmpfile->File().Write(hello));

//     Path dst = "persisted.txt";
//     auto res = VIOLET_MOVE(tmpfile.Value()).Persist(dst);
//     ASSERT_TRUE(res);
//     ASSERT_TRUE(filesystem::Exists(dst));
//     ASSERT_TRUE(filesystem::RemoveFile(dst));
// }
