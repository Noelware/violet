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

#include <violet/Experimental/Threading/Thread.h>

namespace violet::experimental::threading {

auto ThreadId::ToString() const -> String
{
    return std::format("ThreadId({:x})", this->n_value);
}

Thread::Thread(Thread&& other) noexcept
    : n_joinable(std::exchange(other.n_joinable, false))
    , n_nativeThread(std::exchange(other.n_nativeThread, nullptr))
    , n_state(VIOLET_MOVE(other.n_state))
{
}

auto Thread::operator=(Thread&& other) noexcept -> Thread&
{
    if (this != &other) {
        if (this->n_joinable) {
            this->CancelAndJoin();
        }

        this->n_joinable = std::exchange(other.n_joinable, false);
        this->n_nativeThread = std::exchange(other.n_nativeThread, nullptr);
        this->n_state = VIOLET_MOVE(other.n_state);
    }

    return *this;
}

Thread::~Thread()
{
    if (this->n_joinable) {
        this->CancelAndJoin();
    }
}

auto Thread::Name() const noexcept -> Str
{
    if (!this->n_state->Name.HasValue()) {
        return "";
    }

    return this->n_state->Name.Value();
}

auto Thread::Joinable() const noexcept -> bool
{
    return this->n_joinable;
}

void Thread::Cancel()
{
    this->n_state->Cts.Cancel();
}

void Thread::CancelAndJoin()
{
    this->Cancel();
    this->Join();
}

auto Thread::Builder::WithName(Str name) & noexcept -> Builder&
{
    this->n_name = String(name);
    return *this;
}

auto Thread::Builder::WithName(Str name) && noexcept -> Builder&&
{
    this->n_name = String(name);
    return VIOLET_MOVE(*this);
}

auto Thread::Builder::WithStackSize(UInt bytes) & noexcept -> Builder&
{
    this->n_stackSize = bytes;
    return *this;
}

auto Thread::Builder::WithStackSize(UInt bytes) && noexcept -> Builder&&
{
    this->n_stackSize = bytes;
    return VIOLET_MOVE(*this);
}

} // namespace violet::experimental::threading
