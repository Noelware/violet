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

/// Initializes the runfiles system, this is only useful when Bazel is the
/// build system for running tests.
void Init(CStr argv0);

/// Returns the absolute path of a binary from the runfiles system.
/// @param path the path to locate
/// @returns absolute path, if any.
auto Get(Str path) -> Optional<String>;

} // namespace violet::testing::runfiles
