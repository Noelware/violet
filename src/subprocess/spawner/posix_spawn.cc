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

#include <violet/Violet.h>

#if VIOLET_PLATFORM(APPLE_MACOS)

#include <violet/Subprocess.h>
#include <violet/Subprocess/PipeReader.h>
#include <violet/Subprocess/__detail/Impl.unix.h>

#include <unordered_set>

#include <fcntl.h>
#include <grp.h>
#include <spawn.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std::chrono_literals;

using violet::subprocess::Child;
using violet::subprocess::Command;
using violet::subprocess::Stdio;

using violet::Array;
using violet::Err;
using violet::Int32;
using violet::Int64;
using violet::UInt8;
using violet::Unsafe;
using violet::Vec;

extern char** environ; // NOLINT

namespace {

void closePipes(Int32 fds[2])
{
    if (fds[0] >= 0) {
        ::close(fds[0]);
        fds[0] = -1;
    }

    if (fds[1] >= 0) {
        ::close(fds[1]);
        fds[1] = -1;
    }
}

void provideAction(
    posix_spawn_file_actions_t* actions, Int32 pipe, Int32 target, const Stdio& config, bool readonly = false)
{
    if (pipe >= 0) {
        ::posix_spawn_file_actions_adddup2(actions, pipe, target);
        ::posix_spawn_file_actions_addclose(actions, pipe);
    } else if (config.IsNull()) {
        ::posix_spawn_file_actions_addopen(actions, target, "/dev/null", readonly ? O_RDONLY : O_WRONLY, 0);
    } else if (config.PipedIntoFile()) {
        auto path = config.PipedFile().UnwrapUnchecked(Unsafe("we are in the condition if we are a piped into a file"));
        Int32 flags = readonly ? O_RDONLY : (O_WRONLY | O_CREAT | O_TRUNC);
        ::posix_spawn_file_actions_addopen(actions, target, path.Data().c_str(), flags, 0644);
    }
}

template<typename T>
void dedup(Vec<T>& data)
{
    std::unordered_set<T> seen;
    data.erase(
        std::remove_if(data.begin(), data.end(), [&](const T& value) -> bool { return !seen.insert(value).second; }),
        data.end());
}

struct FileActionsGuard final {
    VIOLET_DISALLOW_COPY_AND_MOVE(FileActionsGuard);
    VIOLET_IMPLICIT FileActionsGuard() // NOLINT
    {
        ::posix_spawn_file_actions_init(&this->n_actions);
    }

    ~FileActionsGuard()
    {
        ::posix_spawn_file_actions_destroy(&this->n_actions);
    }

    auto Get() -> posix_spawn_file_actions_t*
    {
        return &this->n_actions;
    }

private:
    posix_spawn_file_actions_t n_actions;
};

struct SpawnAttrGuard final {
    VIOLET_DISALLOW_COPY_AND_MOVE(SpawnAttrGuard);
    VIOLET_IMPLICIT SpawnAttrGuard() // NOLINT
    {
        ::posix_spawnattr_init(&this->n_attrs);
    }

    ~SpawnAttrGuard()
    {
        ::posix_spawnattr_destroy(&this->n_attrs);
    }

    auto Get() -> posix_spawnattr_t*
    {
        return &this->n_attrs;
    }

private:
    posix_spawnattr_t n_attrs;
};

} // namespace

auto violet::subprocess::detail::MakePipes(Int32 fds[2]) -> bool
{
    if (::pipe(fds) != 0) {
        return false;
    }

    ::fcntl(fds[0], F_SETFD, FD_CLOEXEC);
    ::fcntl(fds[1], F_SETFD, FD_CLOEXEC);
    return true;
}

auto violet::subprocess::detail::SpawnWithPosix(Command& command) -> io::Result<Child>
{
    Int32 stdinPipes[2] = { -1, -1 };
    Int32 stdoutPipes[2] = { -1, -1 };
    Int32 stderrPipes[2] = { -1, -1 };

    if (command.n_impl->n_stdin.Piped() && !command.n_impl->n_stdin.PipedIntoFile()) {
        if (!detail::MakePipes(stdinPipes)) {
            return Err(io::Error::OSError());
        }
    }

    if (command.n_impl->n_stdout.Piped() && !command.n_impl->n_stdout.PipedIntoFile()) {
        if (!detail::MakePipes(stdoutPipes)) {
            closePipes(stdinPipes);
            return Err(io::Error::OSError());
        }
    }

    if (command.n_impl->n_stderr.Piped() && !command.n_impl->n_stderr.PipedIntoFile()) {
        if (!detail::MakePipes(stderrPipes)) {
            closePipes(stdinPipes);
            closePipes(stdoutPipes);
            return Err(io::Error::OSError());
        }
    }

    Vec<CStr> argv;
    argv.reserve(command.n_impl->n_args.size() + 2);
    argv.push_back(command.n_impl->n_program.c_str());
    for (const auto& arg: command.n_impl->n_args) {
        argv.push_back(arg.c_str());
    }

    argv.push_back(nullptr);

    // Build the environment variables from current environment and overrides
    // TODO(@auguwu/Noel): should `Command` have a `.WipeEnvs()` where it doesn't
    // populate from the current environment?
    Vec<String> storage;
    Vec<CStr> envp;

    if (environ != nullptr) {
        for (char** env = environ; *env != nullptr; env++) {
            storage.emplace_back(*env);
        }
    }

    for (const auto& [key, value]: command.n_impl->n_environ) {
        storage.emplace_back(std::format("{}={}", key, value));
    }

    dedup(storage);

    for (const auto& ent: storage) {
        envp.push_back(ent.c_str());
    }

    envp.push_back(nullptr);

    FileActionsGuard actions;
    provideAction(actions.Get(), stdinPipes[0], STDIN_FILENO, command.n_impl->n_stdin, /*readonly=*/true);
    provideAction(actions.Get(), stdoutPipes[1], STDOUT_FILENO, command.n_impl->n_stdout);
    provideAction(actions.Get(), stderrPipes[1], STDERR_FILENO, command.n_impl->n_stderr);

    // Close parent-side pipes in the child
    if (stdinPipes[1] >= 0) {
        ::posix_spawn_file_actions_addclose(actions.Get(), stdinPipes[1]);
    }

    if (stdoutPipes[0] >= 0) {
        ::posix_spawn_file_actions_addclose(actions.Get(), stdoutPipes[0]);
    }

    if (stderrPipes[0] >= 0) {
        ::posix_spawn_file_actions_addclose(actions.Get(), stderrPipes[0]);
    }

    if (auto wd = command.n_impl->n_wd; wd.HasValue()) {
        ::posix_spawn_file_actions_addchdir_np(actions.Get(), wd->Data().c_str());
    }

    SpawnAttrGuard spawnAttrs;
    ::posix_spawnattr_setflags(spawnAttrs.Get(), POSIX_SPAWN_CLOEXEC_DEFAULT);

    pid_t pid = 0;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-const-cast)
    if (Int32 ret = ::posix_spawnp(&pid, command.n_impl->n_program.c_str(), actions.Get(), spawnAttrs.Get(),
            const_cast<char* const*>(argv.data()), const_cast<char* const*>(envp.data()));
        ret != 0) {
        // NOLINTEND(cppcoreguidelines-pro-type-const-cast)
        closePipes(stdinPipes);
        closePipes(stdoutPipes);
        closePipes(stderrPipes);

        return Err(io::Error::FromOSError(ret));
    }

    if (stdinPipes[0] >= 0) {
        ::close(stdinPipes[0]);
    }

    if (stdoutPipes[1] >= 0) {
        ::close(stdoutPipes[1]);
    }

    if (stderrPipes[1] >= 0) {
        ::close(stderrPipes[1]);
    }

    Child child(pid);

    if (stdinPipes[1] >= 0) {
        child.Stdin = ChildStdin(stdinPipes[1]);
    }
    if (stdoutPipes[0] >= 0) {
        child.Stdout = ChildStdout(stdoutPipes[0]);
    }
    if (stderrPipes[0] >= 0) {
        child.Stderr = ChildStderr(stderrPipes[0]);
    }

    child.DeathTimeout = VIOLET_MOVE(command.n_impl->n_deathTimeout);
    return child;
}

#endif
