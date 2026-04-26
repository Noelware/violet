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

BOOL_FLAGS = {
    "asan": {
        "default": False,
        "doc": "Enables the **Address** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions.",
    },
    "bitflags_free_function_impls": {
        "default": False,
        "doc": """Disables the free-functions for the following operators when using the `violet::Bitflags` class:

        * `operator|`
        * `operator&`
        * `operator^`
        * `operator~`
        """,
    },
    "msan": {
        "default": False,
        "doc": """Enables the **Memory** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions.

        When invoked on `cc_test`s, the C++ standard library implementation will require to be compiled with MemorySanitizer. This will always fail in libstdc++, but libc++
        has MSan support, but you will need to compile it yourself; default toolchains of libc++ don't compile with MSan by default.""",
    },
    "runfiles_logs": {
        "default": True,
        "doc": "Enables verbose logs in the console on each test that uses the Runfiles framework.",
    },
    "tsan": {
        "default": False,
        "doc": "Enables the **Thread** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions.",
    },
    "ubsan": {
        "default": False,
        "doc": "Enables the **Undefined Behaviour** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions.",
    },
    "win32_dllexport": {
        "default": False,
        "doc": "When set to **true**, this will use `__declspec(dllexport)` on MSVC toolchains instead of `__declspec(dllimport)`. This is a no-op on non-MSVC toolchains.",
    },
}

STRING_FLAGS = {
    "dwarf_backend": {
        "default": "libdwarf",
        "doc": """The backend to use when parsing DWARF object files, primarily used in Linux. This is mainly used by the `Noelware.Violet.Experimental.Debugging`
        framework. When you don't have dependents on any target inside of `//violet/experimental/debugging`, this is disabled.

        By default, this will use the **elfutils** `libdw` library.

        * `system-llvm`: Uses the system's LLVM that is available. This will be imported as a [`cc_import`]
                         definition.
        * `libdwarf`: Uses **elfutils**' `libdw` library to parse DWARF object files. Recommended
                      on most instances.
        * `llvm`: Uses LLVM's machinery to parse DWARF object files. Only recommended if you want
                  strong support, but at the cost of more compilation time as libLLVM is a huge
                  project.
        * `violet` (EXPERIMENTAL): Uses Noelware's implementation for parsing DWARF object files. Not recommended
                                   at the slightest; this is VERY experimental and things are subject to break. We're
                                   not sure if we want to do this.
        * `none`: Allows you to build your own DWARF object-file backend. Very not recommended unless you want to have
                  full control over the parsing yourself.""",
        "values": ["libdwarf", "llvm", "violet", "none"],
    },
    "mach-o_backend": {
        "default": "llvm",
        "doc": """The backend to use when parsing Mach-O binaries on macOS. This is mainly used by the `Noelware.Violet.Experimental.Debugging`
        framework. When you don't have dependents on any target inside of `//violet/experimental/debugging`, this is disabled.

        By default, it'll use `libLLVM` to parse Mach-O binaries. This will use the system's LLVM that is available either
        via Homebrew or available using Apple's LLVM.

        * `system-llvm`: Uses the system's LLVM that is available. This will be imported as a [`cc_import`]
                         definition.
        * `llvm`: Uses LLVM's machinery to parse DWARF object files. Only recommended if you want
                  strong support, but at the cost of more compilation time as libLLVM is a huge
                  project.
        * `violet` (EXPERIMENTAL): Uses Noelware's implementation for parsing Mach-O object files. Not recommended
                                   at the slightest; this is VERY experimental and things are subject to break. We're
                                   not sure if we want to do this.
        * `disable`: Disables parsing Mach-O binaries alltogether. The implementation won't resolve line numbers or function names
                   if disabled.
        * `none`: Allows you to build your own Mach-O backend. Very not recommended unless you want to have
                  full control over the parsing yourself.""",
        "values": ["llvm", "violet", "disable", "none"],
    },
}
