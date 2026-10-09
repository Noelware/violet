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
//! # 🌺💜 `violet/Experimental/Coroutines/Task.h`

#pragma once

#include <violet/Defer.h>
#include <violet/Experimental/OneOf.h>

#include <coroutine>
#include <exception>

namespace violet::experimental::coro {

template<typename = void>
struct Task;

namespace task_internal {

/// An awaiter that resummes whoever is `co_await`-ing this task.
///
/// If nobody is waiting yet (fire-and-forget), this hands control back
/// to the scheduler via [`std::noop_coroutine`] instead of falling off
/// the end of the coroutine on its own thread.
struct FinaleAwaiter final {
    [[nodiscard]]
    constexpr auto await_ready() const noexcept -> bool
    {
        return false;
    }

    template<typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> handle) noexcept -> std::coroutine_handle<>
    {
        auto& promise = handle.promise();
        return promise.ContinuationPoint ? promise.ContinuationPoint : std::noop_coroutine();
    }

    void await_resume() noexcept { }
};

/// Records `leaf` as the point the current root task should resume from. No-op outside of a task.
VIOLET_API void RecordSuspendPoint(std::coroutine_handle<> leaf) noexcept;

template<typename T>
concept has_member_co_await = requires(T&& value) { VIOLET_FWD(T, value).operator co_await(); };

template<typename T>
auto GetAwaiter(T&& value) -> decltype(auto)
{
    if constexpr (has_member_co_await<T>) {
        return VIOLET_FWD(T, value).operator co_await();
    } else {
        return VIOLET_FWD(T, value);
    }
}

/// Wraps a leaf awaiter so suspending on it records the leaf handle on the current root task.
template<typename Awaiter>
struct LeafAwaiter final {
    Awaiter Inner;

    auto await_ready() -> bool
    {
        return this->Inner.await_ready();
    }

    template<typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> handle) -> decltype(auto)
    {
        RecordSuspendPoint(handle);
        if constexpr (std::same_as<decltype(this->Inner.await_suspend(handle)), bool>) {
            bool suspended = this->Inner.await_suspend(handle);
            if (!suspended) {
                RecordSuspendPoint(nullptr);
            }

            return suspended;
        } else {
            return this->Inner.await_suspend(handle);
        }
    }

    auto await_resume() -> decltype(auto)
    {
        return this->Inner.await_resume();
    }
};

struct BasePromise {
    std::coroutine_handle<> ContinuationPoint = nullptr;

    constexpr auto initial_suspend() noexcept -> std::suspend_always
    {
        return {};
    }

    constexpr auto final_suspend() noexcept
    {
        return task_internal::FinaleAwaiter{};
    }

    template<typename Awaitable>
    auto await_transform(Awaitable&& awaitable)
    {
        using Inner = decltype(task_internal::GetAwaiter(VIOLET_FWD(Awaitable, awaitable)));
        return task_internal::LeafAwaiter<Inner>{task_internal::GetAwaiter(VIOLET_FWD(Awaitable, awaitable))};
    }

    template<typename U>
    auto await_transform(Task<U>&& child, [[maybe_unused]] SourceLocation loc = std::source_location::current())
    {
        // child.PushNewFrame(/* */);
        return VIOLET_MOVE(child);
    }
};

template<typename T>
struct PromiseStorage: BasePromise {
    OneOf<T, std::exception_ptr> Value;

    template<typename U>
        requires std::convertible_to<U&&, T>
    void return_value(U&& value) noexcept(std::is_nothrow_move_assignable_v<U>)
    {
        this->Value = VIOLET_FWD(U, value);
    }

    void unhandled_exception() noexcept
    {
        this->Value = std::current_exception();
    }

    auto Take() -> T
    {
        return this->Value.Match(
            // clang-format off
            [](T value) -> T { return VIOLET_MOVE(value); },
            [](std::exception_ptr ex) -> T {
                std::rethrow_exception(VIOLET_MOVE(ex));
                VIOLET_UNREACHABLE();
            }
            // clang-format on
        );
    }
};

template<>
struct PromiseStorage<void>: BasePromise {
    std::exception_ptr Exception;

    void unhandled_exception() noexcept
    {
        this->Exception = std::current_exception();
    }

    void return_void() noexcept { }

    void Take() // NOLINT(readability-make-member-function-const)
    {
        if (this->Exception != nullptr) {
            std::rethrow_exception(this->Exception);
        }
    }
};

} // namespace task_internal

/// A lazily-started, single-await coroutine task.
///
/// `Task<T>` represents an asynchronous computation that hasn't started yet: calling a coroutine
/// function that returns `Task<T>` only allocates its coroutine frame, it does not run the body. The
/// body only begins executing once the `Task` is [`co_await`]-ed, which lets a `Task` tree be built
/// up and composed before anything actually runs. `Task<T>` is inspired by Rust's [`Future`] trait.
///
/// `Task<T>` is move-only and may be awaited **exactly once**. Awaiting the same task twice,
/// or after it has been moved is undefined behaviour.
///
/// [`Future`]: https://doc.rust-lang.org/std/future/trait.Future.html
template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("current")
    [[nodiscard("coroutine task are lazily-started")]] VIOLET_CORO_AWAIT_ELIDABLE Task {
    VIOLET_DISALLOW_CONSTRUCTOR(Task);
    VIOLET_DISALLOW_COPY(Task);

    struct promise_type final: task_internal::PromiseStorage<T> {
        auto get_return_object() noexcept -> Task
        {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    NOELDOC_HIDE VIOLET_EXPLICIT Task(handle_type coro) noexcept
        : n_coro(coro)
    {
    }

    VIOLET_IMPLICIT Task(Task&& other) noexcept
        : n_coro(std::exchange(other.n_coro, {}))
    {
    }

    auto operator=(Task&& other) noexcept -> Task&
    {
        if (this != &other) {
            this->destroy();
            this->n_coro = std::exchange(other.n_coro, {});
        }

        return *this;
    }

    ~Task()
    {
        this->destroy();
    }

    /// Starts the task and suspends the caller until it is completed.
    auto operator co_await() && noexcept;

protected:
    handle_type n_coro;

    struct awaiter final {
        VIOLET_DISALLOW_COPY(awaiter);

        handle_type Coroutine;

        VIOLET_EXPLICIT awaiter(handle_type coro) noexcept
            : Coroutine(coro)
        {
        }

        VIOLET_IMPLICIT awaiter(awaiter&& other) noexcept
            : Coroutine(std::exchange(other.Coroutine, {}))
        {
        }

        auto operator=(awaiter&&) noexcept -> awaiter& = delete;

        ~awaiter()
        {
            if (this->Coroutine) {
                this->Coroutine.destroy();
            }
        }

        [[nodiscard]]
        auto await_ready() const noexcept -> bool
        {
            return false;
        }

        auto await_suspend(std::coroutine_handle<> awaitee) noexcept -> handle_type
        {
            this->Coroutine.promise().ContinuationPoint = awaitee;
            return this->Coroutine;
        }

        auto await_resume() -> T
        {
            VIOLET_ASSERT(this->Coroutine, "coroutine was destroyed beforehand");
            VIOLET_DEFER({
                // destroy the coroutine once the value has been retrieved.
                std::exchange(this->Coroutine, {}).destroy();
            });

            promise_type& promise = this->Coroutine.promise();
            return promise.Take();
        }
    };

private:
    void destroy()
    {
        if (this->n_coro) {
            this->n_coro.destroy();
        }
    }
};

template<typename T>
auto Task<T>::operator co_await() && noexcept
{
    return awaiter(std::exchange(this->n_coro, {}));
}

} // namespace violet::experimental::coro
