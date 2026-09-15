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

#pragma once

#include <violet/Experimental/Mutex.h>
#include <violet/Experimental/Own.h>

#include <functional>
#include <queue>
#include <thread>

namespace violet::experimental::log::internals {

/// A single-consumer background worker that drains queued items of type `T` and
/// hands them to a consumer callback on a dedicated thread.
template<typename T>
struct BatchQueue final {
    /// A **consumer** receives a non-empty batch drained from the queue
    using Consumer = std::function<void(Vec<T>)>;

    struct Options {
        /// The maximum amount of items handed to the consumer per call. `0` means
        /// drain everything.
        UInt MaxBatchSize = 0;
    };

    VIOLET_DISALLOW_COPY_AND_MOVE(BatchQueue);
    VIOLET_DISALLOW_CONSTRUCTOR(BatchQueue);

    VIOLET_IMPLICIT BatchQueue(Consumer consumer, Options options = {}) noexcept
        : n_state(Own<state>::New(VIOLET_MOVE(consumer), VIOLET_MOVE(options)))
    {
    }

    ~BatchQueue()
    {
        {
            MutexLock lock(this->n_state->Mutex);
            this->n_state->Running = false;
        }

        this->n_state->CV.SignalAll();
        if (this->n_state->Thread.joinable()) {
            this->n_state->Thread.join();
        }
    }

    auto Options() const noexcept -> Options
    {
        return this->n_state.Options;
    }

    void Push(T item)
    {
        {
            MutexLock lock(this->n_state->Mutex);
            this->n_state->Queue.push(VIOLET_MOVE(item));
        }

        this->n_state->CV.Signal();
    }

    void Flush()
    {
        MutexLock lock(this->n_state->Mutex);
        while (!this->n_state->Queue.empty() || this->n_state->ProcessingData) {
            this->n_state->CV.Wait(&this->n_state->Mutex);
        }
    }

private:
    struct state final {
        std::thread Thread;
        Condvar CV;
        struct Mutex Mutex;
        std::queue<T> Queue;
        bool ProcessingData = false;
        std::atomic<bool> Running{false};
        struct Options Options;
        Consumer ConsumerFn;

        VIOLET_IMPLICIT state(Consumer consumer, struct Options options) noexcept
            : Thread([this] -> void { this->workerLoop(); })
            , Running(true)
            , Options(VIOLET_MOVE(options))
            , ConsumerFn(VIOLET_MOVE(consumer))
        {
        }

    private:
        void workerLoop()
        {
            this->Mutex.Lock();
            while (true) {
                while (this->Queue.empty() && this->Running) {
                    this->CV.Wait(&this->Mutex);
                }

                if (this->Queue.empty() && !this->Running) {
                    break;
                }

                Vec<T> batch;
                const UInt batchLimit = this->Options.MaxBatchSize;
                while (!this->Queue.empty() && (batchLimit == 0 || batch.size() < batchLimit)) {
                    batch.push_back(VIOLET_MOVE(this->Queue.front()));
                    this->Queue.pop();
                }

                this->ProcessingData = true;
                this->Mutex.Unlock();
                std::invoke(this->ConsumerFn, VIOLET_MOVE(batch));

                this->Mutex.Lock();
                this->ProcessingData = false;
                this->CV.SignalAll();
            }

            this->Mutex.Unlock();
        }
    };

    Own<state> n_state;
};

} // namespace violet::experimental::log::internals
