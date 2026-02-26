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

#include <violet/Violet.h>

#include <type_traits>

namespace violet {

template<typename E>
    requires(std::is_enum_v<E> && std::is_unsigned_v<std::underlying_type_t<E>>)
struct VIOLET_API Bitflags final {
    using underlying_type = std::underlying_type_t<E>;

    constexpr VIOLET_IMPLICIT Bitflags() noexcept = default;

    constexpr VIOLET_IMPLICIT Bitflags(underlying_type bits) noexcept
        : n_bits(bits)
    {
    }

    constexpr VIOLET_IMPLICIT Bitflags(E enum_) noexcept
        : Bitflags(static_cast<underlying_type>(enum_))
    {
    }

    constexpr auto Contains(Bitflags flags) const noexcept -> bool
    {
        return (this->n_bits & flags.n_bits) == flags.n_bits;
    }

    constexpr auto Intersects(Bitflags flags) const noexcept -> bool
    {
        return (this->n_bits & flags.n_bits) != 0;
    }

    constexpr auto Remove(Bitflags flags) noexcept -> Bitflags&
    {
        this->n_bits &= ~flags.n_bits;
        return *this;
    }

    constexpr auto Apply(Bitflags flags) noexcept -> Bitflags&
    {
        this->n_bits |= flags.n_bits;
        return *this;
    }

    constexpr auto Toggle(Bitflags flags) noexcept -> Bitflags&
    {
        this->n_bits ^= flags.n_bits;
        return *this;
    }

    constexpr auto Get() const noexcept -> underlying_type
    {
        return this->n_bits;
    }

    constexpr auto operator|=(Bitflags other) noexcept -> Bitflags&
    {
        n_bits |= other.n_bits;
        return *this;
    }

    constexpr auto operator&=(Bitflags other) noexcept -> Bitflags&
    {
        n_bits &= other.n_bits;
        return *this;
    }

    constexpr auto operator^=(Bitflags other) noexcept -> Bitflags&
    {
        n_bits ^= other.n_bits;
        return *this;
    }

    constexpr auto operator|(Bitflags other) const noexcept -> Bitflags
    {
        return Bitflags(n_bits | other.n_bits);
    }

    constexpr auto operator&(Bitflags other) const noexcept -> Bitflags
    {
        return Bitflags(n_bits & other.n_bits);
    }

    constexpr auto operator^(Bitflags other) const noexcept -> Bitflags
    {
        return Bitflags(n_bits ^ other.n_bits);
    }

    constexpr auto operator~() const noexcept -> Bitflags
    {
        return Bitflags(~n_bits);
    }

private:
    underlying_type n_bits;
};

} // namespace violet

template<typename E>
    requires(std::is_enum_v<E> && std::is_unsigned_v<std::underlying_type_t<E>>)
constexpr auto operator|(E lhs, E rhs) noexcept -> violet::Bitflags<E>
{
    return violet::Bitflags<E>(lhs) | violet::Bitflags<E>(rhs);
}

template<typename E>
    requires(std::is_enum_v<E> && std::is_unsigned_v<std::underlying_type_t<E>>)
constexpr auto operator&(E lhs, E rhs) noexcept -> violet::Bitflags<E>
{
    return violet::Bitflags<E>(lhs) & violet::Bitflags<E>(rhs);
}

template<typename E>
    requires(std::is_enum_v<E> && std::is_unsigned_v<std::underlying_type_t<E>>)
constexpr auto operator^(E lhs, E rhs) noexcept -> violet::Bitflags<E>
{
    return violet::Bitflags<E>(lhs) ^ violet::Bitflags<E>(rhs);
}

template<typename E>
    requires(std::is_enum_v<E> && std::is_unsigned_v<std::underlying_type_t<E>>)
constexpr auto operator~(E val) noexcept -> violet::Bitflags<E>
{
    return ~violet::Bitflags<E>(val);
}
