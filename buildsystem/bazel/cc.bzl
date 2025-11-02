# 🌺💜 Violet: Extended standard library for C++26
# Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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

SANITIZER_OPTS = select({
    "//buildsystem/bazel/configs:asan_enabled": ["-fsanitize=address"],
    "//buildsystem/bazel/configs:msan_enabled": ["-fsanitize=memory"],
    "//buildsystem/bazel/configs:tsan_enabled": ["-fsanitize=thread"],
    "//buildsystem/bazel/configs:ubsan_enabled": ["-fsanitize=undefined"],
    "//conditions:default": [],
})

MSAN_OPTS = []
UBSAN_OPTS = ["print_summary=1", "print_stacktrace=1"]
ASAN_OPTS = ["print_summary=1"]
TSAN_OPTS = ["print_summary=1"]

SANITIZER_ENV = select({
    "//buildsystem/bazel/configs:ubsan_enabled": {"UBSAN_OPTIONS": ":".join(UBSAN_OPTS)},
    "//conditions:default": {},
}) | select({
    "//buildsystem/bazel/configs:asan_enabled": {"ASAN_OPTIONS": ":".join(ASAN_OPTS)},
    "//conditions:default": {},
}) | select({
    "//buildsystem/bazel/configs:tsan_enabled": {"TSAN_OPTIONS": ":".join(TSAN_OPTS)},
    "//conditions:default": {},
}) | select({
    "//buildsystem/bazel/configs:tsan_enabled": {"MSAN_OPTIONS": ":".join(MSAN_OPTS)},
    "//conditions:default": {},
})

OS_DEFINES = select({
    "@platforms//os:linux": ["VIOLET_UNIX", "VIOLET_LINUX"],
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
    "//buildsystem/bazel/configs:win32_dllexport_enabled": ["VIOLET_DLL_EXPORT"],
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

    return cc_library_(
        name = name,
        hdrs = hdrs,
        copts = copts + SANITIZER_OPTS + COMPILER_COPTS,
        linkopts = linkopts + SANITIZER_OPTS,
        includes = ["include"],
        defines = ["BAZEL"] + OS_DEFINES + ARCH_DEFINES + COMPILER_DEFINES,
        deps = deps,
        **kwargs
    )

def cc_test(name, **kwargs):
    deps = kwargs.pop("deps", [])
    deps += ["@googletest//:gtest", "@googletest//:gtest_main"]

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
        copts = copts + SANITIZER_OPTS,
        linkopts = linkopts + SANITIZER_OPTS,
        env = env | SANITIZER_ENV,
        visibility = ["//visibility:private"],
        size = size,
        **kwargs
    )
