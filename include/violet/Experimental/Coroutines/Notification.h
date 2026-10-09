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
//! # 🌺💜 `violet/Experimental/Coroutines/Notification.h`

#pragma once

#include <violet/Experimental/Coroutines/Waker.h>
#include <violet/Experimental/Synchronized.h>

#include <coroutine>

namespace violet::experimental::coro {

/// A one-shot signal for coroutines, modelled on `absl::Notification`.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Notification final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Notification);
    VIOLET_IMPLICIT Notification() noexcept = default;
    ~Notification() = default;

    /// Marks the notification as notified and wakes every waiter.
    void Notify() noexcept;

    /// Returns **true** if [`Notify`] was ever called.
    [[nodiscard]]
    auto WasNotified() const noexcept -> bool
    {
        return this->n_notified.load(std::memory_order_acquire);
    }

protected:
    struct Awaiter;

public:
    [[nodiscard]]
    auto Wait() noexcept -> Awaiter;

private:
    std::atomic<bool> n_notified{false};
    Synchronized<Vec<Waker>> n_waiters;
};

struct VIOLET_API NOELDOC_HIDE Notification::Awaiter final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Awaiter);
    ~Awaiter() = default;

    Notification& Self;

    VIOLET_EXPLICIT Awaiter(Notification& self) noexcept
        : Self(self)
    {
    }

    [[nodiscard]]
    auto await_ready() const noexcept -> bool
    {
        return this->Self.WasNotified();
    }

    static void await_resume() noexcept { }

    auto await_suspend(std::coroutine_handle<>) noexcept -> bool;
};

inline auto Notification::Wait() noexcept -> Awaiter
{
    return Awaiter{*this};
}

} // namespace violet::experimental::coro
