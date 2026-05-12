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
//! # 🌺💜 `violet/Experimental/Mutex.h`
//! **violet::experimental::Mutex** is a weird one. We don't implement our own implementation
//! as we prefer to use Abseil's `Mutex`, but we know that `Noelware.Violet` shouldn't really
//! rely on thirdparty dependencies, so we made this opt-in via feature flags:
//!
//! * Bazel: `--@violet//buildsystem/bazel/flags:abseil=True`
//! * Meson: `-Dabseil=true`
//! * CMake: `-DVIOLET_USE_ABSEIL:BOOL=ON`
//! * Foreign: set `-DVIOLET_FEATURE_ABSEIL=1` when compiling this header
//!
//! Features available on Abseil-enabled builds (documentated, not bridged):
//! * Deadlock detection (`EnableDebugLog`, `ForgetDeadlockInfo`)
//! * Invariant debugging (`EnableInvariantDebugging`)
//! * Reader/writer separation lives in `violet::experimental::sync::ReadWriteLock` instead. **Mutex**
//!   is exclusive only.

#pragma once

#include <violet/Violet.h>

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE_ABSEIL
#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"

#define VIOLET_LOCKABLE ABSL_LOCKABLE
#define VIOLET_SCOPED_LOCKABLE ABSL_SCOPED_LOCKABLE
#define VIOLET_GUARDED_BY(...) ABSL_GUARDED_BY(__VA_ARGS__)
#define VIOLET_PT_GUARDED_BY(...) ABSL_PT_GUARDED_BY(__VA_ARGS__)
#define VIOLET_EXCLUSIVE_LOCKS_REQUIRED(...) ABSL_EXCLUSIVE_LOCKS_REQUIRED(__VA_ARGS__)
#define VIOLET_EXCLUSIVE_LOCK_FUNCTION(...) ABSL_EXCLUSIVE_LOCK_FUNCTION(__VA_ARGS__)
#define VIOLET_UNLOCK_FUNCTION(...) ABSL_UNLOCK_FUNCTION(__VA_ARGS__)
#define VIOLET_EXCLUSIVE_TRYLOCK_FUNCTION(...) ABSL_EXCLUSIVE_TRYLOCK_FUNCTION(__VA_ARGS__)
#else
#include <atomic>
#include <condition_variable>
#include <mutex>

#if VIOLET_HAS_ATTRIBUTE(lockable)
#define VIOLET_LOCKABLE __attribute__((lockable))
#else
#define VIOLET_LOCKABLE
#endif

#if VIOLET_HAS_ATTRIBUTE(scoped_lockable)
#define VIOLET_SCOPED_LOCKABLE __attribute__((scoped_lockable))
#else
#define VIOLET_SCOPED_LOCKABLE
#endif

#if VIOLET_HAS_ATTRIBUTE(guarded_by)
#define VIOLET_GUARDED_BY(...) __attribute__((guarded_by(__VA_ARGS__)))
#else
#define VIOLET_GUARDED_BY(...)
#endif

#if VIOLET_HAS_ATTRIBUTE(pt_guarded_by)
#define VIOLET_PT_GUARDED_BY(...) __attribute__((pt_guarded_by(__VA_ARGS__)))
#else
#define VIOLET_PT_GUARDED_BY(...)
#endif

#if VIOLET_HAS_ATTRIBUTE(exclusive_lock_function)
#define VIOLET_EXCLUSIVE_LOCK_FUNCTION(...) __attribute__((exclusive_lock_function(__VA_ARGS__)))
#else
#define VIOLET_EXCLUSIVE_LOCK_FUNCTION(...)
#endif

#if VIOLET_HAS_ATTRIBUTE(unlock_function)
#define VIOLET_UNLOCK_FUNCTION(...) __attribute__((unlock_function(__VA_ARGS__)))
#else
#define VIOLET_UNLOCK_FUNCTION(...)
#endif

#if VIOLET_HAS_ATTRIBUTE(exclusive_locks_required)
#define VIOLET_EXCLUSIVE_LOCKS_REQUIRED(...) __attribute__((exclusive_locks_required(__VA_ARGS__)))
#else
#define VIOLET_EXCLUSIVE_LOCKS_REQUIRED(...)
#endif

#if VIOLET_HAS_ATTRIBUTE(exclusive_trylock_function)
#define VIOLET_EXCLUSIVE_TRYLOCK_FUNCTION(...) __attribute__((exclusive_trylock_function(__VA_ARGS__)))
#else
#define VIOLET_EXCLUSIVE_TRYLOCK_FUNCTION(...)
#endif

#endif

#if VIOLET_HAS_ATTRIBUTE(no_thread_safety_analysis)
#define VIOLET_NO_THREAD_SAFETY_ANALYSIS __attribute__((no_thread_safety_analysis))
#else
#define VIOLET_NO_THREAD_SAFETY_ANALYSIS
#endif

namespace violet::experimental {
namespace chrono {
    struct Duration;
}

struct Condvar;

/// A mutual exclusion primitive for protecting shared data bridged by [`absl::Mutex`] on Abseil-enabled
/// builds or [`std::mutex`] otherwise. View the [module documentation](#) on why we decided to bridge
/// versus building our own.
///
/// Violet's Mutex provides exclusive access to shared data ensuring that only one thread
/// can access the protected resource at a time. This lockable class also supports C++'s standard
/// `Lockable` named requirements, making it compatible with [`std::unique_lock`], [`std::lock_guard`],
/// and other RAII lock wrappers.
///
/// We decided to split the structure of "read / write" locks to
/// [`ReadWriteLock`][violet::experimental::sync::ReadWriteLock], while `Mutex` is an "exclusive only" lock.
///
/// When built with Abseil support (`VIOLET_FEATURE_ABSEIL`), this wraps `absl::Mutex`
/// and gains access to Abseil's efficient condition-based waiting via `Await` and
/// `LockUntil`. Otherwise, it falls back to `std::mutex` with a
/// `std::condition_variable` for equivalent functionality.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Mutex.h>
///
/// using violet::experimental::Mutex;
/// using violet::experimental::MutexLock;
///
/// Mutex mux;
///
/// // manual lock/unlock
/// mux.Lock();
/// /* do some computation here */
/// mux.Unlock();
///
/// // RAII via MutexLock
/// {
///     MutexLock lock(mux);
///     /* access shared data here */
/// }
/// /* lock is dropped */
///
/// // Wait for a condition while holding the lock
/// mux.Lock();
/// mux.Await([&] -> bool { return queue.size() > 0; });
/// /* condition was reached and returned `true`, lock is still held though */
/// mux.Unlock();
/// ```
struct VIOLET_API VIOLET_LOCKABLE Mutex final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Mutex);
    VIOLET_IMPLICIT Mutex() = default;
    ~Mutex() = default;

    /// Acquires the mutex, blocking the current thread until it is available.
    void Lock() VIOLET_EXCLUSIVE_LOCK_FUNCTION();

    /// Releases the mutex.
    ///
    /// The caller must hold the lock when calling this method.
    void Unlock() VIOLET_UNLOCK_FUNCTION();

    // NOLINTBEGIN(modernize-use-trailing-return-type)

    /// Attempts to acquire the mutex without blocking. If the acquision was successful,
    /// **true** is returned, otherwise `false`.
    bool TryLock() VIOLET_EXCLUSIVE_TRYLOCK_FUNCTION(true);

    // NOLINTEND(modernize-use-trailing-return-type)

    /// Implemented for parity of C++'s [`BasicLockable`] named requirement.
    ///
    /// [`BasicLockable`]: https://en.cppreference.com/cpp/named_req/BasicLockable
    void lock() VIOLET_EXCLUSIVE_LOCK_FUNCTION()
    {
        this->Lock();
    }

    /// Implemented for parity of C++'s [`BasicLockable`] named requirement.
    ///
    /// [`BasicLockable`]: https://en.cppreference.com/cpp/named_req/BasicLockable
    void unlock() VIOLET_UNLOCK_FUNCTION()
    {
        this->Unlock();
    }

    // NOLINTBEGIN(modernize-use-trailing-return-type)
    /// Implemented for C++'s [`Lockable`] named requirement.
    ///
    /// [`Lockable`]: https://en.cppreference.com/cpp/named_req/Lockable
    bool try_lock() VIOLET_EXCLUSIVE_TRYLOCK_FUNCTION(true)
    {
        return this->TryLock();
    }
    // NOLINTEND(modernize-use-trailing-return-type)

    /// Blocks until `predicate` returns `true`, keeping the lock held throughout.
    ///
    /// The caller must already hold the lock. The mutex may be temporarily released
    /// internally to allow other threads to make progress, but it is guaranteed to be
    /// held when this method returns and when `predicate` is evaluated.
    ///
    /// @param predicate callable returning `bool` that describes the condition to wait for.
    template<typename F>
    void Await(F&& predicate) VIOLET_EXCLUSIVE_LOCKS_REQUIRED(this);

    /// Acquires the mutex, then blocks until `predicate` returns `true`.
    ///
    /// This is equivalent to calling `Lock()` followed by `Await(predicate)`, but may
    /// be implemented more efficiently.
    ///
    /// @param predicate callable returning `bool` that describes the condition to wait for.
    template<typename F>
    void LockUntil(F&& predicate) VIOLET_EXCLUSIVE_LOCK_FUNCTION();

private:
    friend struct Condvar;

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE_ABSEIL
    absl::Mutex n_mux;
#else
    std::mutex n_mux;
    std::condition_variable n_cv;
    std::atomic<Int32> n_waiters = 0;
#endif
};

/// An RAII guard that holds a [`Mutex`] for the duration of its lifetime.
///
/// The lock is acquired when `MutexLock` is constructed and released when it is
/// destroyed. This ensures the mutex is always released, even if the scope exits
/// via an early return or exception (if enabled).
struct VIOLET_API VIOLET_SCOPED_LOCKABLE MutexLock final {
    VIOLET_DISALLOW_COPY_AND_MOVE(MutexLock);

    /// Releases the held mutex.
    ~MutexLock() VIOLET_UNLOCK_FUNCTION();

    /// Constructs a `MutexLock` that acquires `mux`.
    /// @param mux mutex to acquire and hold.
    VIOLET_EXPLICIT MutexLock(Mutex& mux) VIOLET_EXCLUSIVE_LOCK_FUNCTION(mux);

private:
    Mutex& n_mux;
};

/// A condition variable for coordinating threads waiting on a [`Mutex`].
///
/// `Condvar` allows threads to block until a particular condition becomes true.
/// It is always used in conjunction with a [`Mutex`], the caller must hold the
/// lock before calling [`Wait`] or [`WaitWithTimeout`], and the lock is
/// re-acquired before those methods return.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Mutex.h>
///
/// using violet::experimental::Mutex;
/// using violet::experimental::Condvar;
///
/// Mutex mux;
/// Condvar cv;
/// bool ready = false;
///
/// // waiting thread:
/// mux.Lock();
/// while (!ready) {
///     cv.Wait(&mux);
/// }
///
/// /* proceed with shared data */
/// mux.Unlock();
///
/// // Signalling thread:
/// mux.Lock();
/// ready = true;
///
/// mux.Unlock();
/// cv.Signal();
/// ```
struct VIOLET_API Condvar final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Condvar);
    VIOLET_IMPLICIT Condvar() = default;
    ~Condvar() = default;

    /// Blocks the current thread until the condition variable is signalled.
    ///
    /// The caller must hold `mux`. The mutex is released while waiting and
    /// re-acquired before this method returns.
    ///
    /// @param mux mutex that guards the shared state.
    void Wait(Mutex* mux) VIOLET_EXCLUSIVE_LOCKS_REQUIRED(mux);

    // NOLINTBEGIN(modernize-use-trailing-return-type)

    /// Blocks until the condition variable is signalled or the timeout expires.
    /// Returns **true** if the condition variable was signalled before the timeout expired,
    /// otherwise **false** if the wait timed out.
    ///
    /// @param mux     mutex that guards the shared state.
    /// @param timeout maximum duration to wait as [`std::chrono::nanoseconds`]
    bool WaitWithTimeout(Mutex* mux, std::chrono::nanoseconds timeout) VIOLET_EXCLUSIVE_LOCKS_REQUIRED(mux);

    /// Blocks until the condition variable is signalled or the timeout expires.
    /// Returns **true** if the condition variable was signalled before the timeout expired,
    /// otherwise **false** if the wait timed out.
    ///
    /// @param mux     mutex that guards the shared state.
    /// @param timeout maximum duration to wait as [`violet::experimental::time::Duration`]
    bool WaitWithTimeout(Mutex* mux, chrono::Duration dur) VIOLET_EXCLUSIVE_LOCKS_REQUIRED(mux);

    /// Blocks until the condition variable is signalled or the timeout expires.
    /// Returns **true** if the condition variable was signalled before the timeout expired,
    /// otherwise **false** if the wait timed out.
    ///
    /// This is a convenience overload that accepts any `std::chrono::duration` and
    /// converts it to nanoseconds internally.
    ///
    /// @param mux     mutex that guards the shared state.
    /// @param timeout maximum duration to wait as [`std::chrono::nanoseconds`]
    template<typename Rep, typename Period>
    bool WaitWithTimeout(Mutex* mux, std::chrono::duration<Rep, Period> dur) VIOLET_EXCLUSIVE_LOCKS_REQUIRED(mux)
    {
        return this->WaitWithTimeout(mux, std::chrono::duration_cast<std::chrono::nanoseconds>(dur));
    }

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE_ABSEIL
    /// Blocks until the condition variable is signalled or the timeout expires.
    /// Returns **true** if the condition variable was signalled before the timeout expired,
    /// otherwise **false** if the wait timed out.
    ///
    /// This is a convenience overload that accepts any [`absl::Duration`] and
    /// converts it to [`std::chrono::nanoseconds`]
    ///
    /// @param mux     mutex that guards the shared state.
    /// @param timeout maximum duration to wait as [`std::chrono::nanoseconds`]
    bool WaitWithTimeout(Mutex* mux, absl::Duration dur) VIOLET_EXCLUSIVE_LOCKS_REQUIRED(mux);
#endif
    // NOLINTEND(modernize-use-trailing-return-type)

    /// Wakes one thread waiting on this condition variable.
    void Signal();

    /// Wakes all threads waiting on this condition variable.
    void SignalAll();

private:
#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE_ABSEIL
    absl::CondVar n_cv;
#else
    std::condition_variable n_cv;
#endif
};

template<typename F>
inline void Mutex::LockUntil(F&& predicate)
{
    MutexLock lock(*this);
    this->Await(VIOLET_FWD(F, predicate));
}

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE_ABSEIL

template<typename F>
inline void Mutex::Await(F&& predicate)
{
    using predicate_fn = std::remove_reference_t<F>;
    predicate_fn local = VIOLET_FWD(F, predicate);

    this->n_mux.Await(absl::Condition(+[](predicate_fn* pred) -> bool { return std::invoke(*pred); }, &local));
}

#else

template<typename F>
inline void Mutex::Await(F&& predicate)
{
    std::unique_lock<std::mutex> lock(this->n_mux, std::adopt_lock);
    this->n_waiters.fetch_add(1, std::memory_order_release);
    this->n_cv.wait(lock, VIOLET_FWD(F, predicate));
    this->n_waiters.fetch_sub(1, std::memory_order_release);

    // We most-likely adopted an already-locked mutex; the user still owns
    // the lock and is responsible for calling `Unlock()`. Release the unique lock
    // so its destructor doesn't unlock.
    (void)lock.release();
}

#endif

} // namespace violet::experimental
