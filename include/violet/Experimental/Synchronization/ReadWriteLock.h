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
//! # 🌺💜 `violet/Experimental/Synchronization/ReadWriteLock.h`

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Experimental/Mutex.h>
#include <violet/Experimental/Time/Duration.h>

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE_ABSEIL
#define VIOLET_SHARED_LOCK_FUNCTION(...) ABSL_SHARED_LOCK_FUNCTION(__VA_ARGS__)
#define VIOLET_SHARED_TRYLOCK_FUNCTION(...) ABSL_SHARED_TRYLOCK_FUNCTION(__VA_ARGS__)
#else
#if VIOLET_HAS_ATTRIBUTE(shared_lock_function)
#define VIOLET_SHARED_LOCK_FUNCTION(...) __attribute__((shared_lock_function(__VA_ARGS__)))
#else
#define VIOLET_SHARED_LOCK_FUNCTION(...)
#endif

#if VIOLET_HAS_ATTRIBUTE(shared_trylock_function)
#define VIOLET_SHARED_TRYLOCK_FUNCTION(...) __attribute__((shared_trylock_function(__VA_ARGS__)))
#else
#define VIOLET_SHARED_TRYLOCK_FUNCTION(...)
#endif
#endif

namespace violet::experimental::sync {

template<typename T>
struct ReadWriteLock;

/// RAII guard for reading data. Dereferences to `const T&`.
template<typename T>
class VIOLET_SCOPED_LOCKABLE NOELDOC_EXPERIMENTAL_SINCE("26.07") ReadGuard final {
    friend struct ReadWriteLock<T>;

    struct key final {
        friend struct ReadWriteLock<T>;

    private:
        constexpr VIOLET_EXPLICIT key() = default;
    };

public:
    VIOLET_DISALLOW_COPY_AND_MOVE(ReadGuard);
    VIOLET_DISALLOW_CONSTRUCTOR(ReadGuard);

    /// @internal
    NOELDOC_HIDE VIOLET_EXPLICIT ReadGuard(key, ReadWriteLock<T>* lock, const T* data) noexcept
        VIOLET_SHARED_LOCK_FUNCTION(lock->n_mux)
        : n_lock(lock)
        , n_data(data)
    {
    }

    ~ReadGuard() VIOLET_UNLOCK_FUNCTION()
    {
        if (this->n_lock != nullptr) {
            this->n_lock->unlockReaderSide();
        }
    }

    auto operator*() const noexcept -> const T&
    {
        return *this->n_data;
    }

    auto operator->() const noexcept -> const T*
    {
        return this->n_data;
    }

private:
    ReadWriteLock<T>* n_lock;
    const T* n_data;
};

template<typename T>
class VIOLET_SCOPED_LOCKABLE NOELDOC_EXPERIMENTAL_SINCE("26.07") WriteGuard final {
    friend struct ReadWriteLock<T>;

    struct key final {
        friend struct ReadWriteLock<T>;

    private:
        constexpr VIOLET_EXPLICIT key() = default;
    };

public:
    VIOLET_DISALLOW_CONSTRUCTOR(WriteGuard);
    VIOLET_DISALLOW_COPY_AND_MOVE(WriteGuard);

    /// @internal
    NOELDOC_HIDE VIOLET_EXPLICIT WriteGuard(key, ReadWriteLock<T>* lock, T* data) noexcept
        VIOLET_EXCLUSIVE_LOCK_FUNCTION(lock->n_mux)
        : n_lock(lock)
        , n_data(data)
    {
    }

    ~WriteGuard() VIOLET_UNLOCK_FUNCTION()
    {
        if (this->n_lock != nullptr) {
            this->n_lock->unlockWriterSide();
        }
    }

    auto operator*() noexcept -> T&
    {
        return *this->n_data;
    }

    auto operator*() const noexcept -> const T&
    {
        return *this->n_data;
    }

    auto operator->() noexcept -> T*
    {
        return this->n_data;
    }

    auto operator->() const noexcept -> const T*
    {
        return this->n_data;
    }

private:
    ReadWriteLock<T>* n_lock;
    T* n_data;
};

/// A reader-writer lock that protects a value of type `T`.
///
/// Access to data is only possible through [`ReadWriteLock::Read`], which can have multiple callees
/// to *read data* and [`ReadWriteLock::Write`], which can have a single guard to access and modify data.
/// There is no "peek without locking" escape hatches. If you need that, then... I don't really know what
/// to tell you.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Synchronization/ReadWriteLock.h>
///
/// using namespace violet::experimental::sync;
/// using namespace violet;
///
/// ReadWriteLock<Vec<Int32>> data({ 1, 2, 3 });
///
/// // read path; multiple threads can do this concurrently
/// {
///     auto vec = data.Read();
///     for (auto& v: *vec) { /* ... */ }
/// }
///
/// // write path; exclusive
/// {
///     auto vec = data.Write();
///     vec->push_back(32);
/// }
/// ```
template<typename T>
struct VIOLET_SCOPED_LOCKABLE NOELDOC_EXPERIMENTAL_SINCE("26.07") ReadWriteLock final {
    static_assert(!std::is_reference_v<T>, "`T` cannot be a reference");
    static_assert(std::is_destructible_v<T>, "`T` must be destructible");

    VIOLET_DISALLOW_COPY_AND_MOVE(ReadWriteLock);

    VIOLET_IMPLICIT ReadWriteLock()
        requires(std::default_initializable<T>)
    = default;

    template<typename U = T>
        requires(!std::same_as<std::remove_cvref_t<U>, ReadWriteLock<T>> && std::constructible_from<T, U &&>)
    VIOLET_EXPLICIT ReadWriteLock(U&& value) noexcept(
        std::is_nothrow_constructible_v<T, U&&> && std::is_nothrow_move_constructible_v<U>)
        : n_data(VIOLET_FWD(U, value))
    {
    }

    template<typename... Args>
        requires(std::constructible_from<T, Args...>
            && (sizeof...(Args) != 1
                || !std::same_as<std::remove_cvref_t<std::tuple_element_t<0, std::tuple<Args && ...>>>, ReadWriteLock>))
    VIOLET_IMPLICIT ReadWriteLock(Args&&... args)
        : n_data(VIOLET_FWD(Args, args)...)
    {
    }

    VIOLET_EXPLICIT ReadWriteLock(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires(std::copy_constructible<T>)
        : n_data(value)
    {
    }

    VIOLET_IMPLICIT ReadWriteLock(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires(std::move_constructible<T>)
        : n_data(VIOLET_MOVE(value))
    {
    }

    ~ReadWriteLock() = default;

    // NOLINTBEGIN(modernize-use-trailing-return-type) -- needed so that these functions
    // can have thread safety (hopefully)

    /// Acquires a read lock, blocking until no writer is active.
    ///
    /// Multiple readers may hold guards simultaneously.
    [[nodiscard]] ReadGuard<T> Read() VIOLET_SHARED_LOCK_FUNCTION(this->n_mux)
    {
        this->await(
            // clang-format off
            [](const auto& self) -> bool { return !self.n_writing; },
            [](auto& self) -> void { self.n_readers++; }
            // clang-format on
        );

        constexpr typename ReadGuard<T>::key key{ };
        return ReadGuard<T>(key, this, &this->n_data);
    }

    /// Acquires an exclusive write lock, blocking until no readers
    /// or writers are active.
    [[nodiscard]] WriteGuard<T> Write() VIOLET_EXCLUSIVE_LOCK_FUNCTION(this->n_mux)
    {
        this->await(
            // clang-format off
            [](const auto& self) -> bool { return !self.n_writing && self.n_readers == 0; },
            [](auto& self) -> void { self.n_writing = true; }
            // clang-format on
        );

        constexpr typename WriteGuard<T>::key key{ };
        return WriteGuard<T>(key, this, &this->n_data);
    }

    /// Attempts to acquire a read lock without blocking.
    ///
    /// Returns [`violet::Nothing`] if a writer is currently active.
    Optional<ReadGuard<T>> TryRead() VIOLET_SHARED_TRYLOCK_FUNCTION(true, this->n_mux)
    {
        if (!this->n_mux.TryLock()) {
            return Nothing;
        }

        if (this->n_writing) {
            this->n_mux.Unlock();
            return Nothing;
        }

        this->n_readers++;
        this->n_mux.Unlock();

        constexpr typename ReadGuard<T>::key key{ };
        return Optional<ReadGuard<T>>(std::in_place, key, this, &this->n_data);
    }

    /// Attempts to acquire a write lock without blocking.
    ///
    /// Returns [`violet::Nothing`] if any readers or a writer are active.
    Optional<WriteGuard<T>> TryWrite() VIOLET_EXCLUSIVE_TRYLOCK_FUNCTION(true, this->n_mux)
    {
        if (!this->n_mux.TryLock()) {
            return Nothing;
        }

        if (this->n_writing || this->n_readers > 0) {
            this->n_mux.Unlock();
            return Nothing;
        }

        this->n_writing = true;
        this->n_mux.Unlock();

        constexpr typename WriteGuard<T>::key key{ };
        return Optional<WriteGuard<T>>(std::in_place, key, this, &this->n_data);
    }

    /// Acquires a read lock, blocking until no writer is active
    /// **and** `predicate(value)` returns `true`.
    ///
    /// The predicate is evaluated under the mutex and may be called
    /// multiple times as the condition is re-checked after spurious
    /// wakeups or state changes.
    ///
    /// ## Example
    /// ```cpp
    /// ReadWriteLock<Vec<Int32>> data;
    ///
    /// // Block until the vector is non-empty and no writer is active.
    /// auto guard = data.ReadWhen([](const Vec<Int32>& v) {
    ///     return !v.empty();
    /// });
    /// ```
    template<typename Fun>
        requires(callable<Fun, const T&> && callable_returns<Fun, bool, const T&>)
    [[nodiscard]] ReadGuard<T> ReadWhen(Fun&& predicate) VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        this->await(
            // clang-format off
            [pred = VIOLET_FWD(Fun, predicate)](const auto& self) -> bool {
                return !self.n_writing && std::invoke(pred, self.n_data);
            },
            [](auto& self) -> void { self.n_readers++; }
            // clang-format on
        );

        constexpr typename ReadGuard<T>::key key{ };
        return ReadGuard<T>(key, this, &this->n_data);
    }

    /// Acquires an exclusive write lock, blocking until no readers or
    /// writers are active **and** `predicate(value)` returns `true`.
    ///
    /// ## Example
    /// ```cpp
    /// ReadWriteLock<Vec<Int32>> data;
    ///
    /// // Block until the vector has room and nobody else holds the lock.
    /// auto guard = data.WriteWhen([](const Vec<Int32>& v) {
    ///     return v.size() < 1024;
    /// });
    ///
    /// guard->push_back(42);
    /// ```
    template<typename Fun>
        requires(callable<Fun, const T&> && callable_returns<Fun, bool, const T&>)
    [[nodiscard]] WriteGuard<T> WriteWhen(Fun&& predicate) VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        this->await(
            // clang-format off
            [pred = VIOLET_FWD(Fun, predicate)](const auto& self) -> bool {
                return !self.n_writing && self.n_readers == 0 && std::invoke(pred, self.n_data);
            },
            [](auto& self) -> void { self.n_writing = true; }
            // clang-format on
        );

        constexpr typename WriteGuard<T>::key key{ };
        return WriteGuard<T>(key, this, &this->n_data);
    }

    /// Attempts to acquire a read lock, blocking for at most `timeout`.
    ///
    /// Returns [`violet::Nothing`] if the timeout expires before a read
    /// lock can be acquired (i.e. a writer remained active for the
    /// entire duration).
    [[nodiscard]] Optional<ReadGuard<T>> TryReadUntil(chrono::Duration dur) VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        return this->TryReadUntil(dur.Cast<std::chrono::nanoseconds>());
    }

#if VIOLET_FEATURE(ABSEIL)
    /// Attempts to acquire a read lock, blocking for at most `timeout`.
    ///
    /// Returns [`violet::Nothing`] if the timeout expires before a read
    /// lock can be acquired (i.e. a writer remained active for the
    /// entire duration).
    [[nodiscard]] Optional<ReadGuard<T>> TryReadUntil(absl::Duration dur) VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        return this->TryReadUntil(absl::ToChronoNanoseconds(dur));
    }
#endif

    /// Attempts to acquire a read lock, blocking for at most `timeout`.
    ///
    /// Returns [`violet::Nothing`] if the timeout expires before a read
    /// lock can be acquired (i.e. a writer remained active for the
    /// entire duration).
    template<typename Rep, typename Period>
    [[nodiscard]] Optional<ReadGuard<T>> TryReadUntil(
        std::chrono::duration<Rep, Period> dur) VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        auto deadline = std::chrono::steady_clock::now() + dur;
        this->n_mux.Lock();

        constexpr auto kZeroNanos = std::chrono::nanoseconds::zero();
        while (this->n_writing) {
            auto remaining
                = std::chrono::duration_cast<std::chrono::nanoseconds>(deadline - std::chrono::steady_clock::now());

            if (remaining <= kZeroNanos) {
                this->n_mux.Unlock();
                return Nothing;
            }

            if (!this->n_cv.WaitWithTimeout(&this->n_mux, remaining)) {
                // we are in the timed out stage, let's check one last time
                // since the signal could've arrived right at the deadline
                if (this->n_writing) {
                    this->n_mux.Unlock();
                    return Nothing;
                }
            }
        }

        this->n_readers++;
        this->n_mux.Unlock();

        constexpr typename ReadGuard<T>::key key{ };
        return Optional<ReadGuard<T>>(std::in_place, key, this, &this->n_data);
    }

    /// Attempts to acquire an exclusive write lock, blocking for at
    /// most `timeout`.
    ///
    /// Returns [`violet::Nothing`] if the timeout expires before exclusive
    /// access can be obtained (i.e. readers or another writer remained
    /// active for the entire duration).
    [[nodiscard]] Optional<WriteGuard<T>> TryWriteUntil(chrono::Duration dur) VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        return this->TryWriteUntil(dur.Cast<std::chrono::nanoseconds>());
    }

#if VIOLET_FEATURE(ABSEIL)
    /// Attempts to acquire an exclusive write lock, blocking for at
    /// most `timeout`.
    ///
    /// Returns [`violet::Nothing`] if the timeout expires before exclusive
    /// access can be obtained (i.e. readers or another writer remained
    /// active for the entire duration).
    [[nodiscard]] Optional<WriteGuard<T>> TryWriteUntil(absl::Duration dur) VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        return this->TryWriteUntil(absl::ToChronoNanoseconds(dur));
    }
#endif

    /// Attempts to acquire an exclusive write lock, blocking for at
    /// most `timeout`.
    ///
    /// Returns [`violet::Nothing`] if the timeout expires before exclusive
    /// access can be obtained (i.e. readers or another writer remained
    /// active for the entire duration).
    template<typename Rep, typename Period>
    [[nodiscard]] Optional<WriteGuard<T>> TryWriteUntil(
        std::chrono::duration<Rep, Period> dur) VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        auto deadline = std::chrono::steady_clock::now() + dur;
        this->n_mux.Lock();

        constexpr auto kZeroNanos = std::chrono::nanoseconds::zero();
        while (this->n_writing || this->n_readers > 0) {
            auto remaining
                = std::chrono::duration_cast<std::chrono::nanoseconds>(deadline - std::chrono::steady_clock::now());

            if (remaining <= kZeroNanos) {
                this->n_mux.Unlock();
                return Nothing;
            }

            if (!this->n_cv.WaitWithTimeout(&this->n_mux, remaining)) {
                // we are in the timed out stage, let's check one last time
                // since the signal could've arrived right at the deadline
                if (this->n_writing || this->n_readers > 0) {
                    this->n_mux.Unlock();
                    return Nothing;
                }
            }
        }

        this->n_writing = true;
        this->n_mux.Unlock();

        constexpr typename WriteGuard<T>::key key{ };
        return Optional<WriteGuard<T>>(std::in_place, key, this, &this->n_data);
    }

    // NOLINTEND(modernize-use-trailing-return-type)

private:
    friend struct ReadGuard<T>;
    friend struct WriteGuard<T>;

    Mutex n_mux;
    Condvar n_cv;
    T n_data{ };
    UInt32 n_readers VIOLET_GUARDED_BY(this->n_mux) = 0;
    bool n_writing VIOLET_GUARDED_BY(this->n_mux) = false;

    /// Waits until `predicate` returns `true`, then executes `action` while still holding
    /// the mutex.
    ///
    /// The `predicate` receives a const reference to the lock (`*this`) and is evaluated
    /// during the mutex lock. It may be called multiple times due to spurious wakeups or
    /// interleaved signals. Once `predicate` returns **true**, `action` is invoked immediately,
    /// before the mutex is released, which guarantees the state observed by the predicate cannot
    /// change between the check and side-effect.
    ///
    /// @param predicate a callable function describing the condition to wait for.
    /// @param action    a callable function executed atomically with the predicate under the held mutex.
    template<typename Fun, typename Action>
        requires(callable<Fun, const ReadWriteLock&> && callable_returns<Fun, bool, const ReadWriteLock&>
            && callable<Action, ReadWriteLock&> && callable_returns<Action, void, ReadWriteLock&>)
    void await(Fun&& predicate, Action&& action)
    {
#if VIOLET_FEATURE(ABSEIL)
        MutexLock lock(this->n_mux);
        this->n_mux.Await([this, pred = VIOLET_FWD(Fun, predicate)] -> bool { return std::invoke(pred, *this); });
#else
        MutexLock lock(this->n_mux);

        auto pred = VIOLET_FWD(Fun, predicate);
        while (!std::invoke(pred, *this)) {
            this->n_cv.Wait(&this->n_mux);
        }
#endif

        std::invoke(VIOLET_FWD(Action, action), *this);
    }

    void unlockReaderSide() VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        MutexLock lock(this->n_mux);
        this->n_readers--;
        if (this->n_readers == 0) {
            this->n_cv.SignalAll();
        }
    }

    void unlockWriterSide() VIOLET_NO_THREAD_SAFETY_ANALYSIS
    {
        MutexLock lock(this->n_mux);
        this->n_writing = false;
        this->n_cv.SignalAll();
    }
};

} // namespace violet::experimental::sync
