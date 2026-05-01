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

include(CXX)

## START: Targets ##

## END: Targets ##

add_subdirectory(container)
# add_subdirectory(events)

# if ("Violet.Experimental" IN_LIST VIOLET_FRAMEWORKS)
#     add_subdirectory(experimental)
# endif()

# if ("Violet.Filesystem" IN_LIST VIOLET_FRAMEWORKS)
#     add_subdirectory(filesystem)
# endif()

# if ("Violet.IO" IN_LIST VIOLET_FRAMEWORKS)
#     add_subdirectory(io)
# endif()

# add_subdirectory(iterator)
# add_subdirectory(support)
# add_subdirectory(system)

# if ("Violet.Testing" IN_LIST VIOLET_FRAMEWORKS)
#     add_subdirectory(testing)
# endif()
