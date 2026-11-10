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
//
//! # 🌺💜 `violet/Experimental/Synchronization/SpinLock.h`

#pragma once

#include <violet/Experimental/Mutex.h>

#include <atomic>

namespace violet::experimental::sync {
namespace spin_lock_internal {

void waitOn(std::atomic<UInt32>& word, UInt32 expected) noexcept;
void wakeOne(std::atomic<UInt32>& word) noexcept;

} // namespace spin_lock_internal

/// Tells the CPU thhat the caller of this function is in a busy-wait loop.
///
/// ## Platform Remarks
/// - **x86**:     `pause` (this will avoid a memory-order mis-speculation flush on loop exit and save power)
/// - **aarch64**: `isb`   (same choice as Rust's `spin_loop`; `yield` is a nop on most cores)
/// - Others:      not implemented
VIOLET_API
void RelaxCPU();

/// An adaptive, 4-byte lock. It spins for a bounded time, then sleeps in the kernel (`futex` on Linux,
/// `WaitOnAddress` on Windows, `std::atomic::wait` for other platforms).
///
/// This should be only used in short crticial sections that are usually uncontended. It is neither fair
/// nor reentrant; locking twice on the same thread deadlocks.
struct VIOLET_API VIOLET_LOCKABLE NOELDOC_EXPERIMENTAL_SINCE("26.11") SpinLock final {
    VIOLET_DISALLOW_COPY_AND_MOVE(SpinLock);

    constexpr VIOLET_IMPLICIT SpinLock() noexcept = default;
    ~SpinLock() = default;

    // NOLINTBEGIN(modernize-use-trailing-return-type)

    /// Acquires the lock, spinning briefly and then sleeping until it is available.
    void Lock() noexcept VIOLET_EXCLUSIVE_LOCK_FUNCTION()
    {
        UInt32 observed = kUnlocked;
        VIOLET_LIKELY_IF(this->n_state.compare_exchange_strong(
            observed, kLocked, std::memory_order_acquire, std::memory_order_relaxed))
        {
            return;
        }

        this->lockSlow(observed);
    }

    /// Acquires the lock only if it is free right now. Never spins or sleeps.
    bool TryLock() noexcept VIOLET_EXCLUSIVE_TRYLOCK_FUNCTION(true)
    {
        UInt32 observed = kUnlocked;
        return this->n_state.compare_exchange_strong(
            observed, kLocked, std::memory_order_acquire, std::memory_order_relaxed);
    }

    /// Releases the lock and wakes one sleeper, if there might be one.
    void Unlock() noexcept VIOLET_UNLOCK_FUNCTION()
    {
        const UInt32 previous = this->n_state.exchange(kUnlocked, std::memory_order_release);
        VIOLET_DEBUG_ASSERT(previous != kUnlocked, "`SpinLock::Unlock` called on an unlocked lock");

        VIOLET_UNLIKELY_IF(previous == kContended)
        {
            this->unlockSlow();
        }
    }

    // NOLINTEND(modernize-use-trailing-return-type)

    /// Returns **true** if this spin lock is locked.
    [[nodiscard]]
    auto Locked() const noexcept -> bool
    {
        return this->n_state.load(std::memory_order_relaxed) != kUnlocked;
    }

private:
    static constexpr UInt32 kUnlocked = 0;
    static constexpr UInt32 kLocked = 1;
    static constexpr UInt32 kContended = 2;

    void lockSlow(UInt32 observed) noexcept;
    void unlockSlow() noexcept;

    std::atomic<UInt32> n_state{kUnlocked};
};

/// A RAII guard for [`SpinLock`]; the counterpart of [`MutexLock`].
struct VIOLET_API VIOLET_SCOPED_LOCKABLE NOELDOC_EXPERIMENTAL_SINCE("26.11") SpinLockGuard final {
    VIOLET_DISALLOW_COPY_AND_MOVE(SpinLockGuard);

    VIOLET_EXPLICIT SpinLockGuard(SpinLock& lock) noexcept VIOLET_EXCLUSIVE_LOCK_FUNCTION(lock)
        : n_lock(lock)
    {
        this->n_lock.Lock();
    }

    ~SpinLockGuard() VIOLET_UNLOCK_FUNCTION()
    {
        this->n_lock.Unlock();
    }

private:
    SpinLock& n_lock;
};

static_assert(sizeof(SpinLock) == sizeof(UInt32), "`SpinLock` must stay one futex word");
static_assert(std::atomic<UInt32>::is_always_lock_free, "`SpinLock` requires lock-free 32-bit atomics");

} // namespace violet::experimental::sync
