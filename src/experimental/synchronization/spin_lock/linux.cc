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

#include <violet/Language/Macros.h>

#if defined(VIOLET_PLATFORM_LINUX) && VIOLET_PLATFORM(LINUX)

#include <violet/Experimental/Synchronization/SpinLock.h>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace violet::experimental::sync::spin_lock_internal {

void waitOn(std::atomic<UInt32>& word, UInt32 expected) noexcept
{
    ::syscall(
        SYS_futex, reinterpret_cast<UInt32*>(std::addressof(word)), FUTEX_WAIT_PRIVATE, expected, nullptr, nullptr, 0);
}

void wakeOne(std::atomic<UInt32>& word) noexcept
{
    ::syscall(SYS_futex, reinterpret_cast<UInt32*>(std::addressof(word)), FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
}

} // namespace violet::experimental::sync::spin_lock_internal

#endif
