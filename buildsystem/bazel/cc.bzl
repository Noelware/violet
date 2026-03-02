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

load("@rules_cc//cc:defs.bzl", cc_library_ = "cc_library", cc_test_ = "cc_test")
load(":version.bzl", "DEVBUILD", "encode_as_int")

SANITIZER_OPTS = select({
    "//buildsystem/bazel/flags:asan_enabled": ["-fsanitize=address"],
    "//buildsystem/bazel/flags:msan_enabled": [
        "-fsanitize=memory",
        "-fsanitize-memory-track-origins",
        "-fsanitize-memory-use-after-dtor",
    ],
    "//buildsystem/bazel/flags:tsan_enabled": ["-fsanitize=thread"],
    "//buildsystem/bazel/flags:ubsan_enabled": ["-fsanitize=undefined"],
    "//conditions:default": [],
})

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

SANITIZER_ENV = select({
    "//buildsystem/bazel/flags:ubsan_enabled": {
        "UBSAN_OPTIONS": ":".join(UBSAN_OPTIONS),
    },
    "//conditions:default": {},
}) | select({
    "//buildsystem/bazel/flags:asan_enabled": {
        "ASAN_OPTIONS": ":".join(ASAN_OPTIONS),
        "LSAN_OPTIONS": ":".join(LSAN_OPTIONS),
    },
    "//conditions:default": {},
}) | select({
    "//buildsystem/bazel/flags:tsan_enabled": {
        "TSAN_OPTIONS": ":".join(TSAN_OPTIONS),
    },
    "//conditions:default": {},
}) | select({
    "//buildsystem/bazel/flags:tsan_enabled": {
        "MSAN_OPTIONS": ":".join(MSAN_OPTIONS),
    },
    "//conditions:default": {},
})

OS_DEFINES = select({
    "@platforms//os:linux": ["VIOLET_UNIX", "VIOLET_LINUX", "_GNU_SOURCE"],
    "@platforms//os:macos": ["VIOLET_UNIX", "VIOLET_APPLE_MACOS"],
    "@platforms//os:windows": ["VIOLET_WINDOWS"],
    "//conditions:default": [],
})

ARCH_DEFINES = select({
    "@platforms//cpu:aarch64": ["VIOLET_AARCH64"],
    "@platforms//cpu:x86_64": ["VIOLET_X86_64"],
    "//conditions:default": [],
})

COMPILER_DEFINES = select({
    "@rules_cc//cc/compiler:clang": ["VIOLET_CLANG"],
    "@rules_cc//cc/compiler:clang-cl": ["VIOLET_CLANG"],
    "@rules_cc//cc/compiler:gcc": ["VIOLET_GCC"],
    "@rules_cc//cc/compiler:msvc-cl": ["VIOLET_MSVC"],
    "//conditions:default": [],
}) + select({
    "//buildsystem/bazel/flags:win32_dllexport_enabled": ["VIOLET_DLL_EXPORT"],
    "//conditions:default": [],
})

COMPILER_COPTS = select({
    "@rules_cc//cc/compiler:clang": ["-DNOMINMAX"],
    "@rules_cc//cc/compiler:clang-cl": ["-DNOMINMAX"],
    "@rules_cc//cc/compiler:gcc": ["-DNOMINMAX"],
    "@rules_cc//cc/compiler:msvc-cl": ["/DNOMINMAX", "/DWIN32_LEAN_AND_MEAN"],
    "//conditions:default": [],
})

def cc_library(name, hdrs = [], **kwargs):
    copts = kwargs.pop("copts", [])
    linkopts = kwargs.pop("linkopts", [])
    deps = kwargs.pop("deps", [])

    # buildifier: disable=list-append
    deps += ["//:include_hack"]

    # Remove `includes` from any `cc_library` definition.
    # buildifier: disable=unused-variable
    _ = kwargs.pop("includes", [])

    defines = OS_DEFINES + ARCH_DEFINES + COMPILER_DEFINES + [
        "VIOLET_VERSION=%d" % encode_as_int(),
    ]

    if DEVBUILD:
        # buildifier: disable=list-append
        defines += ["VIOLET_DEVBUILD"]

    return cc_library_(
        name = name,
        hdrs = hdrs,
        copts = copts + SANITIZER_OPTS + COMPILER_COPTS,
        linkopts = linkopts + SANITIZER_OPTS,
        includes = ["include"],
        defines = ["BAZEL"],
        local_defines = defines,
        deps = deps,
        **kwargs
    )

def cc_test(name, with_gtest_main = True, **kwargs):
    deps = kwargs.pop("deps", [])
    deps.append("@googletest//:gtest")

    if with_gtest_main:
        deps.append("@googletest//:gtest_main")

    # remove `visibility` in `cc_test` and make them private
    kwargs.pop("visibility", [])

    # set `size` if it is not defined
    size = kwargs.pop("size", "small")

    copts = kwargs.pop("copts", [])
    linkopts = kwargs.pop("linkopts", [])
    env = kwargs.pop("env", {})

    return cc_test_(
        name = name,
        deps = deps,
        copts = copts + select({
            "//buildsystem/bazel/flags:asan_enabled": ["-fsanitize=address"],
            "//buildsystem/bazel/flags:msan_enabled": ["-fno-sanitize=memory"],
            "//buildsystem/bazel/flags:tsan_enabled": ["-fsanitize=thread"],
            "//buildsystem/bazel/flags:ubsan_enabled": ["-fsanitize=undefined"],
            "//conditions:default": [],
        }),
        linkopts = linkopts + select({
            "//buildsystem/bazel/flags:asan_enabled": ["-fsanitize=address"],
            "//buildsystem/bazel/flags:msan_enabled": ["-fno-sanitize=memory"],
            "//buildsystem/bazel/flags:tsan_enabled": ["-fsanitize=thread"],
            "//buildsystem/bazel/flags:ubsan_enabled": ["-fsanitize=undefined"],
            "//conditions:default": [],
        }),
        env = env | SANITIZER_ENV,
        visibility = ["//visibility:private"],
        size = size,
        **kwargs
    )
