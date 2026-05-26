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

#include <violet/Violet.h>

#include <functional>
#include <initializer_list>
#include <sys/types.h>

namespace violet::subprocess {
struct Command;
}

namespace violet::subprocess::ext {

/// A callback invoked in the child process after `fork()` but before `exec()`.
///
/// `PreExecFun` is a Unix-only hook that runs inside the forked child between
/// the two system calls. At the point the callback is invoked the child is a
/// copy of the parent, all file descriptors, signal masks, and memory mappings
/// are inherited, but no threads other than the calling thread exist. The
/// callback must be **async-signal-safe**: it must not call functions that
/// acquire locks (e.g. `malloc`, `printf`, most C++ standard library functions)
/// because those locks may have been held by other threads in the parent at the
/// moment of the fork.
///
/// Typical uses include:
/// - Adjusting file-descriptor flags with [`fcntl(2)`] before the new image
///   takes over.
/// - Installing a different signal handler or resetting the signal mask.
/// - Dropping Linux capabilities or setting a seccomp filter before `exec`.
///
/// [`fcntl(2)`]: https://www.man7.org/linux/man-pages/man2/fcntl.2.html
///
/// ## Safety
/// Calling async-signal-unsafe functions from inside a `PreExecFun` produces
/// undefined behaviour. Keep the body minimal and restrict it to raw POSIX
/// syscalls when possible.
///
/// ## Example
/// ```cpp
/// #include <violet/Subprocess/Extensions/Unix.h>
/// #include <violet/Subprocess.h>
/// #include <sys/prctl.h>
///
/// auto command = violet::subprocess::Command("my-tool");
/// violet::subprocess::ext::PreExec(command, [] -> void {
///     // Drop all supplementary groups before exec.
///     ::setgroups(0, nullptr);
///
///     // Die if parent dies.
///     ::prctl(PR_SET_PDEATHSIG, SIGTERM);
/// });
///
/// auto child = command.Spawn();
/// ```
struct VIOLET_API NOELDOC_SINCE("26.07") PreExecFun final {
    VIOLET_DISALLOW_CONSTRUCTOR(PreExecFun);

    template<typename Fun>
        requires(callable<Fun> && callable_returns<Fun, void>)
    VIOLET_IMPLICIT PreExecFun(Fun&& fun)
        : n_fun(VIOLET_FWD(Fun, fun))
    {
    }

    void operator()()
    {
        std::invoke(this->n_fun);
    }

private:
    std::function<void()> n_fun;
};

// using PreExecFun = std::function<void()>;

/// Sets the user identity the child process will run as.
///
/// Calls [`setuid(2)`] in the child after [`fork()`] and before [`exec()`]. The
/// calling process must have sufficient privilege (typically `CAP_SETUID`)
/// to change the UID; otherwise the spawn will fail.
///
/// [`setuid(2)`]: https://www.man7.org/linux/man-pages/man2/setuid.2.html
/// [`fork()`]: https://man7.org/linux/man-pages/man2/fork.2.html
/// [`exec()`]: https://man7.org/linux/man-pages/man3/exec.3.html
///
/// @param command the command to use
/// @param uid     the POSIX user identifier to switch to
NOELDOC_SINCE("26.07") VIOLET_API void UID(Command& command, uid_t uid);

/// Sets the primary group identity the child process will run as.
///
/// Calls [`setgid(2)`] in the child after `fork()` and before `exec()`. The
/// calling process must have sufficient privilege (typically `CAP_SETGID`)
/// to change the GID.
///
/// [`setgid(2)`]: https://www.man7.org/linux/man-pages/man2/setgid.2.html
/// [`fork()`]: https://man7.org/linux/man-pages/man2/fork.2.html
/// [`exec()`]: https://man7.org/linux/man-pages/man3/exec.3.html
///
/// @param gid the POSIX group identifier to switch to.
NOELDOC_SINCE("26.07") VIOLET_API void GID(Command& command, gid_t gid);

/// Replaces the child's supplementary group list.
///
/// Calls `setgroups(2)` in the child after `fork()` and before `exec()`.
/// Requires `CAP_SETGID` in the calling process.
///
/// [`setgroups(2)`]: https://www.man7.org/linux/man-pages/man2/setgroups.2.html
/// [`fork()`]: https://man7.org/linux/man-pages/man2/fork.2.html
/// [`exec()`]: https://man7.org/linux/man-pages/man3/exec.3.html
///
/// @param groups supplementary group identifiers to set.
NOELDOC_SINCE("26.07") VIOLET_API void Groups(Command& command, std::initializer_list<gid_t> groups);

/// Replaces the child's supplementary group list from a span.
///
/// @param groups supplementary group identifiers to set.
NOELDOC_SINCE("26.07") VIOLET_API void Groups(Command& command, Span<gid_t> groups);

/// Registers a [`PreExecFun`] callback to run after `fork()` but before `exec()`.
///
/// Only one pre-exec callback can be registered; calling this method a
/// second time replaces the previous one. See [`PreExecFun`] for safety
/// requirements and usage examples.
///
/// @param exec the callback to invoke in the child process.
NOELDOC_SINCE("26.07") VIOLET_API void PreExec(Command& command, PreExecFun exec);

} // namespace violet::subprocess::ext
