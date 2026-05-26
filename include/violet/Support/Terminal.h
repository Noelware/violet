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
//! # 🌺💜 `violet/Support/Terminal.h`
//! This header file provides abstractions and utilities for working with
//! terminal colours, ANSI colour level and styled text output.

#pragma once

#include <violet/IO/Error.h>
#include <violet/Support/Bitflags.h>

namespace violet::terminal {

/// Controls whether color output should be emitted by CLI programs.
///
/// This mirrors conventions used by tools such as `cargo`, `ripgrep`,
/// and `git`, including their `--color` flag and `*_COLOR` environment
/// variables.
///
/// * `Always` will always emit ANSI color sequences.
/// * `Never` will never emit ANSI color sequences.
/// * `Auto` will emit ANSI sequences only when the output stream
///   appears to be a terminal supporting them.
enum struct NOELDOC_SINCE("26.02.01") ColorChoice : UInt8 {
    Always = 0, ///< `--color=always`, `{CLI}_COLOR=always`
    Never = 1, ///< `--color=never`, `{CLI}_COLOR=never`
    Auto = 2 ///< `--color=auto`, auto detection
};

/// Identifies which output stream should be inspected when determining
/// terminal capabilities such as color support or window size.
enum struct NOELDOC_SINCE("26.02.01") StreamSource : UInt8 {
    Stdout = 0, ///< inspect the file descriptor for stdout.
    Stderr = 1 ///< inspect the file descriptor for stderr.
};

/// Describes the set of ANSI color capabilities supported by a terminal.
///
/// This is typically inferred via:
/// * Environment variables (e.g., `COLORTERM`)
/// * `$TERM` values
/// * Terminfo entries
struct VIOLET_API NOELDOC_SINCE("26.02.01") ColorLevel final {
    bool SupportsBasic = false; ///< provides support for 4-bit (16-color) SGR codes
    bool Supports256Bit = false; ///< provides support for 256-bit ANSI colours
    bool Supports16M = false; ///< provides support for 16-million (RGB) ANSI colours
};

/// Terminal window dimensions, measured in character cells.
struct VIOLET_API NOELDOC_SINCE("26.02.01") Window final {
    UInt16 Columns = 0; ///< width of the terminal in columns
    UInt16 Rows = 0; ///< height of the terminal in rows
};

/// Represents a 24-bit RGB color value with an associated foreground/background
/// rendering mode for use in ANSI terminal escape sequences.
///
/// `RGB` stores individual red, green, and blue channel values as unsigned 8-bit
/// integers, along with a flag indicating whether the color should be applied as
/// a foreground or background color when painting terminal output.
///
/// ## Example
/// ```cpp
/// #include <violet/Support/Terminal.h>
/// #include <violet/Print.h>
///
/// using namespace violet::terminal;
///
/// RGB red(255, 0, 0);
/// violet::Println("{}This text is red!\033[0m", red.Paint());
///
/// // Create a blue background colour
/// RGB bg(0, 0, 255, /*foreground=*/false);
///
/// // Colors compare by channel values only, ignoring `foreground` flag.
/// assert(red == RGB(255, 0, 0, /*foreground=*/false));
/// ```
struct VIOLET_API NOELDOC_SINCE("26.02.01") RGB final {
    /// The red channel value, ranging from `0..=255`.
    UInt8 Red = 0;

    /// The green channel value, ranging from `0..=255`.
    UInt8 Green = 0;

    /// The blue channel value, ranging from `0..=255`.
    UInt8 Blue = 0;

    /// Whether this color should be applied as a foreground color (`true`) or
    /// a background color (`false`) when generating ANSI escape sequences.
    bool Foreground = true;

    /// Constructs a default `RGB` with all channels set to zero (black) and
    /// [`RGB::Foreground`] set to `true`.
    constexpr VIOLET_IMPLICIT RGB() noexcept = default;

    /// Constructs an `RGB` from the given channel values.
    ///
    /// @param red        red channel intensity
    /// @param green      green channel intensity
    /// @param blue       blue channel intensity
    /// @param foreground if `true`, the color targets the foreground; otherwise, the background.
    constexpr VIOLET_IMPLICIT RGB(UInt8 red, UInt8 green, UInt8 blue, bool foreground = true) noexcept
        : Red(red)
        , Green(green)
        , Blue(blue)
        , Foreground(foreground)
    {
    }

    /// Returns the ANSI escape sequence string that applies this color to
    /// subsequent terminal output.
    ///
    /// The returned sequence uses SGR parameter 38 (foreground) or 48
    /// (background) with the 24-bit color mode (`;2;r;g;b`), depending on
    /// the value of [`RGB::Foreground`].
    [[nodiscard]] auto Paint() const noexcept -> String;

    /// Returns a human-readable string representation of this color in the
    /// format `rgb(R, G, B)`.
    [[nodiscard]] auto ToString() const noexcept -> String;
    friend auto operator<<(std::ostream& os, const RGB& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

    /// Compares two `RGB` values for equality by their channel values.
    ///
    /// Note that [`Foreground`] is **not** considered in the comparison, two
    /// colors with identical channels but different rendering modes are
    /// treated as equal.
    auto operator==(const RGB& rhs) const noexcept -> bool
    {
        return this->Red == rhs.Red && this->Green == rhs.Green && this->Blue == rhs.Blue;
    }
};

struct VIOLET_API NOELDOC_SINCE("26.02.01") Style final {
    constexpr VIOLET_IMPLICIT Style() noexcept = default;

#define MK_STYLE_FN(NAME, FG, BG)                                                                                      \
    NOELDOC_SINCE("26.02.01") constexpr static auto NAME(bool foreground = true) noexcept -> Style                     \
    {                                                                                                                  \
        Style style;                                                                                                   \
        if (foreground) {                                                                                              \
            style.n_style = Style::fg{ .Value = FG };                                                                  \
        } else {                                                                                                       \
            style.n_style = Style::bg{ .Value = BG };                                                                  \
        }                                                                                                              \
        return style;                                                                                                  \
    }

    MK_STYLE_FN(Black, 30, 40);
    MK_STYLE_FN(Red, 31, 41);
    MK_STYLE_FN(Green, 32, 42);
    MK_STYLE_FN(Yellow, 33, 43);
    MK_STYLE_FN(Blue, 34, 44);
    MK_STYLE_FN(Magenta, 35, 45);
    MK_STYLE_FN(Cyan, 36, 46);
    MK_STYLE_FN(White, 37, 47);

    MK_STYLE_FN(BrightBlack, 90, 100);
    MK_STYLE_FN(BrightRed, 91, 101);
    MK_STYLE_FN(BrightGreen, 92, 102);
    MK_STYLE_FN(BrightYellow, 93, 103);
    MK_STYLE_FN(BrightBlue, 94, 104);
    MK_STYLE_FN(BrightMagenta, 95, 105);
    MK_STYLE_FN(BrightCyan, 96, 106);
    MK_STYLE_FN(BrightWhite, 97, 107);

#undef MK_STYLE_FN

    template<UInt8 Red, UInt8 Green, UInt8 Blue>
    consteval static auto RGB(bool foreground = true) noexcept -> Style
    {
        Style style;
        style.n_style = terminal::RGB(Red, Green, Blue, foreground);

        return style;
    }

    constexpr static auto RGB(UInt8 red, UInt8 blue, UInt8 green, bool foreground = true) noexcept -> Style
    {
        Style style;
        style.n_style = terminal::RGB(red, green, blue, foreground);

        return style;
    }

    constexpr auto Bold() noexcept -> Style&
    {
        this->n_tag = this->n_tag.Apply(tag::kBold);
        return *this;
    }

    constexpr auto Italic() noexcept -> Style&
    {
        this->n_tag = this->n_tag.Apply(tag::kItalic);
        return *this;
    }

    constexpr auto Dim() noexcept -> Style&
    {
        this->n_tag = this->n_tag.Apply(tag::kDim);
        return *this;
    }

    constexpr auto Underline() noexcept -> Style&
    {
        this->n_tag = this->n_tag.Apply(tag::kUnderline);
        return *this;
    }

    constexpr auto Inverse() noexcept -> Style&
    {
        this->n_tag = this->n_tag.Apply(tag::kInverse);
        return *this;
    }

    constexpr auto Strikethrough() noexcept -> Style&
    {
        this->n_tag = this->n_tag.Apply(tag::kStrikethrough);
        return *this;
    }

    [[nodiscard]] auto Paint() const noexcept -> String;
    [[nodiscard]] auto ToString() const noexcept -> String;
    friend auto operator<<(std::ostream& os, const Style& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    enum struct tag : UInt8 {
        kBold = 1 << 0,
        kItalic = 1 << 1,
        kDim = 1 << 2,
        kUnderline = 1 << 3,
        kInverse = 1 << 4,
        kStrikethrough = 1 << 5
    };

    // clang-format off
    struct bg { UInt8 Value; };
    struct fg { UInt8 Value; };
    // clang-format on

    std::variant<std::monostate, bg, fg, struct RGB> n_style;
    Bitflags<tag> n_tag = 0;

    [[nodiscard]] auto createStream() const noexcept -> std::ostringstream;
};

template<typename T>
struct VIOLET_API NOELDOC_SINCE("26.02.01") Styled final {
    T Target;
    struct Style Style;

    VIOLET_IMPLICIT Styled(T target, struct Style style) noexcept
        : Target(target)
        , Style(style)
    {
    }

    [[nodiscard]] auto Paint() const noexcept -> String
    {
        return std::format("{}{}\x1b[0m", this->Style.Paint(), this->Target);
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return std::format("Styled(Target={}, {})", this->Target, this->Style);
    }

    friend auto operator<<(std::ostream& os, const Styled& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }
};

#if VIOLET_PLATFORM(WINDOWS) || VIOLET_FEATURE(NOELDOC)
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
[[nodiscard("always check the error state")]] VIOLET_API NOELDOC_SINCE("26.02.01") auto EnableVTMode(
    StreamSource source = StreamSource::Stdout) -> IO::Result<void>;
#endif

/// Returns **true** if we are in a terminal process instead of a pipe or child process.
/// @param source source to check, by default, stdout.
VIOLET_API NOELDOC_SINCE("26.02.01") auto IsTTY(StreamSource source = StreamSource::Stdout) noexcept -> bool;

/// Queries information about the terminal window.
/// @param source the stream source to check, by default, standard output.
VIOLET_API NOELDOC_SINCE("26.02.01") auto QueryWindowInfo(StreamSource source = StreamSource::Stdout) noexcept
    -> io::Result<Window>;

/// Queries information about color displays for terminals.
/// @param source the stream source to check, by default, standard output.
VIOLET_API NOELDOC_SINCE("26.02.01") auto ColourLevel(StreamSource source = StreamSource::Stdout) noexcept
    -> ColorLevel;

/// Sets the color choice.
///
/// This is used to determine colours when using any of the **Noelware.Violet** frameworks that
/// depend on the terminal framework as well.
///
/// @param choice the color choice to use
VIOLET_API NOELDOC_SINCE("26.02.01") void SetColorChoice(
    violet::terminal::ColorChoice choice = violet::terminal::ColorChoice::Auto) noexcept;

/// Returns **true** if the terminal framework is supported to use colors. This is usually determined
/// either if a user has called [`SetColorChoice`] or by the [`ColourLevel`] function.
VIOLET_API NOELDOC_SINCE("26.02.01") auto ColoursEnabled(StreamSource source = StreamSource::Stdout) noexcept -> bool;

} // namespace violet::terminal

VIOLET_TO_STRING(violet::terminal::ColorChoice, choice, {
    switch (choice) {
    case violet::terminal::ColorChoice::Always:
        return "always";

    case violet::terminal::ColorChoice::Never:
        return "never";

    case violet::terminal::ColorChoice::Auto:
        return "auto";
    }
});

VIOLET_TO_STRING(violet::terminal::StreamSource, src, {
    switch (src) {
    case violet::terminal::StreamSource::Stdout:
        return "standard output";
    case violet::terminal::StreamSource::Stderr:
        return "standard error";
    }
});

VIOLET_FORMATTER_TEMPLATE(violet::terminal::Styled<T>, typename T);
VIOLET_FORMATTER(violet::terminal::Style);
VIOLET_FORMATTER(violet::terminal::RGB);
