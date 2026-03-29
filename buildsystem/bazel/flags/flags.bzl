# 🌺💜 Violet: Extended C++ standard library
# Copyright (c) 2025-2026 Noelware, LLC. <team@noelware.org>, et al.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

BOOL_FLAGS = [
    ## ## `@violet//bazel/flags:bitflags_free_function_impls=[True|False]`
    ## If provided, this will disable the free-functions for the following operators
    ## when using the `violet::Bitflags` class:
    ##
    ## * `operator|`
    ## * `operator&`
    ## * `operator^`
    ## * `operator~`
    "bitflags_free_function_impls",

    ## ## `@violet//bazel/flags:win32_dllexport=[True|False]`
    ## If set to **true**, this will make every public-access API set to `__declspec(dllexport)`
    ## on MSVC toolchains instead of the default `__declspec(dllimport)`.
    "win32_dllexport",

    ## ## `@violet//bazel/flags:ubsan=[True|False]`
    ## Enables the **Undefined Behaviour Sanitizer** on each Violet build. Usually,
    ## this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions,
    ## like library code.
    "ubsan",

    ## ## `@violet//bazel/flags:tsan=[True|False]`
    ## Enables the **ThreadSanitizer** on each Violet build. Usually,
    ## this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions,
    ## like library code.
    "tsan",

    ## ## `@violet//bazel/flags:msan=[True|False]`
    ## Enables **MemorySanitizer** on each Violet build. Usually,
    ## this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions,
    ## like library code.
    "msan",

    ## ## `@violet//bazel/flags:asan=[True|False]`
    ## Enables **AddressSanitizer** on each Violet build. Usually,
    ## this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions,
    ## like library code.
    "asan",
]
