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
# The `.ci/clang/Dockerfile` image is the official image for Violet's CI pipeline that
# tests Clang coverage. Each tag corresponds to what Clang version we test, which as of
# 2025-10-14, we test on the following Clang versions:
#
#    * `clang-21`
#    * `clang-20`
#
# Older compilers are not avaliable under Noelware's container registry. Once a new
# version of Clang is released, we provide tests and soft-deprecate the second latest
# compiler.
#
# Every time at 00:00 PST (America/Los_Angeles), this image does get updated. We also
# pin what Debian version we use as well by the image digest. Renovate will also
# update if needed. To manually update it, you can use the following commands:
#
#   $ docker pull debian:trixie-slim
#   $ docker image inspect debian:trixie-slim | jq '.[0].RepoDigests[0]' | tr -d '"'
FROM debian@sha256:5cf544fad978371b3df255b61e209b373583cb88b733475c86e49faa15ac2104

LABEL org.opencontainers.image.source="https://git.noelware.org/libraries/violet"
LABEL org.opencontainers.image.licenses="MIT"

ENV DEBIAN_FRONTEND=noninteractive

## 0. Setup dependencies to install LLVM
RUN apt update -y && apt install -y curl lsb-release gnupg

## 1. Setup LLVM APT repository
##    -------------------------
##    This will allow us to pin what LLVM version we want to use. You can override this
##    with the `--build-arg LLVM_VERSION=<number>`. We develop on LLVM 20 but more recent
##    versions should work but we don't provide support for older LLVM toolchains.
ARG LLVM_VERSION=20
RUN curl -sSL https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc
RUN echo "deb http://apt.llvm.org/trixie/  llvm-toolchain-trixie-${LLVM_VERSION} main" | tee /etc/apt/sources.list.d/llvm.list

## 2. Update APTs to include LLVM repository and install LLVM-based stuff first.
RUN apt update -y && apt install -y \
    clang-tidy-${LLVM_VERSION} \
    clang-format-${LLVM_VERSION} \
    clang-tools-${LLVM_VERSION} \
    clang-${LLVM_VERSION} \
    libc++-${LLVM_VERSION}-dev \
    libc++abi-${LLVM_VERSION}-dev \
    llvm-${LLVM_VERSION}

## 3. Install Bazelisk (which will be aliased as `bazel`) and Bazel build-tools.
# renovate: datasource=github-tags repository=bazelbuild/bazelisk
ENV BAZELISK_VERSION="1.27.0"

# renovate: datasource=github-tags repository=bazelbuild/buildtools
ENV BAZEL_BUILDTOOLS_VERSION="8.2.1"

RUN set -eux; \
  arch="$(dpkg --print-architecture)"; \
  case "${arch}" in \
    aarch64|arm64) \
      BAZEL_UNUSED_DEPS_DOWNLOAD_URL="https://github.com/bazelbuild/buildtools/releases/download/v${BAZEL_BUILDTOOLS_VERSION}/unused_deps-linux-arm64"; \
      BAZEL_BUILDIFIER_DOWNLOAD_URL="https://github.com/bazelbuild/buildtools/releases/download/v${BAZEL_BUILDTOOLS_VERSION}/buildifier-linux-arm64"; \
      BAZEL_BUILDOZER_DOWNLOAD_URL="https://github.com/bazelbuild/buildtools/releases/download/v${BAZEL_BUILDTOOLS_VERSION}/buildozer-linux-arm64"; \
      BAZELISK_DOWNLOAD_URL="https://github.com/bazelbuild/bazelisk/releases/download/v${BAZELISK_VERSION}/bazelisk-linux-arm64"; \
      ;; \
    amd64|x86_64) \
      BAZEL_UNUSED_DEPS_DOWNLOAD_URL="https://github.com/bazelbuild/buildtools/releases/download/v${BAZEL_BUILDTOOLS_VERSION}/unused_deps-linux-amd64"; \
      BAZEL_BUILDIFIER_DOWNLOAD_URL="https://github.com/bazelbuild/buildtools/releases/download/v${BAZEL_BUILDTOOLS_VERSION}/buildifier-linux-amd64"; \
      BAZEL_BUILDOZER_DOWNLOAD_URL="https://github.com/bazelbuild/buildtools/releases/download/v${BAZEL_BUILDTOOLS_VERSION}/buildozer-linux-amd64"; \
      BAZELISK_DOWNLOAD_URL="https://github.com/bazelbuild/bazelisk/releases/download/v${BAZELISK_VERSION}/bazelisk-linux-amd64"; \
      ;; \
  esac; \
  curl -fsSL -o /usr/local/bin/bazel       ${BAZELISK_DOWNLOAD_URL}; \
  curl -fsSL -o /usr/local/bin/buildifier  ${BAZEL_BUILDIFIER_DOWNLOAD_URL}; \
  curl -fsSL -o /usr/local/bin/buildozer   ${BAZEL_BUILDOZER_DOWNLOAD_URL}; \
  curl -fsSL -o /usr/local/bin/unused_deps ${BAZEL_UNUSED_DEPS_DOWNLOAD_URL}

RUN chmod +x /usr/local/bin/bazel && chmod +x /usr/local/bin/buildifier && chmod +x /usr/local/bin/buildozer && chmod +x /usr/local/bin/unused_deps

## 4. Install required software
##    -------------------------
##    Please update this Dockerfile with any new changes from `nix/devshell.nix`
##    as Noel does use Nix and probably will forget to update this with any new
##    software that is introduced to the project.
RUN apt install -y \
    libmimalloc-dev \
    ninja-build \
    cmake \
    sudo \
    meson \
    cmake \
    git \
    gh \
    pkg-config

## 5. Create user that acts like they own this server.
RUN groupadd -g 1001 noelware && \
    useradd -rm -s /bin/bash -g noelware -u 1001 noelware && \
    passwd -d noelware && \
    usermod -aG sudo noelware

USER noelware
WORKDIR /home/noelware
