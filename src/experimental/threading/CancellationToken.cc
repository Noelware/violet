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

#include <violet/Experimental/Threading/CancellationToken.h>

using violet::experimental::threading::CancellationRequestedEvent;
using violet::experimental::threading::CancellationToken;
using violet::experimental::threading::CancellationTokenSource;

auto CancellationRequestedEvent::ToString() const noexcept -> CStr
{
    return "cancellation was requested";
}

auto CancellationTokenSource::Token() const noexcept -> CancellationToken
{
    return CancellationToken(this->n_state);
}

auto CancellationTokenSource::RequestsCancellation() const noexcept -> bool
{
    return this->n_state->Cancelled.load(std::memory_order_relaxed);
}

void CancellationTokenSource::Cancel() const noexcept
{
    {
        std::unique_lock lock(this->n_state->Mux);
        if (this->RequestsCancellation()) {
            return;
        }

        this->n_state->Cancelled.store(true, std::memory_order_release);
    }

    this->n_state->CV.notify_all();
    this->n_state->Emitter.Fire(CancellationRequestedEvent());
}

auto CancellationToken::RequestsCancellation() const noexcept -> bool
{
    if (this->n_state == nullptr) {
        return false;
    }

    return this->n_state->Cancelled.load(std::memory_order_relaxed);
}

void CancellationToken::WaitForCancellation() const noexcept
{
    if (this->n_state == nullptr) {
        return;
    }

    std::unique_lock lock(this->n_state->Mux);
    this->n_state->CV.wait(lock, [this] -> bool { return this->RequestsCancellation(); });
}
