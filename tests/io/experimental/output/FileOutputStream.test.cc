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
#include <violet/IO/Experimental/Output/FileOutputStream.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet::io::experimental;
using namespace violet::filesystem;
using namespace violet;
// NOLINTEND(google-build-using-namespace)

TEST(FileOutputStream, WriteAndFlush)
{
    auto tempdir = TempBuilder{}.MkDir();
    ASSERT_TRUE(tempdir) << "failed to create temporary directory: " << tempdir.Error();

    auto dirExists = filesystem::TryExists(tempdir->Path());
    ASSERT_TRUE(dirExists) << "tempfile [" << tempdir->Path() << "] wasn't created successfully: " << dirExists.Error();
    ASSERT_TRUE(dirExists.Value()) << "tempfile [" << tempdir->Path() << "] wasn't created successfully";

    auto path = tempdir->Path().Join("test.txt");
    auto fos = FileOutputStream::Open(path);
    ASSERT_TRUE(fos) << "failed to create tempfile [" << path << "]: " << fos.Error();

    auto pathExists = filesystem::TryExists(path);
    ASSERT_TRUE(pathExists) << "tempfile [" << path << "] wasn't created successfully: " << pathExists.Value();
    ASSERT_TRUE(pathExists.Value()) << "tempfile [" << path << "] wasn't created successfully";

    Span<const UInt8> data(reinterpret_cast<const UInt8*>("filedata"), 8);
    auto written = fos->Write(data);
    ASSERT_TRUE(written) << "failed to write: " << written.Error();
    EXPECT_EQ(written.Value(), 8);

    ASSERT_TRUE(fos->Flush());

    // Verify the content
    auto file = File::Open(path, OpenOptions{}.Read());
    ASSERT_TRUE(file) << "failed to open file [" << path << "]: " << file.Error();

    Array<UInt8, 8> buf;
    auto contents = file->Read(buf);
    ASSERT_TRUE(contents) << "failed to read file contents: " << contents.Error();

    EXPECT_EQ(contents.Value(), 8);
    String str(buf.begin(), buf.end());
    EXPECT_EQ(str, "filedata");
}
