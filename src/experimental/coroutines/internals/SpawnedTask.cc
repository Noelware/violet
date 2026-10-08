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

#include <violet/Experimental/Coroutines/Internals/SpawnedTask.h>

namespace violet::experimental::coro::internals {

auto GetHeaderFromFrame(Unsafe, void* frame) noexcept -> RawTask*
{
    return std::launder(reinterpret_cast<RawTask*>(static_cast<std::byte*>(frame) - kSpawnedTaskHeaderSize));
}

void RetainRawTask(Unsafe, RawTask* task) noexcept
{
    VIOLET_ASSUME(task != nullptr);
    task->RefCount.fetch_add(1, std::memory_order_relaxed);
}

void ReleaseRawTask(Unsafe, RawTask* task) noexcept
{
    if (task->RefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        void* frame = task->Frame;
        auto* destroy = task->VTable->Destroy;

        std::invoke(destroy, frame);
    }
}

} // namespace violet::experimental::coro::internals
