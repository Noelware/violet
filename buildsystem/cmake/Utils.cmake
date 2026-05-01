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

function(encode_version_as_integer version out)
    string(REPLACE "-dev" "" version "${version}")
    string(REPLACE "." ";" parts "${version}")
    list(LENGTH parts count)

    if(count EQUAL 2)
        list(GET parts 0 yy)
        list(GET parts 1 mm)
        set(dd "0")
        set(patch "0")
    elseif(count EQUAL 3)
        list(GET parts 0 yy)
        list(GET parts 1 mm)
        list(GET parts 2 dd)
        set(patch "0")
    elseif(count EQUAL 4)
        list(GET parts 0 yy)
        list(GET parts 1 mm)
        list(GET parts 2 dd)
        list(GET parts 3 patch)
    else()
        message(FATAL_ERROR "Invalid version format: ${version}")
    endif()

    math(EXPR result "${yy} * 1000000 + ${mm} * 10000 + ${dd} * 100 + ${patch}")
    set(${out_var} ${result} PARENT_SCOPE)
endfunction()
