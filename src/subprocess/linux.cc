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

#include <violet/Violet.h>

#ifdef VIOLET_LINUX

#include <violet/IO/Error.h>
#include <violet/Subprocess.h>
#include <violet/Subprocess/Unix.h>

#include <grp.h>
#include <unistd.h>

using violet::Int32;
using violet::process::Command;

void Command::Impl::ApplyCredentials(io::FileDescriptor pipe) const noexcept
{
    auto writeErrnoToPipe = [&]() -> void {
        int err = errno;
        ::write(pipe.Get(), &err, sizeof(err));
    };

    if (auto gid = this->n_gid) {
        if (::setgid(*gid) != 0) {
            writeErrnoToPipe();
            _exit(127);
        }
    }

    if (!this->n_extraGids.empty()) {
        if (::setgroups(this->n_extraGids.size(), this->n_extraGids.data()) != 0) {
            writeErrnoToPipe();
            _exit(127);
        }
    }

    if (auto uid = this->n_uid) {
        if (::setuid(*uid) != 0) {
            writeErrnoToPipe();
            _exit(127);
        }
    }
}

void Command::Impl::ConfigureIO(Command::Impl::io_kind_t kind, int pipes[2]) const noexcept {}

#endif
