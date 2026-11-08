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

#include <violet/Experimental/Collections/HashMap.h>
#include <violet/Experimental/Threading/Parallelism.h>
#include <violet/Experimental/Threading/ThreadPool.h>

#include <algorithm>

namespace violet::experimental::threading {
namespace pool_internal {

auto ShutDownError() -> io::experimental::Error
{
    return VIOLET_IO_ERROR(BrokenPipe, "thread pool is shutting down");
}

auto Core::Push(ptr::Unique<Runnable> task) -> bool
{
    {
        MutexLock lock(this->Mutex);
        if (this->ShuttingDown) {
            return false;
        }

        this->Queue.push_back(VIOLET_MOVE(task));
    }

    this->WorkAvailability.Signal();
    return true;
}

void Core::WaitForIdle()
{
    MutexLock lock(this->Mutex);
    this->Idle.WaitUntil(&this->Mutex, [this]() -> bool { return this->Queue.empty() && this->Active == 0; });
}

void Core::Shutdown(bool drain)
{
    std::deque<ptr::Unique<Runnable>> dropped;
    {
        MutexLock lock(this->Mutex);
        this->ShuttingDown = true;
        if (!drain) {
            dropped.swap(this->Queue);
        }

        if (this->Queue.empty() && this->Active == 0) {
            this->Idle.SignalAll();
        }
    }

    if (!drain) {
        this->Cts.Cancel();
    }

    this->WorkAvailability.SignalAll();
}

void Core::RunWorkerLoop()
{
    this->Mutex.Lock();
    while (true) {
        while (this->Queue.empty() && !this->ShuttingDown) {
            this->IdleWorkers++;
            if (this->KeepAlive.HasValue() && this->Live > this->Min) {
                const bool timedOut = this->WorkAvailability.WaitWithTimeout(&this->Mutex, *this->KeepAlive);
                this->IdleWorkers--;

                if (timedOut && this->Queue.empty() && !this->ShuttingDown && this->Live > this->Min) {
                    this->Live--;
                    this->Mutex.Unlock();
                    return;
                }
            } else {
                this->WorkAvailability.Wait(&this->Mutex);
                this->IdleWorkers--;
            }
        }

        if (this->Queue.empty()) {
            this->Live--;
            this->Mutex.Unlock();
            return;
        }

        auto task = VIOLET_MOVE(this->Queue.front());
        this->Queue.pop_front();
        this->Active++;
        this->Mutex.Unlock();

        task->Run(this->Cts.Token());
        Drop(VIOLET_MOVE(task));

        this->Mutex.Lock();
        this->Active--;
        if (this->Queue.empty() && this->Active == 0) {
            this->Idle.SignalAll();
        }
    }
}

} // namespace pool_internal

struct ThreadPool<std::dynamic_extent>::Core final: public pool_internal::Core {
    UInt Max = 0;
    UInt NextIndex = 0;
    String Prefix;
    HashMap<UInt, Thread> Workers;
    Vec<UInt> Exited;
};

ThreadPool<std::dynamic_extent>::ThreadPool(Own<Core> core) noexcept
    : n_core(VIOLET_MOVE(core))
{
}

ThreadPool<std::dynamic_extent>::ThreadPool(ThreadPool&&) noexcept = default;

ThreadPool<std::dynamic_extent>::~ThreadPool()
{
    if (this->n_core != nullptr) {
        this->Shutdown();
    }
}

auto ThreadPool<std::dynamic_extent>::Threads() const noexcept -> UInt
{
    MutexLock lock(this->n_core->Mutex);
    return this->n_core->Live;
}

auto ThreadPool<std::dynamic_extent>::MaxThreads() const noexcept -> UInt
{
    return this->n_core->Max;
}

auto ThreadPool<std::dynamic_extent>::Submit(ptr::Unique<Runnable> task) -> io::experimental::Result<void>
{
    this->reap();

    bool needsWorker = false;
    {
        MutexLock lock(this->n_core->Mutex);
        if (this->n_core->ShuttingDown) {
            return Err(pool_internal::ShutDownError());
        }

        this->n_core->Queue.push_back(VIOLET_MOVE(task));
        needsWorker = this->n_core->Queue.size() > this->n_core->IdleWorkers
            && this->n_core->Live < this->n_core->Max;
    }

    this->n_core->WorkAvailability.Signal();
    if (!needsWorker) {
        return {};
    }

    if (auto result = spawnWorker(this->n_core); result.Err()) {
        // the task stays queued, so a later spawn will still pick it up
        MutexLock lock(this->n_core->Mutex);
        if (this->n_core->Live == 0) {
            return result;
        }
    }

    return {};
}

void ThreadPool<std::dynamic_extent>::WaitForIdle()
{
    this->n_core->WaitForIdle();
}

void ThreadPool<std::dynamic_extent>::Shutdown()
{
    this->n_core->Shutdown(true);
    this->joinAll();
}

void ThreadPool<std::dynamic_extent>::ShutdownNow()
{
    this->n_core->Shutdown(false);
    this->joinAll();
}

auto ThreadPool<std::dynamic_extent>::spawnWorker(Own<Core> core) -> io::experimental::Result<void>
{
    // Holding the lock while spawning keeps the new worker from running (and possibly
    // retiring) before it is registered in `Workers` and counted in `Live`.
    MutexLock lock(core->Mutex);
    if (core->ShuttingDown || core->Live >= core->Max) {
        return {};
    }

    const UInt index = core->NextIndex++;
    auto thread = Thread::Builder()
                      .WithName(std::format("{}[#{}]", core->Prefix, index))
                      .Spawn([core, index]() mutable -> void {
                          core->RunWorkerLoop();

                          MutexLock lock(core->Mutex);
                          core->Exited.push_back(index);
                      });

    if (thread.Err()) {
        return Err(VIOLET_MOVE(thread).Error());
    }

    core->Live++;
    (void)core->Workers.Insert(index, VIOLET_MOVE(thread).Value());

    return {};
}

void ThreadPool<std::dynamic_extent>::reap()
{
    Vec<Thread> finished;
    {
        MutexLock lock(this->n_core->Mutex);
        for (const UInt index: this->n_core->Exited) {
            if (auto worker = this->n_core->Workers.Remove(index); worker.HasValue()) {
                finished.push_back(VIOLET_MOVE(worker).Value());
            }
        }

        this->n_core->Exited.clear();
    }

    for (Thread& worker: finished) {
        if (worker.Joinable()) {
            worker.Join();
        }
    }
}

void ThreadPool<std::dynamic_extent>::joinAll()
{
    HashMap<UInt, Thread> workers;
    {
        MutexLock lock(this->n_core->Mutex);
        workers = VIOLET_MOVE(this->n_core->Workers);
        this->n_core->Workers.Clear();
        this->n_core->Exited.clear();
    }

    for (auto& [_, worker]: workers) {
        if (worker.Joinable()) {
            worker.Join();
        }
    }
}

auto ThreadPool<std::dynamic_extent>::Builder::WithMinThreads(UInt n) & noexcept -> Builder&
{
    this->n_min = n;
    return *this;
}

auto ThreadPool<std::dynamic_extent>::Builder::WithMinThreads(UInt n) && noexcept -> Builder&&
{
    this->n_min = n;
    return VIOLET_MOVE(*this);
}

auto ThreadPool<std::dynamic_extent>::Builder::WithMaxThreads(UInt n) & noexcept -> Builder&
{
    this->n_max = n;
    return *this;
}

auto ThreadPool<std::dynamic_extent>::Builder::WithMaxThreads(UInt n) && noexcept -> Builder&&
{
    this->n_max = n;
    return VIOLET_MOVE(*this);
}

auto ThreadPool<std::dynamic_extent>::Builder::WithKeepAlive(chrono::Duration dur) & noexcept -> Builder&
{
    this->n_keepAlive = dur;
    return *this;
}

auto ThreadPool<std::dynamic_extent>::Builder::WithKeepAlive(chrono::Duration dur) && noexcept -> Builder&&
{
    this->n_keepAlive = dur;
    return VIOLET_MOVE(*this);
}

auto ThreadPool<std::dynamic_extent>::Builder::WithNamePrefix(Str prefix) & -> Builder&
{
    this->n_prefix = String(prefix);
    return *this;
}

auto ThreadPool<std::dynamic_extent>::Builder::WithNamePrefix(Str prefix) && -> Builder&&
{
    this->n_prefix = String(prefix);
    return VIOLET_MOVE(*this);
}

auto ThreadPool<std::dynamic_extent>::Builder::Build() -> io::experimental::Result<ThreadPool>
{
    auto core = Own<Core>::New();
    core->Max = this->n_max == 0 ? std::max<UInt>(AvailableParallelism(), 1) : this->n_max;
    core->Min = std::min(this->n_min, core->Max);
    core->KeepAlive = this->n_keepAlive;
    core->Prefix = this->n_prefix;

    ThreadPool pool(VIOLET_MOVE(core));
    for (UInt i = 0; i < pool.n_core->Min; i++) {
        if (auto result = spawnWorker(pool.n_core); result.Err()) {
            return Err(VIOLET_MOVE(result).Error());
        }
    }

    return pool;
}

} // namespace violet::experimental::threading
