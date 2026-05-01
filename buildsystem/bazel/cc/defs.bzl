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

UBSAN_OPTIONS = [
    "halt_on_error=1",
    "print_summary=1",
    "print_stacktrace=1",
]

TSAN_OPTIONS = [
    "halt_on_error=1",
    "print_summary=1",
    "second_deadlock_state=1",
    "report_atomic_races=0",
]

MSAN_OPTIONS = [
    "poison_in_dtor=1",
]

LSAN_OPTIONS = [
    "report_objects=1",
    "print_summary=1",
]

ASAN_OPTIONS = [
    "detect_leaks=1",
    "color=always",
    "print_summary=1",
]

sanitizer = {
    "copts": select({
        "//buildsystem/bazel/flags:asan_enabled": ["-fsanitize=address"],
        "//conditions:default": [],
    }) + select({
        "//buildsystem/bazel/flags:msan_enabled": [
            "-fsanitize=memory",
            "-fsanitize-memory-track-origins",
            "-fsanitize-memory-use-after-dtor",
        ],
        "//conditions:default": [],
    }) + select({
        "//buildsystem/bazel/flags:tsan_enabled": ["-fsanitize=thread"],
        "//conditions:default": [],
    }) + select({
        "//buildsystem/bazel/flags:ubsan_enabled": ["-fsanitize=undefined"],
        "//conditions:default": [],
    }),
    "env": select({
               "//buildsystem/bazel/flags:asan_enabled": {
                   "ASAN_OPTIONS": ":".join(ASAN_OPTIONS),
                   "LSAN_OPTIONS": ":".join(LSAN_OPTIONS),
               },
               "//conditions:default": {},
           }) | select({
               "//buildsystem/bazel/flags:msan_enabled": {
                   "MSAN_OPTIONS": ":".join(MSAN_OPTIONS),
               },
               "//conditions:default": {},
           }) | select({
               "//buildsystem/bazel/flags:tsan_enabled": {
                   "TSAN_OPTIONS": ":".join(TSAN_OPTIONS),
               },
               "//conditions:default": {},
           }) |
           select({
               "//buildsystem/bazel/flags:ubsan_enabled": {
                   "UBSAN_OPTIONS": ":".join(UBSAN_OPTIONS),
               },
               "//conditions:default": {},
           }),
    "linkopts": select({
        "//buildsystem/bazel/flags:asan_enabled": ["-fsanitize=address"],
        "//conditions:default": [],
    }) + select({
        "//buildsystem/bazel/flags:msan_enabled": ["-fno-sanitize=memory"],
        "//conditions:default": [],
    }) + select({
        "//buildsystem/bazel/flags:tsan_enabled": ["-fsanitize=thread"],
        "//conditions:default": [],
    }) + select({
        "//buildsystem/bazel/flags:ubsan_enabled": ["-fsanitize=undefined", "-lubsan"],
        "//conditions:default": [],
    }),
}

defines = select({
    "@platforms//os:linux": [
        "VIOLET_PLATFORM_UNIX",
        "VIOLET_PLATFORM_LINUX",
        "_GNU_SOURCE",
    ],
    "@platforms//os:macos": [
        "VIOLET_PLATFORM_UNIX",
        "VIOLET_PLATFORM_APPLE_MACOS",
    ],
    "@platforms//os:windows": [
        "VIOLET_PLATFORM_WINDOWS",
    ],
    "//conditions:default": [],
}) + select({
    "@platforms//cpu:aarch64": [
        "VIOLET_ARCH_AARCH64",
    ],
    "@platforms//cpu:x86_64": [
        "VIOLET_ARCH_X86_64",
    ],
    "//conditions:default": [],
}) + select({
    "@rules_cc//cc/compiler:clang": [
        "VIOLET_COMPILER_CLANG",

        # defines that are compiler-specific
        "NOMINMAX",
    ],
    "@rules_cc//cc/compiler:clang-cl": [
        "VIOLET_COMPILER_CLANG",
        "VIOLET_COMPILER_CLANG_CL",  # allows detection if we are `clang-cl` instead of pure `clang`

        # defines that are compiler-specific
        "NOMINMAX",
    ],
    "@rules_cc//cc/compiler:gcc": [
        "VIOLET_COMPILER_GCC",

        # defines that are compiler-specific
        "NOMINMAX",
    ],
    "@rules_cc//cc/compiler:msvc-cl": [
        "VIOLET_COMPILER_MSVC",

        # defines that are compiler-specific
        "NOMINMAX",
        "WIN32_LEAN_AND_MEAN",
    ],
    "//conditions:default": [],
})
