// 🌺💜 Violet: Extended C standard library
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
//! # 🌺💜 `violet/Experimental/Coroutines/Waker.h`

#pragma once

#include <violet/Experimental/Coroutines/Internals/RawTask.h>

namespace violet::experimental::coro {
namespace internals {
struct Scheduler;
}

/// A type-erased "please resume me" handle, analogus to Rust's [`std::task::Waker`].
/// [`std::task::Waker`]: https://doc.rust-lang.org/std/task/struct.Waker.html
///
/// This is handed to any task that suspends waiting on an external event (I/O readiness,
/// timers, another thread, etc). Calling [`Waker::Wake()`] will reschedule the associated
/// task onto whichever executor that owns it.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Waker final {
    /// The virtual dispatch table for the type-erased object that `Waker` holds.
    struct VIOLET_API VTable final {
        void (*Wake)(void* data) noexcept = nullptr;
        void (*WakeByRef)(void* data) noexcept = nullptr;
        void (*Clone)(void* data, void** outData, const VTable** outVt) noexcept = nullptr;
        void (*Drop)(void* data) noexcept = nullptr;
    };

    constexpr VIOLET_IMPLICIT Waker() noexcept = default;

    NOELDOC_HIDE VIOLET_IMPLICIT Waker(void* data, const VTable* vt) noexcept
        : n_object(data)
        , n_vtable(vt)
    {
    }

    VIOLET_IMPLICIT Waker(const Waker& other) noexcept
    {
        if (other.n_vtable != nullptr) {
            other.n_vtable->Clone(other.n_object, &this->n_object, &this->n_vtable);
        }
    }

    auto operator=(const Waker& other) noexcept -> Waker&
    {
        if (this != &other) {
            if (this->n_vtable != nullptr) {
                this->n_vtable->Drop(this->n_object);
            }

            this->n_object = nullptr;
            this->n_vtable = nullptr;
            if (other.n_vtable != nullptr) {
                other.n_vtable->Clone(other.n_object, &this->n_object, &this->n_vtable);
            }
        }

        return *this;
    }

    VIOLET_IMPLICIT Waker(Waker&& other) noexcept
        : n_object(std::exchange(other.n_object, nullptr))
        , n_vtable(std::exchange(other.n_vtable, nullptr))
    {
    }

    auto operator=(Waker&& other) noexcept -> Waker&
    {
        if (this != &other) {
            if (this->n_vtable != nullptr) {
                this->n_vtable->Drop(this->n_object);
            }

            this->n_object = std::exchange(other.n_object, nullptr);
            this->n_vtable = std::exchange(other.n_vtable, nullptr);
        }

        return *this;
    }

    ~Waker()
    {
        if (this->n_vtable != nullptr) {
            this->n_vtable->Drop(this->n_object);
            this->n_vtable = nullptr;
            this->n_object = nullptr;
        }
    }

    /// Returns a waker that does nothing when woken. Useful in tests, or as a placeholder.
    [[nodiscard]]
    static auto Noop() noexcept -> Waker;

    /// Returns a waker for the task the calling thread is currently running.
    ///
    /// This is only valid inside a task driven by a runtime, typically from an awaiter's `await_suspend` call. The
    /// waker keeps the task's header alive, but not its frame: if the task is cancelled meanwhile, waking it is a
    /// harmless no-op.
    [[nodiscard]]
    static auto ForCurrentTask() noexcept -> Waker;

    /// Returns `true` if both wakers would wake the same task, so a waker that is
    /// stored and then re-registered can skip a clone.
    [[nodiscard]]
    auto WillWake(const Waker& other) const noexcept -> bool
    {
        return this->n_object == other.n_object && this->n_vtable == other.n_vtable;
    }

    /// Returns `true` if this waker is valid (default-constructed or moved from).
    [[nodiscard]]
    auto Valid() const noexcept -> bool
    {
        return this->n_vtable == nullptr;
    }

    /// Consumes this waker to reschedule its task.
    void Wake() && noexcept;

    /// Reschedule its task without consuming the waker.
    void WakeByRef() noexcept;

private:
    void* n_object = nullptr;
    const VTable* n_vtable = nullptr;
};

} // namespace violet::experimental::coro
