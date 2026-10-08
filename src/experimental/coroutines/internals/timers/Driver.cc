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

#include <violet/Experimental/Coroutines/Internals/Timers/Driver.h>

using namespace violet::experimental::chrono;

namespace violet::experimental::coro::internals::timers {

namespace {
constexpr Int64 kNanosPerTick = 1'000'000;
}

void Driver::Register(TimerEntry& ent, Instant deadline) noexcept
{
    VIOLET_DEBUG_ASSERT(ent.Fire != nullptr, "timer entry has no `Fire` callback");

    ent.Deadline = this->getDeadlineTick(deadline);
    this->n_wheel.Insert(ent);
}

void Driver::Cancel(TimerEntry& ent) noexcept
{
    if (ent.Linked) {
        this->n_wheel.Remove(ent);
    }
}

auto Driver::NextDeadline() const noexcept -> Optional<Instant>
{
    auto tick = this->n_wheel.NextExpiration();
    if (!tick.HasValue()) {
        return Nothing;
    }

    return this->n_base + Duration::Milliseconds(static_cast<Duration::rep>(*tick));
}

auto Driver::FireExpired(Instant now) -> UInt
{
    UInt fired = 0;
    this->n_wheel.Advance(this->getNowTick(now), [&fired](TimerEntry* ent) noexcept -> void {
        fired++;
        std::invoke(ent->Fire, ent);
    });

    return fired;
}

void Driver::Clear() noexcept
{
    this->n_wheel.Drain([](TimerEntry* ent) noexcept -> void { std::invoke(ent->Fire, ent); });
}

auto Driver::getDeadlineTick(Instant deadline) const noexcept -> UInt64
{
    if (deadline <= this->n_base) {
        return 0;
    }

    Int64 nanos = (deadline - this->n_base).AsNanos();
    return static_cast<UInt64>((nanos + kNanosPerTick - 1) / kNanosPerTick);
}

auto Driver::getNowTick(Instant now) const noexcept -> UInt64
{
    if (now <= this->n_base) {
        return 0;
    }

    return static_cast<UInt64>((now - this->n_base).AsNanos() / kNanosPerTick);
}

} // namespace violet::experimental::coro::internals::timers
