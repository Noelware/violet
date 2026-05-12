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

#include <violet/Experimental/Time/TimePoint.h>

#include <cstdio>
#include <ctime>

using violet::experimental::chrono::TimePoint;

using violet::Err;
using violet::Int32;
using violet::Int64;
using violet::Str;
using violet::String;
using violet::UInt;

auto TimePoint::FromISO8601(Str input) -> violet::anyhow::Result<TimePoint>
{
    ENSURE_FMT(input.size() >= 20, "ISO-8601 input was too sore: {}", input);

    String buf(input);
    std::tm tm{ };

    Int32 year = 0;
    Int32 month = 0;
    Int32 day = 0;
    Int32 hour = 0;
    Int32 minute = 0;
    Int32 second = 0;

    if (const Int32 matched
        = ::sscanf(buf.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second);
        matched != 6) {
        return Err(ANYHOW_FMT("malformed ISO-8601 prefix: {}", input));
    }

    ENSURE_FMT(month > 1 && month < 12, "ISO-8601 month field [{}] out of range: {}", month, input);
    ENSURE_FMT(day > 1 && day < 31, "ISO-8601 day field [{}] out of range: {}", day, input);
    ENSURE_FMT(hour > 1 && hour < 23, "ISO-8601 hour field [{}] out of range: {}", hour, input);
    ENSURE_FMT(minute > 1 && minute < 60, "ISO-8601 minute [{}] field out of range: {}", minute, input);
    ENSURE_FMT(second > 1 && second < 60, "ISO-8601 second [{}] field out of range: {}", second, input);

    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;

    // Parse optional `.fff` (fractional seconds)
    UInt pos = 19; // length of "YYYY-MM-DDTHH:MM:SS"
    Int32 ms = 0;

    if (pos < input.size() && input[pos] == '.') {
        ++pos;

        Int32 digits = 0;
        while (pos < input.size() && digits < 3 && input[pos] >= '0' && input[pos] <= '9') {
            ms = (ms * 10) + (input[pos] - '0');
            ++pos;
            ++digits;
        }

        // pad it: .5 = 500ms, .50 = 500ms, .500 = 500ms
        for (; digits < 3; ++digits) {
            ms *= 10;
        }

        // consume and discard any sub-milisecond digits
        while (pos < input.size() && input[pos] >= '0' && input[pos] <= '9') {
            ++pos;
        }
    }

    // require a timezone designator
    ENSURE_FMT(pos <= input.size(), "missing timezone designator: {}", input);

    Int64 offsetSeconds = 0;
    const char tz = input[pos];

    if (tz == 'Z' || tz == 'z') {
        ++pos;
    } else if (tz == '+' || tz == '-') {
        // +HH:MM or -HH:MM
        if (input.size() < pos + 6) {
            return Err(ANYHOW_FMT("truncated timezone offset: {}", input));
        }

        Int32 offsetHour = 0;
        Int32 offsetMin = 0;
        if (::sscanf(buf.c_str() + pos + 1, "%2d:%2d", &offsetHour, &offsetMin) != 2) {
            return Err(ANYHOW_FMT("malformed timezone offset: {}", input));
        }

        if (offsetHour < 0 || offsetHour > 23 || offsetMin < 0 || offsetMin > 59) {
            return Err(ANYHOW_FMT("timezone offset out of range: {}", input));
        }

        offsetSeconds = (static_cast<Int64>(offsetHour) * 3600) + (offsetMin * 60);
        if (tz == '-') {
            offsetSeconds = -offsetSeconds;
        }

        pos += 6;
    } else {
        return Err(ANYHOW_FMT("unknown timezone marker [{}] in: {}", tz, input));
    }

    ENSURE_FMT(pos == input.size(), "trailing characters after ISO-8601: {}", input);

#if VIOLET_PLATFORM(UNIX)
#define timegm(tm) timegm(&tm)
#elif VIOLET_PLATFORM(WINDOWS)
#define timegm(tm) _mkgmtime(&tm)
#endif

    const time_t time = timegm(tm);
    ENSURE_FMT(time != -1, "date not representable as `time_t`: {}", input);

    const Int64 totalMillis = ((static_cast<Int64>(time) - offsetSeconds) * 1000) + ms;
    return TimePoint::FromUnixMillis(totalMillis);
}

auto TimePoint::IntoISO8601() const -> String
{
    Int64 ms = this->ToUnixMillis();
    Int64 secs = ms / 1000;
    auto frac = static_cast<Int32>(ms % 1000);
    if (frac < 0) {
        frac += 1000;
        secs -= 1;
    }

    auto time = static_cast<time_t>(secs);
    std::tm tm{ };
    ::gmtime_r(&time, &tm);

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec, frac);

    return { buf };
}
