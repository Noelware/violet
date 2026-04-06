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
// # 🌺💜 `violet/Experimental/Threading/CancellationToken.h`

#pragma once

#include <violet/Events/EventEmitter.h>

namespace violet::experimental::threading {

struct CancellationTokenSource;
struct CancellationToken;

/// Fired by a [`CancellationTokenSource::Cancel()`] callsite when cancellation is requested.
///
/// Listeners registered via [`CancellationToken::OnCancelRequested()`] receive this event.
struct VIOLET_API CancellationRequestedEvent final {
    /// A stringified representation of this object that conforms to the [`violet::Stringify`] concept.
    [[nodiscard]] VIOLET_API auto ToString() const noexcept -> CStr;
    friend auto operator<<(std::ostream& os, const CancellationRequestedEvent& self) noexcept -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    friend struct CancellationTokenSource;
    friend struct CancellationToken;

    VIOLET_IMPLICIT CancellationRequestedEvent() noexcept = default;
};

/// A owning side of a cancellation pair, analogous to .NET's [`CancellationTokenSource`].
///
/// [`CancellationTokenSource`]:
/// https://learn.microsoft.com/en-us/dotnet/api/system.threading.cancellationtokensource?view=net-10.0
///
/// **CancellationTokenSource** owns the shared cancellation state and is the only type that can
/// trigger cancellation. A source can produce any number of [`CancellationToken`]s via the
/// [`CancellationTokenSource::Token()`] method, which shares the underlying state.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Threading/CancellationToken.h>
///
/// using namespace violet::experimental::threading;
///
/// CancellationTokenSource cts;
///
/// auto _guard1 = cts.Token().OnCancelled([&](const CancellationRequestedEvent&) -> void { /* do stuff here... */ });
/// auto _guard2 = cts.Token().OnCancelled([&](const CancellationRequestedEvent&) -> void { /* do stuff here... */ });
///
/// /* ... */
/// cts.Cancel();
/// ```
///
/// ## A one-way street...
/// Once [`Cancel()`] has been called it cannot be undone. If you need
/// re-arming semantics, create a new `CancellationTokenSource`.
struct VIOLET_API CancellationTokenSource final {
    VIOLET_DISALLOW_COPY(CancellationTokenSource);
    VIOLET_IMPLICIT_MOVE(CancellationTokenSource);

    VIOLET_IMPLICIT CancellationTokenSource() noexcept
        : n_state(std::make_shared<state_t>())
    {
    }

    ~CancellationTokenSource() = default;

    /// Produce a new [`CancellationToken`] that reflects the cancellation state
    /// of this source. Multiple tokens from the same source all share the same
    /// state.
    [[nodiscard]] VIOLET_API auto Token() const noexcept -> CancellationToken;

    /// Returns **true** if cancellation had been requested on this source.
    [[nodiscard]] VIOLET_API auto RequestsCancellation() const noexcept -> bool;

    /// Requests cancellation on this source.
    ///
    /// All tokens derived from this source will observe [`RequestsCancellation`] returning
    /// **true**, all threads blocked on [`CancellationToken::WaitOnCancellation()`] will be unblocked.
    ///
    /// > [!NOTE]
    /// > Calling [`CancellationTokenSource::Cancel()`] more than once is a no-op.
    VIOLET_API void Cancel() const noexcept;

private:
    friend struct CancellationToken;

    struct state_t final {
        Mutex Mux;
        Condvar CV;
        std::atomic<bool> Cancelled = false;
        events::Emitter<CancellationRequestedEvent> Emitter;
    };

    SharedPtr<state_t> n_state;
};

/// A lightweight handle to a cancellation signal, analogous to .NET's [`CancellationToken`].
///
/// [`CancellationToken`]: https://learn.microsoft.com/en-us/dotnet/api/system.threading.cancellationtoken?view=net-10.0
///
/// Tokens are cheap to copy and share across threads, they hold a shared pointer to its state
/// that is owned by a [`CancellationTokenSource`].
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Threading/CancellationToken.h>
///
/// using namespace violet::experimental::threading;
///
/// CancellationTokenSource cts;
/// auto token = cts.Token();
///
/// auto thread = Thread::Builder()
///     .WithName("worker")
///     .Spawn([token] -> void {
///         while (!token.RequestsCancellation()) {
///             doWork();
///         }
///     });
///
///
/// // ...after a while
/// cts.Cancel();
/// ```
struct VIOLET_API CancellationToken final {
    VIOLET_DISALLOW_CONSTRUCTOR(CancellationToken);
    VIOLET_DISALLOW_COPY(CancellationToken);
    VIOLET_IMPLICIT_MOVE(CancellationToken);
    ~CancellationToken() = default;

    /// Creates a dummy, non-reactive cancellation token.
    VIOLET_API static auto None() noexcept -> CancellationToken
    {
        return CancellationToken(nullptr);
    }

    /// Returns **true** if cancellation has been requested from the source.
    [[nodiscard]] VIOLET_API auto RequestsCancellation() const noexcept -> bool;

    /// Blocks the calling thread until cancellation is requested.
    ///
    /// This is the equivalent to writing `while (token.RequestsCancellation()) {}` but without
    /// busy-waiting, it uses the source's condition variable internally.
    VIOLET_API void WaitForCancellation() const noexcept;

    template<typename Fun>
        requires(callable<Fun, const CancellationRequestedEvent&>
            && callable_returns<Fun, void, const CancellationRequestedEvent&>)
    auto OnCancelled(Fun&& fun) const noexcept -> events::Emitter<CancellationRequestedEvent>::Guard
    {
        if (this->n_state == nullptr) {
            return { };
        }

        auto callable = VIOLET_FWD(Fun, fun);
        if (this->RequestsCancellation()) {
            std::invoke(fun, CancellationRequestedEvent{ });
            return { };
        }

        std::unique_lock lock(this->n_state->Mux);
        if (this->n_state->Cancelled.load(std::memory_order_relaxed)) {
            lock.unlock();
            std::invoke(fun, CancellationRequestedEvent{ });

            return { };
        }

        return this->n_state->Emitter.On(callable, /*persist=*/false);
    }

private:
    friend struct CancellationTokenSource;

    VIOLET_EXPLICIT CancellationToken(SharedPtr<CancellationTokenSource::state_t> state) noexcept
        : n_state(VIOLET_MOVE(state))
    {
    }

    SharedPtr<CancellationTokenSource::state_t> n_state = nullptr;
};

} // namespace violet::experimental::threading
