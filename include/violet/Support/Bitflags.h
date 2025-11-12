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

#include <type_traits>

namespace violet {

/// Determines if `E` is a valid enumeration to be used with `Bitflags`.
template<typename E>
concept ValidBitflagsEnum = std::is_enum_v<E> && std::is_unsigned_v<std::underlying_type_t<E>>;

template<ValidBitflagsEnum E>
struct VIOLET_API Bitflags final {
    using underlying_type = std::underlying_type_t<E>;

    constexpr VIOLET_IMPLICIT Bitflags() = default;
    constexpr VIOLET_IMPLICIT Bitflags(E enum_)
        : n_bits(static_cast<underlying_type>(enum_))
    {
    }

    constexpr VIOLET_EXPLICIT Bitflags(underlying_type bits)
        : n_bits(bits)
    {
    }

    constexpr auto Contains(E enum_) const noexcept -> bool
    {
        return Contains(static_cast<underlying_type>(enum_));
    }

    constexpr auto Contains(underlying_type ty) const noexcept -> bool
    {
        return (this->n_bits & ty) != 0;
    }

    constexpr void Add(underlying_type bit) noexcept
    {
        this->n_bits |= bit;
    }

    constexpr void Add(E enum_) noexcept
    {
        return Add(static_cast<underlying_type>(enum_));
    }

    constexpr void Remove(underlying_type ty) noexcept
    {
        this->n_bits &= ty;
    }

    constexpr void Remove(E enum_) noexcept
    {
        return Remove(static_cast<underlying_type>(enum_));
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_bits != 0;
    }

    constexpr VIOLET_EXPLICIT operator underlying_type() const noexcept
    {
        return this->n_bits;
    }

#define OPERATOR_ARG2(OP)                                                                                              \
    friend constexpr auto operator OP(Bitflags lhs, Bitflags rhs) noexcept -> Bitflags                                 \
    {                                                                                                                  \
        return Bitflags(lhs.n_bits OP rhs.n_bits);                                                                     \
    }

#define OPERATOR_ARG1(OP)                                                                                              \
    friend constexpr auto operator OP(Bitflags other) noexcept -> Bitflags                                             \
    {                                                                                                                  \
        return Bitflags(OP other.n_bits);                                                                              \
    }

    OPERATOR_ARG2(|);
    OPERATOR_ARG2(^);
    OPERATOR_ARG1(~);

#undef OPERATOR_ARG1
#undef OPERATOR_ARG2

#define OPERATOR_MUTATE(OP)                                                                                            \
    constexpr auto operator OP(Bitflags other) noexcept -> Bitflags&                                                   \
    {                                                                                                                  \
        this->n_bits OP other.n_bits;                                                                                  \
        return *this;                                                                                                  \
    }

    OPERATOR_MUTATE(|=);
    OPERATOR_MUTATE(&=);
    OPERATOR_MUTATE(^=);

#undef OPERATOR_MUTATE

private:
    underlying_type n_bits;
};

} // namespace violet
