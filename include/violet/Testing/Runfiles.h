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
//
//! # 🌺💜 `violet/Testing/Runfiles.h`
//! This header is a bridge for accessing files from Bazel, CMake, or Meson depending
//! on what is being invoked.
//!
//! * Bazel: Uses Runfiles (requires `-DBAZEL`)
//! * CMake: Accesses "runfiles" from `$CMAKE_BINARY_DIR` (requires `-DCMAKE`)
//! * Meson: Accesses "runfiles" from `$MESON_BUILD_DIR` (requires `-DMESON`)

#pragma once

#include <violet/Violet.h>

namespace violet {
template<typename T>
struct Optional;
}

namespace violet::testing::runfiles {
namespace detail {

#if VIOLET_BUILDSYSTEM(BAZEL)
    constexpr static bool methodCanBeMarkedNoexcept = false;
#else
    constexpr static bool methodCanBeMarkedNoexcept = true;
#endif

} // namespace detail

/// Initializes the runfiles system, this is only useful when Bazel is the
/// build system for running tests.
VIOLET_API void Init(CStr argv0) noexcept(detail::methodCanBeMarkedNoexcept);

/// Returns the absolute path of a binary from the runfiles system.
/// @param path the path to locate
VIOLET_API auto Get(Str path) noexcept(detail::methodCanBeMarkedNoexcept) -> Optional<String>;

/// Returns the workspace name that was provided either by Bazel's runfiles system
/// or with the `$VIOLET_TESTING_RUNFILES_WORKSPACE` environment variable.
///
/// If [`violet::testing::runfiles::SetWorkspaceName`] was set prior, it'll return
/// that workspace name instead of detection.
VIOLET_API auto WorkspaceName() noexcept(detail::methodCanBeMarkedNoexcept) -> Optional<String>;

/// Set the workspace name to `ws`.
VIOLET_API void SetWorkspaceName(Str ws) noexcept;

} // namespace violet::testing::runfiles
