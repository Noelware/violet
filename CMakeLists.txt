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

if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(VIOLET_INSTALL "Enables the install rule" OFF)
else()
    option(VIOLET_INSTALL "Enables the install rule" ON)
endif()

include(FetchContent)

if (VIOLET_USE_SYSTEM_ABSEIL)
    message(STATUS "Using system Abseil C++ libraries")
    find_package(absl REQUIRED)
else()
    message(STATUS "Using vendored version of Abseil C++ libraries")

    FetchContent_Declare(
        absl
        GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
        GIT_TAG "20250814.1"
    )

    # Disable install rules to avoid polluting system
    set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(absl)
endif()

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

# Set the minimum CXX Standard to C++26
set(CMAKE_CXX_STANDARD 26)

add_subdirectory(violet)

if(VIOLET_INSTALL)
    install(TARGETS violet EXPORT violet FILE_SET HEADERS)
    install(EXPORT VioletTargets NAMESPACE violet:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/violet")
    configure_package_config_file(
        buildsystem/cmake/VioletConfig.cmake.in
        "${PROJECT_BINARY_DIR}/VioletConfig.cmake"

        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/violet"
    )

    # INCLUDE EXPORT +-+ libviolet
    install(FILES ${PROJECT_SOURCE_DIR}/include/violet/Violet.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet)

    # INCLUDE EXPORT +-+ libviolet_container
    file(GLOB_RECURSE CONTAINER_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Container/*.h")
    install(FILES ${CONTAINER_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Container)

    # INCLUDE EXPORT +-+ libviolet_filesystem
    file(GLOB_RECURSE FILESYSTEM_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Filesystem/*.h")
    install(FILES ${FILESYSTEM_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Filesystem)

    # INCLUDE EXPORT +-+ libviolet_io
    file(GLOB_RECURSE IO_HDRS "${PROJECT_SOURCE_DIR}/include/violet/IO/*.h")
    install(FILES ${IO_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/IO)

    # INCLUDE EXPORT +-+ libviolet_iterator
    file(GLOB_RECURSE ITERATOR_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Iterator/*.h")
    install(FILES ${ITERATOR_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Iterator)

    # INCLUDE EXPORT +-+ libviolet_language
    file(GLOB_RECURSE LANG_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Language/*.h")
    install(FILES ${LANG_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Language)

    # INCLUDE EXPORT +-+ libviolet_networking
    file(GLOB_RECURSE NETWORKING_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Networking/*.h")
    file(GLOB_RECURSE NETWORKING_IP_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Networking/IP/*.h")
    file(GLOB_RECURSE NETWORKING_SOCKET_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Networking/*.h")
    install(FILES ${NETWORKING_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Networking)
    install(FILES ${NETWORKING_IP_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Networking/IO)
    install(FILES ${NETWORKING_SOCKET_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Networking/Socket)

    # INCLUDE EXPORT +-+ libviolet_numeric
    file(GLOB_RECURSE NUMERIC_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Numeric/*.h")
    install(FILES ${NUMERIC_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Numeric)

    # INCLUDE EXPORT +-+ libviolet_subprocess
    file(GLOB_RECURSE SUBPROCESS_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Subprocess/*.h")
    install(FILES ${SUBPROCESS_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Subprocess)

    # INCLUDE EXPORT +-+ libviolet_support
    file(GLOB_RECURSE SUPPORT_HDRS "${PROJECT_SOURCE_DIR}/include/violet/Support/*.h")
    install(FILES ${SUPPORT_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/Support)

    # INCLUDE EXPORT +-+ libviolet_system
    file(GLOB_RECURSE SYSTEM_HDRS "${PROJECT_SOURCE_DIR}/include/violet/System/*.h")
    install(FILES ${SYSTEM_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/violet/System)
endif()
