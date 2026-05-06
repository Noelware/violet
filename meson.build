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

project(
    'violet',
    'cpp',

    license: 'MIT',
    meson_version: '>=1.1',
    version: files('.violet-version'),
    default_options: ['cpp_std=c++23', 'warning_level=3'],
)

cpp = meson.get_compiler('cpp')
compiler_id = cpp.get_id()
copts = cpp.get_supported_arguments(
    '/DNOMINMAX',
    '-fdiagnostics-color=always',
    '-Wmost',
    '-Wpedantic',
    '-Wconversion-null',
    '-Wmissing-declarations',
    '-Wnon-virtual-dtor',
    '-Wundef',
    '-Wunused-local-typedef',
    '-Wunused-result',
    '-Wvarargs',
    '-Wno-comma',
    '-Wno-conversion',
    '-Wno-gcc-compat',
    '-Wno-gnu-statement-expression',
    '-Wno-c2y-extensions',
    '-Wno-c++26-extensions',
    '-Wno-pragmas',
)

fs = import('fs')
version_raw = fs.read('.violet-version').strip()

version_parts = version_raw.split('.')
if version_parts.length() == 2
    yy = version_parts[0].to_int()
    mm = version_parts[1].replace('-dev', '').to_int()
    dd = 0
    patch = 0
elif version_parts.length() == 3
    yy = version_parts[0].to_int()
    mm = version_parts[1].to_int()
    dd = version_parts[2].replace('-dev', '').to_int()
    patch = 0
elif version_parts.length() == 4
    yy = version_parts[0].to_int()
    mm = version_parts[1].to_int()
    dd = version_parts[2].replace('-dev', '').to_int()
    patch = version_parts[3].to_int()
else
    error('invalid version format: ' + version_raw)
endif

violet_version = yy * 1000000 + mm * 10000 + dd * 100 + patch
violet_devbuild = false

add_project_arguments(
    '-DVIOLET_VERSION=' + violet_version.to_string(),
    language: ['c', 'cpp'],
)

if violet_devbuild
    add_project_arguments('-DVIOLET_DEVBUILD=1', language: ['c', 'cpp'])
endif

violet_platform_defines = ['-DVIOLET_BUILDSYSTEM_MESON']
if host_machine.system() == 'linux'
    violet_platform_defines += ['-DVIOLET_PLATFORM_UNIX', '-DVIOLET_PLATFORM_LINUX', '-D_GNU_SOURCE']
elif host_machine.system() == 'darwin'
    violet_platform_defines += ['-DVIOLET_PLATFORM_UNIX', '-DVIOLET_PLATFORM_APPLE_MACOS']
elif host_machine.system() == 'windows'
    violet_platform_defines += ['-DVIOLET_PLATFORM_WINDOWS']
endif

if host_machine.cpu_family() == 'aarch64'
    violet_platform_defines += '-DVIOLET_ARCH_AARCH64'
elif host_machine.cpu_family() == 'x86_64'
    violet_platform_defines += '-DVIOLET_ARCH_X86_64'
endif

if compiler_id == 'clang'
    violet_platform_defines += [
        '-DVIOLET_COMPILER_CLANG',
        '-DNOMINMAX',
    ]
elif compiler_id == 'clang-cl'
    violet_platform_defines += [
        '-DVIOLET_COMPILER_CLANG',
        '-DVIOLET_COMPILER_CLANG_CL',
        '-DNOMINMAX',
    ]
elif compiler_id == 'gcc'
    violet_platform_defines += [
        '-DVIOLET_COMPILER_GCC',
        '-DNOMINMAX',
    ]
elif compiler_id == 'msvc'
    violet_platform_defines += [
        '-DVIOLET_COMPILER_MSVC',
        '-DNOMINMAX',
        '-DWIN32_LEAN_AND_MEAN',
    ]
endif

add_project_arguments(violet_platform_defines, language: ['c', 'cpp'])
add_project_arguments(copts, language: ['cpp'])

violet_include_dir = include_directories('include')

conf = configuration_data()
conf.set('PREFIX', get_option('prefix'))
conf.set('VERSION', meson.project_version())

if get_option('default_library') == 'shared'
    violet_build_defines = ['-DVIOLET_BUILDING', '-DVIOLET_BUILD_SHARED']
    violet_consumer_defines = ['-DVIOLET_BUILD_SHARED']
else
    violet_build_defines = ['-DVIOLET_BUILDING']
    violet_consumer_defines = []
endif

## ~^=w=^~ START: Noelware.Violet ~^=w=^~ ##

libviolet_srcs = files(
    'src/panic.cc',
    'src/strings.cc',
    'src/system/ci.cc',
    'src/support/demangle.cc',
)

if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
    libviolet_srcs += files('src/system/platform/env/unix.cc')
elif host_machine.system() == 'windows'
    libviolet_srcs += files('src/system/platform/env/windows.cc')
else
    libviolet_srcs += files('src/system/platform/env/unsupported.cc')
endif

violet_lib = library(
    'violet',
    libviolet_srcs,
    include_directories: violet_include_dir,
    cpp_args: violet_build_defines,
    install: true
)

violet_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_lib,
    compile_args: violet_platform_defines + violet_consumer_defines,
)

configure_file(
    input: 'hack/pkg-config/libviolet.pc',
    output: 'libviolet.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)

## ~^=w=^~ END: Noelware.Violet ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Experimental ~^=w=^~ ##

libviolet_experimental_srcs = files('src/experimental/Any.cc')

violet_experimental_lib = library(
    'violet_experimental',
    libviolet_experimental_srcs,
    dependencies: [violet_dep],
    include_directories: violet_include_dir,
    cpp_args: violet_build_defines,
    install: true
)

violet_experimental_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: [violet_experimental_lib],
    dependencies: [violet_dep],
    compile_args: violet_consumer_defines
)

configure_file(
    input: 'hack/pkg-config/libviolet_experimental.pc',
    output: 'libviolet_experimental.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)

## ~^=w=^~ END: Noelware.Violet.Experimental ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Experimental.Threading ~^=w=^~ ##

libviolet_experimental_threading_srcs = files('src/experimental/threading/CancellationToken.cc')

violet_experimental_threading_lib = library(
    'violet_experimental_threading',
    libviolet_experimental_threading_srcs,
    include_directories: violet_include_dir,
    cpp_args: violet_build_defines,
    install: true
)

violet_experimental_threading_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: [violet_experimental_threading_lib],
    dependencies: [violet_dep],
    compile_args: violet_consumer_defines
)

configure_file(
    input: 'hack/pkg-config/libviolet_experimental_threading.pc',
    output: 'libviolet_experimental_threading.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)

## ~^=w=^~ END: Noelware.Violet.Experimental.Threading ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.IO ~^=w=^~ ##

libviolet_io_srcs = files(
    'src/io/error.cc',
    'src/io/read.cc',
    'src/io/write.cc'
)

if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
    libviolet_io_srcs += files(
        'src/io/platform/descriptor/unix.cc',
        'src/io/platform/error/unix.cc',
        'src/system/platform/unix.cc'
    )
elif host_machine.system() == 'windows'
    libviolet_io_srcs += files(
        'src/io/platform/descriptor/windows.cc',
        'src/io/platform/error/windows.cc',
        'src/system/platform/windows.cc'
    )
else
    libviolet_io_srcs += files(
        'src/io/platform/descriptor/unsupported.cc',
        'src/io/platform/error/unsupported.cc',
        'src/system/platform/unsupported.cc'
    )
endif

violet_io_lib = library(
    'violet_io',
    libviolet_io_srcs,
    include_directories: violet_include_dir,
    cpp_args: violet_build_defines,
    install: true
)

violet_io_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_io_lib,
    dependencies: [violet_dep],
    compile_args: violet_consumer_defines
)

configure_file(
    input: 'hack/pkg-config/libviolet_io.pc',
    output: 'libviolet_io.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)

## ~^=w=^~ END: Noelware.Violet.IO ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Filesystem ~^=w=^~ ##

libviolet_filesystem_srcs = files()

if host_machine.system() == 'linux'
    libviolet_filesystem_srcs += files(
        'src/filesystem/platform/linux.cc',
        'src/filesystem/platform/unix.cc',
        'src/filesystem/platform/temporary/unix.cc',
        'src/filesystem/platform/permissions/unix.cc',
        'src/filesystem/platform/path/linux.cc',
        'src/filesystem/platform/file/linux.cc',
        'src/filesystem/platform/file/unix.cc',
        'src/filesystem/platform/extensions/xattr/unix.cc'
    )
elif host_machine.system() == 'darwin'
    libviolet_filesystem_srcs += files(
        'src/filesystem/platform/macos.cc',
        'src/filesystem/platform/unix.cc',
        'src/filesystem/platform/temporary/unix.cc',
        'src/filesystem/platform/permissions/unix.cc',
        'src/filesystem/platform/path/macos.cc',
        'src/filesystem/platform/file/macos.cc',
        'src/filesystem/platform/file/unix.cc',
        'src/filesystem/platform/extensions/xattr/macos.cc'
    )
elif host_machine.system() == 'windows'
    libviolet_filesystem_srcs += files(
        'src/filesystem/platform/windows.cc',
        'src/filesystem/platform/temporary/windows.cc',
        'src/filesystem/platform/permissions/windows.cc',
        'src/filesystem/platform/path/windows.cc',
        'src/filesystem/platform/file/windows.cc',
        'src/filesystem/platform/extensions/xattr/unsupported.cc'
    )
else
    libviolet_filesystem_srcs += files(
        'src/filesystem/platform/unsupported.cc',
        'src/filesystem/platform/temporary/unsupported.cc',
        'src/filesystem/platform/permissions/unsupported.cc',
        'src/filesystem/platform/path/unsupported.cc',
        'src/filesystem/platform/file/unsupported.cc',
        'src/filesystem/platform/extensions/xattr/unsupported.cc'
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
    input: 'hack/pkg-config/libviolet_filesystem.pc',
    output: 'libviolet_filesystem.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)

## ~^=w=^~ END: Noelware.Violet.Filesystem ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.IO.Experimental ~^=w=^~ ##

libviolet_io_experimental_srcs = files(
    'src/io/experimental/input/buffered.cc',
    'src/io/experimental/input/bytearray.cc',
    'src/io/experimental/input/file.cc',
    'src/io/experimental/input/stdin.cc',
    'src/io/experimental/input/string.cc',

    'src/io/experimental/output/buffered.cc',
    'src/io/experimental/output/bytearray.cc',
    'src/io/experimental/output/file.cc',
    'src/io/experimental/output/stdout.cc',
    'src/io/experimental/output/stderr.cc',
    'src/io/experimental/output/string.cc',
)

if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
    libviolet_io_experimental_srcs += files(
        'src/io/experimental/input/file/unix.cc',
        'src/io/experimental/input/stdin/unix.cc',

        'src/io/experimental/output/stdout/unix.cc',
        'src/io/experimental/output/stderr/unix.cc',
    )
elif host_machine.system() == 'windows'
    libviolet_io_experimental_srcs += files(
        'src/io/experimental/input/file/windows.cc',
        'src/io/experimental/input/stdin/windows.cc',

        'src/io/experimental/output/stdout/windows.cc',
        'src/io/experimental/output/stderr/windows.cc',
    )
else
    libviolet_io_experimental_srcs += files(
        'src/io/experimental/input/file/unsupported.cc',
        'src/io/experimental/input/stdin/unsupported.cc',

        'src/io/experimental/output/stdout/unsupported.cc',
        'src/io/experimental/output/stderr/unsupported.cc',
    )
endif

violet_io_experimental_lib = library(
    'violet_io_experimental',
    libviolet_io_experimental_srcs,
    include_directories: violet_include_dir,
    dependencies: [violet_io_dep, violet_filesystem_dep],
    cpp_args: violet_build_defines,
    install: true
)

violet_io_experimental_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_io_experimental_lib,
    dependencies: [violet_io_dep, violet_filesystem_dep],
    compile_args: violet_consumer_defines
)

configure_file(
    input: 'hack/pkg-config/libviolet_io_experimental.pc',
    output: 'libviolet_io_experimental.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)

## ~^=w=^~ END: Noelware.Violet.IO.Experimental ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Terminal ~^=w=^~ ##

libviolet_terminal_srcs = files('src/support/terminal.cc')

if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
    libviolet_terminal_srcs += files('src/support/platform/terminal/unix.cc')
elif host_machine.system() == 'windows'
    libviolet_terminal_srcs += files('src/support/platform/terminal/windows.cc')
else
    libviolet_terminal_srcs += files('src/support/platform/terminal/unsupported.cc')
endif

violet_terminal_lib = library(
    'violet_terminal',
    libviolet_terminal_srcs,
    include_directories: violet_include_dir,
    dependencies: [violet_dep, violet_io_dep],
    cpp_args: violet_build_defines,
    install: true
)

violet_terminal_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_terminal_lib,
    dependencies: [violet_dep, violet_io_dep],
    compile_args: violet_consumer_defines,
)

configure_file(
    input: 'hack/pkg-config/libviolet_terminal.pc',
    output: 'libviolet_terminal.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)

## ~^=w=^~ END: Noelware.Violet.Terminal ~^=w=^~ ##

## `violet::anyhow` belongs in its own dependency as it depends on libviolet and libviolet_io,
## even though `anyhow` is in Noelware.Violet, but whatever I guess.
violet_anyhow_lib = library(
    'violet_anyhow',
    'src/anyhow.cc',
    include_directories: violet_include_dir,
    dependencies: [violet_dep, violet_terminal_dep, violet_io_dep],
    cpp_args: violet_build_defines,
    install: true
)

violet_anyhow_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_anyhow_lib,
    compile_args: violet_consumer_defines
)

configure_file(
    input: 'hack/pkg-config/libviolet_anyhow.pc',
    output: 'libviolet_anyhow.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)

install_subdir(
    'include/violet',
    install_dir: get_option('prefix') / 'include',
    exclude_files: [
        './BUILD.bazel',
        './Container/BUILD.bazel',
        './Events/BUILD.bazel',
        './Experimental/BUILD.bazel',
        './Experimental/Language/BUILD.bazel',
        './Experimental/Threading/BUILD.bazel',
        './Filesystem/BUILD.bazel',
        './Filesystem/Extensions/BUILD.bazel',
        './IO/BUILD.bazel',
        './IO/Experimental/BUILD.bazel',
        './IO/Experimental/Input/BUILD.bazel',
        './IO/Experimental/Output/BUILD.bazel',
        './Iterator/BUILD.bazel',
        './Language/BUILD.bazel',
        './Runtime/BUILD.bazel',
        './Support/BUILD.bazel',
        './System/BUILD.bazel',
        './Testing/BUILD.bazel'
    ]
)
