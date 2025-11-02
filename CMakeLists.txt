# 🌺💜 Violet: Extended standard library for C++26
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

cmake_minimum_required(VERSION 3.25)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/buildsystem/cmake")

include(ReadFile)
get_violet_version(violet_VERSION)

project(violet
    VERSION ${violet_VERSION}
    DESCRIPTION "🌺💜 Extended standard library for C++26"
    HOMEPAGE_URL "https://docs.noelware.org/libraries/cxx/violet"
    LANGUAGES CXX
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(CTest)

message(STATUS "violet ${violet_VERSION}: using compiler ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")

# Add options to configure this build. This should be similar to the Bazel flags we have.
option(VIOLET_WIN32_DLLEXPORT "Marks all Violet APIs as `__declspec(dllexport)` instead of `__declspec(dllimport)` (Windows only)" OFF)
option(VIOLET_UBSAN "Enables Undefined Behaviour Sanitizers for this build" OFF)
option(VIOLET_TSAN "Enables Thread Sanitizer for this build" OFF)
option(VIOLET_ASAN "Enables Address Sanitizer for this build" OFF)
option(VIOLET_MSAN "Enables Memory Sanitizer for this build" OFF)

option(VIOLET_USE_SYSTEM_ABSEIL "Uses the system's Abseil instead of vendoring" OFF)

# Options relating to tests
option(VIOLET_ENABLE_TESTS "Enables the test suite" OFF)
option(VIOLET_USE_SYSTEM_GOOGLETEST "Uses the system's GoogleTest instead of vendoring" OFF)

include(FetchContent)

if (VIOLET_USE_SYSTEM_ABSEIL)
    message(STATUS "Using system Abseil C++ libraries")
    find_package(absl REQUIRED)
else()

    # TODO(@auguwu): use `FetchContent_Declare` probably?
endif()

if (VIOLET_ENABLE_TESTS)
    enable_testing()

    if(VIOLET_USE_SYSTEM_GOOGLETEST)
        message(STATUS "Using system GoogleTest libraries")
        find_package(GTest REQUIRED)
    else()
        message(STATUS "Using a vendored version of GoogleTest")

        FetchContent_Declare(
            googletest
            GIT_REPOSITORY
                "https://github.com/google/googletest.git"
            GIT_TAG
                "v1.17.0"
        )

        # Disable install rules to avoid polluting system
        set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

        FetchContent_MakeAvailable(googletest)
    endif()
endif()

# Set the minimum CXX Standard to C++20
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(violet)
