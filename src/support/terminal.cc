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

#include <violet/Support/Terminal.h>

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

auto RGB::operator<<(std::ostream& os) const noexcept -> std::ostream&
{
    return os << this->ToString();
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
        os << "state=" << rgb << ", ";
    }

    os << "bold=" << this->n_tag.Contains(tag::kBold) << ", ";
    os << "dim=" << this->n_tag.Contains(tag::kDim) << ", ";
    os << "italic=" << this->n_tag.Contains(tag::kItalic) << ", ";
    os << "underline=" << this->n_tag.Contains(tag::kUnderline) << ", ";
    os << "inverse=" << this->n_tag.Contains(tag::kInverse) << ", ";
    os << "strikethrough=" << this->n_tag.Contains(tag::kStrikethrough) << ')';

    return os.str();
}

auto Style::operator<<(std::ostream& os) const noexcept -> std::ostream&
{
    return os << this->ToString();
}
