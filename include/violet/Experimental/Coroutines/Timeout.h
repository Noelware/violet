
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
//! # 🌺💜 `violet/Experimental/Coroutines/Timeout.h`

#pragma once

#include <violet/Experimental/Coroutines/JoinHandle.h>
#include <violet/Experimental/Coroutines/Runtime.h>
#include <violet/Experimental/Coroutines/Sleep.h>

namespace violet::experimental::coro {
namespace NOELDOC_HIDE timeout_internal {

inline auto AbortAfter(chrono::Duration dur, AbortHandle handle) -> Task<void>
{
    co_await Sleep(dur);
    handle.Abort();
}

} // namespace NOELDOC_HIDE timeout_internal

template<typename T>
auto Timeout(chrono::Duration dur, Task<T> task) -> Task<Optional<join_value_t<T>>>
{
    auto& handle = Handle::Current();

    auto child = handle.Spawn(VIOLET_MOVE(task));
    auto timer = handle.Spawn(timeout_internal::AbortAfter(dur, child.GetAbortHandle()));

    Optional<join_value_t<T>> result = co_await VIOLET_MOVE(child).Cancellable();
    timer.Abort();

    co_return result;
}

} // namespace violet::experimental::coro
