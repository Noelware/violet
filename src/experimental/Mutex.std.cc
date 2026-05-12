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

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE_ABSEIL
#error "don't include this when building Violet with Abseil support"
#endif

#include <violet/Experimental/Mutex.h>
#include <violet/Experimental/Time/Duration.h>

using violet::experimental::Condvar;
using violet::experimental::Mutex;
using violet::experimental::MutexLock;

void Mutex::Lock()
{
    this->n_mux.lock();
}

void Mutex::Unlock()
{
    this->n_mux.unlock();
}

auto Mutex::TryLock() -> bool
{
    return this->n_mux.try_lock();
}

MutexLock::MutexLock(Mutex& mux)
    : n_mux(mux)
{
    this->n_mux.Lock();
}

MutexLock::~MutexLock()
{
    this->n_mux.Unlock();
}

void Condvar::Wait(Mutex* mux)
{
    std::unique_lock<std::mutex> lock(mux->n_mux, std::adopt_lock);
    this->n_cv.wait(lock);
    (void)lock.release();
}

auto Condvar::WaitWithTimeout(Mutex* mux, std::chrono::nanoseconds timeout) -> bool
{
    std::unique_lock<std::mutex> lock(mux->n_mux, std::adopt_lock);

    const auto status = this->n_cv.wait_for(lock, timeout);
    (void)lock.release();

    return status == std::cv_status::timeout;
}

void Condvar::Signal()
{
    this->n_cv.notify_one();
}

void Condvar::SignalAll()
{
    this->n_cv.notify_all();
}

auto Condvar::WaitWithTimeout(Mutex* mux, chrono::Duration dur) -> bool
{
    return this->WaitWithTimeout(mux, dur.Cast<std::chrono::nanoseconds>());
}
