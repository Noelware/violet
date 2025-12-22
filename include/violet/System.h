// 🌺💜 Violet: Extended C++ standard library
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

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Filesystem/Path.h>
#include <violet/Violet.h>

namespace violet::sys {

/// Returns a environment variable of `key`.
auto GetEnv(Str key) noexcept -> Optional<String>;

/// Adds a variable to the system's environment variables.
///
/// ## Safety
/// This method is marked **unsafe** as it is unsafe to call this in a multithreaded
/// context and the first parameter ensures that you know what you're doing and
/// why you are doing it.
///
/// ## Panics
/// This function will use the [`VIOLET_ASSERT`] macro to check if it was set
/// correctly.
///
/// @param key the name of the environment variable
/// @param value the value to set
/// @param replace does a overwrite if the key exists
void SetEnv(Unsafe, Str key, Str value, bool replace = true);

/// Deletes a variable name from the environment.
/// @param key the name of the environment variable
void RemoveEnv(Unsafe, Str key);

/// Returns the path of the working directory.
auto WorkingDirectory() noexcept -> io::Result<filesystem::Path>;

/// Sets the working directory to `path`.
auto SetWorkingDir(filesystem::PathRef path) -> io::Result<void>;

} // namespace violet::sys
