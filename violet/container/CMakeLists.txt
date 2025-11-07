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

include(CXX)

violet_cc_library(
    optional
    HDRS
        ../../include/violet/Container/Optional.h
    DEPS
        violet::violet
    PRIVATE_DEPS
        violet::cxx_demangle
)

violet_cc_test(
    optional_test
    SRCS
        ../../tests/container/Optional.test.cc
    DEPS
        violet::optional
)

violet_cc_library(
    result
    HDRS
        ../../include/violet/Container/Result.h
    DEPS
        violet::violet
    PRIVATE_DEPS
        violet::cxx_demangle
)

violet_cc_test(
    result_test
    SRCS
        ../../tests/container/Result.test.cc
    DEPS
        violet::result
)
