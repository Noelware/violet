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
//! # 🌺💜 `violet/io/Read.h`

#pragma once

#include "violet/container/Result.h"
#include "violet/io/Error.h"
#include "violet/violet.h"

#include <algorithm>
#include <concepts>

namespace Noelware::Violet::IO {

inline auto Read(Vec<uint8>& data, Span<uint8> buf) noexcept -> Result<usize, Error>
{
    usize minCopy = std::min(buf.size(), data.size());
    std::copy_n(data.begin(), minCopy, buf.begin());

    return minCopy;
}

template<typename T>
concept Readable = requires(T ty, Span<uint8> buf) {
    { ty.Read(buf) } -> std::same_as<Result<usize, Error>>;
} || requires(T& data, Span<uint8> buf) {
    { Noelware::Violet::IO::Read(data, buf) } -> std::same_as<Result<usize, Error>>;
};

template<Readable R>
inline auto ReadToString(R& reader) -> Result<String, Error>
{
    String buf;
    constexpr static auto CHUNK_SIZE = 4096;

    Vec<uint8> chunk(CHUNK_SIZE);
    while (true) {
        auto res = reader.Read(chunk);
        if (res.IsErr()) {
            return Err(res.Error());
        }

        if (res.Value() == 0) {
            break;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        buf.append(reinterpret_cast<const char*>(chunk.data()), res.Value());
    }

    buf.shrink_to_fit();
    return VIOLET_MOVE(buf);
}

} // namespace Noelware::Violet::IO
