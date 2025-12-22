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
{
  mkShell,
  stdenv,
  lib,
  stdenvAdapters,
  llvmPackages_21,
  ## os-specific
  ### - linux
  valgrind,
  ### - darwin
  apple-sdk_15,
  ## tools
  pkg-config,
  python3,
  bazel_8,
  bazel-buildtools,
  meson,
  cmake,
  ninja,
  ## lsp
  starpls,
  nil,
  ## libraries
  abseil-cpp,
  gtest,
  ## used for devshell
  bash,
  ripgrep,
}: let
  darwinPackages = [apple-sdk_15];
  linuxPackages = [valgrind];

  # Alias for `llvmPackages_XX` that we aim to support. At the moment,
  # we develop Violet in LLVM 21 and above.
  llvm = let
    oldStdenv = stdenv; # keep a copy of the old standard environment
    pkgs = {inherit llvmPackages_21;}; # a hack so that we can do `pkgs."llvmPackages_${version}"`
    version = "21";
  in rec {
    inherit version;

    package = pkgs."llvmPackages_${version}";

    inherit (package) compiler-rt libcxx clang-tools bintools lldb;

    stdenv =
      (
        if oldStdenv.hostPlatform.isLinux
        then stdenvAdapters.useMoldLinker
        else lib.id
      )
      package.stdenv;
  };

  packages =
    [
      # For sanitizers
      llvm.compiler-rt
      llvm.libcxx

      llvm.clang-tools
      llvm.bintools
      llvm.lldb

      bazel-buildtools
      pkg-config
      python3

      # build systems (we support Bazel, CMake, and Meson)
      bazel_8
      cmake
      meson
      ninja

      # libraries (so it's easier to develop with)
      abseil-cpp
      gtest

      # LSPs
      starpls
      nil
    ]
    ++ (lib.optionals stdenv.isLinux linuxPackages)
    ++ (lib.optionals stdenv.isDarwin darwinPackages);

  mkShell' = mkShell.override {stdenv = llvm.stdenv;};

  devshell = builtins.readFile ../hack/nix/devshell.sh;
  shellHook =
    builtins.replaceStrings [
      "@bash@"
      "@ripgrep@"
      "@compiler-rt@"
      "@libcxx@"
    ] [
      "${bash}"
      "${ripgrep}"
      "${llvm.compiler-rt.dev}"
      "${llvm.libcxx}"
    ]
    devshell;
in
  mkShell' {
    inherit packages shellHook;

    name = "violet-dev";
  }
