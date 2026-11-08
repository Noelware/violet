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

#include <violet/Language/Macros.h>

#if VIOLET_PLATFORM(LINUX)

#include <violet/Experimental/Threading/Sleep.h>

#include <cerrno>
#include <ctime>

namespace violet::experimental::threading {
namespace {
constexpr auto kNanosPerSecond = Int64{1'000'000'000};
}

void SleepFor(chrono::Duration dur)
{
    if (dur <= chrono::Duration::Zero()) {
        return;
    }

    struct timespec deadline{};
    ::clock_gettime(CLOCK_MONOTONIC, &deadline);

    auto nanos = dur.AsNanos();
    deadline.tv_sec += static_cast<time_t>(nanos / kNanosPerSecond);
    deadline.tv_nsec += static_cast<Int64>(nanos % kNanosPerSecond);
    if (deadline.tv_nsec >= kNanosPerSecond) {
        deadline.tv_sec++;
        deadline.tv_nsec -= kNanosPerSecond;
    }

    while (::clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, nullptr) == EINTR) { }
}

} // namespace violet::experimental::threading

#endif
