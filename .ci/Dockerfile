# -----------------------------------------------------------------------------
# Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
#
# This file is part of the Eous project.
#
# This file is dual-licensed under the terms of the Apache 2.0 License
# and the GNU General Public License v3.0 License. You are freely able
# to choose either license to govern your use of this file.
#
# You should've received copies of both licenses in the source tree
# of the project. You can also visit the licenses at:
#
#         *** https://apache.org/licenses/LICENSE-2.0 ***
#          *** https://gnu.org/licenses/gpl-3.0.html ***
# -----------------------------------------------------------------------------
# The `ci.Dockerfile` image is the official image we use for CI on TeamCity and
# GitHub Actions. Only TC and GHA have little to no access to Noelware's
# container registry, where we store the image. So, only TC and GHA have access
# to the `cr.noelware.cloud/private/violet/ci:latest` image.
#
# Every time at 00:00 PST, this image gets updated so we get the latest updates
# from the Debian repositories for our dependencies. We do pin LLVM to 20 as that
# is what we support (for now) but newer versions will work as well.
#
# We pin from `debian:bookworm-slim` so that if any supply chain occurs, our CI isn't
# really affected by it. Renovate will also update this if needed but if you need
# to manually update it:
#
#   $ docker pull debian:trixie-slim
#   $ docker image inspect debian:trixie-slim | jq '.[0].RepoDigests[0]' | tr -d '"'
FROM debian@sha256:1caf1c703c8f7e15dcf2e7769b35000c764e6f50e4d7401c355fb0248f3ddfdb

LABEL org.opencontainers.image.source="https://git.noelware.org/libraries/violet"
LABEL org.opencontainers.image.licenses="GPL-3.0-only OR Apache-2.0"

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
    gh

## 6. Create user that acts like they own this server.
RUN groupadd -g 1001 noelware && \
    useradd -rm -s /bin/bash -g noelware -u 1001 noelware && \
    passwd -d noelware && \
    usermod -aG sudo noelware

USER noelware
WORKDIR /home/noelware
