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

#include "violet/Numeric/Int128.h"
#include "violet/Violet.h"
#include <violet/Container/Result.h>

namespace violet::resource {

struct ParseError final {
    [[nodiscard]] constexpr auto ToString() const noexcept -> String
    {
        switch (this->n_tag) {
        case tag::kUnknownSuffix:
            return "unknown suffix";

        case tag::kInvalidDouble:
            return "invalid double conversion";

        case tag::kOutOfRange:
            return "out of range";
        }
    }

    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

private:
    friend struct CPU;
    friend struct Memory;

    enum struct tag : UInt8 {
        kUnknownSuffix = 0,
        kInvalidDouble,
        kOutOfRange
    };

    constexpr VIOLET_EXPLICIT ParseError(tag tag)
        : n_tag(tag)
    {
    }

    tag n_tag;
};

struct CPU final {
    constexpr VIOLET_IMPLICIT CPU() = default;
    constexpr VIOLET_EXPLICIT CPU(UInt128 value)
        : n_value(value)
    {
    }

    static auto Parse(Str str) noexcept -> Result<CPU, ParseError>;

    [[nodiscard]] constexpr auto ToDouble() const noexcept -> double
    {
        return numeric::uint128::ToDouble(this->n_value);
    }

    [[nodiscard]] constexpr auto AsCores() const noexcept -> double
    {
        return ToDouble() / 1e9;
    }

    [[nodiscard]] auto ToString() const noexcept -> String;
    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

#define IMPL_OP(OP)                                                                                                    \
    constexpr auto operator OP(const CPU& other) const noexcept -> CPU                                                 \
    {                                                                                                                  \
        return CPU(this->n_value OP other.n_value);                                                                    \
    }

    IMPL_OP(+);
    IMPL_OP(-);
    IMPL_OP(/);
    IMPL_OP(*);

#undef IMPL_OP

private:
    UInt128 n_value = 0; ///< nano cores (1 core = 1e9)
};

struct Memory final {
    constexpr VIOLET_IMPLICIT Memory() = default;
    constexpr VIOLET_EXPLICIT Memory(UInt128 value)
        : n_value(value)
    {
    }

    static auto Parse(Str str) noexcept -> Result<Memory, ParseError>;

    [[nodiscard]] constexpr auto ToDouble() const noexcept -> double
    {
        return numeric::uint128::ToDouble(this->n_value);
    }

    [[nodiscard]] auto ToString() const noexcept -> String;
    auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << this->ToString();
    }

#define IMPL_OP(OP)                                                                                                    \
    constexpr auto operator OP(const Memory& other) const noexcept -> Memory                                           \
    {                                                                                                                  \
        return Memory(this->n_value OP other.n_value);                                                                 \
    }

    IMPL_OP(+);
    IMPL_OP(-);
    IMPL_OP(/);
    IMPL_OP(*);

#undef IMPL_OP

private:
    UInt128 n_value; ///< bytes
};

} // namespace violet::resource
