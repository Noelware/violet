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
pkgs: {
  version = pkgs.lib.strings.trimWith {
    start = true;
    end = true;
  } (builtins.readFile ../../.violet-version);

  llvm = let
    oldStdenv = pkgs.stdenv;
    version = "21";
    llvmPkgs = {
      inherit (pkgs) llvmPackages_21;
    };

    package = llvmPkgs."llvmPackages_${version}";
  in {
    inherit version package;
    inherit (package) compiler-rt libcxx clang-tools bintools lldb;

    stdenv =
      (
        if oldStdenv.hostPlatform.isLinux
        then pkgs.stdenvAdapters.useMoldLinker
        else pkgs.lib.id
      )
      package.stdenv;
  };
}
