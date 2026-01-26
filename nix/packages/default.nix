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
  stdenv,
  llvmPackages_21,
  lib,
  buildBazelPackage,
  bazel_7,
  nix-gitignore,
  stdenvAdapters,
  git,
}: let
  inherit (import ../lib/common.nix {inherit stdenv llvmPackages_21 stdenvAdapters lib;}) llvm version;

  buildBazelPackage' = buildBazelPackage.override {
    inherit (llvm) stdenv;
  };

  pkg-config =
    builtins.replaceStrings
    ["@version@"]
    [version]
    (builtins.readFile ../../hack/pkg-config/libviolet.pc);
in
  buildBazelPackage' rec {
    inherit version;

    pname = "violet";
    src = nix-gitignore.gitignoreSource [] ../../.;

    bazel = bazel_7;
    bazelTargets = ["violet:base_uber"];
    bazelBuildFlags = [
      "--config=opt"

      "--cxxopt=-x"
      "--cxxopt=c++"
      "--host_cxxopt=-x"
      "--host_cxxopt=c++"

      # Set C++ standard to 20
      "--cxxopt=-std=c++20"
      "--host_cxxopt=-std=c++20"
    ];

    nativeBuildInputs = [git];

    removeRulesCC = false;
    removeLocalConfigCc = false;

    fetchAttrs.hash = "sha256-8OZFJhhbzHH46rVs5NWBeVkFLKJ6Mli7l8hpUIBiT08=";
    buildAttrs.installPhase = ''
      mkdir -p $out/{lib,include/violet}
      mkdir -p $out/include/violet/{Container,Iterator,Language,Numeric,Support,System}

      cp ${src}/include/violet/Violet.h $out/include/violet/Violet.h
      cp ${src}/include/violet/System.h $out/include/violet/System.h
      cp ${src}/include/violet/Iterator.h $out/include/violet/Iterator.h

      cp -r ${src}/include/violet/Container/. $out/include/violet/Container
      cp -r ${src}/include/violet/Iterator/. $out/include/violet/Iterator
      cp -r ${src}/include/violet/Language/. $out/include/violet/Language
      cp -r ${src}/include/violet/Numeric/. $out/include/violet/Numeric
      cp -r ${src}/include/violet/Support/. $out/include/violet/Support
      cp -r ${src}/include/violet/System/. $out/include/violet/System

      cp -v bazel-bin/violet/libbase_uber.a $out/lib/libviolet.a
      cp -v bazel-bin/violet/libbase_uber.so $out/lib/libviolet.so

      mkdir -p $out/lib/pkgconfig
      echo "${pkg-config}" > $out/lib/pkgconfig/libviolet.pc

      substituteInPlace $out/lib/pkgconfig/libviolet.pc \
        --replace-fail "prefix=@out@" "prefix=$out"
    '';

    meta = {
      description = "🌺💜 Extended C++ standard library";
      homepage = "https://docs.noelware.org/library/cpp/violet/${version}";
      license = lib.licenses.mit;
      platforms = lib.platforms.linux ++ lib.platforms.darwin;
      maintainers = [lib.maintainers.auguwu];
    };
  }
