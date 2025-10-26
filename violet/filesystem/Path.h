// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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
//
//! # 🌺💜 `violet/filesystem/Path.h`

#pragma once

#include "violet/container/Optional.h"
#include "violet/io/Descriptor.h"
#include "violet/io/Error.h"
#include "violet/support/StringRef.h"
#include "violet/violet.h"

#ifdef VIOLET_WINDOWS
#    include <windows.h>
#endif

namespace Noelware::Violet::Filesystem {

#ifdef VIOLET_WINDOWS
constexpr static auto PathSeparator = '\\';
#else
constexpr static auto PathSeparator = '/';
#endif

struct PathRef;

/// A mutable, owned path akin to [`std::string`].
struct Path final {
    Path() = default;
    VIOLET_IMPLICIT Path(CStr path) noexcept;
    VIOLET_IMPLICIT Path(String path) noexcept;
    VIOLET_IMPLICIT Path(StringRef path) noexcept;
    VIOLET_EXPLICIT Path(PathRef&& path) noexcept;
    VIOLET_EXPLICIT Path(const PathRef& path) noexcept;

    static auto FromDescriptor(IO::Descriptor descriptor) -> IO::Result<PathRef>;

    auto Root() const noexcept -> bool;
    auto Empty() const noexcept -> bool;
    auto Absolute() const noexcept -> bool;
    auto Relative() const noexcept -> bool;

    auto Stem() const noexcept -> String;
    auto Filename() const noexcept -> String;
    auto Extension() const noexcept -> Optional<String>;
    auto Parent() const noexcept -> Optional<Path>;
    auto Join(StringRef piece) const noexcept -> Path;
    void Canonicalize() noexcept;

    auto WithFilename(StringRef piece) noexcept -> Path;
    auto WithExtension(StringRef piece) noexcept -> Path;

    VIOLET_EXPLICIT operator bool() const noexcept;
    VIOLET_EXPLICIT operator String() const noexcept;
    VIOLET_EXPLICIT operator PathRef() const noexcept;
    VIOLET_EXPLICIT operator StringRef() const noexcept;

    VIOLET_IMPL_EQUALITY_SINGLE(Path, lhs, rhs, { return lhs.n_path == rhs.n_path; });
    VIOLET_IMPL_EQUALITY(const Path&, CStr, lhs, rhs, { return lhs.n_path == rhs; });
    VIOLET_IMPL_EQUALITY(const Path&, const Str&, lhs, rhs, { return lhs.n_path == rhs; });
    VIOLET_IMPL_EQUALITY(const Path&, const String&, lhs, rhs, { return lhs.n_path == rhs; });
    VIOLET_IMPL_EQUALITY(const Path&, const StringRef&, lhs, rhs, { return lhs.n_path == rhs; });

    VIOLET_OSTREAM_IMPL(const Path&)
    {
        return os << self.n_path;
    }

private:
    friend struct PathRef;

    String n_path;
};

/// A immutable, non-owning path akin to [`StringRef`].
struct PathRef final {
    PathRef() = default;
    VIOLET_IMPLICIT PathRef(CStr path) noexcept;
    VIOLET_IMPLICIT PathRef(const String& path) noexcept;
    VIOLET_EXPLICIT PathRef(const Path& path) noexcept;
    VIOLET_IMPLICIT PathRef(StringRef path) noexcept;
    VIOLET_EXPLICIT PathRef(Path&& path) noexcept;

    static auto FromDescriptor(IO::Descriptor descriptor) -> IO::Result<PathRef>;

    auto Root() const noexcept -> bool;
    auto Empty() const noexcept -> bool;
    auto Absolute() const noexcept -> bool;
    auto Relative() const noexcept -> bool;

    auto Stem() const noexcept -> String;
    auto Filename() const noexcept -> String;
    auto Extension() const noexcept -> Optional<String>;
    auto Parent() const noexcept -> Optional<Path>;
    auto Join(StringRef piece) const noexcept -> Path;
    auto Canonicalize() noexcept -> Path;

    VIOLET_EXPLICIT operator bool() const noexcept;
    VIOLET_EXPLICIT operator Path() const noexcept;
    VIOLET_EXPLICIT operator StringRef() const noexcept;

    VIOLET_IMPL_EQUALITY_SINGLE(PathRef, lhs, rhs, { return lhs.n_path == rhs.n_path; });
    VIOLET_IMPL_EQUALITY(const PathRef&, CStr, lhs, rhs, { return lhs.n_path == rhs; });
    VIOLET_IMPL_EQUALITY(const PathRef&, const Str&, lhs, rhs, { return lhs.n_path == rhs; });
    VIOLET_IMPL_EQUALITY(const PathRef&, const String&, lhs, rhs, { return lhs.n_path == rhs; });
    VIOLET_IMPL_EQUALITY(const PathRef&, const StringRef&, lhs, rhs, { return lhs.n_path == rhs; });

    VIOLET_OSTREAM_IMPL(const PathRef&)
    {
        return os << self.n_path;
    }

private:
    friend struct Path;

    StringRef n_path;
};

} // namespace Noelware::Violet::Filesystem
