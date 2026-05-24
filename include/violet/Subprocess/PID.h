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

/// Represents a process identifier (PID) in a cross-platform manner.
///
/// On Unix systems, `value_type` is `pid_t`. On Windows, `value_type` is a `HANDLE`
/// to the process, since Windows does not use integer PIDs in the same way Unix does.
///
/// `PID` is implicitly constructible from any type convertible to `value_type`, making
/// it easy to wrap raw platform handles or IDs without explicit casting.
///
/// # Examples
///
/// ```cpp
/// auto pid = violet::subprocess::PID::Current();
/// std::cout << "Current PID: " << pid << '\n';
///
/// if (auto parent = violet::subprocess::PID::Parent()) {
///     std::cout << "Parent PID: " << *parent << '\n';
/// }
/// ```
struct VIOLET_API PID final {
#if VIOLET_PLATFORM(UNIX)
    using value_type = pid_t;
#elif VIOLET_PLATFORM(WINDOWS)
    using value_type = HANDLE;
#endif

    /// Returns the `PID` of the currently running process.
    static auto Current() noexcept -> PID;

    /// Returns the `PID` of the parent process, if one exists.
    ///
    /// Returns an empty `Optional` on platforms or in contexts where a parent
    /// process cannot be determined (e.g., init/PID 1 on Unix).
    static auto Parent() noexcept -> Optional<PID>;

    constexpr VIOLET_DISALLOW_CONSTRUCTOR(PID);
    VIOLET_IMPLICIT_CONSTEXPR_COPY_AND_MOVE(PID);
    ~PID() = default;

    /// Constructs a `PID` from any value implicitly convertible to `value_type`.
    ///
    /// @param id raw platform process identifier or handle to wrap.
    template<std::convertible_to<value_type> ID>
    constexpr VIOLET_IMPLICIT PID(ID&& id) noexcept
        : n_value(static_cast<value_type>(VIOLET_FWD(ID, id)))
    {
    }

    /// Returns the underlying platform-native process identifier or handle.
    [[nodiscard]] constexpr auto Get() const noexcept -> value_type
    {
        return this->n_value;
    }

    /// Returns a string representation of this `PID`.
    [[nodiscard]] auto ToString() const noexcept -> String;

    /// Writes the string representation of this `PID` to an output stream.
    friend auto operator<<(std::ostream& os, const PID& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

    /// Returns `true` if this `PID` refers to a valid process.
    ///
    /// On Unix, a PID of `0` or negative is considered invalid. On Windows,
    /// a `NULL` or `INVALID_HANDLE_VALUE` handle is considered invalid.
    VIOLET_EXPLICIT operator bool() const noexcept;

    /// Returns the underlying `value_type` for this `PID`.
    VIOLET_EXPLICIT operator value_type() const noexcept;

    auto operator<=>(const PID& other) const noexcept -> std::strong_ordering;
    auto operator<=>(value_type other) const noexcept -> std::strong_ordering;

    auto operator==(const PID& other) const noexcept -> bool;
    auto operator!=(const PID& other) const noexcept -> bool;

    auto operator==(value_type other) const noexcept -> bool;
    auto operator!=(value_type other) const noexcept -> bool;

private:
    value_type n_value;
};

} // namespace violet::subprocess

VIOLET_FORMATTER(violet::subprocess::PID);
