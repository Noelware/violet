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
//
//! # 🌺💜 `violet/Experimental/Language/Int128.h`

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Container/Result.h>
#include <violet/Experimental/Pair.h>
#include <violet/SmolString.h>

#include <bit>
#include <functional>
#include <iterator>

#if defined(__SIZEOF_INT128__)
#define VIOLET_FEATURE_NATIVE_INT128 1
#else
#define VIOLET_FEATURE_NATIVE_INT128 0
#endif

namespace violet::experimental {
namespace int128_internal {

/// Computes the full 128-bit product of `a * b`, returning the low half and writing the high half into `hi`.
constexpr auto MulWide(UInt64 first, UInt64 second, UInt64& hi) noexcept -> UInt64
{
    constexpr UInt64 kMask = 0xFFFF'FFFF;

    const UInt64 p00 = (first & kMask) * (second & kMask);
    const UInt64 p01 = (first & kMask) * (second >> 32);
    const UInt64 p10 = (first >> 32) * (second & kMask);
    const UInt64 p11 = (first >> 32) * (second >> 32);

    const UInt64 mid = (p00 >> 32) + (p01 & kMask) + (p10 & kMask);
    hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);

    return (mid << 32) | (p00 & kMask);
}

/// Divides `(u1:u0)` by `v`, requiring `u1 < v` (Hacker's Delight, `divlu`).
constexpr auto DivWide(UInt64 u1, UInt64 u0, UInt64 value, UInt64& remainder) noexcept -> UInt64
{
    constexpr UInt64 kBase = UInt64{1} << 32;
    constexpr UInt64 kMask = kBase - 1;

    const auto shift = std::countl_zero(value);
    value <<= shift;

    const auto vn1 = value >> 32;
    const auto vn0 = value & kMask;
    const auto un32 = shift == 0 ? u1 : (u1 << shift) | (u0 >> (64 - shift));
    const auto un10 = u0 << shift;
    const auto un1 = un10 >> 32;
    const auto un0 = un10 & kMask;

    auto q1 = un32 / vn1;
    auto rhat = un32 - (q1 * vn1);
    while (q1 >= kBase || q1 * vn0 > (rhat << 32) + un1) {
        --q1;
        rhat += vn1;

        if (rhat >= kBase) {
            break;
        }
    }

    const auto un21 = (un32 << 32) + un1 - (q1 * value);
    auto q0 = un21 / vn1;
    rhat = un21 - (q0 * vn1);

    while (q0 >= kBase || q0 * vn0 > (rhat << 32) + un0) {
        --q0;
        rhat += vn1;
        if (rhat >= kBase) {
            break;
        }
    }

    remainder = ((un21 << 32) + un0 - (q0 * value)) >> shift;
    return (q1 << 32) + q0;
}

constexpr auto DigitValueOf(char ch) noexcept -> UInt32
{
    if (ch >= '0' && ch <= '9') {
        return static_cast<UInt32>(ch - '0');
    }

    if (ch >= 'a' && ch <= 'z') {
        return static_cast<UInt32>(ch - 'a') + 10;
    }

    if (ch >= 'A' && ch <= 'Z') {
        return static_cast<UInt32>(ch - 'A') + 10;
    }

    return 36;
}

} // namespace int128_internal

struct Int128;

// NOLINTBEGIN(cppcoreguidelines-prefer-member-initializer)

/// A 128-bit unsigned integer that behaves like the builtin unsigned integer types.
///
/// All arithmetic wraps modulo 2¹²⁸. Every integral type converts implicitly (sign-extending
/// like the builtins do), while floating-point types and [`Int128`] require an explicit cast.
///
/// ## Remarks
/// When the compiler provides `__int128`, multiplication and division lower to it; otherwise
/// a portable 64-bit-limb implementation is used. Both paths are usable in constant expressions.
///
/// ## Example
/// ```cpp
/// using namespace violet::experimental::int128_literals;
///
/// constexpr auto x = 0xFFFF'FFFF'FFFF'FFFF_u128;
/// static_assert(x + 1 == (violet::experimental::UInt128(1) << 64));
/// ```
struct
#if VIOLET_FEATURE(NATIVE_INT128)
    alignas(unsigned __int128)
#endif
        VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") UInt128 final {
    constexpr VIOLET_IMPLICIT UInt128() noexcept = default;

    template<std::integral N>
    constexpr VIOLET_IMPLICIT UInt128(N value) noexcept
        : n_lo(static_cast<UInt64>(value))
    {
        if constexpr (std::is_signed_v<N>) {
            this->n_hi = value < 0 ? ~UInt64{0} : 0;
        }
    }

    template<std::floating_point F>
    constexpr VIOLET_IMPLICIT UInt128(F value) noexcept
    {
        constexpr auto kTwo64 = static_cast<F>(0x1P64);
        if (value >= kTwo64) {
            this->n_hi = static_cast<UInt64>(value / kTwo64);
            this->n_lo = static_cast<UInt64>(value - (static_cast<F>(this->n_hi) * kTwo64));
        } else {
            this->n_lo = static_cast<UInt64>(value);
        }
    }

#if VIOLET_FEATURE(NATIVE_INT128)
    constexpr VIOLET_IMPLICIT UInt128(unsigned __int128 value) noexcept
    {
        this->n_lo = static_cast<UInt64>(value);
        this->n_hi = static_cast<UInt64>(value >> 64);
    }

    constexpr VIOLET_EXPLICIT operator unsigned __int128() const noexcept
    {
        return (static_cast<unsigned __int128>(this->n_hi) << 64) | this->n_lo;
    }

    constexpr VIOLET_IMPLICIT UInt128(__int128 value) noexcept
        : UInt128(static_cast<unsigned __int128>(value))
    {
    }

    constexpr VIOLET_EXPLICIT operator __int128() const noexcept
    {
        return static_cast<__int128>(static_cast<unsigned __int128>(*this));
    }
#endif

    constexpr static auto FromParts(UInt64 lo, UInt64 hi) noexcept -> UInt128
    {
        return UInt128(private_key{}, hi, lo);
    }

    constexpr static auto Min() noexcept -> UInt128
    {
        return 0;
    }

    constexpr static auto Max() noexcept -> UInt128
    {
        return UInt128::FromParts(~UInt64{0}, ~UInt64{0});
    }

    /// Computes the quotient and remainder of `lhs / rhs` at once.
    static constexpr auto DivRem(UInt128 lhs, UInt128 rhs) noexcept -> Pair<UInt128, UInt128>;

    [[nodiscard]]
    constexpr auto HighBits() const noexcept -> UInt64
    {
        return this->n_hi;
    }

    [[nodiscard]]
    constexpr auto LowBits() const noexcept -> UInt64
    {
        return this->n_lo;
    }

    [[nodiscard]]
    constexpr auto LeadingZeros() const noexcept -> UInt32
    {
        return this->n_hi != 0 ? std::countl_zero(this->n_hi) : 64 + std::countl_zero(this->n_lo);
    }

    [[nodiscard]]
    constexpr auto TrailingZeros() const noexcept -> UInt32
    {
        return this->n_lo != 0 ? std::countr_zero(this->n_lo) : 64 + std::countr_zero(this->n_hi);
    }

    [[nodiscard]]
    constexpr auto CountOnes() const noexcept -> UInt32
    {
        return std::popcount(this->n_lo) + std::popcount(this->n_hi);
    }

    [[nodiscard]]
    constexpr auto BitWidth() const noexcept -> UInt32
    {
        return 128 - this->LeadingZeros();
    }

    [[nodiscard]]
    constexpr auto PowerOfTwo() const noexcept -> bool
    {
        return this->CountOnes() == 1;
    }

    [[nodiscard]]
    constexpr auto RotateLeft(UInt32 amount) const noexcept -> UInt128
    {
        amount %= 128;
        return amount == 0 ? *this : (*this << amount) | (*this >> (128 - amount));
    }

    [[nodiscard]]
    constexpr auto RotateRight(UInt32 amount) const noexcept -> UInt128
    {
        amount %= 128;
        return amount == 0 ? *this : (*this >> amount) | (*this << (128 - amount));
    }

    [[nodiscard]]
    constexpr auto SwapBytes() const noexcept -> UInt128
    {
        constexpr auto swap = [](UInt64 value) noexcept -> UInt64 {
            UInt64 result = 0;
            for (Int32 i = 0; i < 8; ++i) {
                result = (result << 8) | ((value >> (i * 8)) & 0xFF);
            }

            return result;
        };

        return UInt128::FromParts(swap(this->n_hi), swap(this->n_lo));
    }

    [[nodiscard]]
    constexpr auto CheckedAdd(UInt128 rhs) const noexcept -> Optional<UInt128>
    {
        const UInt128 result = *this + rhs;
        if (result < *this) {
            return Nothing;
        }

        return Some(result);
    }

    [[nodiscard]]
    constexpr auto CheckedSub(UInt128 rhs) const noexcept -> Optional<UInt128>
    {
        const UInt128 result = *this - rhs;
        if (rhs > *this) {
            return Nothing;
        }

        return Some(result);
    }

    [[nodiscard]]
    constexpr auto CheckedMul(UInt128 rhs) const noexcept -> Optional<UInt128>
    {
        if (this->n_hi != 0 && rhs.n_hi != 0) {
            return Nothing;
        }

        UInt64 hi = 0;
        UInt64 crossHi1 = 0;
        UInt64 crossHi2 = 0;

        const UInt64 lo = int128_internal::MulWide(this->n_lo, rhs.n_lo, hi);
        const UInt64 cross = int128_internal::MulWide(this->n_hi, rhs.n_lo, crossHi1)
            + int128_internal::MulWide(this->n_lo, rhs.n_hi, crossHi2);

        if (crossHi1 != 0 || crossHi2 != 0 || hi + cross < hi) {
            return Nothing;
        }

        return UInt128::FromParts(lo, hi + cross);
    }

    [[nodiscard]]
    constexpr auto CheckedDiv(UInt128 rhs) const noexcept -> Optional<UInt128>
    {
        if (rhs == 0) {
            return Nothing;
        }

        return *this / rhs;
    }

    [[nodiscard]]
    constexpr auto SaturatingAdd(UInt128 rhs) const noexcept -> UInt128
    {
        auto result = this->CheckedAdd(rhs);
        return result.HasValue() ? VIOLET_MOVE(result).Value() : UInt128::Max();
    }

    [[nodiscard]]
    constexpr auto SaturatingSub(UInt128 rhs) const noexcept -> UInt128
    {
        auto result = this->CheckedSub(rhs);
        return result.HasValue() ? VIOLET_MOVE(result).Value() : UInt128::Min();
    }

    [[nodiscard]]
    constexpr auto SaturatingMul(UInt128 rhs) const noexcept -> UInt128
    {
        auto result = this->CheckedMul(rhs);
        return result.HasValue() ? VIOLET_MOVE(result).Value() : UInt128::Max();
    }

    [[nodiscard]]
    constexpr auto Pow(UInt32 exp) const noexcept -> UInt128
    {
        UInt128 base = *this;
        UInt128 result = 1;
        while (exp != 0) {
            if ((exp & 1) != 0) {
                result *= base;
            }

            base *= base;
            exp >>= 1;
        }

        return result;
    }

    constexpr static auto Parse(Str input, UInt32 radix = 10) noexcept -> Optional<UInt128>
    {
        if (radix < 2 || radix > 36) {
            return Nothing;
        }

        if (!input.empty() && input.front() == '+') {
            input.remove_prefix(1);
        }

        if (input.empty()) {
            return Nothing;
        }

        UInt128 result = 0;
        for (auto ch: input) {
            const auto digit = int128_internal::DigitValueOf(ch);
            if (digit >= radix) {
                return Nothing;
            }

            auto mul = UInt128{result}.CheckedMul(radix);
            if (!mul.HasValue()) {
                return Nothing;
            }

            auto add = UInt128{*mul}.CheckedAdd(digit);
            if (!add.HasValue()) {
                return Nothing;
            }

            result = VIOLET_MOVE(add).Value();
        }

        return result;
    }

    [[nodiscard]]
    auto ToString(UInt32 radix = 10, bool uppercase = false) const -> String
    {
        VIOLET_DEBUG_ASSERT(radix >= 2 && radix <= 36, "radix must be within [2, 36]");

        constexpr Str kLower = "0123456789abcdefghijklmnopqrstuvwxyz";
        constexpr Str kUpper = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        if (*this == 0) {
            return "0";
        }

        Str digits = uppercase ? kUpper : kLower;
        Array<char, 128> buf{};
        auto it = buf.end(); // NOLINT(readability-qualified-auto)

        UInt128 value = *this;
        while (value != 0) {
            auto [quotient, remainder] = UInt128::DivRem(value, radix);
            *--it = digits[remainder.n_lo];

            value = quotient;
        }

        return {it, buf.end()};
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return (this->n_hi | this->n_lo) != 0;
    }

    template<std::integral N>
        requires(!std::same_as<N, bool>)
    constexpr VIOLET_EXPLICIT operator N() const noexcept
    {
        return static_cast<N>(this->n_lo);
    }

    template<std::floating_point F>
    constexpr VIOLET_EXPLICIT operator F() const noexcept
    {
        return (static_cast<F>(this->n_hi) * static_cast<F>(0x1p64)) + static_cast<F>(this->n_lo);
    }

    friend constexpr auto operator==(const UInt128&, const UInt128&) noexcept -> bool = default;
    friend constexpr auto operator<=>(UInt128 lhs, UInt128 rhs) noexcept -> std::strong_ordering
    {
        if (auto cmp = lhs.n_hi <=> rhs.n_hi; cmp != 0) {
            return cmp;
        }

        return lhs.n_lo <=> rhs.n_lo;
    }

    constexpr auto operator+() const noexcept -> UInt128
    {
        return *this;
    }

    constexpr auto operator-() const noexcept -> UInt128
    {
        return ~*this + 1;
    }

    constexpr auto operator~() const noexcept -> UInt128
    {
        return UInt128::FromParts(~this->n_lo, ~this->n_hi);
    }

    friend constexpr auto operator+(UInt128 lhs, UInt128 rhs) noexcept -> UInt128
    {
        const UInt64 lo = lhs.n_lo + rhs.n_lo;
        return UInt128::FromParts(lo, lhs.n_hi + rhs.n_hi + static_cast<UInt64>(lo < lhs.n_lo));
    }

    friend constexpr auto operator-(UInt128 lhs, UInt128 rhs) noexcept -> UInt128
    {
        return UInt128::FromParts(lhs.n_lo - rhs.n_lo, lhs.n_hi - rhs.n_hi - static_cast<UInt64>(lhs.n_lo < rhs.n_lo));
    }

    friend constexpr auto operator*(UInt128 lhs, UInt128 rhs) noexcept -> UInt128
    {
#if VIOLET_FEATURE(NATIVE_INT128)
        return static_cast<unsigned __int128>(lhs) * static_cast<unsigned __int128>(rhs);
#else
        UInt64 hi = 0;
        UInt64 lo = int128_internal::MulWide(lhs.n_lo, rhs.n_lo, hi);

        return UInt128::FromParts(lo, hi + (lhs.n_lo * rhs.n_hi) + (lhs.n_hi * rhs.n_lo));
#endif
    }

    friend constexpr auto operator/(UInt128 lhs, UInt128 rhs) noexcept -> UInt128
    {
        return UInt128::DivRem(lhs, rhs).First;
    }

    friend constexpr auto operator%(UInt128 lhs, UInt128 rhs) noexcept -> UInt128
    {
        return UInt128::DivRem(lhs, rhs).Second;
    }

    friend constexpr auto operator&(UInt128 lhs, UInt128 rhs) noexcept -> UInt128
    {
        return UInt128::FromParts(lhs.n_lo & rhs.n_lo, lhs.n_hi & rhs.n_hi);
    }

    friend constexpr auto operator|(UInt128 lhs, UInt128 rhs) noexcept -> UInt128
    {
        return UInt128::FromParts(lhs.n_lo | rhs.n_lo, lhs.n_hi | rhs.n_hi);
    }

    friend constexpr auto operator^(UInt128 lhs, UInt128 rhs) noexcept -> UInt128
    {
        return UInt128::FromParts(lhs.n_lo ^ rhs.n_lo, lhs.n_hi ^ rhs.n_hi);
    }

    template<std::integral N>
    friend constexpr auto operator<<(UInt128 value, N amount) noexcept -> UInt128
    {
        auto shift = static_cast<UInt32>(amount);
        VIOLET_DEBUG_ASSERT(shift < 128, "shift must be less than 128");

        if (shift == 0) {
            return value;
        }

        if (shift >= 64) {
            return UInt128::FromParts(0, value.n_lo << (shift - 64));
        }

        return UInt128::FromParts(value.n_lo << shift, (value.n_hi << shift) | (value.n_lo >> (64 - shift)));
    }

    template<std::integral N>
    friend constexpr auto operator>>(UInt128 value, N amount) noexcept -> UInt128
    {
        auto shift = static_cast<UInt32>(amount);
        VIOLET_DEBUG_ASSERT(shift < 128, "shift must be less than 128");

        if (shift == 0) {
            return value;
        }

        if (shift >= 64) {
            return UInt128::FromParts(value.n_hi >> (shift - 64), 0);
        }

        return UInt128::FromParts((value.n_lo >> shift) | (value.n_hi << (64 - shift)), value.n_hi >> shift);
    }

    constexpr auto operator+=(UInt128 rhs) noexcept -> UInt128&
    {
        return *this = *this + rhs;
    }

    constexpr auto operator-=(UInt128 rhs) noexcept -> UInt128&
    {
        return *this = *this - rhs;
    }

    constexpr auto operator*=(UInt128 rhs) noexcept -> UInt128&
    {
        return *this = *this * rhs;
    }

    constexpr auto operator/=(UInt128 rhs) noexcept -> UInt128&
    {
        return *this = *this / rhs;
    }

    constexpr auto operator%=(UInt128 rhs) noexcept -> UInt128&
    {
        return *this = *this % rhs;
    }

    constexpr auto operator&=(UInt128 rhs) noexcept -> UInt128&
    {
        return *this = *this & rhs;
    }

    constexpr auto operator|=(UInt128 rhs) noexcept -> UInt128&
    {
        return *this = *this | rhs;
    }

    constexpr auto operator^=(UInt128 rhs) noexcept -> UInt128&
    {
        return *this = *this ^ rhs;
    }

    template<std::integral N>
    constexpr auto operator<<=(N amount) noexcept -> UInt128&
    {
        return *this = *this << amount;
    }

    template<std::integral N>
    constexpr auto operator>>=(N amount) noexcept -> UInt128&
    {
        return *this = *this >> amount;
    }

    constexpr auto operator++() noexcept -> UInt128&
    {
        return *this += 1;
    }

    constexpr auto operator--() noexcept -> UInt128&
    {
        return *this -= 1;
    }

    constexpr auto operator++(int) noexcept -> UInt128
    {
        UInt128 copy = *this;

        ++*this;
        return copy;
    }

    constexpr auto operator--(int) noexcept -> UInt128
    {
        UInt128 copy = *this;

        --*this;
        return copy;
    }

private:
    struct private_key final { };

    constexpr VIOLET_EXPLICIT UInt128(private_key, UInt64 hi, UInt64 lo) noexcept
    {
        this->n_lo = lo;
        this->n_hi = hi;
    }

// Ordered so the object representation matches a native `unsigned __int128`.
#ifdef VIOLET_PLATFORM_LITTLE_ENDIAN
    UInt64 n_lo = 0;
    UInt64 n_hi = 0;
#else
    UInt64 n_hi = 0;
    UInt64 n_lo = 0;
#endif
};

constexpr auto UInt128::DivRem(UInt128 lhs, UInt128 rhs) noexcept -> Pair<UInt128, UInt128>
{
    VIOLET_DEBUG_ASSERT(rhs != 0, "division by zero");

#if VIOLET_FEATURE(NATIVE_INT128)
    const auto lhs1 = static_cast<unsigned __int128>(lhs);
    const auto rhs1 = static_cast<unsigned __int128>(rhs);

    return {lhs1 / rhs1, lhs1 % rhs1};
#else
    if (rhs > lhs) {
        return {0, lhs};
    }

    UInt64 remainder = 0;
    if (rhs.n_hi == 0) {
        if (lhs.n_hi < rhs.n_lo) {
            const UInt64 quotient = int128_internal::DivWide(lhs.n_hi, lhs.n_lo, rhs.n_lo, remainder);
            return {quotient, remainder};
        }

        UInt64 qHi = lhs.n_hi / rhs.n_lo;
        UInt64 qLo = int128_internal::DivWide(lhs.n_hi % rhs.n_lo, lhs.n_lo, rhs.n_lo, remainder);
        return {UInt128::FromParts(qLo, qHi), remainder};
    }

    // Normalize so the divisor's top bit is set, estimate the quotient from its high
    // limb, then correct the estimate by at most one.
    const auto shift = std::countl_zero(rhs.n_hi);
    const auto v1 = (rhs << shift).n_hi;
    const auto u1 = lhs >> 1;
    auto estimate = int128_internal::DivWide(u1.n_hi, u1.n_lo, v1, remainder) >> (63 - shift);
    if (estimate != 0) {
        --estimate;
    }

    UInt128 quotient(estimate);
    UInt128 remaindr(lhs - (quotient * rhs));
    if (remaindr >= rhs) {
        ++quotient;
        remaindr -= rhs;
    }

    return {quotient, remaindr};
#endif
}

/// A 128-bit two's complement signed integer that behaves like the builtin signed integer types.
///
/// Unlike the builtins, overflow is well-defined and wraps. Every integral type converts implicitly,
/// while floating-point types and [`UInt128`] require an explicit cast.
///
/// ## Example
/// ```cpp
/// using namespace violet::experimental::int128_literals;
///
/// constexpr auto x = -170141183460469231731687303715884105727_i128;
/// static_assert(x - 1 == violet::experimental::Int128::Min());
/// ```
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Int128 final {
    constexpr VIOLET_IMPLICIT Int128() noexcept = default;

    template<std::integral N>
    constexpr VIOLET_IMPLICIT Int128(N value) noexcept
        : n_bits(value)
    {
    }

    template<std::floating_point F>
    constexpr VIOLET_EXPLICIT Int128(F value) noexcept
        : n_bits(value < 0 ? -UInt128{-value} : UInt128(value))
    {
    }

    constexpr VIOLET_EXPLICIT Int128(UInt128 value) noexcept
        : n_bits(value)
    {
    }

#if VIOLET_FEATURE(NATIVE_INT128)
    constexpr VIOLET_IMPLICIT Int128(__int128 value) noexcept
        : n_bits(value)
    {
    }

    constexpr VIOLET_EXPLICIT operator __int128() const noexcept
    {
        return static_cast<__int128>(this->n_bits);
    }

    constexpr VIOLET_IMPLICIT Int128(unsigned __int128 value) noexcept
        : n_bits(value)
    {
    }

    constexpr VIOLET_EXPLICIT operator unsigned __int128() const noexcept
    {
        return static_cast<unsigned __int128>(this->n_bits);
    }
#endif

    constexpr static auto FromParts(UInt64 lo, Int64 hi) noexcept -> Int128
    {
        return Int128{UInt128::FromParts(lo, static_cast<UInt64>(hi))};
    }

    constexpr static auto Min() noexcept -> Int128
    {
        return FromParts(0, std::numeric_limits<Int64>::min());
    }

    constexpr static auto Max() noexcept -> Int128
    {
        return FromParts(~UInt64{0}, std::numeric_limits<Int64>::max());
    }

    constexpr static auto DivRem(Int128 lhs, Int128 rhs) noexcept -> Pair<Int128, Int128>
    {
        auto [quot, rem] = UInt128::DivRem(lhs.UnsignedAbsolute(), rhs.UnsignedAbsolute());
        Int128 quotient(quot);
        Int128 remainder(rem);

        return {lhs.Negative() != rhs.Negative() ? -quotient : quotient, lhs.Negative() ? -remainder : remainder};
    }

    [[nodiscard]]
    constexpr auto HighBits() const noexcept -> Int64
    {
        return static_cast<Int64>(this->n_bits.HighBits());
    }

    [[nodiscard]]
    constexpr auto LowBits() const noexcept -> UInt64
    {
        return this->n_bits.LowBits();
    }

    [[nodiscard]]
    constexpr auto Negative() const noexcept -> bool
    {
        return this->HighBits() < 0;
    }

    [[nodiscard]]
    constexpr auto Positive() const noexcept -> bool
    {
        return !this->Negative() && this->n_bits != 0;
    }

    [[nodiscard]]
    constexpr auto Signum() const noexcept -> Int32
    {
        return this->Negative() ? -1 : static_cast<Int32>(this->n_bits != 0);
    }

    [[nodiscard]]
    constexpr auto Absolute() const noexcept -> Int128
    {
        return this->Negative() ? -*this : *this;
    }

    [[nodiscard]]
    constexpr auto UnsignedAbsolute() const noexcept -> UInt128
    {
        return this->Negative() ? -this->n_bits : this->n_bits;
    }

    [[nodiscard]]
    constexpr auto LeadingZeros() const noexcept -> UInt32
    {
        return this->n_bits.LeadingZeros();
    }

    [[nodiscard]]
    constexpr auto TrailingZeros() const noexcept -> UInt32
    {
        return this->n_bits.TrailingZeros();
    }

    [[nodiscard]]
    constexpr auto CountOnes() const noexcept -> UInt32
    {
        return this->n_bits.CountOnes();
    }

    [[nodiscard]]
    constexpr auto SwapBytes() const noexcept -> Int128
    {
        return Int128{this->n_bits.SwapBytes()};
    }

    [[nodiscard]]
    constexpr auto CheckedAdd(Int128 rhs) const noexcept -> Optional<Int128>
    {
        const Int128 result = *this + rhs;
        if (this->Negative() == rhs.Negative() && result.Negative() != this->Negative()) {
            return Nothing;
        }

        return Some(result);
    }

    [[nodiscard]]
    constexpr auto CheckedSub(Int128 rhs) const noexcept -> Optional<Int128>
    {
        const Int128 result = *this - rhs;
        if (this->Negative() != rhs.Negative() && result.Negative() != this->Negative()) {
            return Nothing;
        }

        return Some(result);
    }

    [[nodiscard]]
    constexpr auto CheckedMul(Int128 rhs) const noexcept -> Optional<Int128>
    {
        auto magnitude = this->UnsignedAbsolute().CheckedMul(rhs.UnsignedAbsolute());
        UInt128 limit = UInt128{1} << 127;
        if (this->Negative() == rhs.Negative()) {
            limit -= 1;
        }

        if (!magnitude.HasValue() || *magnitude > limit) {
            return Nothing;
        }

        return Some(*this * rhs);
    }

    [[nodiscard]]
    constexpr auto CheckedDiv(Int128 rhs) const noexcept -> Optional<Int128>
    {
        if (rhs == 0 || (*this == Int128::Min() && rhs == -1)) {
            return Nothing;
        }

        return *this / rhs;
    }

    [[nodiscard]]
    constexpr auto SaturatingAdd(Int128 rhs) const noexcept -> Int128
    {
        if (const auto result = this->CheckedAdd(rhs); result.HasValue()) {
            return VIOLET_MOVE(result).Value();
        }

        return this->Negative() ? Int128::Min() : Int128::Max();
    }

    [[nodiscard]]
    constexpr auto SaturatingSub(Int128 rhs) const noexcept -> Int128
    {
        if (const auto result = this->CheckedSub(rhs); result.HasValue()) {
            return VIOLET_MOVE(result).Value();
        }

        return this->Negative() ? Int128::Min() : Int128::Max();
    }

    [[nodiscard]]
    constexpr auto SaturatingMul(Int128 rhs) const noexcept -> Int128
    {
        if (const auto result = this->CheckedMul(rhs); result.HasValue()) {
            return VIOLET_MOVE(result).Value();
        }

        return this->Negative() != rhs.Negative() ? Int128::Min() : Int128::Max();
    }

    [[nodiscard]]
    constexpr auto Pow(UInt32 exp) const noexcept -> Int128
    {
        return Int128{this->n_bits.Pow(exp)};
    }

    constexpr static auto Parse(Str input, UInt32 radix = 10) noexcept -> Optional<Int128>
    {
        const bool negative = !input.empty() && input.front() == '-';
        if (negative) {
            input.remove_prefix(1);
            if (!input.empty() && input.front() == '+') {
                return Nothing;
            }
        }

        const auto magnitude = UInt128::Parse(input, radix);
        if (!magnitude.HasValue()) {
            return Nothing;
        }

        UInt128 limit(UInt128{1} << 127);
        if (*magnitude > (negative ? limit : limit - 1)) {
            return Nothing;
        }

        Int128 result(VIOLET_MOVE(magnitude).Value());
        return negative ? -result : result;
    }

    [[nodiscard]]
    auto ToString(UInt32 radix = 10, bool uppercase = false) const -> String
    {
        const auto digits = this->UnsignedAbsolute().ToString(radix, uppercase);
        return this->Negative() ? '-' + digits : digits;
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return static_cast<bool>(this->n_bits);
    }

    constexpr VIOLET_EXPLICIT operator UInt128() const noexcept
    {
        return this->n_bits;
    }

    template<std::integral N>
    constexpr VIOLET_EXPLICIT operator N() const noexcept
    {
        return static_cast<N>(this->n_bits.LowBits());
    }

    template<std::floating_point F>
    constexpr VIOLET_EXPLICIT operator F() const noexcept
    {
        return this->Negative() ? -static_cast<F>(this->UnsignedAbsolute()) : static_cast<F>(this->n_bits);
    }

    friend constexpr auto operator==(const Int128&, const Int128&) noexcept -> bool = default;
    friend constexpr auto operator<=>(Int128 lhs, Int128 rhs) noexcept -> std::strong_ordering
    {
        if (auto cmp = lhs.HighBits() <=> rhs.HighBits(); cmp != 0) {
            return cmp;
        }

        return lhs.LowBits() <=> rhs.LowBits();
    }

    constexpr auto operator+() const noexcept -> Int128
    {
        return *this;
    }

    constexpr auto operator-() const noexcept -> Int128
    {
        return Int128(-this->n_bits);
    }

    constexpr auto operator~() const noexcept -> Int128
    {
        return Int128(~this->n_bits);
    }

    friend constexpr auto operator+(Int128 lhs, Int128 rhs) noexcept -> Int128
    {
        return Int128(lhs.n_bits + rhs.n_bits);
    }

    friend constexpr auto operator-(Int128 lhs, Int128 rhs) noexcept -> Int128
    {
        return Int128(lhs.n_bits - rhs.n_bits);
    }

    friend constexpr auto operator*(Int128 lhs, Int128 rhs) noexcept -> Int128
    {
        return Int128(lhs.n_bits * rhs.n_bits);
    }

    friend constexpr auto operator/(Int128 lhs, Int128 rhs) noexcept -> Int128
    {
        return DivRem(lhs, rhs).First;
    }

    friend constexpr auto operator%(Int128 lhs, Int128 rhs) noexcept -> Int128
    {
        return DivRem(lhs, rhs).Second;
    }

    friend constexpr auto operator&(Int128 lhs, Int128 rhs) noexcept -> Int128
    {
        return Int128(lhs.n_bits & rhs.n_bits);
    }

    friend constexpr auto operator|(Int128 lhs, Int128 rhs) noexcept -> Int128
    {
        return Int128(lhs.n_bits | rhs.n_bits);
    }

    friend constexpr auto operator^(Int128 lhs, Int128 rhs) noexcept -> Int128
    {
        return Int128(lhs.n_bits ^ rhs.n_bits);
    }

    template<std::integral N>
    friend constexpr auto operator<<(Int128 value, N amount) noexcept -> Int128
    {
        return Int128(value.n_bits << amount);
    }

    template<std::integral N>
    friend constexpr auto operator>>(Int128 value, N amount) noexcept -> Int128
    {
        auto shift = static_cast<UInt32>(amount);
        VIOLET_DEBUG_ASSERT(shift < 128, "shift amount must be less than 128");

        const auto hi = value.HighBits();
        const auto lo = value.LowBits();
        if (shift == 0) {
            return value;
        }

        if (shift >= 64) {
            return FromParts(static_cast<UInt64>(hi >> (shift - 64)), hi >> 63);
        }

        return FromParts((lo >> shift) | (static_cast<UInt64>(hi) << (64 - shift)), hi >> shift);
    }

    constexpr auto operator+=(Int128 rhs) noexcept -> Int128&
    {
        return *this = *this + rhs;
    }

    constexpr auto operator-=(Int128 rhs) noexcept -> Int128&
    {
        return *this = *this - rhs;
    }

    constexpr auto operator*=(Int128 rhs) noexcept -> Int128&
    {
        return *this = *this * rhs;
    }

    constexpr auto operator/=(Int128 rhs) noexcept -> Int128&
    {
        return *this = *this / rhs;
    }

    constexpr auto operator%=(Int128 rhs) noexcept -> Int128&
    {
        return *this = *this % rhs;
    }

    constexpr auto operator&=(Int128 rhs) noexcept -> Int128&
    {
        return *this = *this & rhs;
    }

    constexpr auto operator|=(Int128 rhs) noexcept -> Int128&
    {
        return *this = *this | rhs;
    }

    constexpr auto operator^=(Int128 rhs) noexcept -> Int128&
    {
        return *this = *this ^ rhs;
    }

    template<std::integral S>
    constexpr auto operator<<=(S amount) noexcept -> Int128&
    {
        return *this = *this << amount;
    }

    template<std::integral S>
    constexpr auto operator>>=(S amount) noexcept -> Int128&
    {
        return *this = *this >> amount;
    }

    constexpr auto operator++() noexcept -> Int128&
    {
        return *this += 1;
    }

    constexpr auto operator--() noexcept -> Int128&
    {
        return *this -= 1;
    }

    constexpr auto operator++(int) noexcept -> Int128
    {
        Int128 copy = *this;
        ++*this;

        return copy;
    }

    constexpr auto operator--(int) noexcept -> Int128
    {
        Int128 copy = *this;
        --*this;

        return copy;
    }

private:
    friend struct UInt128;

    UInt128 n_bits;
};

// NOLINTEND(cppcoreguidelines-prefer-member-initializer)

static_assert(sizeof(UInt128) == 16 && std::is_trivially_copyable_v<UInt128>);
static_assert(sizeof(Int128) == 16 && std::is_trivially_copyable_v<Int128>);

inline auto operator<<(std::ostream& os, UInt128 value) -> std::ostream&
{
    const auto flags = os.flags();
    auto base = flags & std::ios_base::basefield;
    const UInt32 radix = base == std::ios_base::hex
        ? 16
        : (base == std::ios_base::oct ? 8 : 10); // NOLINT(readability-avoid-nested-conditional-operator)

    const bool uppercase = (flags & std::ios_base::uppercase) != 0;
    auto text = value.ToString(radix, uppercase);

    if ((flags & std::ios_base::showbase) != 0 && value != 0 && radix != 10) {
        text.insert(
            0, radix == 8 ? "0" : (uppercase ? "0X" : "0x")); // NOLINT(readability-avoid-nested-conditional-operator)
    }

    return os << text;
}

inline auto operator<<(std::ostream& os, Int128 value) -> std::ostream&
{
    // Like the builtins, non-decimal bases print the two's complement bit pattern.
    if ((os.flags() & std::ios_base::basefield) == std::ios_base::hex
        || (os.flags() & std::ios_base::basefield) == std::ios_base::oct) {
        return os << static_cast<UInt128>(value);
    }

    if ((os.flags() & std::ios_base::showpos) != 0 && !value.Negative()) {
        os << '+';
    }

    return os << value.ToString();
}

template<typename T>
    requires(std::same_as<T, UInt128> || std::same_as<T, Int128>)
inline auto operator>>(std::istream& is, T& value) -> std::istream&
{
    String token;
    if (!(is >> token)) {
        return is;
    }

    auto base = is.flags() & std::ios_base::basefield;
    UInt32 radix = base == std::ios_base::hex
        ? 16
        : (base == std::ios_base::oct ? 8 : 10); // NOLINT(readability-avoid-nested-conditional-operator)

    if (auto parsed = T::Parse(token, radix); parsed.HasValue()) {
        value = VIOLET_MOVE(parsed).Value();
    } else {
        is.setstate(std::ios_base::failbit);
    }

    return is;
}

namespace int128_internal {

struct Error final {
    enum struct Kind : UInt8 {
        None,
        InvalidDigit,
        OutOfRange
    } Kind = Kind::None;

    char Offending = '\0';
    UInt Pos = 0;

#if defined(__cpp_static_assert) && __cpp_static_assert >= 202306L
    [[nodiscard]]
    constexpr auto ToString() const -> SmolString<96>
    {
        SmolString<96> msg;
        if (this->Kind == Kind::InvalidDigit) {
            msg.Append("invalid digit '");
            msg.Push(this->Offending);
            msg.Append("' in 128-bit integer literal");
        } else {
            msg.Append("integer literal is too large");
        }

        return msg;
    }
#endif
};

template<char... Characters>
constexpr auto TryParseLiteral() -> Result<UInt128, Error>
{
    constexpr Array<char, sizeof...(Characters)> raw{Characters...};
    Str text(raw.data(), raw.size());

    UInt32 radix = 10;
    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        radix = 16;
        text.remove_prefix(2);
    } else if (text.size() > 2 && text[0] == '0' && (text[1] == 'b' || text[1] == 'B')) {
        radix = 2;
        text.remove_prefix(2);
    } else if (text.size() > 1 && text[0] == '0') {
        radix = 8;
        text.remove_prefix(1);
    }

    UInt128 result = 0;
    for (UInt i = 0; i < text.size(); i++) {
        char ch = text.at(i);
        if (ch == '\'') {
            continue;
        }

        const UInt32 digit = DigitValueOf(ch);
        if (digit >= radix) {
            return Err(Error{
                .Kind = Error::Kind::InvalidDigit,
                .Offending = ch,
                .Pos = i,
            });
        }

        auto mul = UInt128{result}.CheckedMul(radix);
        if (!mul.HasValue()) {
            return Err(Error{
                .Kind = Error::Kind::OutOfRange,
                .Offending = ch,
                .Pos = i,
            });
        }

        auto add = UInt128{*mul}.CheckedAdd(digit);
        if (!add.HasValue()) {
            return Err(Error{
                .Kind = Error::Kind::OutOfRange,
                .Offending = ch,
                .Pos = i,
            });
        }

        result = VIOLET_MOVE(add).Value();
    }

    return result;
}

#if defined(__cpp_static_assert) && __cpp_static_assert >= 202306L

template<UInt Size, typename R>
constexpr auto asLiteralMessage(const R& result) -> SmolString<Size>
{
    return result.Ok() ? SmolString<Size>{} : result.Error().ToString();
}

#define __violet_int128_literal_check__(result)                                                                        \
    static_assert((result).Ok(), ::violet::experimental::int128_internal::asLiteralMessage<96>((result)))
#else
#define __violet_int128_literal_check__(result)                                                                        \
    static_assert(                                                                                                     \
        (result).Ok() || (result).Error().Kind != ::violet::experimental::int128_internal::Error::Kind::InvalidDigit,  \
        "invalid digit in 128-bit integer limit");                                                                     \
                                                                                                                       \
    static_assert(                                                                                                     \
        (result).Ok() || (result).Error().Kind != ::violet::experimental::int128_internal::Error::Kind::OutOfRange,    \
        "integer literal is too large for a 128-bit integer");
#endif
} // namespace int128_internal

inline namespace int128_literals {

template<char... Characters>
consteval auto operator""_u128() -> UInt128
{
    constexpr auto result = int128_internal::TryParseLiteral<Characters...>();
    __violet_int128_literal_check__(result);

    return VIOLET_MOVE(result).Value();
}

template<char... Characters>
consteval auto operator""_i128() -> Int128
{
    constexpr auto result = int128_internal::TryParseLiteral<Characters...>();
    __violet_int128_literal_check__(result);
    static_assert(!result.Ok() || result.Value() <= static_cast<UInt128>(Int128::Max()),
        "integer literal is too large for Int128");

    return Int128{VIOLET_MOVE(result).Value()};
}

} // namespace int128_literals

#undef __violet_int128_literal_check__

namespace int128_internal {

/// The format spec for [`std::format`] to be usable with [`UInt128`]/[`Int128`].
struct FormatSpec final {
    char Fill = ' ';
    char Align = '\0';
    char Sign = '-';
    bool Alternate = false;
    bool ZeroPad = false;
    UInt Width = 0;
    char Type = 'd';

    constexpr auto Parse(std::format_parse_context& ctx) -> std::format_parse_context::iterator
    {
        const auto* it = ctx.begin();
        const auto* end = ctx.end();
        auto isAlignmentChar = [](char ch) -> bool { return ch == '<' || ch == '>' || ch == '^'; };

        if (it != end && std::next(it) != end && isAlignmentChar(*std::next(it))) {
            this->Fill = *it++;
            this->Align = *it++;
        } else if (it != end && isAlignmentChar(*it)) {
            this->Align = *it++;
        }

        if (it != end && (*it == '+' || *it == '-' || *it == ' ')) {
            this->Sign = *it++;
        }

        if (it != end && *it == '#') {
            this->Alternate = true;
            ++it;
        }

        if (it != end && *it == '0') {
            this->ZeroPad = true;
            ++it;
        }

        while (it != end && *it >= '0' && *it <= '9') {
            this->Width = (Width * 10) + static_cast<UInt>(*it++ - '0');
        }

        if (it != end && *it != '}') {
            this->Type = *it++;
            if (Str("dbBoxX").find(Type) == Str::npos) {
                VIOLET_THROW(std::format_error, "invalid presentation type for a 128-bit integer");
            }
        }

        if (it != end && *it != '}') {
            VIOLET_THROW(std::format_error, "invalid format spec for a 128-bit integer");
        }

        return it;
    }

    [[nodiscard]] constexpr auto Radix() const noexcept -> UInt32
    {
        switch (Type) {
        case 'b':
        case 'B':
            return 2;
        case 'o':
            return 8;
        case 'x':
        case 'X':
            return 16;
        default:
            return 10;
        }
    }

    template<typename Out>
    auto Write(Out out, bool negative, const String& digits) const -> Out
    {
        String prefix;
        if (negative) {
            prefix += '-';
        } else if (this->Sign != '-') {
            prefix += Sign;
        }

        if (this->Alternate) {
            switch (this->Type) {
            case 'b':
                prefix += "0b";
                break;

            case 'B':
                prefix += "0B";
                break;

            case 'o':
                prefix += '0';
                break;

            case 'x':
                prefix += "0x";
                break;

            case 'X':
                prefix += "0X";
                break;

            default:
                break;
            }
        }

        UInt length = prefix.size() + digits.size();
        UInt padding = this->Width > length ? this->Width - length : 0;

        if (ZeroPad && Align == 0) {
            out = std::copy(prefix.begin(), prefix.end(), out);
            out = std::fill_n(out, padding, '0');
            return std::copy(digits.begin(), digits.end(), out);
        }

        UInt before = this->Align == '<' ? 0
            : this->Align == '^'         ? padding / 2 // NOLINT(readability-avoid-nested-conditional-operator)
                                         : padding;

        out = std::fill_n(out, before, this->Fill);
        out = std::copy(prefix.begin(), prefix.end(), out);
        out = std::copy(digits.begin(), digits.end(), out);
        return std::fill_n(out, padding - before, this->Fill);
    }
};

struct numeric_limits_base {
    static constexpr bool is_specialized = true;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true; // both types wrap on overflow
    static constexpr bool tinyness_before = false;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr std::float_round_style round_style = std::round_toward_zero;
};

} // namespace int128_internal
} // namespace violet::experimental

template<>
struct std::formatter<violet::experimental::UInt128> {
    violet::experimental::int128_internal::FormatSpec Spec;

    constexpr auto parse(std::format_parse_context& cx)
    {
        return this->Spec.Parse(cx);
    }

    template<typename FC>
    auto format(violet::experimental::UInt128 value, FC& cx) const
    {
        return this->Spec.Write(cx.out(), false, value.ToString(this->Spec.Radix(), this->Spec.Type == 'X'));
    }
};

template<>
struct std::formatter<violet::experimental::Int128> {
    violet::experimental::int128_internal::FormatSpec Spec;

    constexpr auto parse(std::format_parse_context& cx)
    {
        return this->Spec.Parse(cx);
    }

    template<typename FC>
    auto format(violet::experimental::Int128 value, FC& cx) const
    {
        return this->Spec.Write(
            cx.out(), value.Negative(), value.UnsignedAbsolute().ToString(this->Spec.Radix(), this->Spec.Type == 'X'));
    }
};

template<>
struct std::hash<violet::experimental::UInt128> {
    auto operator()(violet::experimental::UInt128 value) const noexcept -> violet::UInt
    {
        constexpr std::hash<violet::UInt64> hasher{};

        auto seed = hasher(value.LowBits());
        return seed ^ (hasher(value.HighBits()) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }
};

template<>
struct std::hash<violet::experimental::Int128> {
    auto operator()(violet::experimental::Int128 value) const noexcept -> violet::UInt
    {
        constexpr std::hash<violet::experimental::UInt128> hasher{};
        return hasher(static_cast<violet::experimental::UInt128>(value));
    }
};

template<>
class std::numeric_limits<violet::experimental::UInt128>
    : public violet::experimental::int128_internal::numeric_limits_base {
    using T = violet::experimental::UInt128;

public:
    static constexpr bool is_signed = false;
    static constexpr violet::Int32 digits = 128;

#if VIOLET_FEATURE(NATIVE_INT128)
    static constexpr bool traps = std::numeric_limits<unsigned __int128>::traps;
#else
    static constexpr bool traps = std::numeric_limits<violet::UInt64>::traps;
#endif

    static constexpr auto min() noexcept -> T
    {
        return T::Min();
    }

    static constexpr auto lowest() noexcept -> T
    {
        return T::Min();
    }

    static constexpr auto max() noexcept -> T
    {
        return T::Max();
    }

    static constexpr auto epsilon() noexcept -> T
    {
        return 0;
    }

    static constexpr auto round_error() noexcept -> T
    {
        return 0;
    }

    static constexpr auto infinity() noexcept -> T
    {
        return 0;
    }

    static constexpr auto quiet_NaN() noexcept -> T
    {
        return 0;
    }

    static constexpr auto signaling_NaN() noexcept -> T
    {
        return 0;
    }

    static constexpr auto denorm_min() noexcept -> T
    {
        return 0;
    }
};

template<>
class std::numeric_limits<violet::experimental::Int128>
    : public violet::experimental::int128_internal::numeric_limits_base {
    using T = violet::experimental::Int128;

public:
    static constexpr bool is_signed = true;
    static constexpr violet::Int32 digits = 127;
#if VIOLET_FEATURE(NATIVE_INT128)
    static constexpr bool traps = std::numeric_limits<__int128>::traps;
#else
    static constexpr bool traps = std::numeric_limits<violet::UInt64>::traps;
#endif

    static constexpr auto min() noexcept -> T
    {
        return T::Min();
    }

    static constexpr auto lowest() noexcept -> T
    {
        return T::Min();
    }

    static constexpr auto max() noexcept -> T
    {
        return T::Max();
    }

    static constexpr auto epsilon() noexcept -> T
    {
        return 0;
    }

    static constexpr auto round_error() noexcept -> T
    {
        return 0;
    }

    static constexpr auto infinity() noexcept -> T
    {
        return 0;
    }

    static constexpr auto quiet_NaN() noexcept -> T
    {
        return 0;
    }

    static constexpr auto signaling_NaN() noexcept -> T
    {
        return 0;
    }

    static constexpr auto denorm_min() noexcept -> T
    {
        return 0;
    }
};

template<std::integral T>
struct std::common_type<violet::experimental::UInt128, T> {
    using type = violet::experimental::UInt128;
};

template<std::integral T>
struct std::common_type<T, violet::experimental::UInt128> {
    using type = violet::experimental::UInt128;
};

template<std::integral T>
struct std::common_type<violet::experimental::Int128, T> {
    using type = violet::experimental::Int128;
};

template<std::integral T>
struct std::common_type<T, violet::experimental::Int128> {
    using type = violet::experimental::Int128;
};

template<>
struct std::common_type<violet::experimental::Int128, violet::experimental::UInt128> {
    using type = violet::experimental::UInt128;
};

template<>
struct std::common_type<violet::experimental::UInt128, violet::experimental::Int128> {
    using type = violet::experimental::UInt128;
};
