// 🌺💜 Violet: Extended C standard library
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

#include <violet/Experimental/Coroutines/Internals/Scheduler.h>
#include <violet/Experimental/Coroutines/Waker.h>

namespace violet::experimental::coro {
namespace {

using internals::RawTask;

auto asTask(void* data) noexcept -> RawTask*
{
    return static_cast<RawTask*>(data);
}

constexpr Waker::VTable kTaskWakerVTable{
    // clang-format off
    .Wake = [](void* data) noexcept -> void {
        internals::WakeupTask(Unsafe("the waker owns one reference"), asTask(data));
        internals::ReleaseRawTask(Unsafe("`Wake` consumes the waker's reference"), asTask(data));
    },
    .WakeByRef = [](void* data) noexcept -> void {
        internals::WakeupTask(Unsafe("the waker owns one reference"), asTask(data));
    },
    .Clone = [](void* data, void** outData, const Waker::VTable** outVt) noexcept -> void {
        internals::RetainRawTask(Unsafe("the clone owns its own reference"), asTask(data));
        *outData = data;
        *outVt = &kTaskWakerVTable;
    },
    .Drop = [](void* data) noexcept -> void {
        internals::ReleaseRawTask(Unsafe("dropping the waker's reference"), asTask(data));
    }
    // clang-format on
};

constexpr Waker::VTable kNoopWakerVTable{
    // clang-format off
    .Wake = [](void*) noexcept -> void {},
    .WakeByRef = [](void*) noexcept -> void {},
    .Clone = [](void*, void** outData, const Waker::VTable** outVt) noexcept -> void {
        *outData = nullptr;
        *outVt = &kNoopWakerVTable;
    },
    .Drop = [](void*) noexcept -> void {}
    // clang-format on
};

} // namespace

auto Waker::ForCurrentTask() noexcept -> Waker
{
    RawTask* task = internals::CurrentTask();
    VIOLET_ASSERT(task != nullptr, "`Waker::ForCurrentTask` was called outside of a running task");

    internals::RetainRawTask(Unsafe("we are running inside of `task`"), task);
    return Waker{task, &kTaskWakerVTable};
}

auto Waker::Noop() noexcept -> Waker
{
    return Waker{nullptr, &kNoopWakerVTable};
}

void Waker::Wake() && noexcept
{
    if (const auto* vt = std::exchange(this->n_vtable, nullptr))
        vt->Wake(std::exchange(this->n_object, nullptr));
}

void Waker::WakeByRef() noexcept
{
    VIOLET_ASSUME(this->n_vtable != nullptr);
    this->n_vtable->WakeByRef(this->n_object);
}

} // namespace violet::experimental::coro
