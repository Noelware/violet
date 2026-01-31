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

VERSION = "26.02.03-dev.2"
DEVBUILD = True

def encode_as_int():
    parts = VERSION.split(".")
    if len(parts) == 2:
        yy, mm = parts
        if mm.endswith("-dev"):
            mm = mm[:2]

        dd = "00"
        patch = "00"
    elif len(parts) == 3:
        yy, mm, dd = parts
        if dd.endswith("-dev"):
            dd = dd[:2]

        patch = "00"
    elif len(parts) == 4:
        yy, mm, dd, patch = parts
        if dd.endswith("-dev"):
            dd = dd[:2]
    else:
        fail("invalid version format: %s" % VERSION)

    return (int(yy) * 1000000 +
            int(mm) * 10000 +
            int(dd) * 100 +
            int(patch))
