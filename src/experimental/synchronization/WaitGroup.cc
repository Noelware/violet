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

#include <violet/Experimental/Synchronization/WaitGroup.h>

using violet::experimental::sync::WaitGroup;

void WaitGroup::Add(UInt32 delta)
{
    VIOLET_ASSERT(delta != 0, "`delta' cannot be set to zero");

    auto count = this->n_count.Lock();
    *count += delta;
}

void WaitGroup::Done()
{
    auto count = this->n_count.Lock();
    if (--(*count) == 0) {
        this->n_cv.SignalAll();
    }
}

void WaitGroup::Wait()
{
    auto guard = this->n_count.Lock();
    while (*guard != 0) {
        this->n_cv.Wait(&this->n_count.Mutex());
    }
}
