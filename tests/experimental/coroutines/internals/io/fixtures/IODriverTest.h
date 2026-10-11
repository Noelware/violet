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

#include <gtest/gtest.h>
#include <violet/Experimental/Coroutines/Runtime.h>

#if VIOLET_PLATFORM(UNIX)
#include <violet/Experimental/Slice.h>
#include <violet/IO/Experimental/Descriptor.h>

#include <fcntl.h>
#include <unistd.h>
#endif

namespace violet::experimental::coro::internals::io::testing {

struct IODriverTest: public ::testing::Test {
protected:
    void SetUp() override
    {
        // The backend is fixed by a Result type, if we can't build it (i.e., the io_uring driver
        // can fail if the environment disables it via container runtime / seccomp)
        DriverOptions options;
        if (auto error = CreateDriver(options); error.Err()) {
            GTEST_SKIP() << "configured I/O backend is not supported for this environment, skipping!: "
                         << error.Error();
        }
    }

    [[nodiscard]]
    static auto GetRuntime() -> ptr::Unique<Runtime>
    {
        return Runtime::Builder::CurrentThread().EnableAll().Build();
    }
};

#if VIOLET_PLATFORM(UNIX)
struct Pipes final {
    violet::io::experimental::Descriptor Read;
    violet::io::experimental::Descriptor Write;

    VIOLET_IMPLICIT Pipes() noexcept
    {
        Slice<Int32, 2> fds{-1, -1};
        VIOLET_ASSERT(::pipe2(fds.Data(), O_CLOEXEC | O_NONBLOCK) == 0, "failed to build pipes");

        this->Read = violet::io::experimental::Descriptor::FromRaw(fds[0]);
        this->Write = violet::io::experimental::Descriptor::FromRaw(fds[1]);
    }
};
#endif

} // namespace violet::experimental::coro::internals::io::testing
