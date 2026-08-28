#!/usr/bin/env bash
# ---------------------------------------------------------------------------------
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
# ---------------------------------------------------------------------------------
# `./hack/build-ci-images.sh` :: This script will build all the CI images
# inside of the `.ci` directory.

set -eu pipefail

declare -A gccimages

# renovate: ref=gcc:16.1-trixie
gccimages["gcc-16"]="sha256:1cfa8769230debf43594ee1b48bc642f9bead8d479f9926bc61d3014bdf3ecc8"

# renovate: ref=gcc:15.2-trixie
gccimages["gcc-15"]="sha256:3ae15afe768b06d0c0fe088d822ba5f8045c26630bdacc8d8e7713cf5d8e7289"

# renovate: ref=gcc:14.3-trixie
gccimages["gcc-14"]="sha256:4e2d22617a3b4af1ad0150ba6ca3cb8a975c44c1abac6f9a09870b93cb5c363b"

if ! command -v docker >/dev/null; then
    echo "~> missing \`docker\` command :: exiting"
    exit 1
fi

VIOLET_DIR="$(realpath "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"/..)"
DOCKERFLAGS=${DOCKERFLAGS:-"--load"}
IMAGE_REGISTRY="ghcr.io/noelware/violet/ci" # TODO(@auguwu): once cr.noelware.cloud is ready, switch to `cr.noelware.cloud/private/violet/ci`

## START: LLVM Clang
for clangVersion in "23" "22" "21" "20"; do
    echo "~> START: $IMAGE_REGISTRY:clang-$clangVersion"

    time docker buildx build . \
        -t "$IMAGE_REGISTRY:clang-$clangVersion" \
        $DOCKERFLAGS \
        --build-arg LLVM_VERSION="$clangVersion" \
        --file "$VIOLET_DIR/.ci/clang/Dockerfile"

    echo "~> END: $IMAGE_REGISTRY:clang-$clangVersion"
done
## END: LLVM Clang

## START: LLVM Clang (MSan)
for clangVersion in "23" "22" "21" "20"; do
    echo "~> START: $IMAGE_REGISTRY:clang-$clangVersion-msan"

    time docker buildx build . \
        -t "$IMAGE_REGISTRY:clang-$clangVersion-msan" \
        $DOCKERFLAGS \
        --build-arg LLVM_VERSION="$clangVersion" \
        --file "$VIOLET_DIR/.ci/clang/msan.Dockerfile"

    echo "~> END: $IMAGE_REGISTRY:clang-$clangVersion-msan"
done
## END: LLVM Clang (MSan)

## START: GCC
for version in "${!gccimages[@]}"; do
    echo "~> START: $IMAGE_REGISTRY:$version"

    time docker buildx build . \
        -t "$IMAGE_REGISTRY:$version" \
        $DOCKERFLAGS \
        --file "$VIOLET_DIR/.ci/gcc/Dockerfile" \
        --build-arg BASE_IMAGE="gcc@${gccimages[$version]}"

    echo "~> END: $IMAGE_REGISTRY:$version"
done
## END: GCC
