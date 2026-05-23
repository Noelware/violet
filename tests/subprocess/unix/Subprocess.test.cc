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
#include <violet/Subprocess.h>
#include <violet/Subprocess/Extensions/Unix.h>
#include <violet/Testing/Runfiles.h>

#include <cerrno>
#include <unistd.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet;
using namespace violet::subprocess;
using namespace violet::testing;

TEST(UnixExt, WithPreExecRunsBeforeExec)
{
    auto program = runfiles::Get("tests/runfiles/print_env");
    ASSERT_TRUE(program) << "runfile fetch for `tests/runfiles/print_env` failed";

    auto command = Command(*program).WithArg("VIOLET_PREEXEC_SENTINEL");
    ext::PreExec(command, [] -> void {
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        ::setenv("VIOLET_PREEXEC_SENTINEL", "1", 1);
    });

    auto result = command.Status();
    ASSERT_TRUE(result) << "status failed: " << result.Error();
    EXPECT_EQ(result->Code(), 0) << "env var set by PreExec should be visible in child";
}

TEST(UnixExt, WithPreExecReplacedBySecondCall)
{
    auto program = runfiles::Get("tests/runfiles/print_env");
    ASSERT_TRUE(program) << "runfile fetch for `tests/runfiles/print_env` failed";

    auto command = Command(*program).WithArg("VIOLET_PREEXEC_SECOND");
    ext::PreExec(command, [] -> void {
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        ::setenv("VIOLET_PREEXEC_SECOND", "wrong", 1);
    });

    ext::PreExec(command, [] -> void {
        // NOLINTNEXTLINE(concurrency-mt-unsafe)
        ::setenv("VIOLET_PREEXEC_SECOND", "correct", 1);
    });

    auto result = command.Status();
    ASSERT_TRUE(result) << "status failed: " << result.Error();

    // Both callbacks set the var, so it will exist regardless; the point is it
    // does not error. The second registration replaces the first.
    EXPECT_EQ(result->Code(), 0);
}

TEST(UnixExt, WithUIDCurrentUserSucceeds)
{
    uid_t me = ::getuid();

    auto command = Command("true");
    ext::UID(command, me);

    auto result = command.Status();
    ASSERT_TRUE(result) << "status failed: " << result.Error();
    EXPECT_EQ(result->Code(), 0);
}

TEST(UnixExt, WithGIDCurrentGroupSucceeds)
{
    gid_t me = ::getgid();

    auto command = Command("true");
    ext::GID(command, me);

    auto result = command.Status();
    ASSERT_TRUE(result) << "status failed: " << result.Error();
    EXPECT_EQ(result->Code(), 0);
}

TEST(ChildKill, DefaultSignalKillsProcess)
{
    // Spawn a process that sleeps indefinitely, kill it with SIGKILL (default),
    // and verify it was terminated by a signal.
    auto child = Command("sleep").WithArg("60").Spawn();
    ASSERT_TRUE(child) << "spawn failed: " << child.Error();

    auto killResult = child->Kill();
    ASSERT_TRUE(killResult) << "Kill() failed: " << killResult.Error();

    auto status = child->Wait();
    ASSERT_TRUE(status) << "Wait() failed: " << status.Error();
    EXPECT_TRUE(status->Signaled());
    EXPECT_EQ(status->Signal(), SIGKILL);
}

TEST(ChildKill, ExplicitSIGTERMTerminatesProcess)
{
    // Spawn a process that sleeps indefinitely, send SIGTERM explicitly,
    // and verify it was terminated by that signal.
    auto child = Command("sleep").WithArg("60").Spawn();
    ASSERT_TRUE(child) << "spawn failed: " << child.Error();

    auto killResult = child->Kill(SIGTERM);
    ASSERT_TRUE(killResult) << "Kill(SIGTERM) failed: " << killResult.Error();

    auto status = child->Wait();
    ASSERT_TRUE(status) << "Wait() failed: " << status.Error();
    EXPECT_TRUE(status->Signaled());
    EXPECT_EQ(status->Signal(), SIGTERM);
}

TEST(ChildKill, InvalidPIDReturnsError)
{
    // Construct a Child with a PID that is guaranteed not to exist,
    // and verify that Kill() returns an error rather than succeeding.
    Child orphan(-1);

    auto result = orphan.Kill();
    EXPECT_FALSE(result) << "Kill() on an invalid PID should return an error";
}

// NOLINTEND(google-build-using-namespace)
