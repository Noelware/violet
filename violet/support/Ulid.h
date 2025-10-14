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

#include "violet/io/Error.h"
#include "violet/support/StringRef.h"
#include "violet/sys/Randomness.h"
#include "violet/violet.h"

#include <chrono>

namespace Noelware::Violet {

struct DecodeUlidError final {
    DecodeUlidError() = delete;

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        switch (this->n_tag) {
        case Tag::kInvalidLength:
            return "invalid length";

        case Tag::kInvalidChar:
            return "invalid character";
        }
    }

private:
    friend struct Ulid;
    enum struct Tag : uint8 {
        /// Invalid length.
        kInvalidLength,

        /// Invalid character.
        kInvalidChar,
    };

    VIOLET_EXPLICIT DecodeUlidError(Tag tag)
        : n_tag(tag) {};

    Tag n_tag;
};

struct Ulid final {
    constexpr static const uint8 Length = 26;

    constexpr Ulid() = default;

    constexpr VIOLET_IMPLICIT Ulid(uint128 value)
        : n_value(value)
    {
    }

    static auto New() -> IO::Result<Ulid>
    {
        uint64 now = static_cast<uint64>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());

        return Ulid::FromTimestamp(now);
    }

    static auto FromTimestamp(uint64 timestamp) -> IO::Result<Ulid>
    {
        Array<uint8, 16> buf;

        auto result = System::RandomBytes(buf.data(), buf.size());
        if (!result) {
            return Err(result.Error());
        }

        uint128 rnd = 0;
        for (unsigned char idx: buf) {
            rnd = (rnd << 8) | static_cast<uint128>(idx);
        }

        return Ulid::FromParts(timestamp, rnd & getRandomnessMask());
    }

    static auto FromParts(uint64 ts, uint128 rnd) -> Ulid
    {
        return (static_cast<uint128>(ts) << kRandomnessBits) | (rnd & getRandomnessMask());
    }

    static auto FromStr(StringRef) -> Result<Ulid, DecodeUlidError>;

    [[nodiscard]] auto ToString() const noexcept -> String;

    [[nodiscard]] constexpr auto AsUInt128() const noexcept -> uint128
    {
        return this->n_value;
    }

    [[nodiscard]] constexpr auto Timestamp() const noexcept -> uint64
    {
        return static_cast<uint64>(this->n_value >> kRandomnessBits);
    }

    [[nodiscard]] constexpr auto Randomness() const noexcept -> uint128
    {
        return this->n_value & kRandomnessBits;
    }

    constexpr VIOLET_EXPLICIT operator uint128() const noexcept
    {
        return this->n_value;
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_value != 0;
    }

    VIOLET_IMPL_EQUALITY_SINGLE(Ulid, lhs, rhs, { return lhs.n_value == rhs.n_value; });
    VIOLET_IMPL_EQUALITY(const Ulid&, uint128, lhs, rhs, { return lhs.n_value == rhs; });

private:
    uint128 n_value;

    constexpr static const uint8 kTimestampBits = 40;
    constexpr static const uint8 kRandomnessBits = 80;

    constexpr static auto getTimestampMask() -> uint64
    {
        return (static_cast<uint64>(1) << kTimestampBits) - 1;
    }

    constexpr static auto getRandomnessMask() -> uint128
    {
        return ((uint128(1) << kRandomnessBits) - 1);
    }
};

} // namespace Noelware::Violet
