# 🌺💜 Violet: Extended C++ standard library
# Copyright (c) 2025-2026 Noelware, LLC. <team@noelware.org>, et al.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the 'Software'), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

libviolet_experimental_threading_srcs = files('../../../src/experimental/threading/CancellationToken.cc')

violet_experimental_threading_lib = library(
    'violet_experimental_threading',
    libviolet_experimental_threading_srcs,
    include_directories: violet_include_dir,
    cpp_args: violet_build_defines,
    install: true,
)

violet_experimental_threading_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: [violet_experimental_threading_lib],
    dependencies: [violet_dep],
    compile_args: violet_consumer_defines,
)

configure_file(
    input: '../../../hack/pkg-config/libviolet_experimental_threading.pc',
    output: 'libviolet_experimental_threading.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig',
)
