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

#include <violet/Experimental/Coroutines/Internals/IO/Op.h>
#include <violet/Experimental/Coroutines/Runtime.h>

namespace violet::experimental::coro::internals::io {

void CompleteOperation(Unsafe, Op* op, Int64 result) noexcept
{
    op->Result = result;
    op->Status = OpStatus::Done;

    if (op->Orphaned) {
        op->OnOrphanComplete(result);
        delete op;

        return;
    }

    Waker waiter = VIOLET_MOVE(op->Waiter);
    VIOLET_MOVE(waiter).Wake();
}

OpAwaitBase::~OpAwaitBase()
{
    if (this->n_op == nullptr) {
        return;
    }

    if (this->n_op->Status == OpStatus::Inflight) {
        this->n_op->Orphaned = true;
        this->n_driver->Orphan(std::exchange(this->n_op, nullptr));

        return;
    }

    delete this->n_op;
}

auto OpAwaitBase::Ready() noexcept -> bool
{
    auto& handle = Handle::Current();
    VIOLET_DEBUG_ASSERT(
        handle.IO != nullptr, "I/O was not enabled at runtime. Call `EnableIO()` in `Runtime::Builder` to enable it");

    this->n_driver = handle.IO;
    return this->n_driver->TryNow(*this->n_op);
}

void OpAwaitBase::Suspend() noexcept
{
    this->n_op->Waiter = Waker::ForCurrentTask();
    this->n_driver->Submit(*this->n_op);
}

} // namespace violet::experimental::coro::internals::io
