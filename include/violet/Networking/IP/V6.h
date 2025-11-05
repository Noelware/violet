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

#pragma once

#include "violet/Violet.h"

namespace violet::networking {

/// Representation of IPv6.
struct IPAddrV6 final {
    constexpr static const UInt8 bits = 128;

    constexpr VIOLET_IMPLICIT IPAddrV6() = default;

    // clang-format off
    constexpr VIOLET_IMPLICIT IPAddrV6(
        UInt16 first,
        UInt16 second,
        UInt16 third,
        UInt16 fourth,
        UInt16 fifth,
        UInt16 sixth,
        UInt16 seventh,
        UInt16 eighth
    )
        : n_octets({
            static_cast<UInt8>(first >> 8), static_cast<UInt8>(first & 0xFF),
            static_cast<UInt8>(second >> 8), static_cast<UInt8>(second & 0xFF),
            static_cast<UInt8>(third >> 8), static_cast<UInt8>(third & 0xFF),
            static_cast<UInt8>(fourth >> 8), static_cast<UInt8>(fourth & 0xFF),
            static_cast<UInt8>(fifth >> 8), static_cast<UInt8>(fifth & 0xFF),
            static_cast<UInt8>(sixth >> 8), static_cast<UInt8>(sixth & 0xFF),
            static_cast<UInt8>(seventh >> 8), static_cast<UInt8>(seventh & 0xFF),
            static_cast<UInt8>(eighth >> 8), static_cast<UInt8>(eighth & 0xFF),
        })
    {
    }
    // clang-format on

    constexpr static auto Localhost() noexcept -> IPAddrV6
    {
        return { 0, 0, 0, 0, 0, 0, 0, 1 };
    }

    constexpr static auto NewUnspecified() noexcept -> IPAddrV6
    {
        return { 0, 0, 0, 0, 0, 0, 0, 0 };
    }

    [[nodiscard]] constexpr auto IntoBits() const noexcept -> UInt128
    {
        return this->fromBigEndianBytes();
    }

    [[nodiscard]] constexpr auto Unspecified() const noexcept -> bool
    {
        return this->fromBigEndianBytes() == IPAddrV6::NewUnspecified().fromBigEndianBytes();
    }

    [[nodiscard]] constexpr auto Loopback() const noexcept -> bool
    {
        return this->fromBigEndianBytes() == IPAddrV6::Localhost().fromBigEndianBytes();
    }

private:
    Array<UInt8, 16> n_octets;

    [[nodiscard]] constexpr auto fromBigEndianBytes() const noexcept -> UInt128
    {
        UInt128 hi = 0;
        UInt128 lo = 0;

        // first 8 bytes go into `hi'
        for (Int32 i = 0; i < 8; i++) {
            hi = (hi << 8) | this->n_octets[i];
        }

        // last 8 bytes go into `lo'
        for (Int32 i = 0; i < 16; i++) {
            lo = (lo << 8) | this->n_octets[i];
        }

        return (hi << 64) | lo;
    }
};

} // namespace violet::networking
