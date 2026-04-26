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
//! # 🌺💜 `violet/Language/Panic.h`

#include <violet/SourceLocation.h>

#include <cstdint>
#include <string_view>

namespace violet::panic {

/// The strategy to use when a panic occurs within the Noelware.Violet frameworks.
enum struct Strategy : uint8_t {
    /// Calls [`std::terminate()`], which may invoke [`std::terminate_handler`] and potentionally
    /// unwind or print diagnostics depending on the runtime.
    Unwind,

    /// Calls [`std::abort()`] immediately. Generates a core dump on most Unix systems. No cleanup,
    /// no unwinding.
    Abort
};

/// Information about a panic site, passed to panic hooks.
struct VIOLET_API Info final {
    /// The message that was sent through.
    std::string_view Message;

    /// The callsite location on where this panic happened.
    SourceLocation Location;

    /// If not null, this is the callsite location on where, in userland code,
    /// the panic occured from [`Location`].
    SourceLocation* Userland;
};

/// A user-installable hook that runs before the process terminates.
///
/// Useful for flushing logs, printing backtraces, notifying crash reporters,
/// etc. The hook MUST not throw exceptions.
using Hook = void (*)(const Info&);

/// Installs Violet's default panic hook.
VIOLET_API void InstallDefaultPanicHook() noexcept;

/// Sets a global panic hook. Returns the previously installed hook (or nullptr if
/// none was ever installed).
///
/// This is not thread-safe. Call once during initialization.
VIOLET_API auto SetHook(Hook hook) noexcept -> Hook;

/// Set the global panic strategy.
VIOLET_API void SetStrategy(Strategy strategy) noexcept;

} // namespace violet::panic

namespace violet::detail {

VIOLET_API [[noreturn]] VIOLET_COLD void DoPanic(
    std::string_view message, SourceLocation* userland = nullptr, SourceLocation loc = std::source_location::current());

}

/**
 * @macro VIOLET_PANIC
 *
 * Calls into Violet's panic system with a message.
 */
#define VIOLET_PANIC(message) ::violet::detail::DoPanic(message, nullptr, ::std::source_location::current())

/**
 * @macro VIOLET_PANIC_FMT
 *
 * Calls into Violet's panic system with a formatted message.
 */
#define VIOLET_PANIC_FMT(fmt, ...)                                                                                     \
    ::violet::detail::DoPanic(std::format(fmt __VA_OPT__(, ) __VA_ARGS__), nullptr, std::source_location::current())

/**
 * @macro VIOLET_PANIC_USERLAND
 *
 * Calls into Violet's panic system with userland-callsite location and a message.
 */
#define VIOLET_PANIC_USERLAND(message, userland)                                                                       \
    ::violet::detail::DoPanic(message, (&userland), ::std::source_location::current())

/**
 * @macro VIOLET_PANIC_USERLAND_FMT
 *
 * Calls into Violet's panic system with userland-callsite location and a formatted message.
 */
#define VIOLET_PANIC_USERLAND_FMT(userland, fmt, ...)                                                                  \
    ::violet::detail::DoPanic(std::format(fmt __VA_OPT__(, ) __VA_ARGS__), (&userland), std::source_location::current())
