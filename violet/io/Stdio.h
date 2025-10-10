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
//
//! # 🌺💜 `violet/io/Stdio.h`

#pragma once

#include "violet/container/Result.h"
#include "violet/core/StringRef.h"
#include "violet/io/Read.h"
#include "violet/io/Write.h"
#include "violet/violet.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Noelware::Violet::IO {

struct Stdin;
struct Stdout;
struct Stderr;

namespace Locked {

    struct Stdout final {
        auto Write(Span<const uint8> buf) noexcept -> Result<usize, std::error_code>;
        auto Flush() const noexcept -> Result<void, std::error_code>;

    private:
        friend struct Noelware::Violet::IO::Stdout;

        Stdout();

        mutable Mutex n_mux;

#ifdef _WIN32
        HANDLE n_handle;
#endif
    };

    struct Stderr final {
        auto Write(Span<const uint8> buf) noexcept -> Result<usize, std::error_code>;
        auto Flush() const noexcept -> Result<void, std::error_code>;

    private:
        friend struct Noelware::Violet::IO::Stderr;

        Stderr();

        mutable Mutex n_mux;

#ifdef _WIN32
        HANDLE n_handle;
#endif
    };

    struct Stdin final {
        auto Read(Span<uint8> buf) noexcept -> Result<usize, std::error_code>;

    private:
        friend struct Noelware::Violet::IO::Stdin;

        Stdin();

        mutable Mutex n_mux;

#ifdef _WIN32
        HANDLE n_handle;
#endif
    };

    static_assert(Readable<Stdin>, "`Stdin` is not readable");
    static_assert(Writable<Stdout>, "`Stdout` is not writable");
    static_assert(Writable<Stderr>, "`Stderr` is not writable");

} // namespace Locked

/// Represents a handle to the process' standard output using the library's
/// I/O primitives.
struct Stdout final {
    /// Construct a new [`Stdout`] handle. It'll use `std::cout` to write data.
    Stdout() = default;

    auto Writeln(StringRef str) const noexcept -> Result<usize, std::error_code>
    {
        auto res = this->Write(str);
        if (!res) {
            return Err(res.Error());
        }

        return this->Write("\n");
    }

    /// Extension for [`Stdout::Write(Span<const uint8>)`] to allow writing strings from
    /// a [`StringRef`] into the process' standard output.
    auto Write(StringRef str) const noexcept -> Result<usize, std::error_code>
    {
        return this->Write(str.AsBytes());
    }

    auto Write(Span<const uint8> buf) const noexcept -> Result<usize, std::error_code>
    {
        return this->Lock().Write(buf);
    }

    auto Flush() const noexcept -> Result<void, std::error_code>
    {
        return this->Lock().Flush();
    }

    [[nodiscard]] auto Lock() const noexcept -> Locked::Stdout
    {
        return {};
    }
};

static_assert(Writable<Stdout>, "`Stderr` is not writable");

/// Represents a handle to the process' standard error using the library's
/// I/O primitives.
struct Stderr final {
    /// Construct a new [`Stderr`] handle. It'll use `std::cerr` to write data.
    Stderr() = default;

    auto Writeln(StringRef str) const noexcept -> Result<usize, std::error_code>
    {
        auto res = this->Write(str);
        if (!res) {
            return Err(res.Error());
        }

        return this->Write("\n");
    }

    /// Extension for [`Stdout::Write(Span<const uint8>)`] to allow writing strings from
    /// a [`StringRef`] into the process' standard output.
    auto Write(StringRef str) const noexcept -> Result<usize, std::error_code>
    {
        return this->Write(str.AsBytes());
    }

    auto Write(Span<const uint8> buf) const noexcept -> Result<usize, std::error_code>
    {
        return this->Lock().Write(buf);
    }

    auto Flush() const noexcept -> Result<void, std::error_code>
    {
        return this->Lock().Flush();
    }

    [[nodiscard]] auto Lock() const noexcept -> Locked::Stderr
    {
        return {};
    }
};

static_assert(Writable<Stderr>, "`Stderr` is not writable");

/// Represents a handle to the process' standard input using the library's
/// I/O primitives.
struct Stdin final {
    /// Construct a new [`Stdin`] handle. It'll use `std::cin` to read data.
    Stdin() = default;

    /// Implements the [`Readable`] concept.
    auto Read(Span<uint8> buf) const noexcept -> Result<usize, std::error_code>
    {
        return this->Lock().Read(buf);
    }

    /// Provides a C++-style lock to this [`Stdin`] handle.
    [[nodiscard]] auto Lock() const noexcept -> Locked::Stdin
    {
        return {};
    }
};

static_assert(Readable<Stdin>, "`Stdin` is not readable");

} // namespace Noelware::Violet::IO
