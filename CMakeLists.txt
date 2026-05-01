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

cmake_minimum_required(VERSION 3.25)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/buildsystem/cmake")

project(violet
    DESCRIPTION "🌺💜 Extended C++ standard library"
    HOMEPAGE_URL "https://docs.noelware.org/libraries/cpp/violet"
    LANGUAGES CXX
)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(ReadFile)
include(CTest)

set(VIOLET_DEVBUILD OFF CACHE BOOL "Whether this is a development build")
get_violet_version(VIOLET_VERSION CACHE STRING)

message(STATUS "violet ${VIOLET_VERSION}: using compiler ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

# +^~w~^+ START: Toggles +^~w~^+
option(VIOLET_ASSERT_SUBSCRIPT "Enables asserts in subscript operators" ON)
option(VIOLET_ASAN "Enables the Address Sanitizer on each CMake target" OFF)
option(VIOLET_BITFLAGS_FREE_FUNCTIONS "Enables or disables the free functions that are implemented for `|`, `&`, `^`, and `~` for `violet::Bitflags`" OFF)
option(VIOLET_RUNFILES_LOGS "Enables verbose logs in the console for each test that uses the Runfiles framework" ON)
option(VIOLET_MSAN "Enables the Memory Sanitizer on each CMake target" OFF)
option(VIOLET_UBSAN "Enables the Undefined Behaviour Sanitizer on each CMake target" OFF)
option(VIOLET_TSAN "Enables the Thread Sanitizer on each CMake target" OFF)
option(VIOLET_ALL_FRAMEWORKS "Enables all frameworks" OFF)
# +^~w~^+ END: Toggles +^~w~^+

set(VIOLET_FRAMEWORKS "Violet" CACHE STRING "A list of Violet frameworks to build")
set(VIOLET_ALL_FRAMEWORKS "Violet;Violet.Experimental;Violet.Experimental.Threading;Violet.Filesystem;Violet.IO;Violet.IO.Experimental;Violet.Testing")
set_property(CACHE VIOLET_FRAMEWORKS PROPERTY STRINGS
    "Violet"
    "Violet.Experimental"
    "Violet.Experimental.Threading"
    "Violet.Filesystem"
    "Violet.IO"
    "Violet.IO.Experimental"
    "Violet.Testing"
)

if (VIOLET_ALL_FRAMEWORKS)
    message(STATUS "Enabling all frameworks")
    set(VIOLET_FRAMEWORKS "${VIOLET_ALL_FRAMEWORKS}" CACHE STRING "enabled all frameworks" FORCE)
endif()

# Options relating to tests
option(VIOLET_ENABLE_TESTS "Enables the test suite" OFF)
option(VIOLET_USE_SYSTEM_GOOGLETEST "Uses the system's GoogleTest instead of vendoring" OFF)

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(VIOLET_INSTALL "Enables the install rule" OFF)
else()
    option(VIOLET_INSTALL "Enables the install rule" ON)
endif()

string(REPLACE ";" ", " FRAMEWORKS "${VIOLET_FRAMEWORKS}")
message(STATUS "== Building frameworks: ${FRAMEWORKS} ==")

include(FetchContent)

if (VIOLET_ENABLE_TESTS)
    enable_testing()
    if(VIOLET_USE_SYSTEM_GOOGLETEST)
        message(STATUS "Using system GoogleTest libraries")
        find_package(GTest)
        if (NOT ${GTest_FOUND})
            message(AUTHOR_WARNING "Couldn't find GoogleTest by normal CMake standards, using `pkg-config` as last resort")

            find_package(PkgConfig)
            if (NOT ${PkgConfig_FOUND})
                message(FATAL_ERROR "Couldn't find `pkg-config` as last resort to find GoogleTest")
            endif()

            pkg_search_module(GTEST REQUIRED gtest_main)
        endif()
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

    include(GoogleTest)
endif()

# Set the minimum CXX Standard to C++26 if not defined
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
    message(STATUS "CMAKE_CXX_STANDARD was not set, using C++20 as the default")
endif()

add_subdirectory(violet)

# if(VIOLET_INSTALL)
#     install(EXPORT VioletTargets NAMESPACE violet:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/violet")
#     configure_package_config_file(
#         buildsystem/cmake/VioletConfig.cmake.in
#         "${PROJECT_BINARY_DIR}/VioletConfig.cmake"

#         INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/violet"
#     )

#     # INCLUDE EXPORT +-+ libviolet
#     install(FILES ${PROJECT_SOURCE_DIR}/include/violet/Violet.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet)

#     # INCLUDE EXPORT +-+ libviolet_container
#     file(GLOB_RECURSE CONTAINER_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Container/*.h")
#     install(FILES ${CONTAINER_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Container)

#     # INCLUDE EXPORT +-+ libviolet_filesystem
#     if(VIOLET_ENABLE_FS)
#         file(GLOB_RECURSE FILESYSTEM_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Filesystem/*.h")
#         install(FILES ${FILESYSTEM_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Filesystem)
#     endif()

#     # INCLUDE EXPORT +-+ libviolet_io
#     if(VIOLET_ENABLE_IO)
#         file(GLOB_RECURSE IO_HDRS "${PROJECT_SOURCE_DIR}/include/violet/IO/*.h")
#         install(FILES ${IO_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/IO)
#     endif()

#     # INCLUDE EXPORT +-+ libviolet_subprocess
#     if(VIOLET_ENABLE_SUBPROCESS)
#         install(FILES "${PROJECT_SOURCE_DIR}/include/violet/Subprocess.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet)
#     endif()

#     # INCLUDE EXPORT +-+ libviolet_iterator
#     file(GLOB_RECURSE ITERATOR_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Iterator/*.h")
#     install(FILES ${ITERATOR_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Iterator)

#     # INCLUDE EXPORT +-+ libviolet_language
#     file(GLOB_RECURSE LANG_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Language/*.h")
#     install(FILES ${LANG_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Language)

#     # INCLUDE EXPORT +-+ libviolet_numeric
#     file(GLOB_RECURSE NUMERIC_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Numeric/*.h")
#     install(FILES ${NUMERIC_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Numeric)

#     # INCLUDE EXPORT +-+ libviolet_support
#     file(GLOB_RECURSE SUPPORT_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Support/*.h")
#     install(FILES ${SUPPORT_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Support)

#     # INCLUDE EXPORT +-+ libviolet_system
#     file(GLOB_RECURSE SYSTEM_HDRS "${PROJECT_SOURCE_DIR}/include/violet/System/*.h")
#     install(FILES ${SYSTEM_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/System)
# endif()
