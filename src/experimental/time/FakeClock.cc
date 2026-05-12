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

#include <violet/Experimental/Time/FakeClock.h>

using violet::experimental::FakeClock;
using violet::experimental::MutexLock;
using violet::experimental::chrono::Duration;
using violet::experimental::chrono::Instant;
using violet::experimental::chrono::TimePoint;

FakeClock::FakeClock()
    : FakeClock(TimePoint::FromUnixSeconds(/*January 1st, 2026*/ 1'767'225'600))
{
}

FakeClock::FakeClock(TimePoint tp)
    : n_wall(tp)
{
}

auto FakeClock::Now() const -> Instant
{
    MutexLock lock(this->n_mux);
    return this->n_monotonic;
}

auto FakeClock::WallNow() const -> TimePoint
{
    MutexLock lock(this->n_mux);
    return this->n_wall;
}

void FakeClock::Advance(Duration dur)
{
    MutexLock lock(this->n_mux);

    this->n_monotonic += dur;
    this->n_wall = this->n_wall + dur;
}

void FakeClock::SetMonotonic(Instant time)
{
    MutexLock lock(this->n_mux);
    this->n_monotonic = time;
}

void FakeClock::SetWall(TimePoint ts)
{
    MutexLock lock(this->n_mux);
    this->n_wall = ts;
}
