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

#include "violet/subprocess/Stdio.h"
#include "violet/violet.h"

using Noelware::Violet::Span;
using Noelware::Violet::uint8;
using Noelware::Violet::Process::Stderr;
using Noelware::Violet::Process::Stdin;
using Noelware::Violet::Process::Stdout;

auto Stdin::Write(Span<const uint8>) -> IO::Result<usize>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto Stdin::Flush() -> IO::Result<void>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto Stdout::Read(Span<uint8>) -> IO::Result<usize>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}

auto Stderr::Read(Span<uint8>) -> IO::Result<usize>
{
    return IO::Error::New<String>(IO::ErrorKind::Unsupported, "unsupported operation");
}
