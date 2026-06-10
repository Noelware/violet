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

#include <violet/Experimental/Numeric.h>
#include <violet/Experimental/Time/Duration.h>

#include <cmath>

using violet::experimental::chrono::Duration;

using violet::Array;
using violet::Err;
using violet::Int32;
using violet::Int64;
using violet::Nothing;
using violet::Optional;
using violet::Pair;
using violet::Str;
using violet::String;
using violet::UInt;

namespace {

struct Unit final {
    Str Name;
    Int64 Nanos;
};

// clang-format off
constexpr Array<Unit, 7> kUnits = {
    Unit{.Name = "ns", .Nanos =1 },
    {.Name = "us", .Nanos = 1'000},
    {.Name = "ms", .Nanos = 1'000'000},
    {.Name = "s",  .Nanos = 1'000'000'000LL},
    {.Name = "m",  .Nanos = 60LL * 1'000'000'000LL},
    {.Name = "h",  .Nanos = 3600LL * 1'000'000'000LL},
    {.Name = "d",  .Nanos = 86400LL * 1'000'000'000LL},
};
// clang-format on

auto matchUnit(Str input, UInt pos) -> Optional<Pair<Unit, UInt>>
{
    for (const auto& unit: kUnits) {
        const auto len = unit.Name.size();
        if (input.size() - pos >= len && input.substr(pos, len) == unit.Name) {
            return std::make_pair(unit, len);
        }
    }

    return Nothing;
}

} // namespace

auto Duration::FromStr(Str input) noexcept -> violet::anyhow::Result<Duration>
{
    ENSURE(!input.empty(), "empty string");

    UInt pos = 0;
    if (input[pos] == '-' || input[pos] == '+') {
        return Err(ANYHOW_FMT("do not prefix duration with `+` or `-` ({})", input));
    }

    Duration total;
    while (pos < input.size()) {
        const UInt numStart = pos;
        while (pos < input.size() && ((::isdigit(static_cast<unsigned char>(input[pos])) != 0) || input[pos] == '.')) {
            ++pos;
        }

        if (pos == numStart) {
            return Err(ANYHOW_FMT("expected number in duration: {}", input));
        }

        const auto numStr = input.substr(numStart, pos - numStart);
        auto unit = matchUnit(input, pos);
        if (!unit.HasValue()) {
            return Err(ANYHOW_FMT("expected unit after '{}' (input={})", numStr, input));
        }

        const auto [theUnit, len] = unit.Value();
        pos += len;

        auto value = numeric::Parse<double>(numStr);
        if (value.Err()) {
            return Err(
                ANYHOW_FMT("invalid number '{}' in duration {}", numStr, input).Context(VIOLET_MOVE(value).Error()));
        }

        const auto nsd = *value * static_cast<double>(theUnit.Nanos);
        if (!std::isfinite(nsd)) {
            return Err(ANYHOW_FMT("invalid duration component '{}{}'", numStr, theUnit.Name));
        }

        constexpr double kInt64SafeMax = 9.22e18;
        if (nsd > kInt64SafeMax) {
            return Err(ANYHOW_FMT("duration component '{}{}' exceeds representable range", numStr, theUnit.Name));
        }

        auto sum = total.CheckedAdd(Duration::Nanoseconds(static_cast<Int64>(nsd)));
        if (!sum.HasValue()) {
            return Err(ANYHOW_FMT("duration [{}] overflows", input));
        }

        total = *sum;
    }

    return total;
}

auto Duration::ToString() const -> String
{
    auto nanos = this->AsNanos();
    if (nanos < 0) {
        return std::format("-{}", (-*this).ToString());
    }

    if (nanos == 0) {
        return "0ns";
    }

    String out;
    auto remaining = *this;
    bool coarse = false;

    if (auto hours = remaining.AsHours(); hours > 0) {
        out.append(std::format("{}h", hours));
        remaining = remaining - Duration::Hours(hours);
        coarse = true;
    }

    if (auto mins = remaining.AsMinutes(); mins > 0) {
        out.append(std::format("{}m", mins));
        remaining = remaining - Duration::Minutes(mins);
        coarse = true;
    }

    if (auto secs = remaining.AsSeconds(); secs > 0) {
        out.append(std::format("{}s", secs));
        remaining = remaining - Duration::Seconds(secs);
        coarse = true;
    }

    if (auto ms = remaining.AsMillis(); ms > 0) {
        out.append(std::format("{}ms", ms));
        remaining = remaining - Duration::Milliseconds(ms);
        coarse = true;
    }

    if (!coarse) {
        if (auto us = remaining.AsMicros(); us > 0) {
            out.append(std::format("{}µs", us));
            remaining = remaining - Duration::Microseconds(us);
        }

        if (auto ns = remaining.AsNanos(); ns > 0) {
            out.append(std::format("{}ns", ns));
        }
    }

    return out;
}
