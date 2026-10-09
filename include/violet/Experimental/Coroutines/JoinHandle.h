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
//! # 🌺💜 `violet/Experimental/Coroutines/JoinHandle.h`
//! An owned permission to await a spawned task's result.

#pragma once

#include <violet/Defer.h>
#include <violet/Experimental/Coroutines/Internals/Scheduler.h>
#include <violet/Experimental/Coroutines/Internals/SpawnedTask.h>
#include <violet/Experimental/OneOf.h>

#include <coroutine>

namespace violet::experimental::coro {

struct Runtime;

template<typename T>
using join_value_t = std::conditional_t<std::is_void_v<T>, Mono, T>;

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") AbortHandle final {
    VIOLET_DISALLOW_COPY(AbortHandle);

    VIOLET_IMPLICIT AbortHandle(AbortHandle&& other) noexcept
        : n_task(std::exchange(other.n_task, nullptr))
    {
    }

    auto operator=(AbortHandle&&) -> AbortHandle& = delete;

    ~AbortHandle()
    {
        if (this->n_task != nullptr) {
            internals::ReleaseRawTask(Unsafe("we own one reference"), this->n_task);
        }
    }

    /// Requests cancellation. No-op if the task already finished.
    void Abort() const noexcept
    {
        if (this->n_task != nullptr) {
            internals::AbortTask(Unsafe("we own one reference"), this->n_task);
        }
    }

private:
    template<typename>
    friend struct JoinHandle;

    VIOLET_EXPLICIT AbortHandle(internals::RawTask* task) noexcept
        : n_task(task)
    {
    }

    internals::RawTask* n_task = nullptr;
};

/// An owned permission object to await a spawned task's result, analogous to Tokio's [`JoinHandle`].
/// [`JoinHandle`]: https://docs.rs/tokio/latest/tokio/task/struct.JoinHandle.html
///
/// A `JoinHandle<T>` holds one reference to the spawned task, so the task's frame (and the result
/// is stored in its promise) stays alive until the handle is awaited, `Taken()`n, or dropped.
///
/// ## Remarks
/// * Dropping a `JoinHandle` *detaches* the task: it keeps running to completion and its result is discarded.
/// * A `JoinHandle` can be awaited at most once. `co_await` consumes it.
/// * Awaiting is only valid inside a runtime, because the awaiting root task is what gets woken.
///
/// ## Example
/// ```cpp
/// auto Parent() -> Task<Int32> {
///     JoinHandle child(Spawn(computeAnswer()));
///     co_return co_await VIOLET_MOVE(child);
/// }
/// ```
template<typename T>
struct [[nodiscard("dropping a `JoinHandle` will detach itself")]] NOELDOC_EXPERIMENTAL_SINCE("current")
    JoinHandle final {
    VIOLET_DISALLOW_CONSTRUCTOR(JoinHandle);
    VIOLET_DISALLOW_COPY(JoinHandle);

    NOELDOC_HIDE VIOLET_EXPLICIT JoinHandle(internals::RawTask* task) noexcept
        : n_task(task)
    {
    }

    NOELDOC_HIDE VIOLET_EXPLICIT JoinHandle(internals::SpawnedTask<T>&& task) noexcept
        : n_task(VIOLET_MOVE(task).IntoRaw())
    {
    }

    VIOLET_IMPLICIT JoinHandle(JoinHandle&& other) noexcept
        : n_task(std::exchange(other.n_task, nullptr))
    {
    }

    auto operator=(JoinHandle&& other) noexcept -> JoinHandle&
    {
        if (this != &other) {
            this->reset();
            this->n_task = std::exchange(other.n_task, nullptr);
        }

        return *this;
    }

    ~JoinHandle()
    {
        this->reset();
    }

    /// Returns **true** once the task has run to completion.
    [[nodiscard]]
    auto Finished() const noexcept -> bool
    {
        VIOLET_DEBUG_ASSERT(this->n_task != nullptr, "`JoinHandle` was moved from or already consumed");

        auto state = this->n_task->State.load(std::memory_order_acquire);
        return (state & internals::kTaskStateComplete) != 0;
    }

    /// Returns `true` if the task was cancelled (e.g. its runtime shut down) and has no result.
    [[nodiscard]]
    auto Cancelled() const noexcept -> bool
    {
        return (this->n_task->State.load(std::memory_order_acquire) & internals::kTaskStateCancelled) != 0;
    }

    [[nodiscard]]
    auto GetAbortHandle() const noexcept -> AbortHandle
    {
        internals::RetainRawTask(Unsafe("the abort handle owns one reference"), this->n_task);
        return AbortHandle(this->n_task);
    }

    /// Takes the task's result synchronously, consuming the handle.
    ///
    /// Rethrows the task's exception, if it threw one.
    ///
    /// ## Safety
    /// The task must be [`Finished()`]. Use this outside of a coroutine (e.g. `Runtime::BlockOn`);
    /// inside one, `co_await` the handle instead.
    auto Take() -> T
    {
        VIOLET_ASSERT(!this->Cancelled(), "`JoinHandle::Take` on a cancelled task; check `Cancelled()` first");
        VIOLET_DEBUG_ASSERT(this->Finished(), "`JoinHandle::Take` called before the task finished");

        internals::RawTask* task = std::exchange(this->n_task, nullptr);
        VIOLET_DEFER({ internals::ReleaseRawTask(Unsafe("we owned one reference"), task); });

        return internals::SpawnedTask<T>::PromiseOf(task).Take();
    }

    void Abort() const noexcept
    {
        internals::AbortTask(Unsafe("we own one reference"), this->n_task);
    }

    auto operator co_await() && noexcept;

protected:
    struct CancellableAwaiter;

public:
    auto Cancellable() && noexcept -> CancellableAwaiter;

private:
    friend struct Runtime;

    using promise_type = internals::SpawnedTask<T>::promise_type;

    struct awaiter final {
        JoinHandle Handle;

        [[nodiscard]]
        auto await_ready() const noexcept -> bool
        {
            return this->Handle.Finished();
        }

        auto await_suspend(std::coroutine_handle<>) noexcept -> bool
        {
            return JoinHandle::registerJoinWaiter(this->Handle.n_task);
        }

        auto await_resume() -> T
        {
            return this->Handle.Take();
        }
    };

    static auto registerJoinWaiter(internals::RawTask* target) noexcept -> bool
    {
        internals::RawTask* self = internals::CurrentTask();
        VIOLET_DEBUG_ASSERT(self != nullptr, "`JoinHandle` was awaited outside of a runtime");

        promise_type& promise = internals::SpawnedTask<T>::PromiseOf(target);
        internals::RetainRawTask(Unsafe("we are running inside `self`"), self);
        promise.JoinWaiter = self;

        UInt32 state = target->State.load(std::memory_order_acquire);
        do {
            if ((state & internals::kTaskStateComplete) != 0) {
                promise.JoinWaiter = nullptr;
                internals::ReleaseRawTask(Unsafe("undoing the retain above"), self);
                return false;
            }
        } while (!target->State.compare_exchange_weak(
            state, state | internals::kTaskStateJoinWaiter, std::memory_order_acq_rel, std::memory_order_acquire));

        return true;
    }

    void reset()
    {
        if (this->n_task != nullptr) {
            internals::ReleaseRawTask(Unsafe("we own the reference"), std::exchange(this->n_task, nullptr));
        }
    }

    internals::RawTask* n_task = nullptr;
};

template<typename T>
auto JoinHandle<T>::operator co_await() && noexcept
{
    return awaiter{VIOLET_MOVE(*this)};
}

template<typename T>
struct VIOLET_API NOELDOC_HIDE JoinHandle<T>::CancellableAwaiter final {
    JoinHandle Handle;

    [[nodiscard]]
    auto await_ready() const noexcept -> bool
    {
        return this->Handle.Finished();
    }

    auto await_suspend(std::coroutine_handle<>) noexcept -> bool
    {
        return JoinHandle::registerJoinWaiter(this->Handle.n_task);
    }

    auto await_resume() -> Optional<join_value_t<T>>
    {
        if (this->Handle.Cancelled()) {
            this->Handle.reset();
            return Nothing;
        }

        if constexpr (std::is_void_v<T>) {
            this->Handle.Take();
            return Some(Mono{});
        } else {
            return Some(this->Handle.Take());
        }
    }
};

template<typename T>
auto JoinHandle<T>::Cancellable() && noexcept -> CancellableAwaiter
{
    return CancellableAwaiter{VIOLET_MOVE(*this)};
}

} // namespace violet::experimental::coro
