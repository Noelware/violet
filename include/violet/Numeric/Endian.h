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

#include "violet/Violet.h"

#include <bit>
#include <concepts>

namespace violet::numeric {

template<std::unsigned_integral T, UInt Size>
constexpr auto FromBeBytes(const Array<UInt8, Size>& bits) noexcept -> T
{
    T value = 0;
    for (UInt i = 0; i < Size; i++) {
        value = static_cast<T>((value << 8) | bits[i]);
    }

    if constexpr (std::endian::native == std::endian::little) {
        value = std::byteswap(value);
    }

    return value;
}

template<std::unsigned_integral T>
constexpr auto FromBeBytes(const Array<UInt8, sizeof(T)>& bits) noexcept -> T
{
    T value = 0;
    for (UInt i = 0; i < sizeof(T); i++) {
        value = static_cast<T>((value << 8) | bits[i]);
    }

    if constexpr (std::endian::native == std::endian::little) {
        value = std::byteswap(value);
    }

    return value;
}

} // namespace violet::numeric
