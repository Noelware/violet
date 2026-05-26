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
//! # 🌺💜 `violet/Violet.h`

#pragma once

#include <violet/Violet.h>

#include <functional>

namespace violet {

/// A non-copyable, non-movable scope guard that invokes a callable when it goes
/// out of scope, analgous to the `defer` keyword in Go or the [`scopeguard`] Rust
/// crate.
///
/// [`scopeguard`]: https://docs.rs/scopeguard/latest/scopeguard/
///
/// **Defer** is non-copyable and non-movable, it must remain in the scope
/// it was created in. If you need to share a deferrable action across scopes,
/// wrap the owning scope around a `violet::SharedPtr<violet::Defer>`.
///
/// You can construct a **Defer** like you would normally or using the [`VIOLET_DEFER`] macro,
/// which provides a unique name instead of naming it yourself, if you're weird like that.
///
/// ```cpp
/// #include <violet/Violet.h>
///
/// {
///     // 1. Normal Usage
///     violet::Defer _guard([&] -> void {
///         violet::Println("Hello, world!");
///     });
///
///     // 2. Using the `VIOLET_DEFER` macro
///     VIOLET_DEFER({
///         violet::Println("Hello, world 2: electric boogaloo");
///     });
/// }
/// ```
///
/// When multiple guards are in the same scope, they're destroyed in **reverse declaration order**,
/// matching C++ destructor semantics. In the example above, `_defer_block_0` (the generated name
/// when using [`VIOLET_DEFER`]) will be called first then `_guard` will be called last.
template<typename Fun>
    requires(callable<Fun> && callable_returns<Fun, void>)
struct NOELDOC_SINCE("26.05.05") Defer final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Defer);
    VIOLET_DISALLOW_CONSTRUCTOR(Defer);

    /// Constructs a new `Defer` object with a given callable, which will be invoked
    /// when the object is destroyed.
    VIOLET_EXPLICIT Defer(Fun&& fun) noexcept
        : n_fun(VIOLET_MOVE(fun))
    {
    }

    ~Defer() noexcept(noexcept(std::invoke(std::declval<Fun>())))
    {
        std::invoke(this->n_fun);
    }

private:
    Fun n_fun;
};

template<typename Fun>
Defer(Fun&&) -> Defer<std::decay_t<Fun>>;

/// A non-copyable, non-movable scope guard like [`violet::Defer`] that can be cancelled
/// before it fires.
///
/// When [`CancellableDefer::Cancel()`] is called, the store callable ***WILL NOT*** be invoked
/// when the guard goes out of scope. This is useful when a deferred cleanup action should only run
/// on failure paths. For example, in Noelware's [Eous](https://eous.noelware.cloud) Project reconcillation loop,
/// we want to create a network with a given driver and rollback if the transaction didn't execute successfully:
///
/// ```cpp
/// // Create the network on the driver side.
/// VIOLET_TRY_VOID(driver->Create(network));
///
/// violet::CancellableDefer rollback([&] -> void {
///     (void)driver->Destroy(network);
/// });
///
/// // Commit the new network into the RocksDB storage
/// VIOLET_TRY_VOID(store->Commit(network));
///
/// // At this point, when we go out of scope and `rollback.Cancel()` was set,
/// // then the callback is never executed.
/// rollback.Cancel();
/// ```
///
/// ## Remarks
/// Once [`CancellableDefer::Cancel()`] has been called, it cannot be undone. If you need
/// re-arming semantics, implement it yourself.
///
/// Since **26.07**, the `ThreadSafety` template parameter has been removed in favour of [`LocalCancellableDefer`].
template<typename Fun>
struct NOELDOC_SINCE("26.05.05") CancellableDefer final {
    VIOLET_DISALLOW_COPY_AND_MOVE(CancellableDefer);
    VIOLET_DISALLOW_CONSTRUCTOR(CancellableDefer);

    VIOLET_IMPLICIT CancellableDefer(Fun&& fun) noexcept
        : n_fun(VIOLET_MOVE(fun))
    {
    }

    ~CancellableDefer() noexcept(noexcept(std::invoke(std::declval<Fun>())))
    {
        if (!this->Cancelled()) {
            std::invoke(this->n_fun);
        }
    }

    /// Cancels the deferrable action. Cannot be undone.
    void Cancel() noexcept
    {
        this->n_cancelled.store(true, std::memory_order_release);
    }

    /// Returns **true** if this deferrable action was cancelled.
    [[nodiscard]] auto Cancelled() const noexcept -> bool
    {
        return this->n_cancelled.load(std::memory_order_acquire);
    }

private:
    Fun n_fun;
    std::atomic<bool> n_cancelled = false;
};

template<typename Fun>
CancellableDefer(Fun&&) -> CancellableDefer<std::decay_t<Fun>>;

/// Same as [`CancellableDefer`], but doesn't use atomics.
template<typename Fun>
struct NOELDOC_SINCE("26.07") LocalCancellableDefer final {
    VIOLET_DISALLOW_COPY_AND_MOVE(LocalCancellableDefer);
    VIOLET_DISALLOW_CONSTRUCTOR(LocalCancellableDefer);

    VIOLET_IMPLICIT LocalCancellableDefer(Fun&& fun) noexcept
        : n_fun(VIOLET_MOVE(fun))
    {
    }

    ~LocalCancellableDefer() noexcept(noexcept(std::invoke(std::declval<Fun>())))
    {
        if (!this->Cancelled()) {
            std::invoke(this->n_fun);
        }
    }

    /// Cancels the deferrable action. Cannot be undone.
    void Cancel() noexcept
    {
        this->n_cancelled = true;
    }

    /// Returns **true** if this deferrable action was cancelled.
    [[nodiscard]] auto Cancelled() const noexcept -> bool
    {
        return this->n_cancelled;
    }

private:
    Fun n_fun;
    bool n_cancelled = false;
};

template<typename Fun>
LocalCancellableDefer(Fun&&) -> LocalCancellableDefer<std::decay_t<Fun>>;

} // namespace violet

/**
 * @macro VIOLET_DEFER
 * @since 26.05.05
 * @param fn A brace-enclosed block to execute at scope exit.
 *
 * Creates an anonymous `violet::Defer` object that runs @p fn when the
 * enclosing scope exits. Uses `VIOLET_UNIQUE_NAME` internally to avoid
 * naming collisions.
 *
 * # Example
 *
 * ```cpp
 * void h() {
 *     VIOLET_DEFER({ violet::Println("hello, world!"); });
 * }
 * ```
 *
 * @see VIOLET_CANCELLABLE_DEFER
 * @see VIOLET_LOCAL_CANCELLABLE_DEFER
 */
#define VIOLET_DEFER(fn) ::violet::Defer VIOLET_UNIQUE_NAME(_defer_block_)([&]()->void fn);

/**
 * @macro VIOLET_CANCELLABLE_DEFER
 * @since 26.05.05
 * @param fn A brace-enclosed block to execute at scope exit.
 *
 * Like `VIOLET_DEFER`, but creates a `violet::CancellableDefer` whose
 * execution can be cancelled before scope exit via its `Cancel()` method.
 *
 * @see VIOLET_DEFER
 * @see VIOLET_LOCAL_CANCELLABLE_DEFER
 */
#define VIOLET_CANCELLABLE_DEFER(fn)                                                                                   \
    ::violet::CancellableDefer VIOLET_UNIQUE_NAME(_cancellable_defer_block_)([&]()->void fn);

/**
 * @macro VIOLET_LOCAL_CANCELLABLE_DEFER
 * @since 26.07
 * @param fn A brace-enclosed block to execute at scope exit.
 *
 * Like `VIOLET_CANCELLABLE_DEFER`, but creates a
 * `violet::LocalCancellableDefer`, a non-thread-safe variant that avoids
 * atomic overhead when cancellation is only performed from the same thread.
 *
 * @see VIOLET_DEFER
 * @see VIOLET_CANCELLABLE_DEFER
 */
#define VIOLET_LOCAL_CANCELLABLE_DEFER(fn)                                                                             \
    ::violet::LocalCancellableDefer VIOLET_UNIQUE_NAME(_local_cancellable_defer_block_)([&]()->void fn);
