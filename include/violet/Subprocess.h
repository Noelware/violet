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

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Filesystem/Path.h>
#include <violet/Subprocess/ExitStatus.h>
#include <violet/Subprocess/PID.h>
#include <violet/Subprocess/Stdio.h>

#include <chrono>

#if VIOLET_PLATFORM(UNIX)
#include <csignal>

#include <sys/types.h>
#elif VIOLET_PLATFORM(WINDOWS)
#include <windows.h>
#endif

namespace violet::subprocess {
struct Command;
struct Child;

#if VIOLET_PLATFORM(UNIX)
namespace ext {
    struct PreExecFun;

    void UID(Command&, uid_t);
    void GID(Command&, gid_t);
    void Groups(Command&, std::initializer_list<gid_t>);
    void Groups(Command&, Span<gid_t>);
    void PreExec(Command& command, ext::PreExecFun exec);
} // namespace ext

namespace detail {
    auto SpawnAsForkExec(Command&) -> violet::io::Result<Child>;

#if VIOLET_PLATFORM(APPLE_MACOS)
    auto SpawnWithPosix(Command&) -> violet::io::Result<Child>;
#endif
} // namespace detail
#endif

/// Represents a running or exited child process spawned by [`Command::Spawn()`].
///
/// A `Child` is returned from [`Command::Spawn()`] and holds the process
/// identifier together with optional pipe handles for the three standard
/// streams. Each stream handle is present only when the corresponding
/// [`Stdio`] configuration was [`Stdio::Pipe()`]; otherwise the field is an
/// empty [`Optional`].
///
/// The child process runs independently of the parent. Call [`Wait()`] to
/// block until the process exits and collect its [`ExitStatus`].
///
/// ## Example
/// ```cpp
/// #include <violet/Subprocess.h>
///
/// auto child = violet::subprocess::Command("grep")
///     .WithArg("-r")
///     .WithArg("TODO")
///     .WithStdout(violet::subprocess::Stdio::Pipe())
///     .Spawn();
///
/// if (child) {
///     auto status = child->Wait();
/// }
/// ```
struct VIOLET_API Child final {
    /// The platform process identifier for this child.
    struct PID PID;

    /// Write end of the pipe connected to the child's stdin, if [`Stdio::Pipe()`]
    /// was passed to [`Command::WithStdin()`]. Empty otherwise.
    Optional<ChildStdin> Stdin;

    /// Read end of the pipe connected to the child's stdout, if [`Stdio::Pipe()`]
    /// was passed to [`Command::WithStdout()`]. Empty otherwise.
    Optional<ChildStdout> Stdout;

    /// Read end of the pipe connected to the child's stderr, if [`Stdio::Pipe()`]
    /// was passed to [`Command::WithStderr()`]. Empty otherwise.
    Optional<ChildStderr> Stderr;

    /// Death timeout of when to terminate this child process.
    // TODO(@auguwu/Noel): switch to `violet::chrono::Duration` once stablized
    Optional<std::chrono::milliseconds> DeathTimeout;

    /// Constructs a `Child` from the given `pid`.
    ///
    /// This constructor is called internally by [`Command::Spawn()`] after the
    /// platform process has been created. The stream handles are populated
    /// separately once the pipe file descriptors are known.
    ///
    /// @param pid the platform process identifier of the newly spawned process.
    VIOLET_IMPLICIT Child(struct PID pid)
        : PID(pid)
    {
    }

    /// Blocks the calling thread until the child process exits, then returns
    /// its [`ExitStatus`].
    ///
    /// @returns the exit status of the child on success, or an I/O error if
    /// waiting failed (e.g., the PID is invalid or the process was already
    /// reaped).
    [[nodiscard]] auto Wait() const -> io::Result<ExitStatus>;

    /// Returns a human-readable string representation of this `Child`, suitable
    /// for logging and debugging.
    [[nodiscard]] auto ToString() const -> String;

#if VIOLET_PLATFORM(UNIX)
    /// Terminates the child process with a specific signal or `SIGKILL`.
    /// @param signal the signal to send to the child
    [[nodiscard]] auto Kill(Int32 signal = SIGKILL) const -> io::Result<void>;
#endif

    /// Writes the string representation of this `Child` to an output stream.
    friend auto operator<<(std::ostream& os, const Child& self) -> std::ostream&
    {
        return os << self.ToString();
    }
};

/// Captures the output of a [`Subprocess`].
///
/// ## Example
/// ```cpp
/// #include <violet/Subprocess.h>
///
/// violet::subprocess::Command cmd("echo");
/// cmd = cmd.WithArg("hello");
///
/// if (auto output = cmd.Output()) {
///     violet::String out(output->Stdout.begin(), output->Stdout.end());
///     std::cout << "captured stdout: " << captured << '\n';
/// }
/// ```
struct VIOLET_API Output final {
    ExitStatus Status; ///< the exit code of the subprocess
    Vec<UInt8> Stdout; ///< the standard output from the process
    Vec<UInt8> Stderr; ///< the standard error from the process
};

/// A builder for configuring and spawning a child process.
///
/// `Command` follows a fluent builder pattern: construct it with the name (or
/// path) of the program to run, chain optional configuration methods, then
/// call one of the three terminal methods to launch the process:
///
/// | Method        | Returns                   | Behaviour                                   |
/// |---------------|---------------------------|---------------------------------------------|
/// | [`Spawn()`]   | `io::Result<Child>`       | Spawn and return immediately.               |
/// | [`Status()`]  | `io::Result<ExitStatus>`  | Spawn, wait, return the exit status.        |
/// | [`Output()`]  | `io::Result<Output>`      | Spawn, wait, capture stdout/stderr.         |
///
/// By default all three standard streams inherit the parent's file
/// descriptors. Override this with [`Command::WithStdin`], [`Command::WithStdout`], and
/// [`Command::WithStderr`].
///
/// On Unix and Windows, additional extensions are available under the `violet/Subprocess/Extensions/{Unix|Windows}.h`
/// header.
///
/// ## Examples
/// ### Capture `stdout`
/// ```cpp
/// #include <violet/Subprocess.h>
/// #include <violet/Print.h>
///
/// using namespace violet::subprocess;
/// using namespace violet;
///
/// auto result = Command("git")
///     .WithArgs({"log", "--oneline", "-5"})
///     .WithStdout(Stdio::Pipe())
///     .WithStderr(Stdio::Null())
///     .Output();
///
/// if (result.Ok()) {
///     String log(result->Stdout.begin(), result->Stdout.end());
///     violet::Println("{}", log);
/// }
/// ```
///
/// ### fire-and-forget with a custom environment
/// ```cpp
/// Command("make")
///     .WithArg("install")
///     .WithEnv("DESTDIR", "/tmp/staging")
///     .WithWorkingDirectory("/home/user/myproject")
///     .Status();
/// ```
struct Command final {
    VIOLET_DISALLOW_CONSTRUCTOR(Command);

    VIOLET_IMPLICIT Command(const Command& command);
    auto operator=(const Command& command) -> Command&;

    VIOLET_IMPLICIT Command(Command&& command) noexcept;
    auto operator=(Command&& command) noexcept -> Command&;

    ~Command();

    /// Constructs a `Command` that will run `program` with no arguments.
    ///
    /// `program` may be a bare name (resolved via `PATH`) or an absolute path.
    ///
    /// @param program name or path of the executable to run.
    VIOLET_IMPLICIT Command(Str program);

    /// Constructs a `Command` pre-populated with the given argument list.
    ///
    /// Equivalent to calling [`Command::WithArgs(std::initializer_list<violet::String>())`] immediately after the
    /// single-argument constructor.
    ///
    /// @param program name or path of the executable to run.
    /// @param args    initial argument list (not including `argv[0]`).
    VIOLET_IMPLICIT Command(Str program, std::initializer_list<String> args);

    /// Constructs a `Command` pre-populated with the given argument vector.
    ///
    /// @param program name or path of the executable to run.
    /// @param args    initial argument list (not including `argv[0]`).
    VIOLET_IMPLICIT Command(Str program, Vec<String> args);

    /// Appends a single argument to the command-line.
    ///
    /// @param arg the argument string to append.
    /// @returns a reference to `*this` for method chaining.
    auto WithArg(Str arg) -> Command&;

    /// Appends multiple arguments from an initializer list.
    ///
    /// @param args arguments to append, in order.
    /// @returns a reference to `*this` for method chaining.
    auto WithArgs(std::initializer_list<String> args) -> Command&;

    /// Appends multiple arguments from a span.
    ///
    /// @param args arguments to append, in order.
    /// @returns a reference to `*this` for method chaining.
    auto WithArgs(Span<String> args) -> Command&;

    /// Inserts or overwrites a single environment variable.
    ///
    /// The child's environment starts as a copy of the parent's; this method
    /// adds or replaces one entry within that copy.
    ///
    /// @param key   environment variable name.
    /// @param value value to assign to `key`.
    /// @returns a reference to `*this` for method chaining.
    auto WithEnv(Str key, Str value) -> Command&;

    /// Inserts or overwrites multiple environment variables from an initializer list.
    ///
    /// @param envs key-value pairs to set.
    /// @returns a reference to `*this` for method chaining.
    auto WithEnvs(std::initializer_list<Pair<String, String>> envs) -> Command&;

    /// Inserts or overwrites multiple environment variables from a span.
    ///
    /// @param envs key-value pairs to set.
    /// @returns a reference to `*this` for method chaining.
    auto WithEnvs(Span<Pair<String, String>> envs) -> Command&;

    /// Sets the working directory for the child process.
    ///
    /// If not called, the child inherits the parent's current working directory.
    ///
    /// @param path directory the child should start in.
    /// @returns a reference to `*this` for method chaining.
    auto WithWorkingDirectory(filesystem::PathRef path) -> Command&;

    /// Configures how the child's stdin is connected.
    ///
    /// Defaults to [`Stdio::Inherit()`] if not called.
    ///
    /// @param cfg the [`Stdio`] configuration to apply to stdin.
    /// @returns a reference to `*this` for method chaining.
    auto WithStdin(const Stdio& cfg) -> Command&;

    /// Configures how the child's stdout is connected.
    ///
    /// Defaults to [`Stdio::Inherit()`] if not called.
    ///
    /// @param cfg the [`Stdio`] configuration to apply to stdout.
    /// @returns a reference to `*this` for method chaining.
    auto WithStdout(const Stdio& cfg) -> Command&;

    /// Configures how the child's stderr is connected.
    ///
    /// Defaults to [`Stdio::Inherit()`] if not called.
    ///
    /// @param cfg the [`Stdio`] configuration to apply to stderr.
    /// @returns a reference to `*this` for method chaining.
    auto WithStderr(const Stdio& cfg) -> Command&;

    /// Sets the maximum duration to wait for the subprocess to exit after a kill signal
    /// is sent.
    ///
    /// If the process has not exited within the given duration, it'll be forcefully terminated
    /// with `SIGKILL` on Unix or `TerminateProcess` on Windows.
    ///
    /// ## Remarks
    /// A zero-duration timeout will cause an immediate forceful kill with no grace period. If no death
    /// timeout is set, the default behaviour is to wait indefinitely for the process to exit.
    auto WithDeathTimeout(std::chrono::milliseconds ms) -> Command&;

    /// Sets the maximum duration to wait for the subprocess to exit after a kill signal
    /// is sent.
    ///
    /// If the process has not exited within the given duration, it'll be forcefully terminated
    /// with `SIGKILL` on Unix or `TerminateProcess` on Windows.
    ///
    /// ## Remarks
    /// A zero-duration timeout will cause an immediate forceful kill with no grace period. If no death
    /// timeout is set, the default behaviour is to wait indefinitely for the process to exit.
    ///
    /// @param dur timeout duration. will be converted to millisecond precision.
    template<typename Rep, typename Period>
    auto WithDeathTimeout(std::chrono::duration<Rep, Period> dur) -> Command&
    {
        return this->WithDeathTimeout(std::chrono::duration_cast<std::chrono::milliseconds>(dur));
    }

    /// Spawns the child process and collects its output.
    ///
    /// Internally this method forces both stdout and stderr to [`Stdio::Pipe()`],
    /// spawns the process, waits for it to exit, and returns all captured bytes
    /// together with the [`ExitStatus`].
    ///
    /// Any [`WithStdout()`] or [`WithStderr()`] configuration is overridden.
    ///
    /// @returns an [`Output`] containing the exit status and captured streams,
    /// or an I/O error if the process could not be spawned or waited on.
    [[nodiscard]] auto Output() -> io::Result<Output>;

    /// Spawns the child process, waits for it to finish, and returns its exit status.
    ///
    /// Equivalent to `Spawn()` followed by `Child::Wait()`, but discards the
    /// [`Child`] handle.
    ///
    /// @returns the [`ExitStatus`] of the finished process, or an I/O error.
    [[nodiscard]] auto Status() -> io::Result<ExitStatus>;

    /// Spawns the child process and returns a [`Child`] handle immediately.
    ///
    /// The child runs concurrently with the parent. Call [`Child::Wait()`] to
    /// block until the child exits and reap its exit status.
    ///
    /// @returns a [`Child`] representing the running process, or an I/O error
    /// if the process could not be created (e.g., executable not found, permission
    /// denied, resource limits exceeded).
    [[nodiscard]] auto Spawn() -> io::Result<Child>;

private:
    struct Impl;

    friend void violet::subprocess::ext::UID(violet::subprocess::Command&, uid_t);
    friend void violet::subprocess::ext::GID(violet::subprocess::Command&, gid_t);
    friend void violet::subprocess::ext::Groups(Command&, std::initializer_list<gid_t>);
    friend void violet::subprocess::ext::Groups(Command&, Span<gid_t>);
    friend void violet::subprocess::ext::PreExec(Command& command, ext::PreExecFun exec);

    friend auto violet::subprocess::detail::SpawnAsForkExec(Command&) -> violet::io::Result<Child>;

#if VIOLET_PLATFORM(APPLE_MACOS)
    friend auto violet::subprocess::detail::SpawnWithPosix(Command&) -> violet::io::Result<Child>;
#endif

    // TODO(@auguwu/Noel): switch to `Own<Impl>` once stablized
    Impl* n_impl = nullptr;
};

} // namespace violet::subprocess
