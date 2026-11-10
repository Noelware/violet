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
// # 🌺💜 `violet/Experimental/Threading/ThreadPool.h`

#pragma once

#include <violet/Experimental/Mutex.h>
#include <violet/Experimental/Slice.h>
#include <violet/Experimental/Threading/Thread.h>
#include <violet/Experimental/Time/Duration.h>

#include <deque>
#include <format>

namespace violet::experimental::threading {
namespace NOELDOC_HIDE pool_internal {

/// State shared between a pool and its workers.
struct VIOLET_API Core {
    VIOLET_DISALLOW_COPY_AND_MOVE(Core);
    VIOLET_IMPLICIT Core() noexcept = default;
    virtual ~Core() = default;

    mutable struct Mutex Mutex;
    Condvar WorkAvailability;
    Condvar Idle;
    CancellationTokenSource Cts;
    std::deque<ptr::Unique<Runnable>> Queue;

    UInt Active = 0; ///< tasks currently running
    UInt IdleWorkers = 0; ///< workers waiting for a task
    UInt Live = 0; ///< workers that haven't exited
    bool ShuttingDown = false;

    /// Workers above this count retire after being idle for `KeepAlive`.
    UInt Min = 0;

    /// When `Nothing`, workers never retire.
    Optional<chrono::Duration> KeepAlive;

    /// Queues `task`. Returns **false** if the pool is shutting down.
    auto Push(ptr::Unique<Runnable> task) -> bool;
    void WaitForIdle();
    void Shutdown(bool drain);

    /// Runs tasks until the pool shuts down or this worker retires.
    void RunWorkerLoop();
};

/// Error returned when submitting to a pool that is shutting down.
VIOLET_API
auto ShutDownError() -> io::experimental::Error;

} // namespace NOELDOC_HIDE pool_internal

/// A pool of `MaxThreads` worker threads that run submitted runnable objects.
///
/// Pass a size for a fixed pool whose threads are all started up front, or leave it out
/// (`ThreadPool<>`) for a pool that spawns threads on demand up to a maximum and retires
/// idle ones after a keep-alive period.
///
/// Tasks receive a [`CancellationToken`] that is cancelled by [`ThreadPool::ShutdownNow()`].
template<UInt MaxThreads = std::dynamic_extent>
struct NOELDOC_EXPERIMENTAL_SINCE("26.11") ThreadPool final {
    static_assert(MaxThreads > 0, "a fixed-size `ThreadPool` needs atleast one thread!");

    VIOLET_DISALLOW_CONSTRUCTOR(ThreadPool);
    VIOLET_DISALLOW_COPY(ThreadPool);

    VIOLET_IMPLICIT ThreadPool(ThreadPool&&) noexcept = default;
    auto operator=(ThreadPool&&) -> ThreadPool& = delete;

    ~ThreadPool()
    {
        if (this->n_core != nullptr) {
            this->Shutdown();
        }
    }

    static auto New(Str prefix = "violet-pool") -> io::experimental::Result<ThreadPool>
    {
        auto core = Own<pool_internal::Core>::New();
        core->Min = MaxThreads;
        core->Live = MaxThreads;

        ThreadPool pool(VIOLET_MOVE(core));
        for (UInt i = 0; i < MaxThreads; i++) {
            auto thread = Thread::Builder()
                              .WithName(std::format("{}[#{}]", prefix, i))
                              .Spawn([core = pool.n_core]() mutable -> void { core->RunWorkerLoop(); });

            if (thread.Err()) {
                // the workers that never started can't decrement `Live` themselves
                MutexLock lock(pool.n_core->Mutex);
                pool.n_core->Live -= MaxThreads - i;
                return Err(VIOLET_MOVE(thread).Error());
            }

            pool.n_workers.Push(VIOLET_MOVE(thread).Value());
        }

        return pool;
    }

    [[nodiscard]]
    constexpr static auto Threads() noexcept -> UInt
    {
        return MaxThreads;
    }

    /// Queues `task` to run on a worker.
    /// @return an error if the pool is shutting down
    auto Submit(ptr::Unique<Runnable> task) -> io::experimental::Result<void>
    {
        if (!this->n_core->Push(VIOLET_MOVE(task))) {
            return Err(pool_internal::ShutDownError());
        }

        return {};
    }

    template<std::derived_from<Runnable> R, typename... Args>
        requires std::constructible_from<R, Args...>
    auto Submit(Args&&... args) -> io::experimental::Result<void>
    {
        return this->Submit(ptr::Unique<Runnable>::New<R, Args...>(VIOLET_FWD(Args, args)...));
    }

    template<typename Fun>
        requires((callable<Fun> || callable<Fun, const CancellationToken&>)
            && (callable_returns<Fun, void> || callable_returns<Fun, void, const CancellationToken&>))
    auto Submit(Fun&& fun) -> io::experimental::Result<void>
    {
        return this->Submit(Runnable::From(VIOLET_FWD(Fun, fun)));
    }

    /// Blocks until the queue is empty and no task is running.
    void WaitForIdle()
    {
        this->n_core->WaitForIdle();
    }

    /// Stops accepting tasks, runs everything already queued, then joins all workers.
    void Shutdown()
    {
        this->n_core->Shutdown(true);
        this->joinAll();
    }

    /// Stops accepting tasks, drops everything still queued, cancels running tasks' tokens, then joins.
    void ShutdownNow()
    {
        this->n_core->Shutdown(false);
        this->joinAll();
    }

private:
    VIOLET_EXPLICIT ThreadPool(Own<pool_internal::Core> core) noexcept
        : n_core(VIOLET_MOVE(core))
    {
    }

    void joinAll()
    {
        for (Thread& worker: this->n_workers) {
            if (worker.Joinable()) {
                worker.Join();
            }
        }
    }

    Own<pool_internal::Core> n_core;
    Slice<Thread, MaxThreads> n_workers;
};

/// A [`ThreadPool`] that spawns workers on demand, up to a maximum.
template<>
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("26.11") ThreadPool<std::dynamic_extent> final {
    struct Builder;

    VIOLET_DISALLOW_CONSTRUCTOR(ThreadPool);
    VIOLET_DISALLOW_COPY(ThreadPool);
    VIOLET_IMPLICIT ThreadPool(ThreadPool&&) noexcept;
    auto operator=(ThreadPool&&) -> ThreadPool& = delete;

    ~ThreadPool();

    /// Returns the number of live worker threads.
    [[nodiscard]]
    auto Threads() const noexcept -> UInt;

    /// Returns the maximum number of worker threads.
    [[nodiscard]]
    auto MaxThreads() const noexcept -> UInt;

    /// Queues `task`, spawning a new worker if every worker is busy and the pool isn't at its maximum.
    /// @return an error if the pool is shutting down, or if no worker exists and a new one couldn't be spawned
    auto Submit(ptr::Unique<Runnable> task) -> io::experimental::Result<void>;

    template<std::derived_from<Runnable> R, typename... Args>
        requires std::constructible_from<R, Args...>
    auto Submit(Args&&... args) -> io::experimental::Result<void>
    {
        return this->Submit(ptr::Unique<Runnable>::New<R, Args...>(VIOLET_FWD(Args, args)...));
    }

    template<typename Fun>
        requires((callable<Fun> || callable<Fun, const CancellationToken&>)
            && (callable_returns<Fun, void> || callable_returns<Fun, void, const CancellationToken&>))
    auto Submit(Fun&& fun) -> io::experimental::Result<void>
    {
        return this->Submit(Runnable::From(VIOLET_FWD(Fun, fun)));
    }

    /// Blocks until the queue is empty and no task is running.
    void WaitForIdle();

    /// Stops accepting tasks, runs everything already queued, then joins all workers.
    void Shutdown();

    /// Stops accepting tasks, drops everything still queued, cancels running tasks' tokens, then joins.
    void ShutdownNow();

private:
    struct Core;

    VIOLET_EXPLICIT ThreadPool(Own<Core> core) noexcept;

    static auto spawnWorker(Own<Core> core) -> io::experimental::Result<void>;

    void reap();
    void joinAll();

    Own<Core> n_core;
};

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("26.11") ThreadPool<std::dynamic_extent>::Builder final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Builder);
    VIOLET_IMPLICIT Builder() noexcept = default;
    ~Builder() = default;

    /// Threads started up front and never retired. Defaults to 0.
    auto WithMinThreads(UInt n) & noexcept -> Builder&;
    auto WithMinThreads(UInt n) && noexcept -> Builder&&;

    /// The maximum number of threads. Defaults to [`threading::AvailableParallelism()`].
    auto WithMaxThreads(UInt n) & noexcept -> Builder&;
    auto WithMaxThreads(UInt n) && noexcept -> Builder&&;

    /// How long a thread above the minimum may stay idle before it exits. Defaults to 60 seconds.
    auto WithKeepAlive(chrono::Duration dur) & noexcept -> Builder&;
    auto WithKeepAlive(chrono::Duration dur) && noexcept -> Builder&&;

    /// The prefix of each worker thread's name. Defaults to `violet-pool`.
    auto WithNamePrefix(Str prefix) & -> Builder&;
    auto WithNamePrefix(Str prefix) && -> Builder&&;

    auto Build() -> io::experimental::Result<ThreadPool>;

private:
    UInt n_min = 0;
    UInt n_max = 0;
    chrono::Duration n_keepAlive = chrono::Duration::Minutes(1);
    String n_prefix = "violet-pool";
};

} // namespace violet::experimental::threading
