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

#include "violet/support/Terminal.h"
#include "violet/container/Optional.h"
#include "violet/sys/Environment.h"
#include "violet/violet.h"

#include <sstream>

using Noelware::Violet::Nothing;
using Noelware::Violet::Optional;
using Noelware::Violet::String;
using Noelware::Violet::Terminal;
using Noelware::Violet::uint8;
using Noelware::Violet::usize;
using Noelware::Violet::System::GetEnvironmentVariable;
using Noelware::Violet::Term::RGB;
using Noelware::Violet::Term::StreamSource;
using Noelware::Violet::Term::Style;
using Noelware::Violet::Term::Styled;

constexpr RGB::RGB(uint8 red, uint8 green, uint8 blue, bool foreground) noexcept
    : Red(red)
    , Green(green)
    , Blue(blue)
    , Foreground(foreground)
{
}

constexpr auto RGB::Paint() const noexcept -> String
{
    return std::format("\x1b[{};2;{};{};{}m", this->Foreground ? 38 : 48, this->Red, this->Green, this->Blue);
}

auto RGB::ToString() const noexcept -> String
{
    return std::format(
        "RGB{{Red={},Green={},Blue={},Foreground={}}}", this->Red, this->Green, this->Blue, this->Foreground);
}

#define MK_STYLE_FN(NAME, BG, FG)                                                                                      \
    constexpr auto Style::NAME(bool foreground) noexcept -> Style                                                      \
    {                                                                                                                  \
        Style style;                                                                                                   \
        if (foreground) {                                                                                              \
            style.n_variant = Style::fg{ .value = FG };                                                                \
        } else {                                                                                                       \
            style.n_variant = Style::bg{ .value = BG };                                                                \
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

template<uint8 R, uint8 G, uint8 B>
constexpr auto Style::RGB(bool foreground) noexcept -> Style
{
    Style style;
    style.n_variant = Noelware::Violet::Term::RGB(R, G, B, foreground);

    return style;
}

template<usize N>
constexpr auto Style::HexRGB(const char (&str)[N], bool foreground) -> Style
{
    auto getHexDigit = [](const char ch) constexpr {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }

        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' - 10;
        }

        if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        }

        return 0;
    };

    auto asHexPair = [](char hi, char lo) constexpr { return (getHexDigit(hi) << 4) | getHexDigit(lo); };

    usize offset = str[0] == '#' ? 1 : 0;
    auto rgb = Noelware::Violet::Term::RGB(
        /*red=*/asHexPair(str[offset], str[offset + 1]),
        /*green=*/asHexPair(str[offset + 2], str[offset + 3]),
        /*blue=*/asHexPair(str[offset + 4], str[offset + 5]), foreground);

    Style style;
    style.n_variant = rgb;

    return style;
}

constexpr auto Style::HexRGB(StringRef str, bool foreground) noexcept -> Style
{
    auto getHexDigit = [](const char ch) constexpr {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }

        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' - 10;
        }

        if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        }

        return 0;
    };

    auto asHexPair = [&](char hi, char lo) constexpr { return (getHexDigit(hi) << 4) | getHexDigit(lo); };

    usize offset = str[0] == '#' ? 1 : 0;
    auto rgb = Noelware::Violet::Term::RGB(
        /*red=*/asHexPair(str[offset], str[offset + 1]),
        /*green=*/asHexPair(str[offset + 2], str[offset + 3]),
        /*blue=*/asHexPair(str[offset + 4], str[offset + 5]), foreground);

    Style style;
    style.n_variant = rgb;

    return style;
}

#define MK_STYLE_FN(NAME, VARIANT)                                                                                     \
    constexpr auto Style::NAME() noexcept -> Style&                                                                    \
    {                                                                                                                  \
        if (this->n_style.Contains(style::VARIANT)) {                                                                  \
                                                                                                                       \
        } else {                                                                                                       \
            this->n_style.Add(style::VARIANT);                                                                         \
        }                                                                                                              \
        return *this;                                                                                                  \
    }

MK_STYLE_FN(Bold, kBold);
MK_STYLE_FN(Italic, kItalic);
MK_STYLE_FN(Dim, kDim);
MK_STYLE_FN(Underline, kUnderline);
MK_STYLE_FN(Inverse, kInverse);
MK_STYLE_FN(Strikethrough, kStrikethrough);

#undef MK_STYLE_FN

constexpr auto Style::Paint() const noexcept -> String
{
    // `std::monostate` means reset regardless of styles
    if (std::get_if<std::monostate>(&this->n_variant) != nullptr) {
        return "\x1b[0m";
    }

    if (const auto* fg = std::get_if<Style::fg>(&this->n_variant)) {
        auto os = this->getStringStream();
        os << "\x1b[" << fg->value << "m";

        return os.str();
    }

    if (const auto* bg = std::get_if<Style::bg>(&this->n_variant)) {
        auto os = this->getStringStream();
        os << "\x1b[" << bg->value << "m";

        return os.str();
    }

    auto rgb = std::get<Noelware::Violet::Term::RGB>(this->n_variant);
    auto os = this->getStringStream();
    os << rgb.Paint();

    return os.str();
}

auto Style::ToString() const noexcept -> String
{
    return this->Paint();
}

auto Style::operator<<(std::ostream& os) const noexcept -> std::ostream&
{
    return os << this->Paint();
}

template<typename T>
Styled<T>::Styled(T target, Style style)
    : n_target(VIOLET_MOVE(target))
    , n_style(style)
{
}

template<typename T>
auto Styled<T>::Paint() const noexcept -> String
{
    auto os = std::ostringstream(this->n_style.Paint());

    if constexpr (requires { Noelware::Violet::ToString(this->n_target); }) {
        os << Noelware::Violet::ToString(this->n_target);
    } else if constexpr (requires { os << this->n_target; }) {
        os << this->n_target;
    } else {
        const auto& type = typeid(T);
        os << "«type '" << Utility::DemangleCXXName(type.name()) << '@' << type.hash_code() << "' not streamable»";
    }

    os << "\x1b[0m";
    return os.str();
}

template<typename T>
auto Styled<T>::ToString() const noexcept -> String
{
    return this->Paint();
}

template<typename T>
auto Styled<T>::operator<<(std::ostream& os) const noexcept -> std::ostream&
{
    return os << this->Paint();
}

namespace {
inline auto getEnvForceColourLevel() noexcept -> usize
{
    if (auto force = GetEnvironmentVariable("FORCE_COLOR")) {
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

    if (auto force = GetEnvironmentVariable("CLICOLOR_FORCE")) {
        return *force != "0" ? 1 : 0;
    }

    return 0;
}

inline auto isNoColour() -> bool
{
    auto env = GetEnvironmentVariable("NO_COLOR");
    return !(env == Nothing || *env == "0");
}

inline auto getTermEnv() -> Optional<String>
{
    return GetEnvironmentVariable("TERM");
}

inline auto getTerminalProgram() -> Optional<String>
{
    return GetEnvironmentVariable("TERM_PROGRAM");
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
} // namespace

auto Terminal::ColorLevel(StreamSource source) noexcept -> Term::ColorLevel
{
    usize level = 0;
    if (auto force = getEnvForceColourLevel(); force > 0) {
        Term::ColorLevel level;
        level.SupportsBasic = force >= 1;
        level.Supports256Bit = force >= 2;
        level.Supports16M = force >= 3;

        return level;
    }

    if (isNoColour() || getTermEnv() == "dumb" || !Terminal::TTY(source)) {
        return {};
    }

    if (GetEnvironmentVariable("COLORTERM").Map(checkColorterm16m) || getTermEnv().Map(checkTerm16m)
        || getTerminalProgram() == "ITerm.app") {
        level = 3;
    }

    if (getTerminalProgram() == "Apple_Terminal" || getTermEnv().Map(check256BitColor)) {
        level = 2;
    }

    if (GetEnvironmentVariable("COLORTERM") || getTermEnv().Map(checkPlatformANSIColor)
        || GetEnvironmentVariable("CLICOLOR").MapOr(false, [](const String& str) { return str != "0"; })

        // TODO(@auguwu): better CI detection
        || GetEnvironmentVariable("CI")) {
        level = 1;
    }

    Term::ColorLevel cl;
    cl.SupportsBasic = level >= 1;
    cl.Supports256Bit = level >= 2;
    cl.Supports16M = level >= 3;

    return cl;
}
