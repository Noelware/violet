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
final: prev: {
  violet = prev.lib.makeScope prev.callPackage (self: {
    base = self.callPackage ./packages {};
    events = self.callPackage ./packages/events.nix {};
    filesystem = self.callPackage ./packages/filesystem.nix {};
    io = self.callPackage ./packages/io.nix {};
  });
}
# {
#   lib,
#   stdenv,
#   cmake,
#   gtest,
#   abseil-cpp,
#   nix-gitignore,
#   static ? stdenv.hostPlatform.isStatic,
#   cxxStandard ? null,
#   llvmPackages_21,
#   ## custom flags
#   withNetworking ? false,
#   withFilesystem ? false,
#   withSubprocess ? false,
#   withIO ? false,
#   all ? false,
# }: let
#   version = builtins.readFile ../.violet-version;
# in
#   llvmPackages_21.stdenv.mkDerivation (finalAttrs: {
#     inherit version;
#     pname = "violet";
#     src = nix-gitignore.gitignoreSource [] ../.;
#     cmakeFlags =
#       [
#         "-DBUILD_SHARED_LIBS=${
#           if static
#           then "OFF"
#           else "ON"
#         }"
#         "-DVIOLET_USE_SYSTEM_GOOGLETEST=ON"
#         "-DVIOLET_USE_SYSTEM_ABSEIL=ON"
#         "-DVIOLET_ENABLE_TESTS=ON"
#       ]
#       ++ lib.optionals (cxxStandard != null) [
#         "-DCMAKE_CXX_STANDARD=${cxxStandard}"
#       ]
#       ++ lib.optionals all [
#         "-DVIOLET_ENABLE_ALL=ON"
#       ]
#       ++ lib.optionals withNetworking [
#         "-DVIOLET_ENABLE_NET=ON"
#       ]
#       ++ lib.optionals withFilesystem [
#         # withFilesystem implies that IO is enabled
#         "-DVIOLET_ENABLE_IO=ON"
#         "-DVIOLET_ENABLE_FS=ON"
#       ]
#       ++ lib.optionals withIO [
#         "-DVIOLET_ENABLE_IO=ON"
#       ]
#       ++ lib.optionals withSubprocess [
#         "-DVIOLET_ENABLE_SUBPROCESS=ON"
#       ];
#     strictDeps = true;
#     nativeBuildInputs = [cmake];
#     buildInputs = [gtest abseil-cpp];
#     meta = {
#       description = "Extended C++26 standard library";
#       changelog = "https://oss.noelware.org/libraries/cxx/violet#${version}";
#       homepage = "https://docs.noelware.org/library/violet/${version}";
#       license = with lib.licenses; [mit];
#       platforms = ["x86_64-linux" "x86_64-darwin" "aarch64-darwin"];
#       maintainers = with lib.maintainers; [auguwu];
#     };
#   })
