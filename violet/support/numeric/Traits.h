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

#include <concepts>
#include <limits>

namespace Noelware::Violet::Numeric {

/// Extra traits for integral types that extend C++'s [`std::numeric_limits`].
template<std::integral N>
struct Traits final {

#define MK_WRAPPING(NAME, OP)                                                                                          \
    static constexpr auto Wrapping##NAME(N lhs, N rhs) noexcept -> N                                                   \
    {                                                                                                                  \
        if constexpr (std::unsigned_integral<N>) {                                                                     \
            return lhs OP rhs;                                                                                         \
        } else {                                                                                                       \
            using U = std::make_unsigned_t<N>;                                                                         \
            return static_cast<N>(static_cast<U>(lhs) OP static_cast<U>(rhs));                                         \
        }                                                                                                              \
    }

    MK_WRAPPING(Add, +);
    MK_WRAPPING(Sub, -);
    MK_WRAPPING(Mul, *);

#undef MK_WRAPPING

    static constexpr auto WrappingDiv(N lhs, N rhs) noexcept -> N
    {
        if constexpr (std::unsigned_integral<N>) {
            return lhs / rhs;
        } else {
            if (rhs == -1 && lhs == baseTraits().min()) {
                return lhs;
            }

            return lhs / rhs;
        }
    }

private:
    static constexpr auto baseTraits() noexcept -> std::numeric_limits<N>
    {
        static auto limits = std::numeric_limits<N>{};
        return limits;
    }
}; // namespace Noelware::Violet::Numeric

template<typename T>
struct Traits<const T>: public Traits<T> {};

template<typename T>
struct Traits<volatile T>: public Traits<T> {};

template<typename T>
struct Traits<const volatile T>: public Traits<T> {};

} // namespace Noelware::Violet::Numeric
