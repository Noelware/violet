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
#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Temporary.h>
#include <violet/Subprocess.h>
#include <violet/Testing/Runfiles.h>

#include <cerrno>
#include <unistd.h>

using namespace std::chrono_literals;

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet;
using namespace violet::subprocess;
using namespace violet::testing;
using namespace violet::filesystem;

TEST(Spawn, ReturnsChildWithValidPID)
{
    auto result = Command("true").Spawn();
    ASSERT_TRUE(result) << "failed to spawn `true` command: " << result.Error();

    auto& child = result.Value();
    EXPECT_GT(child.PID, 0);

    auto status = child.Wait();
    ASSERT_TRUE(status) << "waiting failed: " << status.Error();
    EXPECT_EQ(status->Code(), 0);
}

TEST(Spawn, FailsForNonExistentProgram)
{
    auto result = Command("/program/that/doesnt/exist").Spawn();
    ASSERT_FALSE(result) << "expected `Spawn()` to fail but didn't";
}

TEST(Spawn, StdioAbsentByDefault)
{
    auto result = Command("true").Spawn();
    ASSERT_TRUE(result) << "failed to spawn `true` command: " << result.Error();

    auto& child = result.Value();
    EXPECT_GT(child.PID, 0);
    EXPECT_FALSE(child.Stdin) << "expected stdin to not be present";
    EXPECT_FALSE(child.Stdout) << "expected stdout to not be present";
    EXPECT_FALSE(child.Stderr) << "expected stderr to not be present";

    auto status = child.Wait();
    ASSERT_TRUE(status) << "waiting failed: " << status.Error();
    EXPECT_EQ(status->Code(), 0);
}

TEST(Status, ExitsSuccessfully)
{
    auto result = Command("true").Status();
    ASSERT_TRUE(result) << "failed to spawn `true` command: " << result.Error();
    EXPECT_TRUE(result->Exited());
    EXPECT_EQ(result->Code(), 0);
}

TEST(Status, ExitsWithNonZeroCode)
{
    auto result = Command("false").Status();
    ASSERT_TRUE(result) << "failed to spawn `false` command: " << result.Error();
    EXPECT_TRUE(result->Exited());
    EXPECT_NE(result->Code(), 0);
}

TEST(Status, FailsForNonExistentProgram)
{
    auto result = Command("/program/that/doesnt/exist").Status();
    ASSERT_FALSE(result) << "expected `Spawn()` to fail but didn't";
}

TEST(Output, CapturesStdout)
{
    auto result = Command("echo").WithArg("hello").WithStdout(Stdio::Pipe()).Output();
    ASSERT_TRUE(result) << "`echo 'hello'` failed: " << result.Error();

    String out(result->Stdout.begin(), result->Stdout.end());
    EXPECT_EQ(out, "hello\n");
    EXPECT_TRUE(result->Stderr.empty());
}

TEST(Output, ExitStatusIsPresent)
{
    auto result = Command("true").Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();

    EXPECT_TRUE(result->Status.Exited());
    EXPECT_EQ(result->Status.Code(), 0);
}

TEST(Output, CapturesNonZeroExitStatus)
{
    auto result = Command("false").Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();
    EXPECT_TRUE(result->Status.Exited());
    EXPECT_NE(result->Status.Code().UnwrapOr(0), 0);
}

TEST(Output, StdoutEmptyForQuietCommand)
{
    auto result = Command("true").Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();
    EXPECT_TRUE(result->Stdout.empty());
    EXPECT_TRUE(result->Stderr.empty());
}

TEST(Arguments, WithArgPassedToChild)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/print_args");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_args` failed";

    auto result = Command(*program).WithArg("hello").WithStdout(Stdio::Pipe()).Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();
    EXPECT_EQ(result->Status.Code().UnwrapOr(-1), 0);

    String out(result->Stdout.begin(), result->Stdout.end());
    EXPECT_EQ(out, "2\nargv[1]=hello\n");
}

TEST(Arguments, WithArgsInitializerList)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/print_args");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_args` failed";

    auto result = Command(*program).WithArgs({ "foo", "bar", "baz" }).WithStdout(Stdio::Pipe()).Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();
    EXPECT_EQ(result->Status.Code().UnwrapOr(-1), 0);

    String out(result->Stdout.begin(), result->Stdout.end());
    EXPECT_EQ(out, "4\nargv[1]=foo\nargv[2]=bar\nargv[3]=baz\n");
}

TEST(Arguments, WithArgsSpan)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/print_args");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_args` failed";

    Vec<String> args = { "one", "two" };
    auto result = Command(*program).WithArgs(args).WithStdout(Stdio::Pipe()).Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();
    EXPECT_EQ(result->Status.Code().UnwrapOr(-1), 0);

    String out(result->Stdout.begin(), result->Stdout.end());
    EXPECT_EQ(out, "3\nargv[1]=one\nargv[2]=two\n");
}

TEST(Arguments, ConstructorWithInitializerList)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/print_args");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_args` failed";

    auto result = Command(*program, { "alpha", "beta" }).WithStdout(Stdio::Pipe()).Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();

    String out(result->Stdout.begin(), result->Stdout.end());
    EXPECT_EQ(out, "3\nargv[1]=alpha\nargv[2]=beta\n");
}

TEST(Arguments, ConstructorWithVector)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/print_args");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_args` failed";

    Vec<String> args = { "x", "y", "z" };
    auto result = Command(*program, args).WithStdout(Stdio::Pipe()).Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();

    String out(result->Stdout.begin(), result->Stdout.end());
    EXPECT_EQ(out, "4\nargv[1]=x\nargv[2]=y\nargv[3]=z\n");
}

TEST(Arguments, NoArgsExitsWithFailure)
{
    // print_args returns 1 when invoked with no arguments
    auto program = runfiles::Get("tests/subprocess/runfiles/print_args");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_args` failed";

    auto result = Command(*program).Status();
    ASSERT_TRUE(result) << "status failed: " << result.Error();
    EXPECT_NE(result->Code(), 0);
}

TEST(Environ, WithEnvMakesVariableVisibleToChild)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/print_env");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_env` failed";

    // Without the variable set, the child should return exit code 1 (not found)
    {
        auto result = Command(*program).WithArg("VIOLET_TEST_UNIQUE_VAR_XYZ987").Status();
        ASSERT_TRUE(result);
        EXPECT_EQ(result->Code(), 1);
    }

    // With the variable set, the child should return exit code 0 (found)
    {
        auto result = Command(*program)
                          .WithArg("VIOLET_TEST_UNIQUE_VAR_XYZ987")
                          .WithEnv("VIOLET_TEST_UNIQUE_VAR_XYZ987", "hello")
                          .Status();

        ASSERT_TRUE(result) << "status failed: " << result.Error();
        EXPECT_EQ(result->Code(), 0);
    }
}

TEST(Environ, WithEnvsInitializerList)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/print_env");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_env` failed";

    auto result = Command(*program)
                      .WithArg("VIOLET_MULTI_ENV_A")
                      .WithEnvs({ { "VIOLET_MULTI_ENV_A", "1" }, { "VIOLET_MULTI_ENV_B", "2" } })
                      .Status();

    ASSERT_TRUE(result) << "status failed: " << result.Error();
    EXPECT_EQ(result->Code(), 0);
}

TEST(Environ, WithEnvsSpan)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/print_env");
    ASSERT_TRUE(program) << "runfile fetch for `tests/subprocess/runfiles/print_env` failed";

    Vec<Pair<String, String>> envs = { { "VIOLET_SPAN_ENV_C", "42" } };
    auto result = Command(*program).WithArg("VIOLET_SPAN_ENV_C").WithEnvs(envs).Status();

    ASSERT_TRUE(result) << "status failed: " << result.Error();
    EXPECT_EQ(result->Code(), 0);
}

TEST(WorkingDirectory, ChangesChildCwd)
{
    auto result = Command("pwd").WithWorkingDirectory("/").WithStdout(Stdio::Pipe()).Output();
    ASSERT_TRUE(result) << "output failed: " << result.Error();

    String out(result->Stdout.begin(), result->Stdout.end());
    EXPECT_EQ(out, "/\n");
}

TEST(Stdio, PipedStdoutGivesHandle)
{
    // Use `true` so the child exits immediately without writing anything.
    auto result = Command("true").WithStdout(Stdio::Pipe()).Spawn();
    ASSERT_TRUE(result) << "spawn failed: " << result.Error();

    auto& child = result.Value();
    ASSERT_TRUE(child.Stdout) << "expected stdout pipe handle";

    ::close(child.Stdout->Descriptor.Get());

    auto status = child.Wait();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->Code(), 0);
}

TEST(Stdio, PipedStderrGivesHandle)
{
    auto result = Command("true").WithStderr(Stdio::Pipe()).Spawn();
    ASSERT_TRUE(result) << "spawn failed: " << result.Error();

    auto& child = result.Value();
    ASSERT_TRUE(child.Stderr) << "expected stderr pipe handle";

    ::close(child.Stderr->Descriptor.Get());
    auto status = child.Wait();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->Code(), 0);
}

TEST(Stdio, PipedStdinGivesHandle)
{
    // `cat` will exit when its stdin pipe is closed (EOF).
    auto result = Command("cat").WithStdin(Stdio::Pipe()).WithStdout(Stdio::Null()).WithStderr(Stdio::Null()).Spawn();
    ASSERT_TRUE(result) << "spawn failed: " << result.Error();

    auto& child = result.Value();
    ASSERT_TRUE(child.Stdin) << "expected stdin pipe handle";

    ::close(child.Stdin->Descriptor.Get());
    auto status = child.Wait();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->Code(), 0);
}

TEST(Stdio, NullStdoutDiscardsOutput)
{
    // `echo` writes to stdout; with Null that output is silently discarded.
    auto result = Command("echo").WithArg("this should be discarded").WithStdout(Stdio::Null()).Spawn();
    ASSERT_TRUE(result) << "spawn failed: " << result.Error();

    auto& child = result.Value();
    EXPECT_FALSE(child.Stdout) << "Null stdout should not produce a pipe handle";

    auto status = child.Wait();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->Code(), 0);
}

TEST(Stdio, NullStderrDiscardsErrorOutput)
{
    auto result = Command("sh").WithArgs({ "-c", "echo error >&2" }).WithStderr(Stdio::Null()).Spawn();
    ASSERT_TRUE(result) << "spawn failed: " << result.Error();

    auto& child = result.Value();
    EXPECT_FALSE(child.Stderr.HasValue()) << "Null stderr should not produce a pipe handle";

    auto status = child.Wait();
    ASSERT_TRUE(status);
}

TEST(Stdio, PipedStdinAndStdoutAllowDataRoundtrip)
{
    // Write to the child's stdin via the pipe; read what `cat` reflects on stdout.
    auto result = Command("cat").WithStdin(Stdio::Pipe()).WithStdout(Stdio::Pipe()).WithStderr(Stdio::Null()).Spawn();
    ASSERT_TRUE(result) << "spawn failed: " << result.Error();

    auto& child = result.Value();
    ASSERT_TRUE(child.Stdin);
    ASSERT_TRUE(child.Stdout);

    Array<unsigned char, 5> message = { 'p', 'i', 'n', 'g', '\n' };
    auto writeResult = child.Stdin->Write(message);
    ASSERT_TRUE(writeResult) << "expected to write 'ping\n' to stdin but couldn't: " << writeResult.Error();
    child.Stdin->Descriptor.Close();

    Vec<UInt8> buf(5, '\0');
    auto readResult = child.Stdout->Read(buf);
    ASSERT_TRUE(readResult) << "expected to read from process but couldn't: " << readResult.Error();
    child.Stdout->Descriptor.Close();

    EXPECT_GT(*readResult, 0);
    EXPECT_EQ(String(buf.begin(), buf.end()), "ping\n");

    auto status = child.Wait();
    ASSERT_TRUE(status);
    EXPECT_EQ(status->Code(), 0);
}

TEST(Stdio, PipeIntoFileWritesOutput)
{
    Path path;
    auto file = TempBuilder{ }.MkFile();
    ASSERT_TRUE(file) << "failed to build temporary file: " << file.Error();
    ASSERT_TRUE(file->Path()) << "a path should be present";

    path = file->Path().Unwrap();

    auto result = Command("echo").WithArg("piped into this file").WithStdout(Stdio::Pipe(path)).Status();
    ASSERT_TRUE(result) << "status failed: " << result.Error();
    EXPECT_EQ(result->Code(), 0);

    // Read from the file
    auto openedFile = OpenOptions{ }.Read().Open(path);
    ASSERT_TRUE(openedFile) << "expected to open file but couldn't: " << openedFile.Error();

    Vec<UInt8> buf(21, '\0');
    auto readResult = openedFile->Read(buf);
    ASSERT_TRUE(readResult) << "expected to read file but couldn't: " << readResult.Error();
    EXPECT_GT(*readResult, 0) << "no data was streamed?!";

    String str(buf.begin(), buf.end());
    EXPECT_EQ(str, "piped into this file\n");
}

TEST(SubprocessTimeout, DeathTimeoutKillsProcess)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/hang");
    ASSERT_TRUE(program) << "runfile `tests/subprocess/runfiles/hang' failed";

    auto before = std::chrono::steady_clock::now();
    auto result = Command(*program).WithDeathTimeout(200ms).Output();
    ASSERT_FALSE(result);

    auto elapsed = std::chrono::steady_clock::now() - before;
    EXPECT_LT(elapsed, 2s);
}

TEST(SubprocessTimeout, ZeroTimeoutImmediateKill)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/hang");
    ASSERT_TRUE(program) << "runfile `tests/subprocess/runfiles/hang' failed";

    auto before = std::chrono::steady_clock::now();
    auto result = Command(*program).WithDeathTimeout(0ms).Output();
    ASSERT_FALSE(result);

    auto elapsed = std::chrono::steady_clock::now() - before;
    EXPECT_LT(elapsed, 1s);
}

TEST(SubprocessTimeout, ProcessExitsBeforeTimeout)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/hang");
    ASSERT_TRUE(program) << "runfile `tests/subprocess/runfiles/hang' failed";

    auto result = Command(*program).WithArg("--exit-after=100").WithDeathTimeout(5s).Output();
    ASSERT_TRUE(result) << "failed to spawn subprocess with program [" << *program << "]: " << result.Error();
    EXPECT_EQ(result->Status.Code(), 0);
}

TEST(SubprocessTimeout, ProcessRespectsSigterm)
{
    auto program = runfiles::Get("tests/subprocess/runfiles/hang");
    ASSERT_TRUE(program) << "runfile `tests/subprocess/runfiles/hang' failed";

    auto child = Command(*program).WithArg("--respect-sigterm").WithDeathTimeout(1s).Spawn();
    ASSERT_TRUE(child) << "failed to spawn subprocess with program [" << *program << "]: " << child.Error();

    auto killed = child->Kill(SIGTERM);
    ASSERT_TRUE(killed) << "child failed to be killed: " << killed.Error();

    auto result = child->Wait();
    ASSERT_TRUE(result) << "failed to wait: " << result.Error();
    EXPECT_EQ(result->Signal(), SIGTERM);
}

// NOLINTEND(google-build-using-namespace)
