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

#include "violet/Numeric/Endian.h"
#include "violet/Violet.h"

namespace violet::networking {

/// Representation of a IPv4 address.
///
/// An IPv4 address are defined as 32-bit integers in [IETF RFC 791], usually
/// represented as four octets.
///
/// [IETF RFC 791]: https://datatracker.ietf.org/doc/html/rfc791
///
/// To represent either a IPv4 or IPv6 address, look into the [`violet::networking::IPAddr`]
/// struct.
///
/// ## Example
/// ```cpp
/// #include <violet/Networking/IP/V4.h>
///
/// auto localhost = violet::networking::LocalhostV4();
/// ```
struct IPAddrV4 final {
    /// Size of a IPv4 address in bits.
    constexpr static const UInt8 BITS = 32;

    /// Cannot construct a [`IPAddrV4`] without data.
    constexpr VIOLET_IMPLICIT IPAddrV4() = delete;

    constexpr VIOLET_IMPLICIT IPAddrV4(UInt8 first, UInt8 second, UInt8 third, UInt8 fourth) noexcept
        : n_octets({ first, second, third, fourth })
    {
    }

    constexpr VIOLET_EXPLICIT IPAddrV4(Array<UInt8, 4> octets)
        : n_octets(octets)
    {
    }

    [[nodiscard]] constexpr auto Loopback() const noexcept -> bool
    {
        return this->n_octets[0] == 127;
    }

    [[nodiscard]] constexpr auto Private() const noexcept -> bool
    {
        return (this->n_octets[0] == 10
            || (this->n_octets[0] == 172 && (this->n_octets[1] >= 16 && this->n_octets[1] <= 31))
            || (this->n_octets[0] == 192 && this->n_octets[1] == 168));
    }

    [[nodiscard]] constexpr auto LinkLocal() const noexcept -> bool
    {
        return this->n_octets[0] == 169 && this->n_octets[1] == 254;
    }

    [[nodiscard]] constexpr auto Unspecified() const noexcept -> bool
    {
        return numeric::FromBeBytes<UInt8>(this->n_octets) == 0;
    }

    [[nodiscard]] constexpr auto ToString() const noexcept -> String
    {
        return std::format("{}.{}.{}.{}", this->n_octets[0], this->n_octets[1], this->n_octets[2], this->n_octets[3]);
    }

private:
    Array<UInt8, 4> n_octets;
};

/// Creates a new [`IPAddrV4`] that resolves to `localhost`.
constexpr auto LocalhostV4() noexcept -> IPAddrV4
{
    return { 127, 0, 0, 1 };
}

/// Creates a new, unspecified [`IPAddrV4`] that resolves to `0.0.0.0`.
constexpr auto UnspecifiedV4() noexcept -> IPAddrV4
{
    return { 0, 0, 0, 0 };
}

/// Creates a new broadcast [`IPAddrV4`].
constexpr auto Broadcast() noexcept -> IPAddrV4
{
    return { 255, 255, 255, 255 };
}

} // namespace violet::networking
