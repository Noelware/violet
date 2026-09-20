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
        : n_running(true)
        , n_options(VIOLET_MOVE(options))
        , n_consumer(VIOLET_MOVE(consumer))
    {
        this->n_thread = std::thread([this] -> void { this->workerLoop(); });
    }

    ~BatchQueue()
    {
        {
            MutexLock lock(this->n_mux);
            this->n_running = false;
        }

        this->n_cv.SignalAll();
        if (this->n_thread.joinable()) {
            this->n_thread.join();
        }
    }

    auto Options() const noexcept -> Options
    {
        return this->n_options;
    }

    void Push(T item)
    {
        {
            MutexLock lock(this->n_mux);
            this->n_queue.push(VIOLET_MOVE(item));
        }

        this->n_cv.Signal();
    }

    void Flush()
    {
        MutexLock lock(this->n_mux);
        while (!this->n_queue.empty() || this->n_processingData) {
            this->n_cv.Wait(&this->n_mux);
        }
    }

private:
    Condvar n_cv;
    struct Mutex n_mux;
    std::queue<T> n_queue;
    bool n_processingData = false;
    std::atomic<bool> n_running{false};
    struct Options n_options;
    Consumer n_consumer;
    std::thread n_thread;

    void workerLoop()
    {
        this->n_mux.Lock();
        while (true) {
            while (this->n_queue.empty() && this->n_running) {
                this->n_cv.Wait(&this->n_mux);
            }

            if (this->n_queue.empty() && !this->n_running) {
                break;
            }

            Vec<T> batch;
            const UInt batchLimit = this->n_options.MaxBatchSize;
            while (!this->n_queue.empty() && (batchLimit == 0 || batch.size() < batchLimit)) {
                batch.push_back(VIOLET_MOVE(this->n_queue.front()));
                this->n_queue.pop();
            }

            this->n_processingData = true;
            this->n_mux.Unlock();
            std::invoke(this->n_consumer, VIOLET_MOVE(batch));

            this->n_mux.Lock();
            this->n_processingData = false;
            this->n_cv.SignalAll();
        }

        this->n_mux.Unlock();
    }
};

} // namespace violet::experimental::log::internals
