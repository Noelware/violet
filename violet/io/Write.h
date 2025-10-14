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
//! # 🌺💜 `violet/io/Write.h`

#pragma once

#include "violet/io/Error.h"
#include "violet/violet.h"

#include <concepts>

namespace Noelware::Violet::IO {

/// Writes a byte buffer into a string.
///
/// This function appends the contents of `buf` to `data`.
///
/// It performs a `reinterpret_cast` from `const uint8*` (alias for `std::byte`) to `const char*`
/// to allow insertion into a `String`. Use this only when the bytes in `buf` represent valid characters,
/// otherwise the behavior is implementation-defined.
///
/// @param data The target string to append bytes to.
/// @param buf The source buffer containing bytes to write.
/// @returns A `Result` holding the number of bytes written, or an error code on failure.
inline auto Write(String& data, Span<const uint8> buf) noexcept -> Result<usize>
{
    // SAFETY: reinterpret_cast from const std::byte* to const char* is required to append bytes to a string.
    // Only safe if buf contains valid character data.
    //
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    data.append(reinterpret_cast<CStr>(buf.data()), buf.size());

    return buf.size();
}

/// Writes a byte buffer into a vector.
///
/// This function appends the contents of `buf` directly into the vector `data`.
/// Suitable for arbitrary binary data.
///
/// @param data The target vector to append bytes to.
/// @param buf The source buffer containing bytes to write.
/// @returns A `Result` holding the number of bytes written.
inline auto Write(Vec<uint8>& data, Span<const uint8> buf) noexcept -> Result<usize>
{
    data.insert(data.end(), buf.begin(), buf.end());
    return buf.size();
}

/// Concept to ensure a type is "writable".
///
/// A type `T` satisfies `Writable` if:
///
/// * It has member functions `Write(Span<const uint8>)` and `Flush()` returning `Result<usize>` and `Result<void>`
/// respectively.
///
/// * Or, a free function `Noelware::Violet::IO::Write(T&, Span<const uint8>)` is available that returns
/// `Result<usize>`.
///
/// This concept allows generic algorithms to work with strings, vectors, or any custom writable type or lambda.
template<typename T>
concept Writable = requires(T ty, Span<const uint8> cnt) {
    { ty.Write(cnt) } -> std::same_as<Result<usize>>;
    { ty.Flush() } -> std::same_as<Result<void>>;
} || requires(T& data, Span<const uint8> cnt) {
    { Noelware::Violet::IO::Write(data, cnt) } -> std::same_as<Result<usize>>;
};

} // namespace Noelware::Violet::IO
