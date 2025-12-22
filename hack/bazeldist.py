#!/usr/bin/env python3
# ~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+
# 🌺💜 Violet: Extended C++ standard library
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
# ~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+~+
# The `hack/bazeldist.py` script will prepare distributing Violet to any Bazel
# registry without the development-related tooling, this will:
#
# * Removes the following directories:
#   * `.agents`
#   * `.ci`
#   * `.github`
#   * `.idea`
#   * `.vscode`
#   * `hack`
#   * `patches`
#   * `nix`
# * Removes CMake and Meson build files as they shouldn't pollute the final Bazel
#   distribution.
# * Removes any files that don't need to be in the final distribution from the root
#   directory.
# * Strips anything that is in the `DEVELOPMENT DEPENDENCIES` comments in `MODULE.bazel`

from pathlib import Path

from python.runfiles import runfiles
import subprocess
import tarfile
import shutil
import sys
import re
import os

ROOT = runfiles.Create()

remap_module_bzl_pattern = re.compile(
    r"\n*## :- START ~ DEVELOPMENT DEPENDENCIES -: ##.*?## :- END ~ DEVELOPMENT DEPENDENCIES -: ##\n*",
    re.DOTALL
)

def remap_module_bazel(mod: Path, new_file: Path) -> None:
    text = mod.read_text()
    new_file.write_text(remap_module_bzl_pattern.sub("", text))

def remap(src: Path, dst: Path) -> None:
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src.resolve() if src.is_symlink() else src, dst)

# This filter will ensure that tarballs are determinstic
def _tar_filter(info):
    info.mtime = 0
    return info

def construct_tarball(dst: Path, output: Path) -> None:
    with tarfile.open(output, "w:gz") as t:
        for f in dst.rglob("*"):
            if f.is_file():
                name = f.relative_to(dst)
                t.add(f, arcname=name, filter=_tar_filter)

def main(dir: str, tarball: str) -> None:
    distdir = Path(dir)
    if distdir.exists():
        shutil.rmtree(distdir)

    # Re-create the directory
    distdir.mkdir()

    ws = Path(os.environ.get("RUNFILES_DIR") or os.environ.get("RUNFILES_MANIFEST_FILE")).parent.joinpath("bazeldist.runfiles/_main")
    print(f"recursively finding files in {ws}")

    for f in ws.rglob("*"):
        rel = f.relative_to(ws)
        if str(rel).startswith("hack"):
            continue

        if str(rel).startswith(distdir.name):
            continue

        if not f.is_file():
            continue

        # Unsure why Bazel is self-referencing the tarball if it is inside
        # of the root directory, but if the file contents resembles the tarball,
        # then skip it.
        if f.name in Path(tarball).name:
            continue

        dst = distdir.joinpath(rel)
        if rel.name == "MODULE.bazel":
            remap_module_bazel(f, dst)
            continue

        print(f"copying file {rel} -> {dst}")
        remap(f, dst)

    tar = Path(tarball)
    if tar.exists():
        tar.unlink()

    construct_tarball(distdir, tar)
    shutil.rmtree(distdir)

    print(">> Finished! A tarball of the contents is available")

if __name__ == "__main__":
    if len(sys.argv) == 1:
        print(">> a directory must be specified")
        sys.exit(1)

    if len(sys.argv) == 2:
        print(">> a tarball file must be specified")
        sys.exit(1)

    main(sys.argv[1], sys.argv[2])
