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
    '-Wshadow',
    '-Wconversion',
    '-Wconversion-null',
    '-Wmissing-declarations',
    '-Wnon-virtual-dtor',
    '-Wold-style-cast',
    '-Wnull-dereference',
    '-Wdouble-promotion',
    '-Wimplicit-fallthrough',
    '-Wundef',
    '-Wunused-local-typedef',
    '-Wunused-result',
    '-Wvarargs',
    '-Wno-old-style-cast',
    '-Wno-unused-parameter',
    '-Wno-implicit-int-conversion',
    '-Wno-unused-command-line-argument',
    '-Wno-sign-conversion',
    '-Wno-unterminated-string-initialization',
    '-Wno-implicit-int-float-conversion',
    '-Wno-deprecated-literal-operator',
    '-Wno-missing-designated-field-initializers',
    '-Wno-c++11-narrowing',
    '-Wno-nullability-extension',
    '-Wno-gcc-compat',
    '-Wno-c2y-extensions',
    '-Wno-c99-extensions',
    '-Wno-gnu-statement-expression-from-macro-expansion',
    '-Wno-extra-semi',
    '-Wno-overlength-strings',
    '-Wno-c23-extensions',
    '-Wno-c11-extensions',
    '-Wno-c++26-extensions',
)

subdir('buildsystem/meson/version')
subdir('buildsystem/meson/defines')

## ~^=w=^~ START: Noelware.Violet ~^=w=^~ ##
subdir('violet')
## ~^=w=^~ END: Noelware.Violet ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Experimental ~^=w=^~ ##
subdir('violet/experimental')
## ~^=w=^~ END: Noelware.Violet.Experimental ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Experimental.Threading ~^=w=^~ ##
subdir('violet/experimental/threading')
## ~^=w=^~ END: Noelware.Violet.Experimental.Threading ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.IO ~^=w=^~ ##
subdir('violet/io')
## ~^=w=^~ END: Noelware.Violet.IO ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Filesystem ~^=w=^~ ##
subdir('violet/filesystem')
## ~^=w=^~ END: Noelware.Violet.Filesystem ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.IO.Experimental ~^=w=^~ ##
subdir('violet/io/experimental')
## ~^=w=^~ END: Noelware.Violet.IO.Experimental ~^=w=^~ ##

## ~^=w=^~ START: Noelware.Violet.Terminal ~^=w=^~ ##
subdir('violet/terminal')
## ~^=w=^~ END: Noelware.Violet.Terminal ~^=w=^~ ##

subdir('violet/anyhow')

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
        './Testing/BUILD.bazel',
    ],
)
