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

#define __unsupported__(decl)                                                                                          \
    decl                                                                                                               \
    {                                                                                                                  \
        VIOLET_UNREACHABLE();                                                                                          \
    }

__unsupported__(auto ThreadId::Current() noexcept -> ThreadId);
__unsupported__(auto ThreadId::Main() noexcept -> ThreadId);
__unsupported__(auto ThreadId::ToString() const noexcept -> String);

__unsupported__(Thread::Thread(Thread&&) noexcept);
__unsupported__(Thread::~Thread());
__unsupported__(auto Thread::operator=(Thread&&) noexcept -> Thread&);
__unsupported__(auto Thread::Current() noexcept -> Thread);
__unsupported__(auto Thread::Main() noexcept -> Thread);
__unsupported__(auto Thread::Id() const noexcept -> ThreadId);
__unsupported__(auto Thread::Name() const noexcept -> Str);
__unsupported__(auto Thread::Joinable() const noexcept -> bool);
__unsupported__(void Thread::Cancel());
__unsupported__(void Thread::CancelAndJoin());
__unsupported__(void Thread::Join());
__unsupported__(void Thread::Detach());
__unsupported__(auto Thread::trampoline(void*) noexcept -> void*);

__unsupported__(auto Thread::Builder::WithName(Str) & noexcept -> Builder&);
__unsupported__(auto Thread::Builder::WithName(Str) && noexcept -> Builder&&);
__unsupported__(auto Thread::Builder::WithStackSize(UInt) & noexcept -> Builder&);
__unsupported__(auto Thread::Builder::WithStackSize(UInt) && noexcept -> Builder&&);
__unsupported__(auto Thread::Builder::spawn(ptr::Unique<Runnable>)->io::experimental::Result<Thread>);

} // namespace violet::experimental::threading
