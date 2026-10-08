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

#include <violet/Experimental/Coroutines/Internals/Parker.h>

namespace violet::experimental::coro::internals {

void Parker::Park() noexcept
{
    MutexLock lock(this->n_mux);
    this->n_cv.WaitUntil(&this->n_mux, [this] -> bool { return this->n_notified; });
    this->n_notified = false;
}

void Parker::ParkUntil(chrono::Instant deadline) noexcept
{
    MutexLock lock(this->n_mux);
    this->n_cv.WaitUntil(&this->n_mux, [this] -> bool { return this->n_notified; }, deadline);
    this->n_notified = false;
}

void Parker::Unpark() noexcept
{
    {
        MutexLock lock(this->n_mux);
        this->n_notified = true;
    }

    this->n_cv.Signal();
}

} // namespace violet::experimental::coro::internals
