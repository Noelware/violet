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

#include <violet/Support/Terminal.h>
#include <violet/System.h>
#include <violet/System/CI.h>

#include <sstream>

using violet::String;
using violet::UInt;
using violet::terminal::RGB;
using violet::terminal::Style;

auto RGB::Paint() const noexcept -> String
{
    return std::format("\x1b[{};2;{};{};{}m", this->Foreground ? 38 : 48, this->Red, this->Green, this->Blue);
}

auto RGB::ToString() const noexcept -> String
{
    return std::format(
        "RGB(Red={}, Green={}, Blue={}, Foreground={})", this->Red, this->Green, this->Blue, this->Foreground);
}

auto Style::createStream() const noexcept -> std::ostringstream
{
    std::ostringstream os;
    if (this->n_tag.Contains(tag::kBold)) {
        os << "\x1b[1m";
    }

    if (this->n_tag.Contains(tag::kDim)) {
        os << "\x1b[2m";
    }

    if (this->n_tag.Contains(tag::kItalic)) {
        os << "\x1b[3m";
    }

    if (this->n_tag.Contains(tag::kUnderline)) {
        os << "\x1b[4m";
    }

    if (this->n_tag.Contains(tag::kInverse)) {
        os << "\x1b[7m";
    }

    if (this->n_tag.Contains(tag::kStrikethrough)) {
        os << "\x1b[9m";
    }

    return os;
}

auto Style::Paint() const noexcept -> String
{
    auto os = this->createStream();

    // If a `std::monostate` is provided, it means no flags
    // were set, so we should just pass on the flags
    // that were probably set.
    if (std::get_if<std::monostate>(&this->n_style) != nullptr) {
        return os.str();
    }

    if (const auto* fg = std::get_if<Style::fg>(&this->n_style)) {
        os << "\x1b[" << static_cast<Int32>(fg->Value) << 'm';
        return os.str();
    }

    if (const auto* bg = std::get_if<Style::bg>(&this->n_style)) {
        os << "\x1b[" << static_cast<Int32>(bg->Value) << 'm';
        return os.str();
    }

    auto rgb = std::get<terminal::RGB>(this->n_style);
    os << rgb.Paint();

    return os.str();
}

auto Style::ToString() const noexcept -> String
{
    std::ostringstream os("Style(");

    if (const auto* fg = std::get_if<Style::fg>(&this->n_style)) {
        os << "state=Foreground(Value=" << fg->Value << "), ";
    }

    if (const auto* bg = std::get_if<Style::bg>(&this->n_style)) {
        os << "state=Background(Value=" << bg->Value << "), ";
    }

    if (const auto* rgb = std::get_if<terminal::RGB>(&this->n_style)) {
        os << "state=" << rgb->ToString() << ", ";
    }

    os << "bold=" << std::boolalpha << this->n_tag.Contains(tag::kBold) << ", ";
    os << "dim=" << std::boolalpha << this->n_tag.Contains(tag::kDim) << ", ";
    os << "italic=" << std::boolalpha << this->n_tag.Contains(tag::kItalic) << ", ";
    os << "underline=" << std::boolalpha << this->n_tag.Contains(tag::kUnderline) << ", ";
    os << "inverse=" << std::boolalpha << this->n_tag.Contains(tag::kInverse) << ", ";
    os << "strikethrough=" << std::boolalpha << this->n_tag.Contains(tag::kStrikethrough) << ')';

    return os.str();
}

namespace {

auto getForceColorLevel() noexcept -> UInt
{
    if (auto level = violet::sys::GetEnv("FORCE_COLOR")) {
        if (level->empty() || *level == "") {
            return 1;
        }

        if (*level == "false") {
            return 0;
        }

#ifdef VIOLET_HAS_EXCEPTIONS
        UInt lvl = 1;
        try {
            lvl = static_cast<UInt>(std::stoi(*level));
        } catch (const std::invalid_argument&) {
            // ...
        } catch (const std::out_of_range&) {
            // ...
        }
#else
        UInt lvl = static_cast<UInt>(std::stoi(*level));
#endif

        return std::min(lvl, static_cast<UInt>(3));
    }

    if (auto force = violet::sys::GetEnv("CLICOLOR_FORCE")) {
        return *force != "0" ? 1 : 0;
    }

    return 0;
}

auto isNoColour() noexcept -> bool
{
    auto env = violet::sys::GetEnv("NO_COLOR");
    return !(env == violet::Nothing || *env == "0");
}

constexpr const violet::CStr kTerm = "TERM";
constexpr const violet::CStr kTermProgram = "TERM_PROGRAM";
constexpr const violet::CStr kColorTerm = "COLORTERM";

auto checkColorterm16m(const String& value) -> bool
{
    return value == "truecolor" || value == "24bit";
}

auto checkTerm16m(const String& value) -> bool
{
    return value.ends_with("direct") || value.ends_with("truecolor");
}

auto check256BitColor(const String& value)
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

auto violet::terminal::ColourLevel(StreamSource source) noexcept -> ColorLevel
{
    UInt level = 0;
    if (auto forced = getForceColorLevel(); forced > 0) {
        return { .SupportsBasic = forced >= 1, .Supports256Bit = forced >= 2, .Supports16M = forced >= 3 };
    }

    if (isNoColour() || violet::sys::GetEnv(kTerm) == Some<String>("dumb") || !terminal::IsTTY(source)) {
        return {};
    }

    if (violet::sys::GetEnv(kColorTerm).HasValueAnd(checkColorterm16m)
        || violet::sys::GetEnv(kTerm).HasValueAnd(checkTerm16m)
        || violet::sys::GetEnv(kTermProgram) == Some<String>("iTerm.app")) {
        level = 3;
        return { .SupportsBasic = level >= 1, .Supports256Bit = level >= 2, .Supports16M = level >= 3 };
    }

    if (violet::sys::GetEnv(kTermProgram) == Some<String>("Apple_Terminal")
        || violet::sys::GetEnv(kTerm).HasValueAnd(check256BitColor)) {
        level = 2;
        return { .SupportsBasic = level >= 1, .Supports256Bit = level >= 2, .Supports16M = level >= 3 };
    }

    if (violet::sys::GetEnv(kTerm).HasValueAnd(checkPlatformANSIColor)
        || violet::sys::GetEnv("CLICOLOR").MapOr(false, [](const String& val) { return val != "0"; })
        || violet::sys::ContinuousIntegration()) {
        level = 1;
        return { .SupportsBasic = level >= 1, .Supports256Bit = level >= 2, .Supports16M = level >= 3 };
    }

    return { .SupportsBasic = level >= 1, .Supports256Bit = level >= 2, .Supports16M = level >= 3 };
}
