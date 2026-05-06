#!/usr/bin/env python3
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

from urllib.request import Request, urlopen
from pathlib import Path
from io import StringIO

import hashlib
import base64
import json
import os

REPOSITORY = "Noelware/violet"

token = os.environ.get("GITHUB_TOKEN")
tag = os.environ.get("GITHUB_REF_NAME")
wd = os.environ.get("GITHUB_WORKSPACE")

if not wd:
    wd = str(Path.cwd())

def GitHubRequest(url: str, method: str = "GET", data = None):
    headers = {
        "Accept": "application/vnd.github+json",
        "Authorization": "token %s" % token
    }

    if data is not None:
        data = json.dumps(data).encode("utf-8")
        headers["Content-Type"] = "application/json"

    req = Request(url, headers=headers, method=method, data=data)
    with urlopen(req) as resp:
        return json.load(resp)

def MesonHotpatch(f: StringIO, tag: str):
    mapping = {
        "violet": "violet_dep",
        "violet_experimental": "violet_experimental_dep",
        "violet_io": "violet_io_dep",
        "violet_filesystem": "violet_filesystem_dep",
        "violet_experimental_threading": "violet_experimental_threading_dep",
        "violet_io_experimental": "violet_io_experimental_dep",
        "violet_terminal": "violet_terminal_dep",
        "violet_anyhow": "violet_anyhow_dep"
    }

    with open("%s/mesondist.tgz" % wd, "rb") as fd:
        source_hash = hashlib.sha256(fd.read()).hexdigest()

    f.write('## Meson\n')
    f.write('> **subprojects/violet.wrap**:\n')
    f.write('```ini\n')
    f.write('[wrap-file]\n')
    f.write('source_url  = https://github.com/%s/releases/download/%s/mesondist.tgz\n' % (REPOSITORY, tag))
    f.write('source_hash = %s\n\n' % source_hash)
    f.write('[provide]\n')

    for name, dep in mapping.items():
        f.write('%s = %s\n' % (name, dep))

    f.write('```\n\n')

def BazelHotpatch(f: StringIO, tag: str):
    with open("%s/bazeldist.tgz" % wd, "rb") as fd:
        digest = hashlib.sha256(fd.read()).digest()

    integrity = "sha256-%s" % base64.b64encode(digest).decode()
    f.write('## Bazel\n')
    f.write('> As of **21/01/26**, we plans on uploading all **Noelware.Violet** frameworks onto the BCR and our own registry (`https://bzl.noelware.cloud`). If either are available, prefer using `--registry=https://bzl.noelware.cloud`.\n')
    f.write('\n```python\n')
    f.write('bazel_dep(name = "violet", version = "%s")\n' % tag)
    f.write('archive_override(\n')
    f.write('    module_name = "violet",\n')
    f.write('    integrity = "%s",\n' % integrity)
    f.write('    urls = [\n')
    f.write('          # "https://artifacts.noelware.org/bazel-registry/violet/%s/bazeldist.tgz",\n' % tag)
    f.write('          "https://github.com/%s/releases/download/%s/bazeldist.tgz",\n' % (REPOSITORY, tag))
    f.write('    ]\n')
    f.write(')\n')
    f.write('```\n')

def Main():
    if token is None or tag is None == 0:
        raise RuntimeError("$GITHUB_TOKEN or $GITHUB_REF_NAME not defined in environment")

    release = GitHubRequest("https://api.github.com/repos/%s/releases/tags/%s" % (REPOSITORY, tag))
    body = release["body"]

    print("=== current release notes ====")
    print(body)
    print("===                       ====")

    f = StringIO()
    f.write(body)

    BazelHotpatch(f, tag)
    MesonHotpatch(f, tag)

    f.write('## CMake\n')
    f.write('As of this release, CMake distribution is not available~\n\n')
    f.write('## GN\n')
    f.write('As of this release, GN distribution is not available~\n\n')

    GitHubRequest(
        "https://api.github.com/repos/%s/releases/%s" % (REPOSITORY, release["id"]),
        method="PATCH",
        data={"body": f.getvalue()}
    )

if __name__ == '__main__':
    Main()
