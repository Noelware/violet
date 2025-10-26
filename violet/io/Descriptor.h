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
//! # 🌺💜 `violet/io/Descriptor.h`

#pragma once

#include "violet/violet.h"

#ifdef VIOLET_WINDOWS
#    include <windows.h>
#endif

namespace Noelware::Violet::IO {

/// A opaque identifier for a file descriptor on POSIX and `HANDLE` on Windows.
struct Descriptor final {
#ifdef VIOLET_WINDOWS
    using native_type = HANDLE;
    constexpr static native_type INVALID_HANDLE = INVALID_HANDLE_VALUE;
#elif defined(VIOLET_UNIX)
    using native_type = int32;
    constexpr static native_type INVALID_VALUE = -1;
#endif

    /// Creates a new [`Descriptor`] object, this will initialize the handle
    /// as invalid.
    Descriptor() noexcept = default;

#if defined(VIOLET_WINDOWS) || defined(VIOLET_UNIX)
    /// Creates a new [`Descriptor`] with the native handle/fd type.
    /// @param ty the handle or file descriptor to construct this with.
    VIOLET_EXPLICIT Descriptor(native_type ty) noexcept;
#endif

    /// Returns **true** if this file descriptor is considered valid.
    [[nodiscard]] auto Valid() const noexcept -> bool;

    /// Returns a string representation of this file descriptor.
    [[nodiscard]] auto ToString() const noexcept -> String;

    VIOLET_EXPLICIT operator bool() const noexcept;

#ifdef VIOLET_WINDOWS
    /// Returns the raw Windows `HANDLE`
    /// @returns the handle
    [[nodiscard]] constexpr auto Handle() const noexcept -> HANDLE
    {
        return this->n_handle;
    }
#elif defined(VIOLET_UNIX)
    /// Returns the raw POSIX file descriptor
    /// @returns the descriptor
    [[nodiscard]] constexpr auto AsFD() const noexcept -> int32
    {
        return this->n_fd;
    }
#endif

    VIOLET_OSTREAM_IMPL(const Descriptor&)
    {
        return os << self.ToString();
    }

    constexpr auto operator<=>(const Descriptor&) const noexcept = default;

private:
#ifdef VIOLET_WINDOWS
    HANDLE n_handle = INVALID_HANDLE_VALUE;
#elif defined(VIOLET_UNIX)
    int32 n_fd = -1;
#endif
};

} // namespace Noelware::Violet::IO
