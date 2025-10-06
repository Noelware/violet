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
//
//! # 🌺💜 `violet/serialization/Serializer.h`
//! This header contains the main interface for serialization itself.

#pragma once

#include "violet/container/Result.h"
#include "violet/serialization/Descriptor.h"

namespace Noelware::Violet::Serialization {

struct Serializer;

template<class S>
struct SerializeMap {
    virtual ~SerializeMap() = default;

    virtual auto Key(const Str&) noexcept -> Result<void, Error> = 0;
};

template<class S>
struct SerializeSeq {};

struct Serializer {
    virtual ~Serializer() = default;

    virtual auto SerializeBool(bool) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeInt8(int8) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeInt16(int16) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeInt32(int32) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeInt64(int64) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeUInt8(uint8) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeUInt16(uint16) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeUInt32(uint32) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeUInt64(uint64) noexcept -> Result<void, Error> = 0;
    virtual auto SerializeStr(const Str&) noexcept -> Result<void, Error> = 0;

    virtual auto SerializeMap(Optional<usize>) noexcept -> SerializeMap<Serializer>;
    virtual auto SerializeSeq(Optional<usize>) noexcept -> SerializeSeq<Serializer>;

    virtual auto SerializeNothing() noexcept -> Result<void, Error> = 0;
    virtual auto SerializeDescriptor(const Descriptor&) noexcept -> Result<void, Error> = 0;

    auto SerializeMap() noexcept
    {
        return this->SerializeMap(Optional<usize>(0));
    }

    auto SerializeSeq() noexcept
    {
        return this->SerializeSeq(Nothing);
    }

    auto SerializeString(const String& str) noexcept -> Result<void, Error>
    {
        return this->SerializeStr(str);
    }

    auto SerializeCStr(CStr str) noexcept -> Result<void, Error>
    {
        return this->SerializeStr(str);
    }
};

} // namespace Noelware::Violet::Serialization
