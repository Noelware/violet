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
#include <violet/Experimental/Pair.h>
#include <violet/Experimental/Time/Duration.h>

#include <cmath>

namespace violet::experimental::chrono {
namespace {

struct Unit final {
    Str Name;
    Int64 Nanos;

    static auto Match(Str input, UInt position) noexcept -> Optional<Pair<Unit, UInt>>;
};

constexpr Array<Unit, 7> kUnits = {
    Unit{
        .Name = "ns",
        .Nanos = 1,
    },
    {
        .Name = "us",
        .Nanos = 1'000,
    },
    {
        .Name = "ms",
        .Nanos = 1'000'000,
    },
    {
        .Name = "s",
        .Nanos = 1'000'000'000LL,
    },
    {
        .Name = "m",
        .Nanos = 60LL * 1'000'000'000LL,
    },
    {
        .Name = "h",
        .Nanos = 3600LL * 1'000'000'000LL,
    },
    {
        .Name = "d",
        .Nanos = 86400LL * 1'000'000'000LL,
    },
};

auto Unit::Match(Str input, UInt position) noexcept -> Optional<Pair<Unit, UInt>>
{
    for (const auto& unit: kUnits) {
        const auto len = unit.Name.size();
        if (input.size() - position >= len && input.substr(position, len) == unit.Name) {
            return Some<Pair<Unit, UInt>>(unit, len);
        }
    }

    return Nothing;
}

} // namespace

auto Duration::FromStr(Str input) noexcept -> Result<Duration, ParseDurationError>
{
    if (input.empty()) {
        return Err(duration::InputEmpty{});
    }

    UInt pos = 0;
    if (input[pos] == '-' || input[pos] == '+') {
        return Err(duration::DoNotPrefix{.Input = String(input)});
    }

    Duration total;
    while (pos < input.size()) {
        const UInt numStart = pos;
        while (pos < input.size() && ((::isdigit(static_cast<unsigned char>(input[pos])) != 0) || input[pos] == '.')) {
            ++pos;
        }

        if (pos == numStart) {
            return Err(std::make_error_code(std::errc::invalid_argument));
        }

        const auto numStr = input.substr(numStart, pos - numStart);
        auto unit = Unit::Match(input, pos);
        if (!unit.HasValue()) {
            return Err(duration::MissingUnit{.Input = String(input), .At = String(numStr)});
        }

        const auto& matched = unit->First;
        pos += unit->Second;

        auto value = numeric::Parse<double>(numStr);
        if (value.Err()) {
            return Err(VIOLET_MOVE(value).Error());
        }

        const auto nsd = *value * static_cast<double>(matched.Nanos);
        if (!std::isfinite(nsd)) {
            return Err(duration::NotFinite{.Component = String(numStr), .Unit = String(matched.Name)});
        }

        constexpr double kInt64SafeMax = 9.22e18;
        if (nsd > kInt64SafeMax) {
            return Err(std::make_error_code(std::errc::result_out_of_range));
        }

        auto sum = total.CheckedAdd(Duration::Nanoseconds(static_cast<Int64>(nsd)));
        if (!sum.HasValue()) {
            return Err(std::make_error_code(std::errc::result_out_of_range));
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

} // namespace violet::experimental::chrono
