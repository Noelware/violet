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

#include <violet/Experimental/Synchronization/SpinLock.h>
#include <violet/Experimental/Threading/Parallelism.h>

namespace violet::experimental::sync {

static_assert(sizeof(std::atomic<UInt32>) == sizeof(UInt32) && alignof(std::atomic<UInt32>) == alignof(UInt32),
    "the futex word must alias the atomic's storage");

namespace {

constexpr UInt32 kSpinRounds = 40;
constexpr UInt32 kMaxBackoff = 16;

auto isMultiCoreSystem() noexcept -> bool
{
    static const bool isMultiCore = threading::AvailableParallelism() != 1;
    return isMultiCore;
}

} // namespace

VIOLET_NO_INLINE
void SpinLock::lockSlow(UInt32 observed) noexcept
{
    if (isMultiCoreSystem()) {
        UInt32 backoff = 1;
        for (UInt32 round = 0; round < kSpinRounds && observed != kContended; round++) {
            for (UInt32 i = 0; i < backoff; i++) {
                RelaxCPU();
            }

            backoff = std::min(backoff << 1, kMaxBackoff);
            observed = this->n_state.load(std::memory_order_relaxed);
            if (observed == kUnlocked
                && this->n_state.compare_exchange_weak(
                    observed, kLocked, std::memory_order_acquire, std::memory_order_relaxed)) {
                return;
            }
        }
    }

    if (observed != kContended) {
        observed = this->n_state.exchange(kContended, std::memory_order::acquire);
    }

    while (observed != kUnlocked) {
        spin_lock_internal::waitOn(this->n_state, kContended);
        observed = this->n_state.exchange(kContended, std::memory_order_acquire);
    }
}

VIOLET_NO_INLINE
void SpinLock::unlockSlow() noexcept
{
    spin_lock_internal::wakeOne(this->n_state);
}

} // namespace violet::experimental::sync
