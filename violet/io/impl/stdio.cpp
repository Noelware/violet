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

#include "violet/io/Stdio.h"
#include "violet/container/Result.h"
#include "violet/violet.h"

#include <iostream>
#include <mutex>

using Noelware::Violet::Span;
using Noelware::Violet::IO::Locked::Stderr;
using Noelware::Violet::IO::Locked::Stdin;
using Noelware::Violet::IO::Locked::Stdout;

Stdout::Stdout() = default;

auto Stdout::Write(Span<const uint8> buf) noexcept -> Result<usize, Error>
{
    std::lock_guard<Mutex> lock(this->n_mux);

    // Don't try to write more than what `std::streamsize` can handle.
    auto limit = std::numeric_limits<std::streamsize>::max();
    if (buf.size() > limit) {
        return Err(IO::Error::New<String>(IO::ErrorKind::InvalidData,
            std::format("buffer size {} is over the limit for `std::streamsize` ({})", buf.size(), limit)));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::cout.write(reinterpret_cast<CStr>(buf.data()), static_cast<std::streamsize>(buf.size()));
    if (std::cout.fail()) {
        return Err(IO::Error::New<String>(IO::ErrorKind::InvalidInput, "failed to write to stdout"));
    }

    std::cout.flush();
    return buf.size();
}

auto Stdout::Flush() const noexcept -> Result<void, Error>
{
    std::lock_guard<Mutex> lock(this->n_mux);
    std::cout.flush();

    return {};
}

Stderr::Stderr() = default;

auto Stderr::Write(Span<const uint8> buf) noexcept -> Result<usize, Error>
{
    std::lock_guard<Mutex> lock(this->n_mux);

    // Don't try to write more than what `std::streamsize` can handle.
    auto limit = std::numeric_limits<std::streamsize>::max();
    if (buf.size() > limit) {
        return Err(IO::Error::New<String>(IO::ErrorKind::InvalidData,
            std::format("buffer size {} is over the limit for `std::streamsize` ({})", buf.size(), limit)));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::cerr.write(reinterpret_cast<CStr>(buf.data()), static_cast<std::streamsize>(buf.size()));
    if (std::cerr.fail()) {
        return Err(IO::Error::New<String>(IO::ErrorKind::InvalidInput, "failed to write to stdout"));
    }

    std::cerr.flush();
    return buf.size();
}

auto Stderr::Flush() const noexcept -> Result<void, Error>
{
    std::lock_guard<Mutex> lock(this->n_mux);
    std::cerr.flush();

    return {};
}

Stdin::Stdin() = default;

auto Stdin::Read(Span<uint8> buf) noexcept -> Result<usize, Error>
{
    std::lock_guard<Mutex> lock(this->n_mux);

    // Don't try to write more than what `std::streamsize` can handle.
    auto limit = std::numeric_limits<std::streamsize>::max();
    if (buf.size() > limit) {
        return Err(IO::Error::New<String>(IO::ErrorKind::InvalidData,
            std::format("buffer size {} is over the limit for `std::streamsize` ({})", buf.size(), limit)));
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::cin.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(buf.size()));
    if (std::cin.bad()) {
        return Err(IO::Error::New<String>(IO::ErrorKind::InvalidInput, "failed to read from stdin"));
    }

    return std::cin.gcount();
}
