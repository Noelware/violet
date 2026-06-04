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

libviolet_filesystem_srcs = files()

if host_machine.system() == 'linux'
    libviolet_filesystem_srcs += files(
        '../../src/filesystem/platform/linux.cc',
        '../../src/filesystem/platform/unix.cc',
        '../../src/filesystem/platform/temporary/unix.cc',
        '../../src/filesystem/platform/permissions/unix.cc',
        '../../src/filesystem/platform/path/linux.cc',
        '../../src/filesystem/platform/file/linux.cc',
        '../../src/filesystem/platform/file/unix.cc',
        '../../src/filesystem/platform/metadata/linux.cc',
        '../../src/filesystem/platform/extensions/xattr/unix.cc'
    )
elif host_machine.system() == 'darwin'
    libviolet_filesystem_srcs += files(
        '../../src/filesystem/platform/macos.cc',
        '../../src/filesystem/platform/unix.cc',
        '../../src/filesystem/platform/temporary/unix.cc',
        '../../src/filesystem/platform/permissions/unix.cc',
        '../../src/filesystem/platform/path/macos.cc',
        '../../src/filesystem/platform/file/macos.cc',
        '../../src/filesystem/platform/file/unix.cc',
        '../../src/filesystem/platform/metadata/unix.cc',
        '../../src/filesystem/platform/extensions/xattr/macos.cc'
    )
elif host_machine.system() == 'windows'
    libviolet_filesystem_srcs += files(
        '../../src/filesystem/platform/windows.cc',
        '../../src/filesystem/platform/temporary/windows.cc',
        '../../src/filesystem/platform/permissions/windows.cc',
        '../../src/filesystem/platform/path/windows.cc',
        '../../src/filesystem/platform/file/windows.cc',
        '../../src/filesystem/platform/metadata/windows.cc',
        '../../src/filesystem/platform/extensions/xattr/unsupported.cc'
    )
else
    libviolet_filesystem_srcs += files(
        '../../src/filesystem/platform/unsupported.cc',
        '../../src/filesystem/platform/temporary/unsupported.cc',
        '../../src/filesystem/platform/permissions/unsupported.cc',
        '../../src/filesystem/platform/path/unsupported.cc',
        '../../src/filesystem/platform/file/unsupported.cc',
        '../../src/filesystem/platform/metadata/unsupported.cc',
        '../../src/filesystem/platform/extensions/xattr/unsupported.cc'
    )
endif

violet_filesystem_lib = library(
    'violet_filesystem',
    libviolet_filesystem_srcs,
    dependencies: [violet_dep, violet_io_dep],
    include_directories: violet_include_dir,
    cpp_args: violet_build_defines,
    install: true
)

violet_filesystem_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: [violet_filesystem_lib],
    dependencies: [violet_dep, violet_io_dep],
    compile_args: violet_consumer_defines
)

configure_file(
    input: '../../hack/pkg-config/libviolet_filesystem.pc',
    output: 'libviolet_filesystem.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)
