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
//
//! # 🌺💜 `violet/Experimental/Coroutines/Synchronization/Oneshot.h`

#pragma once

#include <violet/Experimental/Coroutines/Waker.h>
#include <violet/Experimental/Mutex.h>
#include <violet/Experimental/Own.h>
#include <violet/Experimental/Pair.h>

#include <coroutine>

namespace violet::experimental::coro::oneshot {
namespace internals {

template<typename T>
struct VIOLET_API Shared final {
    mutable struct Mutex Mutex;
    Optional<T> Value;
    bool SenderClosed = false;
    bool ReceiverClosed = false;
    struct Waker Waker;
};

} // namespace internals

template<typename T>
struct Sender;

template<typename T>
struct Receiver;

template<typename T>
auto Channel() -> Pair<Sender<T>, Receiver<T>>;

template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("current") Sender final {
    VIOLET_DISALLOW_COPY(Sender);

    VIOLET_IMPLICIT Sender(Sender&& other) noexcept = default;
    auto operator=(Sender&& other) noexcept -> Sender&
    {
        if (this != &other) {
            this->close();
            this->n_shared = VIOLET_MOVE(other.n_shared);
        }

        return *this;
    }

    ~Sender()
    {
        this->close();
    }

    auto Send(T value) && -> bool
    {
        Own<internals::Shared<T>> shared = VIOLET_MOVE(this->n_shared);
        VIOLET_ASSERT(shared != nullptr, "called `Send` on a moved-from `oneshot::Sender`");

        Waker waker;
        {
            MutexLock lock(shared->Mutex);
            if (shared->ReceiverClosed) {
                return false;
            }

            shared->Value = VIOLET_MOVE(value);
            shared->SenderClosed = true;
            waker = VIOLET_MOVE(shared->Waker);
        }

        VIOLET_MOVE(waker).Wake();
        return true;
    }

    [[nodiscard]]
    auto Closed() const -> bool
    {
        MutexLock lock(this->n_shared->Mutex);
        return this->n_shared->ReceiverClosed;
    }

private:
    template<typename U>
    friend auto Channel() -> Pair<Sender<U>, Receiver<U>>;

    VIOLET_EXPLICIT Sender(Own<internals::Shared<T>> shared) noexcept
        : n_shared(VIOLET_MOVE(shared))
    {
    }

    void close() noexcept
    {
        if (this->n_shared == nullptr) {
            return;
        }

        Waker waker;
        {
            MutexLock lock(this->n_shared->Mutex);
            this->n_shared->SenderClosed = true;
            waker = VIOLET_MOVE(this->n_shared->Waker);
        }

        VIOLET_MOVE(waker).Wake();
        this->n_shared.Reset();
    }

    Own<internals::Shared<T>> n_shared;
};

template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("current") Receiver final {
    VIOLET_DISALLOW_COPY(Receiver);

    VIOLET_IMPLICIT Receiver(Receiver&& other) noexcept = default;
    auto operator=(Receiver&& other) noexcept -> Receiver& = delete;

    ~Receiver()
    {
        if (this->n_shared == nullptr) {
            return;
        }

        Waker dropped;
        {
            MutexLock lock(this->n_shared->Mutex);
            this->n_shared->ReceiverClosed = true;
            dropped = VIOLET_MOVE(this->n_shared->Waker);
        }
    }

    [[nodiscard]]
    auto TryReceive() -> Optional<T>
    {
        MutexLock lock(this->n_shared->Mutex);
        return std::exchange(this->n_shared->Value, Nothing);
    }

protected:
    struct Awaiter;

public:
    auto operator co_await() && noexcept -> Awaiter;

private:
    template<typename U>
    friend auto Channel() -> Pair<Sender<U>, Receiver<U>>;

    VIOLET_EXPLICIT Receiver(Own<internals::Shared<T>> shared) noexcept
        : n_shared(VIOLET_MOVE(shared))
    {
    }

    Own<internals::Shared<T>> n_shared;
};

template<typename T>
struct VIOLET_API Receiver<T>::Awaiter final {
    Receiver Self;

    [[nodiscard]]
    auto await_ready() const -> bool
    {
        MutexLock lock(this->Self.n_shared->Mutex);
        return this->Self.n_shared->Value.HasValue() || this->Self.n_shared->SenderClosed;
    }

    auto await_resume() -> Optional<T>
    {
        return this->Self.TryReceive();
    }

    auto await_suspend(std::coroutine_handle<>) -> bool
    {
        MutexLock lock(this->Self.n_shared->Mutex);
        if (this->Self.n_shared->Value.HasValue() || this->Self.n_shared->SenderClosed) {
            return false;
        }

        this->Self.n_shared->Waker = Waker::ForCurrentTask();
        return true;
    }
};

template<typename T>
auto Receiver<T>::operator co_await() && noexcept -> Awaiter
{
    return Awaiter{VIOLET_MOVE(*this)};
}

template<typename T>
auto Channel() -> Pair<Sender<T>, Receiver<T>>
{
    auto shared = Own<internals::Shared<T>>::New();
    return {Sender<T>(shared), Receiver<T>(VIOLET_MOVE(shared))};
}

} // namespace violet::experimental::coro::oneshot
