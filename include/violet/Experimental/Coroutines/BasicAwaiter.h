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
//! # 🌺💜 `violet/Experimental/Coroutines/BasicAwaiter.h`

#pragma once

#include <violet/Experimental/Tuple.h>

#include <coroutine>

namespace violet::experimental::coro {
namespace internals {

template<typename T>
concept await_suspend_result
    = std::is_void_v<T> || std::same_as<T, bool> || std::convertible_to<T, std::coroutine_handle<>>;

template<typename Fn, typename Promise, typename... Args>
concept invocable_with_handle = std::is_nothrow_invocable_v<Fn, std::coroutine_handle<Promise>, Args...>
    && await_suspend_result<std::invoke_result_t<Fn, std::coroutine_handle<Promise>, Args...>>;

template<typename Fn, typename... Args>
concept invocable_without_handle
    = std::is_nothrow_invocable_v<Fn, Args...> && await_suspend_result<std::invoke_result_t<Fn, Args...>>;

} // namespace internals

template<typename Fn, bool AwaitReady = false, typename... Args>
struct NOELDOC_EXPERIMENTAL_SINCE("current") BasicAwaiter final {
    VIOLET_DISALLOW_CONSTRUCTOR(BasicAwaiter);

    VIOLET_IMPLICIT BasicAwaiter(Fn fun, Args&&... args) noexcept
        : n_fun(VIOLET_MOVE(fun))
        , n_args(VIOLET_FWD(Args, args)...)
    {
    }

    [[nodiscard]]
    constexpr auto await_ready() const noexcept -> bool
    {
        return AwaitReady;
    }

    template<typename Promise>
        requires(
            internals::invocable_with_handle<Fn, Promise, Args...> || internals::invocable_without_handle<Fn, Args...>)
    auto await_suspend(std::coroutine_handle<Promise> handle) noexcept -> decltype(auto)
    {
        if constexpr (internals::invocable_with_handle<Fn, Promise, Args...>) {
            return this->n_args.Apply(
                [handle, fn = VIOLET_MOVE(this->n_fun)](auto&&... args) noexcept -> decltype(auto) {
                    return std::invoke(fn, handle, VIOLET_FWD(decltype(args), args)...);
                });
        } else {
            return this->n_args.Apply(VIOLET_MOVE(this->n_fun));
        }
    }

    void await_suspend(std::coroutine_handle<>) const noexcept
        requires AwaitReady
    {
        VIOLET_UNREACHABLE();
    }

    auto await_resume() const noexcept -> decltype(auto)
    {
        if constexpr (AwaitReady) {
            static_assert(std::is_nothrow_invocable_v<Fn, Args...>, "function type should never throw anything");
            return this->n_args.Apply(VIOLET_MOVE(this->n_fun));
        }
    }

private:
    Fn n_fun;
    Tuple<Args...> n_args;
};

template<bool AwaitReady = false, typename Fn, typename... Args>
constexpr auto MkBasicAwaiter(Fn fun, Args&&... args) noexcept -> BasicAwaiter<Fn, AwaitReady, Args...>
{
    return {VIOLET_MOVE(fun), VIOLET_FWD(Args, args)...};
}

} // namespace violet::experimental::coro
