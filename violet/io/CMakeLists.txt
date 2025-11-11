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
    io_error
    SRCS
        ${CMAKE_SOURCE_DIR}/src/io/error.cc
    HDRS
        ${CMAKE_SOURCE_DIR}/include/violet/IO/Error.h
    DEPS
        violet::violet
        violet::optional
        violet::result
)

violet_platform_sources(viol_io_error "${CMAKE_SOURCE_DIR}/src/io/platform/error")

violet_cc_library(
    io_read
    SRCS
        ${CMAKE_SOURCE_DIR}/src/io/read.cc
    HDRS
        ${CMAKE_SOURCE_DIR}/include/violet/IO/Read.h
    DEPS
        violet::violet
        violet::io_error
)

violet_cc_test(
    io_read_test
    SRCS
        ${CMAKE_SOURCE_DIR}/tests/io/Read.test.cc
    DEPS
        violet::io_read
)

violet_cc_library(
    io_write
    SRCS
        ${CMAKE_SOURCE_DIR}/src/io/write.cc
    HDRS
        ${CMAKE_SOURCE_DIR}/include/violet/IO/Write.h
    DEPS
        violet::violet
        violet::io_error
)

violet_cc_test(
    io_write_test
    SRCS
        ${CMAKE_SOURCE_DIR}/tests/io/Write.test.cc
    DEPS
        violet::io_write
)

violet_cc_library(
    io_descriptor
    HDRS
        ${CMAKE_SOURCE_DIR}/include/violet/IO/Descriptor.h
    DEPS
        violet::violet
        violet::io_read
        violet::io_write
)

violet_platform_sources(viol_io_descriptor "${CMAKE_SOURCE_DIR}/src/io/platform/descriptor")
