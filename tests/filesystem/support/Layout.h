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

#pragma once

#include <gtest/gtest.h>
#include <violet/Container/Optional.h>
#include <violet/Filesystem/Path.h>
#include <violet/Filesystem/Temporary.h>

namespace violet::filesystem::testing {

/// The canonical scratch tree that is created when filesystem-related tests are being ran.
///
/// ## Scratch Tree
/// ```txt
/// <root>/
///   ├── a.txt                (regular, 5 bytes: "hello")
///   ├── b.bin                (regular, 1024 bytes of `0xAB`)
///   ├── empty/               (directory, empty)
///   ├── nested/
///   │     ├── c.txt          (regular, 3 bytes: "cee")
///   │     ├── loveletter.txt (regular, a love letter~)
///   │     └── deeper/
///   │           └── d.txt    (regular, 0 bytes)
///   ├── link-to-a            (symlink ~> a.txt)
///   ├── dangling             (symlink ~> does-not-exist)
///   └── hardlink-to-a        (hardlink to a.txt)
/// ```
struct Layout final {
    /// `<root>`
    TempDir Root;

    /// `a.txt`
    Path A;

    /// `b.txt`
    Path B;

    /// `empty/`
    Path Empty;

    struct {
        /// `nested/`
        struct Path Path;

        /// `nested/c.txt`
        struct Path C;

        /// `nested/loveletter.txt`
        struct Path LoveLetter;

        struct {
            /// `nested/deeper/`
            struct Path Path;

            /// `nested/deeper/d.txt`
            struct Path D;
        } Deeper;
    } Nested;

    /// `link-to-a`
    Path LinkToA;

    /// `dangling`
    Path Dangling;

    /// `hardlink-to-a`
    Path HardlinkToA;

    [[nodiscard]] static auto New() -> io::Result<Layout>;
};

struct LayoutFixture: public ::testing::Test {
protected:
    Optional<Layout> Layout;

    void SetUp() override
    {
        auto result = Layout::New();
        ASSERT_TRUE(result) << "failed to create filesystem layout: " << result.Error();

        (void)Layout.Replace(VIOLET_MOVE(result.Value()));
    }
};

} // namespace violet::filesystem::testing
