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

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("//buildsystem/bazel:cc/defs.bzl", "defines", "sanitizer")
load(":version.bzl", "DEVBUILD", "encode_as_int")

def violet_cc_library(name, deps = [], copts = [], linkopts = [], local_defines = [], **kwargs):
    if "includes" in kwargs:
        fail("`violet_cc_library`(%s) defined `includes` but it is not allowed" % name)

    return cc_library(
        name = name,
        deps = ["//:include_hack"] + deps,
        copts = copts + sanitizer["copts"],
        linkopts = linkopts + sanitizer["linkopts"],
        includes = ["includes"],
        local_defines = local_defines + ["VIOLET_BUILDING"],
        defines = defines + [
            # deprecated define; use `VIOLET_BUILDSYSTEM_BAZEL` or `VIOLET_BUILDSYSTEM(<system>)`
            # to detect what buildsystem what we are under
            "BAZEL",

            # new flag as of 26.05.05
            "VIOLET_BUILDSYSTEM_BAZEL",
            ("VIOLET_VERSION=%d" % encode_as_int()),
            ("VIOLET_DEVBUILD=%d" % (1 if DEVBUILD else 0)),
        ],
        **kwargs
    )

def violet_cc_test(
        name,
        deps = [],
        size = "small",
        copts = [],
        linkopts = [],
        env = {},
        with_gtest_main = True,
        **kwargs):
    if "visibility" in kwargs:
        fail("`violet_cc_test`(%s) defined `visibility` but all tests are marked as private")

    return cc_test(
        name = name,
        deps = deps + ["@googletest//:gtest"] + (["@googletest//:gtest_main"] if with_gtest_main else []),
        copts = copts + sanitizer["copts"],
        linkopts = linkopts + sanitizer["copts"],
        env = env | sanitizer["env"],
        visibility = ["//visibility:private"],
        size = size,
        **kwargs
    )

def violet_cc_runfile_test(name, deps = [], **kwargs):
    return violet_cc_test(
        name = name,
        with_gtest_main = False,
        deps = deps + ["//violet/testing:runfiles", "//violet/testing:runfiles_main"],
        **kwargs
    )
