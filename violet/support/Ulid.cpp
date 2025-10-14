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

#include "violet/support/Ulid.h"
#include "violet/support/StringRef.h"
#include "violet/violet.h"

using Noelware::Violet::Array;
using Noelware::Violet::uint8;

// clang-format off
static constexpr Array<const char, 32> BASE32_CROCKFORD_ALPHABET = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', // 0..10 index
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',           // no I
    'J', 'K',                                         // no L
    'M', 'N',                                         // no O
    'P', 'Q', 'R', 'S', 'T',                          // no U
    'V', 'W', 'X', 'Y', 'Z'
};
// clang-format on

// clang-format off
constexpr static Array<uint8, 256> LOOKUP_TABLE = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 255, 255, 255,
    255, 255, 255, 255, 10, 11, 12, 13, 14, 15, 16, 17, 255, 18, 19, 255, 20, 21, 255, 22, 23, 24,
    25, 26, 255, 27, 28, 29, 30, 31, 255, 255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 15, 16, 17,
    255, 18, 19, 255, 20, 21, 255, 22, 23, 24, 25, 26, 255, 27, 28, 29, 30, 31, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};
// clang-format on

auto Noelware::Violet::Ulid::FromStr(StringRef str) -> Result<Ulid, DecodeUlidError>
{
    if (str.Length() != Length) {
        return DecodeUlidError(DecodeUlidError::Tag::kInvalidLength);
    }

    uint128 result;
    for (usize i = 0; i < str.Length(); i++) {
        uint8 value = LOOKUP_TABLE[static_cast<uint8>(str[i])];
        if (value == 255) {
            return DecodeUlidError(DecodeUlidError::Tag::kInvalidChar);
        }

        result = (result << 5) | value;
    }

    return Ulid(result);
}

auto Noelware::Violet::Ulid::ToString() const noexcept -> String
{
    String result(Length, '\0');
    uint128 value = this->n_value;

    for (usize i = 0; i < Length; i++) {
        result[Length - 1 - i] = BASE32_CROCKFORD_ALPHABET[static_cast<usize>(value & 0x1f)];
        value >>= 5;
    }

    result.shrink_to_fit();
    return result;
}
