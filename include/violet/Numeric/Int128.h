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

#include "absl/numeric/int128.h"
#include "violet/Violet.h"

namespace violet::numeric {

constexpr const double kTwoPow64 = 18446744073709551616.0; ///< double value for 2^64

namespace uint128 {

    /// Converts a [unsigned 128-bit integer][violet::UInt128] into a `double`.
    ///
    /// The value is split into `hi` and `lo` 64-bit parts and scaled accordingly:
    /// ```
    /// value = (high * 2^64) + low
    /// ```
    ///
    /// ## Remarks
    /// Precision is limited by `double` (~53 bits of mantissa). For larger values than 2^53, conversion
    /// is lossely precise.
    ///
    /// ## Example
    /// ```cpp
    /// UInt128 x = absl::MakeUint128(1, 0); // 2^64
    /// double y = violet::numeric::uint128::ToDouble(x);
    /// ASSERT_EQUALS(y, violet::numeric::kTwoPow64);
    /// ```
    constexpr auto ToDouble(UInt128 value) noexcept -> double
    {
        UInt64 hi = absl::Uint128High64(value);
        UInt64 lo = absl::Uint128Low64(value);

        return (static_cast<double>(hi) * kTwoPow64) + static_cast<double>(lo);
    }

} // namespace uint128

namespace int128 {

    /// Converts a [signed 128-bit integer][violet::Int128] into a `double`.
    ///
    /// Positive numbers are converted directly into a [`UInt128`] using [`uint128::ToDouble`]. Negative
    /// numbers are converted using its negated value of the unsigned conversion.
    ///
    /// ## Remarks
    /// Precision is limited by `double` (~53 bits of mantissa). For larger values than 2^53, conversion
    /// is lossely precise.
    ///
    /// ## Example
    /// ```cpp
    /// Int128 a = absl::MakeInt128(1, 0); // 2^64
    /// double y = violet::numeric::int128::ToDouble(a);
    /// ASSERT_EQUALS(y, violet::numeric::kTwoPow64);
    ///
    /// Int128 b = -absl::MakeInt128(1, 0); // -2^64
    /// double z = violet::numeric::int128::ToDouble(b);
    /// ASSERT_EQUALS(z, -violet::numeric::kTwoPow64);
    /// ```
    constexpr auto ToDouble(Int128 value) noexcept -> double
    {
        if (value >= 0) {
            return uint128::ToDouble(static_cast<UInt128>(value));
        }

        return -uint128::ToDouble(static_cast<UInt128>(-value));
    }

} // namespace int128

} // namespace violet::numeric
