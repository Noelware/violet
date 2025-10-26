#!/usr/bin/env bash
# ---------------------------------------------------------------------------------
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
# ---------------------------------------------------------------------------------
# `./hack/build-ci-images.sh` :: This script will build all the CI images
# inside of the `.ci` directory.

set -eu pipefail

declare -A gccimages

# renovate: image=gcc:15.2-trixie
gccimages["gcc-15"]="gcc@sha256:bf951c48aaa5f4b884d86377071429d0e2e75d1691d33ac884a039c1b15cfb95"

# renovate: image=gcc:14.3-trixie
gccimages["gcc-14"]="gcc@sha256:d1fe2c2366fb0ec8da60b1c561e7469a1109c7da0d3d73084d42e3aa22b7781d"

# renovate: image=gcc:13.4-bookworm
gccimages["gcc-13"]="gcc@sha256:3e40583f378ec250e7bc2388ff64efbe66b2307774983d45a373f1522f879cd3"

if ! command -v docker >/dev/null; then
    echo "~> missing \`docker\` command :: exiting"
    exit 1
fi

VIOLET_DIR="$(realpath "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"/..)"
DOCKERFLAGS=${DOCKERFLAGS:-"--load"}
IMAGE_REGISTRY="ghcr.io/noelware/violet/ci" # TODO(@auguwu): once cr.noelware.cloud is ready, switch to `cr.noelware.cloud/private/violet/ci`

## START: LLVM Clang
for clangVersion in "21" "20"; do
    echo "~> START: $IMAGE_REGISTRY:clang-$clangVersion"

    time docker buildx build . \
        -t "$IMAGE_REGISTRY:clang-$clangVersion" \
        $DOCKERFLAGS \
        --build-arg LLVM_VERSION="$clangVersion" \
        --file "$VIOLET_DIR/.ci/clang/Dockerfile"

    echo "~> END: $IMAGE_REGISTRY:clang-$clangVersion"
done
## END: LLVM Clang

## START: GCC
for version in "${!gccimages[@]}"; do
    echo "~> START: $IMAGE_REGISTRY:$version"

    time docker buildx build . \
        -t "$IMAGE_REGISTRY:$version" \
        $DOCKERFLAGS \
        --file "$VIOLET_DIR/.ci/gcc/Dockerfile" \
        --build-arg BASE_IMAGE="${gccimages[$version]}"

    echo "~> END: $IMAGE_REGISTRY:$version"
done
## END: GCC
