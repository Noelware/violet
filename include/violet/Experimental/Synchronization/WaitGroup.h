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
//! # 🌺💜 `violet/Experimental/Synchronization/WaitGroup.h`

#pragma once

#include <violet/Experimental/Mutex.h>
#include <violet/Experimental/Synchronized.h>

namespace violet::experimental::sync {

/// A synchronization primitive that allows waiting for a collection of threads to finish their work.
///
/// `WaitGroup` is inspired by Go's [`sync.WaitGroup`] and conceptually to Rust's [`std::thread::JoinHandle`]
/// collection pattern. A `WaitGroup` waits for a collection of threads to finish. The "main" thread calls
/// [`WaitGroup::Add`] to set the number of threads to wait for, then each thread will call [`WaitGroup::Done`]
/// when that thread's work is complete.
//
/// [`sync.WaitGroup`]: https://pkg.go.dev/sync#WaitGroup
/// [`std::thread::JoinHandle`]: https://doc.rust-lang.org/1.95.0/std/thread/struct.JoinHandle.html
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Synchronization/WaitGroup.h>
///
/// using namespace violet::experimental::sync;
///
/// WaitGroup wg;
/// wg.Add(2);
///
/// std::thread thread1([&wg] -> void {
///     // thread 1's work here...
///     wg.Done();
/// });
///
/// std::thread thread2([&wg] -> void {
///     // thread 2's work here...
///     wg.Done();
/// });
///
/// wg.Wait(); // now we wait until both threads are done executing
/// ```
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("26.07") WaitGroup final {
    VIOLET_DISALLOW_COPY_AND_MOVE(WaitGroup);

    /// Constructs a `WaitGroup` with an initial count of zero.
    VIOLET_IMPLICIT WaitGroup() noexcept = default;
    ~WaitGroup() = default;

    /// Adds `delta` to the wait group counter.
    ///
    /// [`WaitGroup::Add`] must be called before spawning the threads it tracks. Calling
    /// **Add** after [`WaitGroup::Wait`] has started blocking may result in undefined behaviour
    /// if the counter reaches zero before the new **Add** call is observed.
    ///
    /// @param delta number to add to the counter. must be greater than zero.
    void Add(UInt32 delta = 1);

    /// Decrement the wait group counter by one.
    ///
    /// Should be called by each tracked thread upon completion of its work.
    /// When the counter reaches zero, all threads blocked on [`WaitGroup::Wait`] are
    /// released.
    void Done();

    /// Blocks the calling thread until the `WaitGroup` counter reaches zero.
    ///
    /// If the counter is already zero when [`Wait`] is called, it returns
    /// immediately without blocking.
    void Wait() VIOLET_NO_THREAD_SAFETY_ANALYSIS;

private:
    Condvar n_cv;
    Synchronized<UInt32> n_count = 0;
};

} // namespace violet::experimental::sync
