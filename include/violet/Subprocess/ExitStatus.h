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

#if VIOLET_PLATFORM(WINDOWS)
#include <windows.h>
#endif

namespace violet::subprocess {

/// Represents the termination status of a spawned process.
///
/// `ExitStatus` models how a subprocess finished execution in a
/// platform-aware manner.
struct VIOLET_API ExitStatus final {
#if VIOLET_PLATFORM(UNIX)
    /// Underlying integer type used to represent exit codes and signals.
    ///
    /// On Unix this is `Int32`.
    using value_type = Int32;
#elif VIOLET_PLATFORM(WINDOWS)
    /// Underlying integer type used to represent exit codes.
    ///
    /// On Windows this is `DWORD`.
    using value_type = DWORD;
#endif

    constexpr VIOLET_IMPLICIT ExitStatus() noexcept = default;

    template<std::convertible_to<value_type> Status>
    constexpr VIOLET_IMPLICIT ExitStatus(Status&& status) noexcept
        : n_code(VIOLET_FWD(Status, status))
    {
    }

    /// Returns `true` if the process exited normally.
    ///
    /// ## Platform Semantics
    /// ### Unix
    /// Corresponds to `WIFEXITED`.
    ///
    /// ### Windows
    /// This is always `true` for a completed process.
    [[nodiscard]] auto Exited() const noexcept -> bool;

#if VIOLET_PLATFORM(UNIX)
    /// Returns `true` if the process was terminated by a signal.
    ///
    /// Corresponds to `WIFSIGNALED`.
    [[nodiscard]] auto Signaled() const noexcept -> bool;

    /// Returns `true` if the process is currently stopped.
    ///
    /// Corresponds to `WIFSTOPPED`.
    [[nodiscard]] auto Stopped() const noexcept -> bool;

    /// Returns `true` if the process produced a core dump.
    ///
    /// Only meaningful when [`ExitProcess::Signaled`] returns `true`.
    [[nodiscard]] auto CoreDumped() const noexcept -> bool;

    /// Returns the stored signal number.
    ///
    /// If the process exited normally, the returned value is unspecified.
    [[nodiscard]] auto Signal() const noexcept -> Optional<value_type>;
#endif

    /// Returns the stored exit code.
    ///
    /// If the process did not exit normally, the returned value
    /// is unspecified.
    [[nodiscard]] auto Code() const noexcept -> Optional<value_type>;

    /// Returns the native, raw type as [`Code`] and [`Signal`] will provide
    /// conversions on Unix platforms.
    [[nodiscard]] constexpr auto AsNative() const noexcept -> value_type
    {
        return this->n_code;
    }

    [[nodiscard]] auto ToString() const noexcept -> String;
    friend auto operator<<(std::ostream& os, const ExitStatus& self) -> std::ostream&
    {
        return os << self.ToString();
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return !this->Exited();
    }

    constexpr VIOLET_EXPLICIT operator value_type() const noexcept
    {
        return this->n_code;
    }

    auto operator==(const ExitStatus& other) const noexcept -> bool;
    auto operator!=(const ExitStatus& other) const noexcept -> bool;

    auto operator==(value_type other) const noexcept -> bool;
    auto operator!=(value_type other) const noexcept -> bool;

private:
    value_type n_code;
};

} // namespace violet::subprocess

VIOLET_FORMATTER(violet::subprocess::ExitStatus);
