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

#include <violet/Filesystem/Path.h>
#include <violet/IO/Error.h>
#include <violet/Violet.h>

namespace violet::sys {

/// Configuration options for the [`violet::sys::Which`] function.
struct WhichConfig final {
    /// The `$PATH` environment variable to find binaries in.
    Str PathEnv = "PATH";

    /// If specified, this will use the path to lookup binaries as well in `$PATH` (or
    /// what [`WhichConfig::PathEnv`] is set as)
    Optional<filesystem::PathRef> WorkingDirectory = Nothing;
};

/// Finds a execute path from the system's `$PATH` and returns a canonicalized, absolute path.
///
/// ## Example
/// ```cpp
/// #include <violet/System/Which.h>
///
/// if (auto clang = violet::sys::Which("clang"); clang.Ok()) {
///     std::cout << "resolved `clang` binary: " << clang.Value() << '\n';
/// }
/// ```
///
/// @param command the command's name
/// @param config configuration options
auto Which(Str command, WhichConfig config = {}) -> io::Result<filesystem::Path>;

} // namespace violet::sys
