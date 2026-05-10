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

violet_platform_defines = [
    '-DVIOLET_VERSION=' + violet_version.to_string(),
    '-DVIOLET_BUILDSYSTEM_MESON',
]

if violet_devbuild
    violet_platform_defines += ['-DVIOLET_DEVBUILD=1']
endif

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

violet_include_dir = include_directories('../../../include')

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
