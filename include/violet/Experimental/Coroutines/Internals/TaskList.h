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

#include <violet/Experimental/Coroutines/Internals/RawTask.h>
#include <violet/Experimental/Mutex.h>

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals {

struct VIOLET_API TaskList final {
    VIOLET_DISALLOW_COPY_AND_MOVE(TaskList);

    VIOLET_IMPLICIT TaskList() noexcept = default;

    ~TaskList()
    {
        VIOLET_DEBUG_ASSERT(this->n_head == nullptr, "task list was destroyed with live tasks; call `CancelAll` first");
    }

    void Insert(RawTask* task) noexcept;
    auto Remove(RawTask* task) noexcept -> bool;
    auto PopFront() noexcept -> RawTask*;

private:
    void unlink(RawTask* task) noexcept VIOLET_EXCLUSIVE_LOCKS_REQUIRED(this->n_mux);

    Mutex n_mux;
    RawTask* n_head VIOLET_GUARDED_BY(this->n_mux) = nullptr;
};

} // namespace experimental::coro::internals
} // namespace NOELDOC_HIDE violet
