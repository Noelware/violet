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
//! # 🌺💜 `violet/Experimental/Synchronized.h`

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Experimental/Mutex.h>

#include <utility>

namespace violet::experimental {

/// A wrapper that pairs shared data with a [`Mutex`], enforcing that data is only accessible
/// while the lock is held.
///
/// `Synchronized<T>` prevents accidental unsynchronized access by making it
/// impossible to reach the inner `T` without first acquiring the mutex. Access
/// is granted through [`Lock`], which returns a [`Guard`] RAII handle that
/// dereferences to the protected value.
///
/// **Synchronized<T>** is modeled by Rust's [`Mutex<T>`].
///
/// [`Mutex<T>`]: https://doc.rust-lang.org/std/1.95.0/std/sync/struct.Mutex.html
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Synchronized.h>
///
/// using namespace violet::experimental;
/// using namespace violet;
///
/// Synchronized<Vec<Int32>> queue;
/// {
///     auto lock = queue.Lock();
///     lock->push_back(42);
/// }
///
/// // `TryLock` for non-blocking access
/// if (auto guard = queue->TryLock()) {
///     guard->push_back(69);
/// }
/// ```
///
/// @tparam type of the protected data. Must be move-constructible.
template<typename T>
struct VIOLET_API VIOLET_LOCKABLE Synchronized final {
    VIOLET_DISALLOW_COPY(Synchronized);
    ~Synchronized() = default;

    /// An RAII guard granting exclusive access to the protected data.
    ///
    /// The underlying [`Mutex`] is held for the lifetime of the `Guard`. The
    /// protected value is accessible via `operator*` and `operator->`.
    struct VIOLET_API VIOLET_SCOPED_LOCKABLE Guard final {
        VIOLET_DISALLOW_COPY(Guard);

        VIOLET_IMPLICIT Guard(Guard&& guard) noexcept VIOLET_NO_THREAD_SAFETY_ANALYSIS
            : n_sync(std::exchange(guard.n_sync, nullptr))
        {
        }

        auto operator=(Guard&&) noexcept -> Guard& = delete;

        ~Guard() VIOLET_UNLOCK_FUNCTION()
        {
            if (this->n_sync != nullptr) {
                this->n_sync->n_mux.Unlock();
                this->n_sync = nullptr;
            }
        }

        auto operator*() -> T&
        {
            return this->n_sync->n_data;
        }

        auto operator*() const -> T&
        {
            return this->n_sync->n_data;
        }

        auto operator->() -> T*
        {
            return &this->n_sync->n_data;
        }

        auto operator->() const -> T*
        {
            return &this->n_sync->n_data;
        }

    private:
        friend struct Synchronized;

        VIOLET_IMPLICIT Guard() noexcept = default;
        VIOLET_IMPLICIT Guard(Synchronized* sync, std::adopt_lock_t) VIOLET_EXCLUSIVE_LOCKS_REQUIRED(sync->n_mux)
            : n_sync(sync)
        {
        }

        Synchronized* n_sync = nullptr;
    };

    /// Constructs a `Synchronized<T>` with a default-constructed `T`.
    VIOLET_IMPLICIT Synchronized()
        requires(std::is_default_constructible_v<T>)
    = default;

    /// Constructs a `Synchronized<T>` by moving `value` into the protected storage.
    /// @param value initial value to protect.
    VIOLET_IMPLICIT Synchronized(T value)
        : n_data(VIOLET_MOVE(value))
    {
    }

    /// Constructs the protected `T` in-place from `args`.
    /// @param args arguments forwarded to `T`'s constructor.
    template<typename... Args>
    VIOLET_EXPLICIT Synchronized(std::in_place_t, Args&&... args)
        : n_data(VIOLET_FWD(Args, args)...)
    {
    }

    /// Move-constructs from another `Synchronized<T>`, locking the source first.
    VIOLET_IMPLICIT Synchronized(Synchronized&& other) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        other.n_mux.Lock();
        this->n_data = VIOLET_MOVE(other.n_data); // NOLINT(cppcoreguidelines-prefer-member-initializer)
        other.n_mux.Unlock();
    }

    auto operator=(Synchronized&& other) noexcept -> Synchronized& = delete;

    // NOLINTBEGIN(modernize-use-trailing-return-type)

    /// Acquires the mutex and returns a RAII guard granting access to the protected data.
    Guard Lock() VIOLET_EXCLUSIVE_LOCK_FUNCTION(n_mux)
    {
        this->n_mux.Lock();
        return Guard(this, std::adopt_lock);
    }

    /// Attempts to acquire the mutex without blocking. If the mutex isn't locked, then
    /// a RAII guard is returned otherwise [`violet::Nothing`] is used as "we are locked"
    Optional<Guard> TryLock() VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        if (!this->n_mux.TryLock()) {
            return Nothing;
        }

        return Guard(this, std::adopt_lock);
    }

    // NOLINTEND(modernize-use-trailing-return-type)

    /// Returns a reference to the underlying mutex.
    ///
    /// This is primarily useful for passing the mutex to a conditional variable ([`Condvar`]). It does
    /// not grant access to the protected data.
    auto Mutex() noexcept -> violet::experimental::Mutex&
    {
        return this->n_mux;
    }

private:
    friend struct Guard;

    T n_data{ };
    mutable violet::experimental::Mutex n_mux;
};

} // namespace violet::experimental
