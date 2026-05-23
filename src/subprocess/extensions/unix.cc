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

#if VIOLET_PLATFORM(UNIX)

#include <violet/Subprocess/Extensions/Unix.h>
#include <violet/Subprocess/__detail/Impl.unix.h>

void violet::subprocess::ext::UID(Command& command, uid_t uid)
{
    command.n_impl->n_uid = uid;
}

void violet::subprocess::ext::GID(Command& command, gid_t gid)
{
    command.n_impl->n_gid = gid;
}

void violet::subprocess::ext::Groups(Command& command, std::initializer_list<gid_t> groups)
{
    command.n_impl->n_extraGroupIDs.insert(command.n_impl->n_extraGroupIDs.end(), groups.begin(), groups.end());
}

void violet::subprocess::ext::Groups(Command& command, Span<gid_t> groups)
{
    command.n_impl->n_extraGroupIDs.insert(command.n_impl->n_extraGroupIDs.end(), groups.begin(), groups.end());
}

void violet::subprocess::ext::PreExec(Command& command, PreExecFun exec)
{
    command.n_impl->n_exec = VIOLET_MOVE(exec);
}

#endif
