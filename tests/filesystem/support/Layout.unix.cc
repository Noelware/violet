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

#include <violet/Violet.h>

#if VIOLET_PLATFORM(UNIX)

#include "tests/filesystem/support/Layout.h"

#include <violet/Filesystem.h>

using violet::filesystem::File;
using violet::filesystem::OpenOptions;
using violet::filesystem::PathRef;
using violet::filesystem::testing::Layout;

using violet::CStr;
using violet::Err;
using violet::Span;
using violet::Str;
using violet::UInt;
using violet::UInt8;

// While I did use Claude to write up a rough draft of the layout, it generated this
// for the love letter. It was originally for someone that I love very much, but
// I guess this will work in the context of programming. So, enjoy.
constexpr static CStr kLoveLetter = "My dearest,\n"
                                    "\n"
                                    "I do not know how I made it this far without you. The world before you was\n"
                                    "a draft: endless commits with no reviewer, syscalls returning into the void,\n"
                                    "and a build that never quite went green. Then you arrived. The function\n"
                                    "signatures aligned, the EBADFs stopped firing, and somewhere a CI runner\n"
                                    "exhaled for the first time in months.\n"
                                    "\n"
                                    "Stay close. Stay always.\n"
                                    "\n"
                                    "~ love\n";

namespace {

auto writeFile(PathRef path, Span<const UInt8> contents) -> violet::io::Result<void>
{
    auto file = File::Open(path, OpenOptions{ }.CreateNew().Write());
    if (file.Err()) {
        return Err(VIOLET_MOVE(file.Error()));
    }

    UInt total = 0;
    while (total < contents.size()) {
        total += VIOLET_TRY(file->Write(contents.subspan(total)));
    }

    return file->Flush();
}

auto symlinkAt(PathRef target, PathRef path) -> violet::io::Result<void>
{
    return target.WithCStr([&](CStr target) -> violet::io::Result<void> {
        return path.WithCStr([&](CStr path) -> violet::io::Result<void> {
            if (::symlink(target, path) == -1) {
                return Err(violet::io::Error::OSError());
            }

            return { };
        });
    });
}

auto hardlinkAt(PathRef target, PathRef path) -> violet::io::Result<void>
{
    return target.WithCStr([&](CStr target) -> violet::io::Result<void> {
        return path.WithCStr([&](CStr path) -> violet::io::Result<void> {
            if (::link(target, path) == -1) {
                return Err(violet::io::Error::OSError());
            }

            return { };
        });
    });
}

template<UInt N>
inline auto toBytes(const char (&str)[N]) -> Span<const UInt8>
{
    return { reinterpret_cast<const UInt8*>(str), N - 1 };
}

inline auto toBytes(Str str) -> Span<const UInt8>
{
    return { reinterpret_cast<const UInt8*>(str.data()), str.size() };
}

} // namespace

auto Layout::New() -> io::Result<Layout>
{
    auto tmpdir = VIOLET_TRY(TempBuilder{ }.WithPrefix("violet-fs-").MkDir());
    const auto& root = tmpdir.Path();

    const Path atxt = root.Join("a.txt");
    const Path bbin = root.Join("b.bin");
    const Path empty = root.Join("empty");
    const Path nested = root.Join("nested");
    const Path nestedC = nested.Join("c.txt");
    const Path nestedLoveLetter = nested.Join("loveletter.txt");
    const Path nestedDeeper = nested.Join("deeper");
    const Path nestedDeeperD = nestedDeeper.Join("d.txt");
    const Path linkToA = root.Join("link-to-a");
    const Path dangling = root.Join("dangling");
    const Path hardlinkToA = root.Join("hardlink-to-a");

    VIOLET_TRY_VOID(CreateDirectory(empty));
    VIOLET_TRY_VOID(CreateDirectory(nested));
    VIOLET_TRY_VOID(CreateDirectory(nestedDeeper));

    VIOLET_TRY_VOID(writeFile(atxt, toBytes("hello")));

    Array<UInt8, 1024> filler{ };
    filler.fill(static_cast<UInt8>(0xAB));
    VIOLET_TRY_VOID(writeFile(bbin, { filler.data(), filler.size() }));

    VIOLET_TRY_VOID(writeFile(nestedC, toBytes("cee")));
    VIOLET_TRY_VOID(writeFile(nestedLoveLetter, toBytes(kLoveLetter)));
    VIOLET_TRY_VOID(writeFile(nestedDeeperD, Span<const UInt8>{ }));
    VIOLET_TRY_VOID(symlinkAt("a.txt", linkToA));
    VIOLET_TRY_VOID(symlinkAt("does-not-exist", dangling));
    VIOLET_TRY_VOID(hardlinkAt(atxt, hardlinkToA));

    return Ok<Layout>({
        // clang-format off
        .Root = VIOLET_MOVE(tmpdir),
        .A = atxt,
        .B = bbin,
        .Empty = empty,
        .Nested = {
            .Path = nested,
            .C = nestedC,
            .LoveLetter = nestedLoveLetter,
            .Deeper = {
                .Path = nestedDeeper,
                .D = nestedDeeperD,
            },
        },
        .LinkToA = linkToA,
        .Dangling = dangling,
        .HardlinkToA = hardlinkToA,
        // clang-format on
    });
}

#endif
