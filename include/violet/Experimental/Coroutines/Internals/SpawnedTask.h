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
//! # 🌺💜 `violet/Experimental/Coroutines/Internals/SpawnedTask.h`
//! Header file for spawning tasks internally from an executor.
//!
//! A <code>[`Task`]<\T\></code> is a lazy, user-facing coroutine primitive: it only runs
//! when something `co_await`s it. When a `Task` is handle to the runtime's scheduler, it
//! is wrapped in a [spawned task][SpawnedTask]: a small coroutine that `co_await`s the user's
//! task and stores its result.
//!
//! ## Why is `::operator new`/`::operator delete` defined?
//! The compiler looks up allocation functions in the promise's scope before falling back to the global
//! one. Defining them here lets us control how spawned frames, and only *spawned frames*, are allocated.
//! We use that to put the `RawTask` header and the coroutine frame in a single allocation:
//!
//! ```ignore
//!     base                        base + `kSpawnedTaskHeaderSize`
//!     v                           v
//!     [ RawTask header + padding ][ coroutine frame (promise, params, locals) ... ]
//! ```
//!
//! Without this, every spawn costs two allocations (the frame and a separate `RawTask`), and the
//! scheduler has to chase the pointer from the header to the frame on every resume. Fusing them will:
//!
//! * slice the allocations per spawn;
//! * keeps the header (refcount, run-queue link, flags) on the same cache lines as the start
//!   of the frame the executor is about to resume;
//! * gives the header and frame a single lifetime, managed by `RawTask::RefCount`
//! * lets us use sized deallocation, since the compiler hands `operator delete` the frame size.
//!
//! Plain, user `Task`s does NOT perform this. Inner tasks that are awaited and never escape are candiates
//! for heap allocation elision (HALO / CoroElide), and a custom allocator doesn't do must justice there. Spawned
//! frame always escape into the scheduler, so they are always heap-allocated anyway.
//!
//! ## Invariants
//! 1. [`std::coroutine_handle<>::address()`] equals the pointer returned from our `operator new` implementation.
//!    The C++ standard does not promise this, but Clang, GCC, and MSVC all place the resume/destroy pointers
//!    at the start of the allocation. It breaks if the compiler realigns an over-aligned frame inside the
//!    allocation. Over-aligned promises are rejected with a `static_assert`, and debug builds verify the
//!    invariant on every spawn.
//!
//! 2. The allocation is never elided. If it were, `operator new` would not run and the header
//!    would not exist. Spawned frames escape (their address is stored into the header and handed
//!    to the executor), which prevents elision. The debug check also catches it.
//!
//! 3. The frame is destroyed only when `RawTask::RefCount` reaches zero, never just because the
//!    coroutine finished. Destroying the frame frees the header too, and a `JoinHandle` may still
//!    need the result stored in the promise. That is why `final_suspend` is `suspend_always`.

#pragma once

#include <violet/Experimental/Coroutines/BasicAwaiter.h>
#include <violet/Experimental/Coroutines/Internals/RawTask.h>
#include <violet/Experimental/Coroutines/Task.h>

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals {

constexpr auto AlignUp(UInt value, UInt alignment) noexcept -> UInt
{
    return (value + alignment - 1) / alignment * alignment;
}

static_assert(alignof(RawTask) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__,
    "`RawTask` must not be over-aligned; it is placed at the start of a plain `::operator new` allocation");

/// The byte offset from the start of a spawned task's allocation to its coroutine frame.
///
/// This is rounded up from `__STDCPP_DEFAULT_NEW_ALIGNMENT__` so the frame gets the same alignment
/// the compiler assumes for memory returned by a promise's `operator new`.
inline constexpr UInt kSpawnedTaskHeaderSize = AlignUp(sizeof(RawTask), __STDCPP_DEFAULT_NEW_ALIGNMENT__);

/// Recovers the raw task header that lives directly in front of a spawned coroutine frame.
///
/// ## Safety
/// `frame` must be the address of a frame allocated by [`SpawnedTask<T>::promise_type::operator new`].
[[nodiscard]]
auto GetHeaderFromFrame(Unsafe, void* frame) noexcept -> RawTask*;

/// Adds a reference to the spawned task.
void RetainRawTask(Unsafe, RawTask* task) noexcept;

/// Drops a reference to a spawned task, destroying the frame (and with it the header) when the last
/// reference goes away.
void ReleaseRawTask(Unsafe, RawTask* task) noexcept;

void DeallocateRawTask(Unsafe, RawTask*, UInt frameSize);

#ifndef NDEBUG
/// The frame most recently was returned by [`SpawnedTask<T>::promise_type::operator new`] on this thread.
///
/// `get_return_object` checks it against [`std::coroutine_handle<>::address()`] to verify invariants. Nothing
/// can allocate another spawned frame in between: the compiler only copies parameters and constructs the promise
/// between the two calls.
inline thread_local void* tlastAllocatedFrame = nullptr;
#endif

/// The root coroutine of a task that has been handed to the runtime.
///
/// A `SpawnedTask<T>` owns one reference to its `RawTask`. It is move-only; destroying it
/// releases that reference. Ownership is normally transfered to the scheduler right away
/// with [`IntoRaw()`].
template<typename T>
struct SpawnedTask final {
    VIOLET_DISALLOW_COPY(SpawnedTask);

    struct promise_type final: task_internal::PromiseStorage<T> {
        RawTask* JoinWaiter = nullptr;

        [[nodiscard]]
        static auto operator new(UInt size) -> void*
        {
            auto* base = static_cast<std::byte*>(::operator new(kSpawnedTaskHeaderSize + size));
            ::new (static_cast<void*>(base)) RawTask{};

            void* frame = base + kSpawnedTaskHeaderSize;
#ifndef NDEBUG
            tlastAllocatedFrame = frame;
#endif

            return frame;
        }

        static void operator delete(void* frame, UInt size) noexcept
        {
            RawTask* header = GetHeaderFromFrame(Unsafe("the frame was handed to us by the compiler"), frame);
            if (header->RefCount.load(std::memory_order_acquire) > 0) {
                header->FrameSize = static_cast<UInt32>(size);
                header->State.fetch_or(kTaskStateFrameDropped, std::memory_order_release);

                return;
            }

            DeallocateRawTask(Unsafe("no references are available"), header, size);
        }

        [[nodiscard]]
        auto get_return_object() noexcept -> SpawnedTask
        {
            static_assert(alignof(promise_type) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__,
                "`SpawnedTask<T>` does not support over-aligned result types");

            auto handle = std::coroutine_handle<promise_type>::from_promise(*this);
            void* frame = handle.address();

            VIOLET_DEBUG_ASSERT(frame == tlastAllocatedFrame,
                "spawned coroutine frame was elided or realigned; `RawTask` header is not in front of it.");

#ifndef NDEBUG
            tlastAllocatedFrame = nullptr;
#endif

            RawTask* header = GetHeaderFromFrame(Unsafe("invariants are intact"), frame);
            header->Frame = frame;
            header->VTable = &kRawTaskVTable<promise_type>;

            return SpawnedTask{header};
        }

        [[nodiscard]]
        static auto initial_suspend() noexcept -> std::suspend_always
        {
            return {};
        }

        [[nodiscard]]
        static auto final_suspend() noexcept -> std::suspend_always
        {
            return {};
        }

        [[nodiscard]]
        auto Header() noexcept -> RawTask*
        {
            return GetHeaderFromFrame(Unsafe("this coroutine promise ensures the frame is a valid frame"),
                std::coroutine_handle<promise_type>::from_promise(*this).address());
        }
    };

    VIOLET_IMPLICIT SpawnedTask(SpawnedTask&& other) noexcept
        : n_header(std::exchange(other.n_header, nullptr))
    {
    }

    auto operator=(SpawnedTask&& other) noexcept -> SpawnedTask&
    {
        if (this != &other) {
            this->reset();
            this->n_header = std::exchange(other.n_header, nullptr);
        }

        return *this;
    }

    ~SpawnedTask()
    {
        this->reset();
    }

    [[nodiscard]]
    auto Header() const noexcept -> RawTask*
    {
        return this->n_header;
    }

    [[nodiscard]]
    auto IntoRaw() noexcept -> RawTask*
    {
        return std::exchange(this->n_header, nullptr);
    }

    [[nodiscard]]
    static auto PromiseOf(RawTask* header) noexcept -> promise_type&
    {
        VIOLET_DEBUG_ASSERT0(header != nullptr);
        return std::coroutine_handle<promise_type>::from_address(header->Frame).promise();
    }

private:
    VIOLET_EXPLICIT SpawnedTask(RawTask* hdr) noexcept
        : n_header(hdr)
    {
    }

    void reset()
    {
        if (this->n_header != nullptr) {
            ReleaseRawTask(Unsafe("the header is always non-null"), std::exchange(this->n_header, nullptr));
        }
    }

    RawTask* n_header = nullptr;
};

template<typename T>
auto MkSpawnedTask(Task<T> task) -> SpawnedTask<T>
{
    if constexpr (std::is_void_v<T>) {
        co_await VIOLET_MOVE(task);
        co_return;
    } else {
        co_return co_await VIOLET_MOVE(task);
    }
}

} // namespace experimental::coro::internals
} // namespace NOELDOC_HIDE violet
