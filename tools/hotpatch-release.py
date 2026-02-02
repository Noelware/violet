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

import json
import os

token = os.environ["GITHUB_TOKEN"] or ""
tag = os.environ["GITHUB_REF_NAME"] or ""

def _github_request(url: str, method: str = "GET", data = None):
    headers = {
        "Accept": "application/vnd.github+json",
        "Authorization": f"token {token}"
    }

    if data is not None:
        data = json.dumps(data).encode("utf-8")
        headers["Content-Type"] = "application/json"

    req = Request(url, headers=headers, method=method, data=data)
    with urlopen(req) as resp:
        return json.load(resp)

def main():
    if len(token) == 0 or len(tag) == 0:
        raise RuntimeError("$GITHUB_TOKEN or $GITHUB_REF_NAME not defined in environment")

    release = _github_request(f"https://api.github.com/repos/Noelware/violet/releases/tags/{tag}")
    body = release["body"]

    print("==== current release notes ====")
    print(body)
    print("====                       ====")

    if "BAZELDIST_INTEGRITY" in os.environ:
        integrity = os.environ.get("BAZELDIST_INTEGRITY")

        body += "\n\n## Bazel\n"
        body += "> As of **21/01/26**, we have plans on uploading all **Noelware.Violet** frameworks onto the BCR and our own (`https://bzl.noelware.cloud`). It could exist right now, if you are, you can use `--registry=https://bzl.noelware.cloud` and all frameworks will be available."
        body += "\n"
        body += "\n```python\n"
        body += "bazel_dep(name = \"violet\", version = \"%s\")\n" % tag
        body += "archive_override(\n"
        body += "    module_name = \"violet\",\n"
        body += "    integrity = \"%s\",\n" % integrity
        body += "    urls = [\n"
        body += "        # \"https://artifacts.noelware.org/bazel-registry/violet/%s/bazeldist.tgz\",\n" % tag
        body += "        \"https://github.com/Noelware/violet/releases/download/%s/bazeldist.tgz\",\n" % tag
        body += "    ]\n"
        body += ")\n"
        body += "```\n\n"

    # TODO: Meson, CMake hotpatch
    body += "## CMake\n"
    body += "Right now, it is not the best to use the frameworks in a CMake environment! *ehehe...~*\n"
    body += "\n## Meson \n"
    body += "Using the frameworks in a Meson environment is not ready yet!\n"
    body += "\n## GN (\"Generate Ninja\")\n"
    body += "Using the frameworks in a GN environment is not ready yet!\n"

    _github_request(
        f"https://api.github.com/repos/Noelware/violet/releases/{release["id"]}",
        method="PATCH",
        data={"body": body}
    )

if __name__ == '__main__':
    main()
