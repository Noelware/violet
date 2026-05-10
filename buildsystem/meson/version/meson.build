# 🌺💜 Violet: Extended C++ standard library
# Copyright (c) 2025-2026 Noelware, LLC. <team@noelware.org>, et al.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the 'Software'), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

fs = import('fs')
version_raw = fs.read('../../../.violet-version').strip()

version_parts = version_raw.split('.')
if version_parts.length() == 2
    yy = version_parts[0].to_int()
    mm = version_parts[1].replace('-dev', '').to_int()
    dd = 0
    patch = 0
elif version_parts.length() == 3
    yy = version_parts[0].to_int()
    mm = version_parts[1].to_int()
    dd = version_parts[2].replace('-dev', '').to_int()
    patch = 0
elif version_parts.length() == 4
    yy = version_parts[0].to_int()
    mm = version_parts[1].to_int()
    dd = version_parts[2].replace('-dev', '').to_int()
    patch = version_parts[3].to_int()
else
    error('invalid version format: ' + version_raw)
endif

violet_version = yy * 1000000 + mm * 10000 + dd * 100 + patch
violet_devbuild = false
