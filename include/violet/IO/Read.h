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

#include <violet/IO/Error.h>
#include <violet/Violet.h>

#include <concepts>

namespace violet::io {

/// Reads data from a buffer or data source into a given storage.
///
/// ## Example
///
/// @param data storage for the read data
/// @param buf buffer to read from
/// @returns I/O result representing the number of bytes read, or an error if any read fails.
auto Read(Vec<UInt8>& data, Span<UInt8> buf) -> Result<UInt>;

template<typename T>
concept FreeMemberRead = requires(T& ty, Span<UInt8> buf) {
    { ::violet::io::Read(ty, buf) } -> std::same_as<Result<UInt>>;
};

template<typename T>
concept MemberRead = requires(T& ty, Span<UInt8> buf) {
    { ty.Read(buf) } -> std::same_as<Result<UInt>>;
};

/// Concept to determine if `T` supports reading operations.
template<typename T>
concept Readable = FreeMemberRead<T> || MemberRead<T>;

/// Generic read function for any type `T`.
///
/// This overload allows reading from any type `T` that does not have a member `Read`
/// but has a free-function `violet::io::Read(T&, Span<UInt8>)`.
///
/// @param reader The object or storage from which to read data.
/// @param buf The buffer into which data is read.
/// @return I/O result indicating the number of bytes successfully read or an error.
template<typename T>
inline auto Read(T& reader, Span<UInt8> buf) -> Result<UInt>
{
    if constexpr (requires { reader.Read(buf); }) {
        return reader.Read(buf);
    } else if constexpr (requires { ::violet::io::Read(reader, buf); }) {
        return ::violet::io::Read(reader, buf);
    } else {
        VIOLET_UNREACHABLE();
    }
}

/// Reads all data from a readable source into a `String`.
///
/// This function provides a convenient way to read all available data from
/// any object satisfying the `Readable` concept and accumulate it into a `String`.
///
/// @tparam R A type that satisfies the `Readable` concept.
/// @param reader The readable source to read from.
/// @return I/O result containing the accumulated string data or an error.
template<Readable R, typename StringType = String>
    requires(requires(StringType str) {
        typename StringType::value_type;
        typename StringType::size_type;

        { str.append(std::declval<const String::value_type*>(), std::declval<typename StringType::size_type>()) };
    } && !std::same_as<std::decay_t<StringType>, Str>)
auto ReadToString(R& reader) -> Result<StringType>
{
    StringType out;

    Array<UInt8, 4096> buf;
    while (true) {
        const UInt bytes = VIOLET_TRY(violet::io::Read(reader, buf));
        if (bytes == 0) {
            break;
        }

        out.append(reinterpret_cast<const char*>(buf.data()), bytes);
    }

    return out;
}

} // namespace violet::io
