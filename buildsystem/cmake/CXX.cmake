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

## --=-- Definitions and C++ Options --=--
set(VIOLET_DEFINES "CMAKE")

if(UNIX)
    list(APPEND VIOLET_DEFINES "VIOLET_UNIX")
    if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        list(APPEND VIOLET_DEFINES "VIOLET_APPLE_MACOS")
    elseif(LINUX)
        list(APPEND VIOLET_DEFINES "VIOLET_LINUX")
    endif()
elseif(WIN32)
    list(APPEND VIOLET_DEFINES "VIOLET_WINDOWS")
    if(VIOLET_WIN32_DLLEXPORT)
        list(APPEND VIOLET_DEFINES "VIOLET_DLL_EXPORT")
    endif()
endif()

# Architecture Defines
if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    list(APPEND VIOLET_DEFINES "VIOLET_AARCH64")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    list(APPEND VIOLET_DEFINES "VIOLET_X86_64")
endif()

# Compiler Defines and C++ options
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    list(APPEND VIOLET_OS_DEFINES "VIOLET_CLANG")
    set(VIOLET_COMPILER_COPTS "-DNOMINMAX")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    list(APPEND VIOLET_OS_DEFINES "VIOLET_GCC")
    set(VIOLET_COMPILER_COPTS "-DNOMINMAX")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    list(APPEND VIOLET_OS_DEFINES "VIOLET_MSVC")
    set(VIOLET_COMPILER_COPTS "/DNOMINMAX /DWIN32_LEAN_AND_MEAN")
else()
    set(VIOLET_COMPILER_COPTS "")
endif()

## --=-- Sanitizer Options --=--
if (VIOLET_ASAN)
    set(VIOLET_SANITIZER_OPTS "-fsanitize=address")
    set(VIOLET_ASAN_OPTIONS "print_summary=1:print_stacktrace=1")
    set(VIOLET_SANITIZER_ENV "ASAN_OPTIONS=${VIOLET_ASAN_OPTIONS}")
elseif (VIOLET_MSAN)
    set(VIOLET_SANITIZER_OPTS "-fsanitize=memory")
    set(VIOLET_SANITIZER_ENV "MSAN_OPTIONS=")
elseif (VIOLET_TSAN)
    set(VIOLET_SANITIZER_OPTS "-fsanitize=thread")
    set(VIOLET_SANITIZER_ENV "TSAN_OPTIONS=print_summary=1")
elseif (VIOLET_UBSAN)
    set(VIOLET_SANITIZER_OPTS "-fsanitize=undefined")
    set(VIOLET_UBSAN_OPTIONS "print_summary=1:print_stacktrace=1")
    set(VIOLET_SANITIZER_ENV "UBSAN_OPTIONS=${VIOLET_UBSAN_OPTIONS}")
else()
    set(VIOLET_SANITIZER_OPTS "")
    set(VIOLET_SANITIZER_ENV "")
endif()

function(violet_cc_library TARGET_NAME)
    # Parse all arguments that are mimicked from `buildsystem/bazel/cc.bzl`.
    cmake_parse_arguments(
        VCC
        "SHARED"
        ""
        "SRCS;HDRS;DEPS;COPTS;LINKOPTS;PRIVATE_DEPS"
        ${ARGN}
    )

    if(NOT VCC_SRCS AND VCC_HDRS)
        add_library(viol_${TARGET_NAME} INTERFACE)

        target_include_directories(viol_${TARGET_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

        # Add compiler options to library
        target_compile_options(
            viol_${TARGET_NAME} INTERFACE
            ${VCC_COPTS}
            ${VIOLET_SANITIZER_OPTS}
            ${VIOLET_COMPILER_COPTS}
        )

        # Add linker options
        target_link_options(
            viol_${TARGET_NAME} INTERFACE
            ${VCC_LINKOPTS}
            ${VIOLET_SANITIZER_OPTS}
        )

        # Add definitions
        target_compile_definitions(
            viol_${TARGET_NAME} INTERFACE
            ${VIOLET_DEFINES}
        )

        # Add dependencies, if any were given
        target_link_libraries(viol_${TARGET_NAME} INTERFACE ${VCC_DEPS})
    else()
        set(VCC_SRCS_AND_HDRS ${VCC_SRCS} ${VCC_HDRS})

        # Create the library itself
        add_library(viol_${TARGET_NAME} "")
        target_sources(viol_${TARGET_NAME} PRIVATE ${VCC_SRCS} ${VCC_HDRS})

        # Add compiler options to library
        target_compile_options(
            viol_${TARGET_NAME} PRIVATE
            ${VCC_COPTS}
            ${VIOLET_SANITIZER_OPTS}
            ${VIOLET_COMPILER_COPTS}
        )

        # Add linker options
        target_link_options(
            viol_${TARGET_NAME} PRIVATE
            ${VCC_LINKOPTS}
            ${VIOLET_SANITIZER_OPTS}
        )

        # Add definitions
        target_compile_definitions(
            viol_${TARGET_NAME} PRIVATE
            ${VIOLET_DEFINES}
        )

        target_include_directories(viol_${TARGET_NAME} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

        target_link_libraries(viol_${TARGET_NAME} PUBLIC ${VCC_DEPS})

        if(VCC_PRIVATE_DEPS)
            target_link_libraries(viol_${TARGET_NAME} PUBLIC ${VCC_PRIVATE_DEPS})
        endif()

        set_property(TARGET viol_${TARGET_NAME} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
        target_sources(viol_${TARGET_NAME} PRIVATE ${VCC_HDRS})
    endif()

    if (VIOLET_INSTALL)
        install(TARGETS viol_${TARGET_NAME} EXPORT VioletTargets
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()

    add_library(violet::${TARGET_NAME} ALIAS viol_${TARGET_NAME})
endfunction()

# --- Custom cc_test mimic ---
function(violet_cc_test TARGET_NAME)
    if(NOT VIOLET_ENABLE_TESTS)
        return()
    endif()

    # Parse arguments
    cmake_parse_arguments(
        VCT
        ""
        "SIZE"
        "SRCS;DEPS;COPTS;LINKOPTS;ENV"
        ${ARGN}
    )

    # Set default size (mimics Bazel's `size = "small"`)
    if (NOT VCT_SIZE)
        set(VCT_SIZE "small")
    endif()

    add_executable(${TARGET_NAME} ${VCT_SRCS})

    target_compile_options(
        ${TARGET_NAME} PRIVATE
        ${VCT_COPTS}
        ${VIOLET_SANITIZER_OPTS}
    )

    target_link_options(
        ${TARGET_NAME} PRIVATE
        ${VCT_LINKOPTS}
        ${VIOLET_SANITIZER_OPTS}
    )

    set(TEST_DEPS ${VCT_DEPS})

    # GTEST_{CFLAGS|LDFLAGS} is only set if GoogleTest was found via pkg-config when
    # using the system's version of GoogleTest. Yes, it is cursed. No, I don't like it.
    # And no, there is no better way of doing this.
    if(DEFINED GTEST_CFLAGS AND DEFINED GTEST_LDFLAGS)
        target_link_libraries(${TARGET_NAME} PRIVATE ${VCT_DEPS} ${GTEST_LDFLAGS})
        target_compile_options(${TARGET_NAME} PRIVATE ${GTEST_CFLAGS})
    else()
        target_link_libraries(${TARGET_NAME} PRIVATE ${VCT_DEPS} GTest::gtest GTest::gtest_main)
    endif()

    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
    gtest_discover_tests(${TARGET_NAME})

    if (VIOLET_SANITIZER_ENV)
        list(APPEND TEST_ENVIRONMENT_VARS ${VIOLET_SANITIZER_ENV})
    endif()

    foreach (ITEM ${VCT_ENV})
        list(APPEND TEST_ENVIRONMENT_VARS ${ITEM})
    endforeach()

    if (TEST_ENVIRONMENT_VARS)
        set_tests_properties(${TARGET_NAME} PROPERTIES ENVIRONMENT "${TEST_ENVIRONMENT_VARS}")
    endif()

    set_tests_properties(${TARGET_NAME} PROPERTIES LABELS "size=${VCT_SIZE}")
endfunction()
