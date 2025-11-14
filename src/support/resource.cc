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

#include <stdexcept>
#include <violet/Container/Result.h>
#include <violet/Numeric/Int128.h>
#include <violet/Support/Quantity.h>
#include <violet/Violet.h>

using violet::CStr;
using violet::FlatHashMap;
using violet::Str;
using violet::UInt128;
using violet::resource::CPU;
using violet::resource::Memory;

static const FlatHashMap<Str, UInt128> kBinaryUnits = {
    { "", 1 },
    { "Ki", 1024LL },
    { "Mi", 1024LL * 1024 },
    { "Gi", 1024LL * 1024 * 1024 },
    { "Ti", 1024LL * 1024 * 1024 * 1024 },
    { "Pi", 1024LL * 1024 * 1024 * 1024 * 1024 },
    { "Ei", 1024LL * 1024 * 1024 * 1024 * 1024 * 1024 },
};

static const FlatHashMap<Str, double> kDecimalUnits
    = { { "", 1 }, { "k", 1e3 }, { "M", 1e6 }, { "G", 1e9 }, { "T", 1e12 }, { "P", 1e15 }, { "E", 1e18 } };

constexpr static CStr kDoubleLikeStr = "0123456789.eE-";

auto CPU::Parse(Str str) noexcept -> Result<CPU, ParseError>
{
    auto posWithDouble = str.find_first_not_of(kDoubleLikeStr);

    double num = 0.0;
    try {
        Str pos = str.substr(0, posWithDouble);
        num = std::stod(String(pos));
    } catch (const std::invalid_argument&) {
        return Err(ParseError(ParseError::tag::kInvalidDouble));
    } catch (const std::out_of_range&) {
        return Err(ParseError(ParseError::tag::kOutOfRange));
    }

    String suffix = String(str.substr(posWithDouble));

    switch (suffix.at(0)) {
    case 'n':
        return CPU(UInt128(num));
    case 'u':
        return CPU(UInt128(num * 1000));
    case 'm':
        return CPU(UInt128(num * 1'000'000));
    default:
        break;
    }

    if (suffix == "µ") {
        return CPU(UInt128(num * 1000));
    }

    if (suffix.empty()) {
        return CPU(UInt128(num * 1'000'000'000));
    }

    return Err(ParseError(ParseError::tag::kUnknownSuffix));
}

auto Memory::Parse(Str str) noexcept -> Result<Memory, ParseError>
{
    auto posWithDouble = str.find_first_not_of(kDoubleLikeStr);

    double num = 0.0;
    try {
        Str pos = str.substr(0, posWithDouble);
        num = std::stod(String(pos));
    } catch (const std::invalid_argument&) {
        return Err(ParseError(ParseError::tag::kInvalidDouble));
    } catch (const std::out_of_range&) {
        return Err(ParseError(ParseError::tag::kOutOfRange));
    }

    String suffix = String(str.substr(posWithDouble));
    if (kBinaryUnits.count(suffix) != 0U) {
        return Memory(UInt128(num * numeric::uint128::ToDouble(kBinaryUnits.at(suffix))));
    }

    if (kDecimalUnits.count(suffix) != 0U) {
        return Memory(UInt128(num * kDecimalUnits.at(suffix)));
    }

    return Err(ParseError(ParseError::tag::kUnknownSuffix));
}

auto CPU::ToString() const noexcept -> String
{
    return this->n_value % 1'000'000 == 0 ? std::format("{:.3f}", AsCores())
                                          : std::format("{:.0f}m", numeric::uint128::ToDouble(this->n_value) / 1e6);
}

auto Memory::ToString() const noexcept -> String
{
    for (const auto& [suffix, unit]: kBinaryUnits) {
        if (this->n_value % unit == 0) {
            double scaled = numeric::uint128::ToDouble(this->n_value) / numeric::uint128::ToDouble(unit);
            return std::format("{:.0f}{}", scaled, suffix);
        }
    }

    // fallback (should not happen)
    return std::format("{}B", numeric::uint128::ToDouble(this->n_value));
}
