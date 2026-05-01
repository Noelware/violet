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

include(CMakeDependentOption)
include(Utils)

encode_version_as_integer("${VIOLET_VERSION}" VIOLET_VERSION_INT)

if(VIOLET_DEVBUILD)
    set(VIOLET_DEVBUILD_INT 1)
else()
    set(VIOLET_DEVBUILD_INT 0)
endif()

set(VIOLET_DEFINES
    "NOMINMAX"
    "VIOLET_BUILDSYSTEM_CMAKE"
    "VIOLET_VERSION=${VIOLET_VERSION_INT}"
    "VIOLET_DEVBUILD=${VIOLET_DEVBUILD_INT}"
)

if(UNIX)
    list(APPEND VIOLET_DEFINES "VIOLET_PLATFORM_UNIX")
    if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        list(APPEND VIOLET_DEFINES "VIOLET_PLATFORM_APPLE_MACOS")
    elseif(LINUX)
        list(APPEND VIOLET_DEFINES "_GNU_SOURCE;VIOLET_PLATFORM_LINUX")
    endif()
elseif(WIN32)
    list(APPEND VIOLET_DEFINES "VIOLET_PLATFORM_WINDOWS")
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    list(APPEND VIOLET_DEFINES "VIOLET_ARCH_AARCH64")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    list(APPEND VIOLET_DEFINES "VIOLET_ARCH_X86_64")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(MSVC)
        list(APPEND VIOLET_DEFINES "VIOLET_COMPILER_CLANG_CL")
    else()
        list(APPEND VIOLET_DEFINES "VIOLET_COMPILER_CLANG")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND VIOLET_DEFINES "VIOLET_COMPILER_GCC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND VIOLET_DEFINES "VIOLET_COMPILER_MSVC;WIN32_LEAN_AND_MEAN")
endif()

## --=-- Sanitizer Options --=--
if (VIOLET_ASAN)
    set(VIOLET_SANITIZER_COPTS "-fsanitize=address")
    set(VIOLET_SANITIZER_LINKOPTS "-fsanitize=address")
    set(VIOLET_ASAN_OPTIONS "detect_leaks=1:color=always:print_summary=1")
    set(VIOLET_LSAN_OPTIONS "report_objects=1:print_summary=1")
    set(VIOLET_SANITIZER_ENV "ASAN_OPTIONS=${VIOLET_ASAN_OPTIONS};LSAN_OPTIONS=${VIOLET_LSAN_OPTIONS}")
elseif (VIOLET_MSAN)
    set(VIOLET_SANITIZER_COPTS "-fsanitize=memory")
    set(VIOLET_SANITIZER_LINKOPTS "-fsanitize=memory")
    set(VIOLET_MSAN_OPTIONS "poison_in_dtor=1")
    set(VIOLET_SANITIZER_ENV "MSAN_OPTIONS=${VIOLET_MSAN_OPTIONS}")
elseif (VIOLET_TSAN)
    set(VIOLET_SANITIZER_COPTS "-fsanitize=thread")
    set(VIOLET_SANITIZER_LINKOPTS "-fsanitize=thread")
    set(VIOLET_TSAN_OPTIONS "halt_on_error=1:print_summary=1:second_deadlock_state=1:report_atomic_races=0")
    set(VIOLET_SANITIZER_ENV "TSAN_OPTIONS=${VIOLET_TSAN_OPTIONS}")
elseif (VIOLET_UBSAN)
    set(VIOLET_SANITIZER_COPTS "-fsanitize=undefined")
    set(VIOLET_SANITIZER_LINKOPTS "-fsanitize=undefined")
    set(VIOLET_UBSAN_OPTIONS "halt_on_error=1:print_summary=1:print_stacktrace=1")
    set(VIOLET_SANITIZER_ENV "UBSAN_OPTIONS=${VIOLET_UBSAN_OPTIONS}")
else()
    set(VIOLET_SANITIZER_COPTS "")
    set(VIOLET_SANITIZER_LINKOPTS "")
    set(VIOLET_SANITIZER_ENV "")
endif()

function(violet_cc_library TARGET)
    cmake_parse_arguments(
        VCC
        "PUBLIC"
        ""
        "SRCS;HDRS;DEPS;COPTS;LINKOPTS;DEFINES;LOCAL_DEFINES"
        ${ARGN}
    )

    if (NOT VCC_SRCS AND VCC_HDRS)
        add_library(violet_${TARGET} INTERFACE)
        add_library(violet::${TARGET} ALIAS violet_${TARGET})

        target_include_directories(violet_${TARGET} INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

        target_compile_definitions(violet_${TARGET} INTERFACE ${VIOLET_DEFINES} ${VCC_DEFINES})
        target_compile_options(violet_${TARGET} INTERFACE ${VCC_COPTS} ${VIOLET_SANITIZER_COPTS})
        target_link_options(violet_${TARGET} INTERFACE ${VCC_LINKOPTS} ${VIOLET_SANITIZER_LINKOPTS})
        target_compile_definitions(violet_${TARGET} INTERFACE
            VIOLET_BUILDING
            ${VCC_LOCAL_DEFINES}
        )

        if (VCC_DEPS)
            target_link_libraries(violet_${TARGET} INTERFACE ${VCC_DEPS})
        endif()

        return()
    else()
    if (BUILD_SHARED_LIBS)
            add_library(violet_${TARGET} SHARED ${VCC_SRCS} ${VCC_HDRS})
        else()
            add_library(violet_${TARGET} STATIC ${VCC_SRCS} ${VCC_HDRS})
        endif()
    endif()

    target_include_directories(violet_${TARGET} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )

    target_compile_options(violet_${TARGET} PRIVATE ${VCC_COPTS} ${VIOLET_SANITIZER_COPTS})
    target_link_options(violet_${TARGET} PRIVATE ${VCC_LINKOPTS} ${VIOLET_SANITIZER_LINKOPTS})

    target_compile_definitions(violet_${TARGET} PRIVATE
        VIOLET_BUILDING
        ${VCC_LOCAL_DEFINES}
    )

    target_compile_definitions(violet_${TARGET} PUBLIC ${VIOLET_DEFINES} ${VCC_DEFINES})

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(violet_${TARGET} PUBLIC VIOLET_BUILD_SHARED)
        if (VIOLET_COMPILER_MSVC OR VIOLET_COMPILER_CLANG_CL)
            target_compile_options(violet_${TARGET} PRIVATE "/Zc:dllexportInlines-")
        else()
            target_compile_options(violet_${TARGET} PRIVATE
                "-fvisibility=hidden"
                "-fvisibility-inlines-hidden"
            )
        endif()
    endif()

    if (VCC_DEPS)
        target_link_libraries(violet_${TARGET} PUBLIC ${VCC_DEPS})
    endif()

    add_library(violet::${TARGET} ALIAS violet_${TARGET})
endfunction()

# # --- Custom cc_test mimic ---
# function(violet_cc_test TARGET_NAME)
#     if(NOT VIOLET_ENABLE_TESTS)
#         return()
#     endif()

#     # Parse arguments
#     cmake_parse_arguments(
#         VCT
#         ""
#         "SIZE"
#         "SRCS;DEPS;COPTS;LINKOPTS;ENV"
#         ${ARGN}
#     )

#     # Set default size (mimics Bazel's `size = "small"`)
#     if (NOT VCT_SIZE)
#         set(VCT_SIZE "small")
#     endif()

#     add_executable(viol_${TARGET_NAME} ${VCT_SRCS})

#     target_compile_options(
#         viol_${TARGET_NAME} PRIVATE
#         ${VCT_COPTS}
#         ${VIOLET_SANITIZER_OPTS}
#     )

#     target_link_options(
#         viol_${TARGET_NAME} PRIVATE
#         ${VCT_LINKOPTS}
#         ${VIOLET_SANITIZER_OPTS}
#     )

#     set(TEST_DEPS ${VCT_DEPS})

#     # GTEST_{CFLAGS|LDFLAGS} is only set if GoogleTest was found via pkg-config when
#     # using the system's version of GoogleTest. Yes, it is cursed. No, I don't like it.
#     # And no, there is no better way of doing this.
#     if(DEFINED GTEST_CFLAGS AND DEFINED GTEST_LDFLAGS)
#         target_link_libraries(viol_${TARGET_NAME} PRIVATE ${VCT_DEPS} ${GTEST_LDFLAGS})
#         target_compile_options(viol_${TARGET_NAME} PRIVATE ${GTEST_CFLAGS})
#     else()
#         target_link_libraries(viol_${TARGET_NAME} PRIVATE ${VCT_DEPS} GTest::gtest GTest::gtest_main)
#     endif()

#     add_test(NAME viol_${TARGET_NAME} COMMAND viol_${TARGET_NAME})
#     gtest_discover_tests(viol_${TARGET_NAME})

#     if (VIOLET_SANITIZER_ENV)
#         list(APPEND TEST_ENVIRONMENT_VARS ${VIOLET_SANITIZER_ENV})
#     endif()

#     foreach (ITEM ${VCT_ENV})
#         list(APPEND TEST_ENVIRONMENT_VARS ${ITEM})
#     endforeach()

#     if (TEST_ENVIRONMENT_VARS)
#         set_tests_properties(viol_${TARGET_NAME} PROPERTIES ENVIRONMENT "${TEST_ENVIRONMENT_VARS}")
#     endif()

#     set_tests_properties(viol_${TARGET_NAME} PROPERTIES LABELS "size=${VCT_SIZE}")
# endfunction()

# function(violet_platform_sources target base)
#     if(NOT TARGET ${target})
#         message(FATAL_ERROR "Target '${target}' does not exist yet!")
#     endif()

#     cmake_parse_arguments(
#         PF
#         ""
#         "APPLE;LINUX;WINDOWS"
#         ""
#         ${ARGN}
#     )

#     set(sources "")
#     if(WIN32)
#         if (PF_WINDOWS)
#             list(APPEND sources "${base}/${PF_WINDOWS}")
#         else()
#             list(APPEND sources "${base}/windows.cc")
#         endif()
#     elseif(UNIX)
#         if(APPLE AND PF_APPLE)
#             if (EXISTS PF_APPLE)
#                 list(APPEND sources "${base}/${PF_APPLE}")
#             else()
#                 message(FATAL_ERROR "Unable to find file [${base}/${PF_LINUX}]")
#             endif()
#         elseif(LINUX AND PF_LINUX)
#             if(EXISTS "${base}/${PF_LINUX}")
#                 list(APPEND sources "${base}/${PF_LINUX}")
#             else()
#                 message(FATAL_ERROR "Unable to find file [${base}/${PF_LINUX}]")
#             endif()
#         endif()

#         if (EXISTS "${base}/unix.cc")
#             list(APPEND sources "${base}/unix.cc")
#         endif()
#     endif()

#     if(${sources} STREQUAL "")
#         list(APPEND sources "${base}/unsupported.cc")
#     endif()

#     target_sources(${target} PRIVATE ${sources})
# endfunction()
