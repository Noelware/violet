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
//
//! # 🌺💜 `violet/Subprocess.h`
//! This header contains the core types and definitions for the `Noelware.Violet.Subprocess` C++20
//! library, a cross-platform way of spawning processes.
//!
//! The API and design of this is directly inspired by Rust's [`std::process`] module.
//! [`std::process`]: https://doc.rust-lang.org/1.91.1/std/process/index.html

#pragma once

#include <violet/Filesystem/Path.h>
#include <violet/IO/Descriptor.h>
#include <violet/Violet.h>

#ifdef VIOLET_WINDOWS
#include <windows.h>
#endif

namespace violet::process {

struct ExitStatus;

struct Stdio final {
    constexpr VIOLET_IMPLICIT Stdio() noexcept = delete;

    constexpr static auto Inherit() noexcept -> Stdio
    {
        return Stdio(tag_t::kInherit);
    }

    constexpr static auto Null() noexcept -> Stdio
    {
        return Stdio(tag_t::kNull);
    }

    constexpr static auto Pipe(Optional<filesystem::PathRef> path = {}) noexcept -> Stdio
    {
        Stdio stdio = Stdio(tag_t::kPipe);
        if (path) {
            stdio.n_pipeTo = *path.Value();
        }

        return stdio;
    }

    constexpr auto IsInherited() const noexcept -> bool
    {
        return this->n_tag == tag_t::kInherit;
    }

    constexpr auto IsNull() const noexcept -> bool
    {
        return this->n_tag == tag_t::kNull;
    }

    constexpr auto IsPipe() const noexcept -> bool
    {
        return this->n_tag == tag_t::kPipe;
    }

    constexpr auto PipedToFile() const noexcept -> bool
    {
        return this->IsPipe() && this->n_pipeTo.HasValue();
    }

    constexpr auto PipedFile() const noexcept -> Optional<filesystem::Path>
    {
        return this->n_pipeTo;
    }

private:
    enum struct tag_t : UInt8 {
        kInherit,
        kNull,
        kPipe
    };

    constexpr VIOLET_EXPLICIT Stdio(tag_t tag)
        : n_tag(tag)
    {
    }

    tag_t n_tag;
    Optional<filesystem::Path> n_pipeTo = Nothing;
};

struct ChildStdin final {
    constexpr VIOLET_IMPLICIT ChildStdin() noexcept = delete;
    constexpr VIOLET_EXPLICIT ChildStdin(io::FileDescriptor::value_type fd)
        : n_descriptor(fd)
    {
    }

    [[nodiscard]] auto Write(Span<const UInt8> buf) const noexcept -> io::Result<UInt>
    {
        return this->n_descriptor.Write(buf);
    }

    [[nodiscard]] auto Flush() const noexcept -> io::Result<void>
    {
        return this->n_descriptor.Flush();
    }

    void Close()
    {
        this->n_descriptor.Close();
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return std::format("ChildStdin(fd={})", this->n_descriptor);
    }

    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

private:
    io::FileDescriptor n_descriptor;
};

static_assert(io::Writable<ChildStdin>, "`ChildStdin` doesn't conform to io::Writable concept");

struct ChildStdout final {
    constexpr VIOLET_IMPLICIT ChildStdout() noexcept = delete;
    constexpr VIOLET_EXPLICIT ChildStdout(io::FileDescriptor::value_type fd)
        : n_descriptor(fd)
    {
    }

    [[nodiscard]] auto Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>
    {
        return this->n_descriptor.Read(buf);
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return std::format("ChildStdout(fd={})", this->n_descriptor);
    }

    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

private:
    io::FileDescriptor n_descriptor;
};

static_assert(io::Readable<ChildStdout>, "`ChildStdout` doesn't conform to io::Readable concept");

struct ChildStderr final {
    constexpr VIOLET_IMPLICIT ChildStderr() noexcept = delete;
    constexpr VIOLET_EXPLICIT ChildStderr(io::FileDescriptor::value_type fd)
        : n_descriptor(fd)
    {
    }

    [[nodiscard]] auto Read(Span<UInt8> buf) const noexcept -> io::Result<UInt>
    {
        return this->n_descriptor.Read(buf);
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return std::format("ChildStderr(fd={})", this->n_descriptor);
    }

    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

private:
    io::FileDescriptor n_descriptor;
};

static_assert(io::Readable<ChildStderr>, "`ChildStderr` doesn't conform to io::Readable concept");

struct Child final {
#ifdef VIOLET_UNIX
    using pid_t = ::pid_t;
#elif defined(VIOLET_WINDOWS)
    using pid_t = DWORD;
#else
    using pid_t = void*;
#endif

    Optional<ChildStdin> Stdin;
    Optional<ChildStdout> Stdout;
    Optional<ChildStderr> Stderr;
    pid_t ProcessID;

    auto Wait() const noexcept -> io::Result<ExitStatus>;

    auto ToString() const noexcept -> String
    {
        return std::format("Child(pid={})", this->ProcessID);
    }

    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }
};

/// Representation of a subprocess' exit status.
///
/// `ExitStatus` is a tiny abstraction that encapsulates the return code
/// of a spawned [`Subprocess`] and provides convenient methods for checking
/// the state.
///
/// ## Example
/// ```cpp
/// #include <violet/Subprocess.h>
///
/// violet::Command cmd("true");
/// if (auto status = cmd.Status(); status.Ok() && status.Value().Success()) {
///     std::cout << "process exited successfully!\n";
/// }
/// ```
struct ExitStatus final {
#ifdef VIOLET_WINDOWS
    using value_type = DWORD;
#else
    using value_type = Int32;
#endif

    constexpr VIOLET_IMPLICIT ExitStatus() noexcept = default;
    constexpr VIOLET_IMPLICIT ExitStatus(value_type value) noexcept
        : n_value(value)
    {
    }

    [[nodiscard]] auto Success() const noexcept -> bool;
    [[nodiscard]] auto Code() const noexcept -> Optional<value_type>;

#ifdef VIOLET_UNIX
    [[nodiscard]] auto Signal() const noexcept -> Optional<value_type>;
#endif

    /// Returns a debuggable, stringified representation of this object.
    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return std::format("ExitStatus(code={})", this->n_value);
    }

    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->Success();
    }

    constexpr VIOLET_EXPLICIT operator value_type() const noexcept
    {
        return this->n_value;
    }

private:
    value_type n_value = value_type(-1); // NOLINT(google-readability-casting)
};

/// Captures the output of a [`Subprocess`].
///
/// ## Example
/// ```cpp
/// #include <violet/Subprocess.h>
///
/// violet::Command cmd("echo");
/// cmd.WithArg("hello");
///
/// if (auto output = cmd.Output()) {
///     violet::String out(output->Stdout.begin(), output->Stdout.end());
///     std::cout << "captured stdout: " << captured << '\n';
/// }
/// ```
struct Output final {
    ExitStatus Status; ///< the exit code of the subprocess
    Vec<UInt8> Stdout; ///< the standard output from the process
    Vec<UInt8> Stderr; ///< the standard error from the process
};

struct Command final {
#ifdef VIOLET_UNIX
    using pre_exec_t = std::function<void()>;
#endif

    constexpr VIOLET_IMPLICIT Command() noexcept = delete;

    VIOLET_IMPLICIT Command(Str command) noexcept;
    VIOLET_IMPLICIT Command(Str command, std::initializer_list<String> args) noexcept;
    VIOLET_IMPLICIT Command(Str command, Span<String> args) noexcept;

    auto WithArg(Str arg) noexcept -> Command&;
    auto WithArgs(std::initializer_list<String> args) noexcept -> Command&;
    auto WithArgs(Span<String> args) noexcept -> Command&;
    auto WithEnv(Str key, Str value) noexcept -> Command&;
    auto WithEnvs(std::initializer_list<Pair<String, String>> envs) noexcept -> Command&;
    auto WithEnvs(Span<Pair<String, String>> envs) noexcept -> Command&;
    auto WithWorkingDirectory(filesystem::PathRef path) noexcept -> Command&;
    auto WithStdin(Stdio cfg) noexcept -> Command&;
    auto WithStdout(Stdio cfg) noexcept -> Command&;
    auto WithStderr(Stdio cfg) noexcept -> Command&;

#ifdef VIOLET_UNIX
    auto WithUID(uid_t uid) noexcept -> Command&;
    auto WithGID(gid_t gid) noexcept -> Command&;
    auto WithGroups(std::initializer_list<gid_t> groups) noexcept -> Command&;
    auto WithGroups(Span<gid_t> groups) noexcept -> Command&;
    auto WithPreExec(pre_exec_t exec) noexcept -> Command&;
#endif

    [[nodiscard]] auto Output() const noexcept -> io::Result<Output>;
    [[nodiscard]] auto Status() const noexcept -> io::Result<ExitStatus>;
    [[nodiscard]] auto Spawn() const noexcept -> io::Result<Child>;

private:
    struct Impl;

    Impl* n_impl;
};

} // namespace violet::process
