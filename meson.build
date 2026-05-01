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
        '/DVIOLET_COMPILER_MSVC',
        '/DNOMINMAX',
        '/DWIN32_LEAN_AND_MEAN',
    ]
endif

add_project_arguments(violet_platform_defines, language: ['c', 'cpp'])
add_project_arguments(copts, language: ['cpp'])

violet_include_dir = include_directories('include')

## ~^=w=^~ START: Noelware.Violet ~^=w=^~ ##
libviolet_srcs = files(
    'src/anyhow.cc',
    'src/panic.cc',
    'src/strings.cc',
    'src/support/demangle.cc',
    'src/support/terminal.cc',
)

if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
    libviolet_srcs += files('src/support/platform/terminal/unix.cc')
elif host_machine.system() == 'windows'
    libviolet_srcs += files('src/support/platform/terminal/windows.cc')
else
    libviolet_srcs += files('src/support/platform/terminal/unsupported.cc')
endif

libviolet_hdrs = files(
    'include/violet/Container/Optional.h',
    'include/violet/Container/Result.h',
    'include/violet/Defer.h',
    'include/violet/Events/EventEmitter.h',
    'include/violet/Iterator.h',
    'include/violet/Iterator/Enumerate.h',
    'include/violet/Iterator/Filter.h',
    'include/violet/Iterator/Map.h',
    'include/violet/Iterator/Peekable.h',
    'include/violet/Iterator/Skip.h',
    'include/violet/Iterator/Take.h',
    'include/violet/Language/Assert.h',
    'include/violet/Language/Macros.h',
    'include/violet/Language/Panic.h',
    'include/violet/Language/Policy.h',
    'include/violet/Print.h',
    'include/violet/Runtime/Info.h',
    'include/violet/SmolString.h',
    'include/violet/SourceLocation.h',
    'include/violet/Strings.h',
    'include/violet/Support/Bitflags.h',
    'include/violet/Support/Demangle.h',
    'include/violet/Support/Terminal.h',
    'include/violet/System.h',
    'include/violet/System/CI.h',
    'include/violet/Violet.h',
    'include/violet/anyhow.h',
)

violet_lib = static_library(
    'violet',
    libviolet_srcs,
    include_directories: violet_include_dir,
)

violet_dep = declare_dependency(include_directories: violet_include_dir, link_with: violet_lib)

## ~^=w=^~ END: Noelware.Violet ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Experimental ~^=w=^~ ##

libviolet_experimental_hdrs = files(
    'include/violet/Experimental/Language/TypeId.h',
    'include/violet/Experimental/OneOf.h',
)

violet_experimental_dep = declare_dependency(
    include_directories: violet_include_dir,
    dependencies: [violet_dep],
)

## ~^=w=^~ END: Noelware.Violet.Experimental ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Experimental.Threading ~^=w=^~ ##

libviolet_experimental_threading_srcs = files('src/experimental/threading/CancellationToken.cc')
libviolet_experimental_threading_hdrs = files('include/violet/Experimental/Threading/CancellationToken.h')

violet_experimental_threading_lib = static_library(
    'violet_experimental_threading',
    libviolet_experimental_threading_srcs,
    include_directories: violet_include_dir,
)

violet_experimental_threading_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: [violet_experimental_threading_lib],
    dependencies: [violet_dep],
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
        'src/io/platform/error/unix.cc'
    )
elif host_machine.system() == 'windows'
    libviolet_io_srcs += files(
        'src/io/platform/descriptor/windows.cc',
        'src/io/platform/error/windows.cc'
    )
else
    libviolet_io_srcs += files(
        'src/io/platform/descriptor/unsupported.cc',
        'src/io/platform/error/unsupported.cc'
    )
endif

libviolet_io_hdrs = files(
    'include/violet/IO/Descriptor.h',
    'include/violet/IO/Error.h',
    'include/violet/IO/Read.h',
    'include/violet/IO/Write.h'
)

violet_io_lib = static_library(
    'violet_io',
    libviolet_io_srcs,
    include_directories: violet_include_dir
)

violet_io_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_io_lib,
    dependencies: [violet_dep]
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

libviolet_filesystem_hdrs = files(
    'include/violet/Filesystem.h',
    'include/violet/Filesystem/File.h',
    'include/violet/Filesystem/Path.h',
    'include/violet/Filesystem/Permissions.h',
    'include/violet/Filesystem/Temporary.h',
    'include/violet/Filesystem/Extensions/XAttr.h'
)

violet_filesystem_lib = static_library(
    'violet_filesystem',
    libviolet_filesystem_srcs,
    include_directories: violet_include_dir,
)

violet_filesystem_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: [violet_filesystem_lib],
    dependencies: [violet_dep, violet_io_dep],
)

## ~^=w=^~ END: Noelware.Violet.Filesystem ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.IO.Experimental ~^=w=^~ ##

libviolet_io_experimental_hdrs = files(
    'include/violet/IO/Experimental/BufferedInputStream.h',
    'include/violet/IO/Experimental/BufferedOutputStream.h',
    'include/violet/IO/Experimental/OutputStream.h',
    'include/violet/IO/Experimental/InputStream.h',

    'include/violet/IO/Experimental/Input/ByteArrayInputStream.h',
    'include/violet/IO/Experimental/Input/FileInputStream.h',
    'include/violet/IO/Experimental/Input/StdinInputStream.h',
    'include/violet/IO/Experimental/Input/StringInputStream.h',

    'include/violet/IO/Experimental/Output/ByteArrayOutputStream.h',
    'include/violet/IO/Experimental/Output/FileOutputStream.h',
    'include/violet/IO/Experimental/Output/StdoutOutputStream.h',
    'include/violet/IO/Experimental/Output/StderrOutputStream.h',
    'include/violet/IO/Experimental/Output/StringOutputStream.h',
)

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

violet_io_experimental_lib = static_library(
    'violet_io_experimental',
    libviolet_io_experimental_srcs,
    include_directories: violet_include_dir
)

violet_io_experimental_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_io_experimental_lib,
    dependencies: [violet_io_dep, violet_filesystem_dep]
)

## ~^=w=^~ END: Noelware.Violet.IO.Experimental ~^=w=^~ ##
