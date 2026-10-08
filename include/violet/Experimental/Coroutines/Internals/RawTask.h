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

#include <violet/SourceLocation.h>
#include <violet/Violet.h>

#include <coroutine>

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals {

struct Scheduler;

struct VIOLET_API NOELDOC_HIDE RawTask final {
    struct VIOLET_API VTable final {
        void (*Resume)(void* frame) noexcept = nullptr;
        bool (*Done)(void* frame) noexcept = nullptr;
        void (*Destroy)(void* frame) noexcept = nullptr;
        RawTask* (*TakeJoinWaiter)(void* frame) noexcept = nullptr;

        template<typename Promise>
        constexpr static auto For() noexcept -> VTable
        {
            return {
                // clang-format off
                .Resume = [](void* frame) noexcept -> void {
                    std::coroutine_handle<Promise>::from_address(frame).resume();
                },
                .Done = [](void* frame) noexcept -> bool {
                    return std::coroutine_handle<Promise>::from_address(frame).done();
                },
                .Destroy = [](void* frame) noexcept -> void {
                    std::coroutine_handle<Promise>::from_address(frame).destroy();
                },
                .TakeJoinWaiter = [](void* frame) noexcept -> RawTask* {
                    Promise& promise = std::coroutine_handle<Promise>::from_address(frame).promise();
                    return std::exchange(promise.JoinWaiter, nullptr);
                }
                // clang-format on
            };
        }
    };

    void* Frame = nullptr;
    Scheduler* Owner = nullptr;
    const struct VTable* VTable = nullptr;
    std::atomic<Int32> RefCount{1};
    std::atomic<UInt32> State{0};
    std::atomic<bool> Cancelled{false};
    RawTask* Next = nullptr;
    std::coroutine_handle<> Leaf = nullptr; // innermost suspended coroutine; null = resume `Frame`
    SourceLocation Location;
};

/// The static vtable for a spawned coroutine whose promise type is `Promise`.
template<typename Promise>
inline constexpr struct RawTask::VTable kRawTaskVTable = RawTask::VTable::template For<Promise>();

} // namespace experimental::coro::internals
} // namespace NOELDOC_HIDE violet
