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
# The `.ci/clang/msan.Dockerfile` is the official image for Violet's CI pipeline that
# tests Clang coverage with MemorySanitizer enabled. Each tag corresponds to what Clang
# version we test. As of 26-08-27, we test on the following:
#     * `clang-23`
#     * `clang-22`
#     * `clang-21`
#     * `clang-20`
#
# We provide a matrix of Clang versions so that other projects can use Violet's CI
# architecture that has both non-MSan and MSan-enabled builds.

ARG LLVM_VERSION="20"
ARG IMAGE="ghcr.io/noelware/violet/ci:clang-${LLVM_VERSION}"

FROM ${IMAGE} AS build

ARG LLVM_VERSION

LABEL org.opencontainers.image.source="https://git.noelware.org/libraries/violet"
LABEL org.opencontainers.image.licenses="MIT"

## Step 1. Pull in `llvm-project` into the workspace.
RUN git clone --depth 1 --branch "release/${LLVM_VERSION}.x" https://github.com/llvm/llvm-project.git /tmp/llvm

## Step 2. Configure, build, and install the instrumented binaries
RUN mkdir -p /tmp/build && sudo mkdir -p /opt/llvm/${LLVM_VERSION}-msan && \
    cmake -G Ninja \
    -S/tmp/llvm/runtimes \
    -B/tmp/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang-${LLVM_VERSION} \
    -DCMAKE_CXX_COMPILER=clang++-${LLVM_VERSION} \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi" \
    -DLLVM_USE_SANITIZER=MemoryWithOrigins \
    -DLIBCXXABI_USE_LLVM_UNWINDER=OFF \
    -DLIBCXX_INCLUDE_TESTS=OFF \
    -DLIBCXXABI_INCLUDE_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=/opt/llvm/${LLVM_VERSION}-msan

## Step 3. Build everything
RUN sudo ninja -C /tmp/build install-cxx install-cxxabi

## Step 4. Drop everything instead of our install prefix
RUN sudo rm -rf /tmp/llvm /tmp/build

# this point is our final image
FROM ${IMAGE}

ARG LLVM_VERSION

RUN sudo apt install -y libclang-rt-${LLVM_VERSION}-dev

COPY --from=build /opt/llvm /opt/llvm

ENV BAZEL_CXXOPTS="-nostdinc++:-isystem/opt/llvm/${LLVM_VERSION}-msan/include/c++/v1"
ENV BAZEL_LINKOPTS="-nostdlib++:-L/opt/llvm/${LLVM_VERSION}-msan/lib:-Wl,-rpath,/opt/llvm/${LLVM_VERSION}-msan/lib"
