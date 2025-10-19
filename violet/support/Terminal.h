// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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
//! # 🌺💜 `violet/support/Terminal.h`
//! This header library provides support for terminal-related utilities like ANSI colors,
//! terminal width on Unix and Windows, and much more. This is inspired by the [`owo-colors`]
//! and [`supports-color`] Rust crates.
//!
//! [`supports-color`]: https://crates.io/crates/supports-color
//! [`owo-colors`]: https://crates.io/crates/owo-colors

#pragma once

#include "violet/container/Optional.h"
#include "violet/io/Error.h"
#include "violet/support/StringRef.h"
#include "violet/sys/Environment.h"
#include "violet/violet.h"
#include <stdexcept>
#include <string>

namespace Noelware::Violet {

/// Possible stream source to retrieve information about the terminal.
enum struct TerminalStreamSource : uint8 {
    Stdout = 0, ///< uses stdout's file descriptor.
    Stderr = 1 ///< uses stderr's file descriptor.
};

struct ColorLevel final {
    bool SupportsBasic = false; ///< provides basic support for ANSI colours
    bool Supports256Bit = false; ///< provides support for 256-bit ANSI colours
    bool Supports16M = false; ///< provides support for 16-million (RGB) ANSI colours

    /// Constructs this object with default values.
    constexpr ColorLevel() = default;

    /// Translation operator from **ColorLevel** -> `bool`
    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->SupportsBasic;
    }
};

namespace __detail {
    inline auto getEnvForceColourLevel() noexcept -> usize
    {
        if (auto force = System::GetEnvironmentVariable("FORCE_COLOR")) {
            if (force->empty() || *force == "") {
                return 1;
            }

            if (*force == "false") {
                return 0;
            }

            usize level = 1;
            try {
                level = static_cast<usize>(std::stoi(*force));
            } catch (const std::invalid_argument&) {
                level = 1;
            } catch (const std::out_of_range& ex) {
                level = 1;
            }

            return std::min(level, static_cast<usize>(3));
        }

        if (auto force = System::GetEnvironmentVariable("CLICOLOR_FORCE")) {
            return *force != "0" ? 1 : 0;
        }

        return 0;
    }

    inline auto isNoColour() -> bool
    {
        auto env = System::GetEnvironmentVariable("NO_COLOR");
        return !(env == Nothing || *env == "0");
    }

    inline auto getTermEnv() -> Optional<String>
    {
        return System::GetEnvironmentVariable("TERM");
    }

    inline auto getTerminalProgram() -> Optional<String>
    {
        return System::GetEnvironmentVariable("TERM_PROGRAM");
    }

    inline auto checkColorterm16m(const String& value) -> bool
    {
        return value == "truecolor" || value == "24bit";
    }

    inline auto checkTerm16m(const String& value) -> bool
    {
        return value.ends_with("direct") || value.ends_with("truecolor");
    }

    inline auto check256BitColor(const String& value)
    {
        return value.ends_with("256") || value.ends_with("256color");
    }

    inline auto checkPlatformANSIColor(const String& value) -> bool
    {
#ifdef VIOLET_WINDOWS
        return value != "dumb";
#else
        return value != "dumb";
#endif
    }
} // namespace __detail

/// A descriptor about a terminal's window.
struct TermWindow final {
    /// Number specifying the number of columns this TTY has.
    uint16 Columns = 0;

    /// Number specifying the number of rows this TTY has.
    uint16 Rows = 0;
};

struct Terminal final {
    /// Returns **true** if we are in a terminal process instead of pipe or a child process.
    /// @param source the source to check if we are a TTY.
    /// @returns a boolean to indicate if we are a TTY process.
    static auto TTY(TerminalStreamSource source = TerminalStreamSource::Stdout) noexcept -> bool;

    /// Returns information about the terminal's window if we are in a TTY.
    /// @param source the source to check about the terminal's window.
    /// @returns a I/o result about the window descriptor.
    static auto Window(TerminalStreamSource source = TerminalStreamSource::Stdout) noexcept -> IO::Result<TermWindow>;

    /// Returns the information about the current stream source's color level.
    static auto ColorLevel(TerminalStreamSource source = TerminalStreamSource::Stdout) noexcept -> Violet::ColorLevel
    {
        usize level = 0;
        if (auto forceColor = __detail::getEnvForceColourLevel(); forceColor > 0) {
            Violet::ColorLevel cl;
            cl.SupportsBasic = level >= 1;
            cl.Supports256Bit = level >= 2;
            cl.Supports16M = level >= 3;

            return cl;
        }

        if (__detail::isNoColour() || __detail::getTermEnv() == "dumb" || !Terminal::TTY(source)) {
            return {};
        }

        if (System::GetEnvironmentVariable("COLORTERM").Map(__detail::checkColorterm16m)
            || __detail::getTermEnv().Map(__detail::checkTerm16m) || __detail::getTerminalProgram() == "iTerm.app") {
            level = 3;
        }

        if (__detail::getTerminalProgram() == "Apple_Terminal"
            || __detail::getTermEnv().Map(__detail::check256BitColor)) {
            level = 2;
        }

        if (System::GetEnvironmentVariable("COLORTERM") || __detail::getTermEnv().Map(__detail::checkPlatformANSIColor)
            || System::GetEnvironmentVariable("CLICOLOR").MapOr(false, [](const String& value) { return value != "0"; })

            // TODO(@auguwu): better CI detection
            || System::GetEnvironmentVariable("CI")) {
            level = 1;
        }

        Violet::ColorLevel cl;
        cl.SupportsBasic = level >= 1;
        cl.Supports256Bit = level >= 2;
        cl.Supports16M = level >= 3;

        return cl;
    }
};

} // namespace Noelware::Violet
