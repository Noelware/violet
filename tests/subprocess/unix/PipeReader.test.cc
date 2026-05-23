// 🌺💜 Violet: Extended C++ standard library
// Copyright (c) 2025-2026 Noelware, LLC. <team@noelware.org>, et al.
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

#include <gtest/gtest.h>
#include <violet/Subprocess/PID.h>
#include <violet/Subprocess/PipeReader.h>
#include <violet/Testing/Runfiles.h>

#include <cerrno>

// NOLINTBEGIN(google-build-using-namespace,cppcoreguidelines-pro-type-const-cast)
using namespace violet;
using namespace violet::subprocess;
using namespace violet::testing;

namespace {

struct SpawnResult final {
    PID Child;
    Int32 ReadFD;
};

auto SpawnWithPipe(const String& program, std::initializer_list<CStr> args) -> SpawnResult
{
    int fds[2];
    if (::pipe(fds) != 0) {
        ADD_FAILURE() << "`pipe()' failed: " << strerror(errno);
        return { .Child = -1, .ReadFD = -1 };
    }

    PID child = ::fork();
    if (child < 0) {
        ADD_FAILURE() << "`fork()' failed: " << strerror(errno);
        ::close(fds[0]);
        ::close(fds[1]);

        return { .Child = -1, .ReadFD = -1 };
    }

    if (child == 0) {
        ::close(fds[0]);
        ::dup2(fds[1], STDOUT_FILENO);
        ::close(fds[1]);

        Vec<char*> argv;
        argv.push_back(const_cast<char*>(program.c_str()));
        for (const char* arg: args) {
            argv.push_back(const_cast<char*>(arg));
        }

        argv.push_back(nullptr);
        ::execv(program.c_str(), argv.data());
        ::_exit(127);
    }

    ::close(fds[1]);
    return { .Child = child, .ReadFD = fds[0] };
}

} // namespace

TEST(PipeReader, CapturesOutputFromPrintArgs)
{
    auto program = runfiles::Get("tests/runfiles/print_args");
    ASSERT_TRUE(program) << "runfile fetch for `tests/runfiles/print_args` failed?!";

    auto [child, fd] = SpawnWithPipe(*program, { "hello", "world" });
    ASSERT_GE(fd, 0);

    auto reader = GetPipeReader();
    ASSERT_TRUE(reader) << "unable to find a proper pipe reader";

    reader->Register(fd);
    auto result = reader->CaptureAll();

    // Wait for the child to finish
    Int32 status = 0;
    ::waitpid(child.Get(), &status, 0);
    ::close(fd);

    ASSERT_TRUE(result) << "`CaptureAll()' failed: " << result.Error();

    String output(result->begin(), result->end());
    EXPECT_EQ(output, "3\nargv[1]=hello\nargv[2]=world\n");
}

TEST(PipeReader, CapturesNoOutputFromPrintEnv)
{
    auto program = runfiles::Get("tests/runfiles/print_env");
    ASSERT_TRUE(program) << "runfile fetch for `tests/runfiles/print_env` failed?!";

    auto [child, fd] = SpawnWithPipe(*program, { "PATH" });
    ASSERT_GE(fd, 0);

    auto reader = GetPipeReader();
    ASSERT_TRUE(reader) << "unable to find a proper pipe reader";

    reader->Register(fd);
    auto result = reader->CaptureAll();

    // Wait for the child to finish
    Int32 status = 0;
    ::waitpid(child.Get(), &status, 0);
    ::close(fd);

    ASSERT_TRUE(result) << "`CaptureAll()' failed: " << result.Error();
    EXPECT_TRUE(result->empty()) << "expected no stdout output from `print_env' runfile";
}

// NOLINTEND(google-build-using-namespace,cppcoreguidelines-pro-type-const-cast)
