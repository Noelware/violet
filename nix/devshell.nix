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
{
  mkShell,
  stdenv,
  lib,
  stdenvAdapters,
  llvmPackages_20,
  ## os-specific
  ### - linux
  valgrind,
  ### - darwin
  apple-sdk_15,
  ## tools
  pkg-config,
  python3,
  bazel_7,
  bazel-buildtools,
  ## lsp
  starpls,
  nil,
}: let
  darwinPackages = [apple-sdk_15];
  linuxPackages = [valgrind];

  # Alias for `llvmPackages_XX` that we aim to support. At the moment,
  # we develop Violet in LLVM 20 and above.
  llvm = llvmPackages_20;
  llvmStdenv =
    (
      if stdenv.hostPlatform.isLinux
      then stdenvAdapters.useMoldLinker
      else lib.id
    )
    llvm.stdenv;

  packages =
    [
      llvm.clang-tools
      bazel-buildtools
      pkg-config
      bazel_7
      python3

      # LSPs
      starpls
      nil
    ]
    ++ (lib.optional stdenv.isLinux linuxPackages)
    ++ (lib.optional stdenv.isDarwin darwinPackages);

  mkShell' = mkShell.override {stdenv = llvmStdenv;};
in
  mkShell' {
    inherit packages;

    name = "eous-dev";
  }
