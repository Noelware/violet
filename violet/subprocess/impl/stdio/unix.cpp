// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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

#include "violet/violet.h"

#ifdef VIOLET_UNIX

// clang-format off
#include "violet/io/Descriptor.h"
#include "violet/subprocess/Stdio.h"

#include <cerrno>
// clang-format on

using Noelware::Violet::Span;
using Noelware::Violet::uint8;
using Noelware::Violet::Process::Stderr;
using Noelware::Violet::Process::Stdin;
using Noelware::Violet::Process::Stdout;

Stdin::Stdin(IO::Descriptor::native_type ty, Stdio stdio) noexcept
    : n_descriptor(ty)
    , n_stdio(stdio)
{
}

auto Stdin::Write(Span<const uint8> buf) -> IO::Result<usize>
{
    if (!this->Valid()) {
        return 0;
    }

    int64 ret = ::write(this->n_descriptor.AsFD(), buf.data(), buf.size());
    if (ret == -1) {
        return Err(IO::Error::Platform(IO::ErrorKind::InvalidData));
    }

    return ret;
}

auto Stdin::Flush() -> IO::Result<void>
{
    if (!this->Valid()) {
        return {};
    }

    if (::fsync(this->n_descriptor.AsFD()) == -1) {
        return Err(IO::Error::Platform(IO::ErrorKind::Other));
    }

    return {};
}

Stdout::Stdout(IO::Descriptor::native_type ty, Stdio stdio) noexcept
    : n_descriptor(ty)
    , n_stdio(stdio)
{
}

auto Stdout::Read(Span<uint8> buf) -> IO::Result<usize>
{
    if (!this->Valid()) {
        return 0;
    }

    int64 ret = ::read(this->n_descriptor.AsFD(), buf.data(), buf.size());
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }

        return Err(IO::Error::Platform(IO::ErrorKind::Other));
    }

    return ret;
}

Stderr::Stderr(IO::Descriptor::native_type ty, Stdio stdio) noexcept
    : n_descriptor(ty)
    , n_stdio(stdio)
{
}

auto Stderr::Read(Span<uint8> buf) -> IO::Result<usize>
{
    if (!this->Valid()) {
        return 0;
    }

    int64 ret = ::read(this->n_descriptor.AsFD(), buf.data(), buf.size());
    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }

        return Err(IO::Error::Platform(IO::ErrorKind::Other));
    }

    return ret;
}

#endif
