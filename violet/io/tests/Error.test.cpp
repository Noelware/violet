// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "violet/io/Error.h"
#include "violet/container/Result.h"
#include "violet/support/Demangle.h"
#include "violet/violet.h"

#include <cerrno>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#ifdef _WIN32
#    include <windows.h>
#endif

using namespace Noelware::Violet; // NOLINT
using testing::HasSubstr;

namespace noelware::testutil {

struct ErrorChecker final {
    ErrorChecker()
        : n_errno(errno)
#ifdef _WIN32
        , n_windows(GetLastError())
#endif
    {
        resetSystemError();
    }

    ~ErrorChecker()
    {
        errno = this->n_errno;

#ifdef _WIN32
        SetLastError(this->n_windows);
#endif
    }

    ErrorChecker(const ErrorChecker&) = delete;
    auto operator=(const ErrorChecker) = delete;
    ErrorChecker(ErrorChecker&&) = default;
    auto operator=(const ErrorChecker&&) = delete;

private:
    int32 n_errno;

#ifdef _WIN32
    DWORD n_windows;
#endif

    void resetSystemError()
    {
        errno = 0;

#ifdef _WIN32
        SetLastError(0);
#endif
    }
};

inline void ExpectErrno(int expected) // NOLINT(misc-use-internal-linkage)
{
    int32 actual = errno;
    EXPECT_EQ(expected, actual) << "expected [errno=" << strerror(expected) << "] but received: " << strerror(actual)
                                << " instead.";
}

#ifdef _WIN32
static inline void ExpectLastError(DWORD expected)
{
    DWORD actual = GetLastError();
    EXPECT_EQ(expected, actual) << "expected [GetLastError() = " << expected << "] but received: " << actual
                                << " instead.";
}
#endif

} // namespace noelware::testutil

namespace {

#ifdef _WIN32
inline void ExpectSystemError(int32 posix, DWORD windows)
{
    noelware::testutil::ExpectErrno(posix);
    noelware::testutil::ExpectLastError(windows);
}
#else
inline void ExpectSystemError(int32 posix)
{
    noelware::testutil::ExpectErrno(posix);
}
#endif

} // namespace

TEST(IoError, Platform)
{
    noelware::testutil::ErrorChecker checker;

#ifdef _WIN32
#else
    EXPECT_FALSE(errno);

    FILE* file = fopen("hecc.txt", "r");
    EXPECT_EQ(file, nullptr);

    ExpectSystemError(ENOENT);

    auto error = IO::Error::Platform(IO::ErrorKind::NotFound);
    EXPECT_TRUE(error.RawOSError());
    EXPECT_EQ(*error.RawOSError(), ENOENT);
    EXPECT_EQ(error.ToString(), "I/O error (system): No such file or directory");
#endif
}

TEST(IoError, Custom)
{
    IO::Error custom = IO::Error::New<String>(IO::ErrorKind::Other, "some message here");
    EXPECT_EQ(custom.ToString(), "I/O error [other error] (custom): some message here");

// If we had `cxxabi.h`, then type names should be consistent *enough* but who knows.
#ifdef VIOLET_HAS_CXXABI_HDR
    IO::Error withCustomType = IO::Error::New<Result<String, usize>>(IO::ErrorKind::Other, Err<usize>(32));
    EXPECT_THAT(withCustomType.ToString(),
        HasSubstr("Noelware::Violet::Result<std::__cxx11::basic_string<char, std::char_traits<char>, "
                  "std::allocator<char> >, unsigned long>"));
#endif
}
