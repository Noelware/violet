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

#include "violet/io/Error.h"
#include "violet/support/Bitflags.h"
#include "violet/support/StringRef.h"
#include "violet/violet.h"

#include <ostream>
#include <sstream>

#ifdef VIOLET_WINDOWS
#    include <windows.h>
#endif

namespace Noelware::Violet::Term {

/// For CLI programs, this enumeration allows to configure colours for users. This is similar to Rust-like CLI programs
/// that have the `--color` CLI flag, like `ripgrep` and `cargo`.
enum struct ColorChoice : uint8 {
    Always = 0, ///< `--color=always`, `{CLI}_COLOR=always`
    Never = 1, ///< `--color=never`, `{CLI}_COLOR=never`
    Auto = 2 ///< `--color=auto`, auto detection
};

/// Possible stream source to retrieve information about the terminal.
enum struct StreamSource : uint8 {
    Stdout = 0, ///< uses stdout's file descriptor.
    Stderr = 1 ///< uses stderr's file descriptor.
};

struct ColorLevel final {
    bool SupportsBasic = false; ///< provides basic support for ANSI colours
    bool Supports256Bit = false; ///< provides support for 256-bit ANSI colours
    bool Supports16M = false; ///< provides support for 16-million (RGB) ANSI colours
};

/// Instance of a RGB-based colour that is supported on truecolor terminals.
struct RGB final {
    /// red variant
    uint8 Red;

    /// green variant
    uint8 Green;

    /// blue variant
    uint8 Blue;

    /// whether if the color is for the foreground or background
    bool Foreground;

    constexpr RGB() noexcept = default;
    constexpr RGB(uint8, uint8, uint8, bool foreground = true) noexcept;

    constexpr auto Paint() const noexcept -> String;
    auto ToString() const noexcept -> String;

    // clang-format off
    VIOLET_IMPL_EQUALITY_SINGLE(
        RGB,
        lhs,
        rhs,
        {
            return lhs.Red == rhs.Red && lhs.Green == rhs.Green && lhs.Blue == rhs.Blue;
        }
    );
    // clang-format on
};

/// Represents a style that can appear in a terminal.
struct Style final {
    constexpr Style() = default;

    static constexpr auto Black(bool foreground = true) noexcept -> Style;
    static constexpr auto Red(bool foreground = true) noexcept -> Style;
    static constexpr auto Green(bool foreground = true) noexcept -> Style;
    static constexpr auto Yellow(bool foreground = true) noexcept -> Style;
    static constexpr auto Blue(bool foreground = true) noexcept -> Style;
    static constexpr auto Magenta(bool foreground = true) noexcept -> Style;
    static constexpr auto Cyan(bool foreground = true) noexcept -> Style;
    static constexpr auto White(bool foreground = true) noexcept -> Style;
    static constexpr auto BrightBlack(bool foreground = true) noexcept -> Style;
    static constexpr auto BrightRed(bool foreground = true) noexcept -> Style;
    static constexpr auto BrightGreen(bool foreground = true) noexcept -> Style;
    static constexpr auto BrightYellow(bool foreground = true) noexcept -> Style;
    static constexpr auto BrightBlue(bool foreground = true) noexcept -> Style;
    static constexpr auto BrightMagenta(bool foreground = true) noexcept -> Style;
    static constexpr auto BrightCyan(bool foreground = true) noexcept -> Style;
    static constexpr auto BrightWhite(bool foreground = true) noexcept -> Style;

    template<uint8 R, uint8 G, uint8 B>
    static constexpr auto RGB(bool foreground = true) noexcept -> Style;

    template<usize N>
    static constexpr auto HexRGB(const char (&str)[N], bool foreground = true) -> Style;
    static constexpr auto HexRGB(StringRef, bool foreground = true) noexcept -> Style;

    constexpr auto Bold() noexcept -> Style&;
    constexpr auto Italic() noexcept -> Style&;
    constexpr auto Dim() noexcept -> Style&;
    constexpr auto Underline() noexcept -> Style&;
    constexpr auto Inverse() noexcept -> Style&;
    constexpr auto Strikethrough() noexcept -> Style&;

    constexpr auto Paint() const noexcept -> String;
    auto ToString() const noexcept -> String;

    auto operator<<(std::ostream&) const noexcept -> std::ostream&;

private:
    friend struct Color;

    enum struct style : uint8 {
        kBold = 1 << 0,
        kItalic = 1 << 1,
        kDim = 1 << 2,
        kUnderline = 1 << 3,
        kInverse = 1 << 4,
        kStrikethrough = 1 << 5,
    };

    struct bg {
        uint8 value;
    };

    struct fg {
        uint8 value;
    };

    std::variant<std::monostate, bg, fg, struct RGB> n_variant;
    Bitflags<style> n_style{};

    auto getStringStream() const noexcept -> std::ostringstream
    {
        std::ostringstream os;
        if (this->n_style.Contains(style::kBold)) {
            os << "\x1b[1m";
        }

        if (this->n_style.Contains(style::kDim)) {
            os << "\x1b[2m";
        }

        if (this->n_style.Contains(style::kItalic)) {
            os << "\x1b[3m";
        }

        if (this->n_style.Contains(style::kUnderline)) {
            os << "\x1b[4m";
        }

        if (this->n_style.Contains(style::kInverse)) {
            os << "\x1b[7m";
        }

        if (this->n_style.Contains(style::kStrikethrough)) {
            os << "\x1b[9m";
        }

        return os;
    }
};

template<typename T>
struct Styled final {
    Styled() = delete;

    VIOLET_EXPLICIT Styled(T target, Style style);

    auto Paint() const noexcept -> String;
    auto ToString() const noexcept -> String;

    auto operator<<(std::ostream&) const noexcept -> std::ostream&;

private:
    T n_target;
    Style n_style;
};

/// Information about a terminal's window.
struct Window final {
    /// Number specifying the number of columns this TTY has.
    uint16 Columns = 0;

    /// Number specifying the number of rows this TTY has.
    uint16 Rows = 0;
};

#ifdef VIOLET_WINDOWS
/// On Windows, this will enable virtual terminal processing mode for terminals that don't support
/// ANSI colour output via SGR.
///
/// This will use the [`GetStdHandle`] for either the standard input or error streams denoted by the `source`
/// property, get the console mode from the handle via [`GetConsoleMode`] and sets the mode to
/// `ENABLE_VIRTUAL_TERMINAL_PROCESSING` with [`SetConsoleMode`].
///
/// At any point, this *could* fail for any reason, which is why it returns a [`IO::Result`] so you can
/// check the last error that was set.
///
/// @param source the source to enable virtual terminal processing
/// @return the result of the operation, always check the error state before continuing.
[[nodiscard("always check the error state")]] auto EnableVTMode(StreamSource source = StreamSource::Stdout)
    -> IO::Result<void>;
#endif

} // namespace Noelware::Violet::Term

namespace Noelware::Violet {

struct Terminal final {
    Terminal() = delete;

    /// Returns **true** if we are in a terminal process instead of pipe or a child process.
    /// @param source the source to check if we are a TTY.
    /// @returns a boolean to indicate if we are a TTY process.
    static auto TTY(Term::StreamSource source = Term::StreamSource::Stdout) noexcept -> bool;

    /// Returns information about the terminal's window if we are in a TTY.
    /// @param source the source to check about the terminal's window.
    /// @returns a I/o result about the window descriptor.
    static auto Window(Term::StreamSource source = Term::StreamSource::Stdout) noexcept -> IO::Result<Term::Window>;

    /// Returns the information about the current stream source's color level.
    static auto ColorLevel(Term::StreamSource source = Term::StreamSource::Stdout) noexcept -> Term::ColorLevel;
};

} // namespace Noelware::Violet

VIOLET_TO_STRING(const Term::ColorChoice&, choice, {
    switch (choice) {
    case Noelware::Violet::Term::ColorChoice::Always:
        return "always";

    case Noelware::Violet::Term::ColorChoice::Never:
        return "never";

    case Noelware::Violet::Term::ColorChoice::Auto:
        return "auto";
    }
});

// } // namespace Noelware::Violet
