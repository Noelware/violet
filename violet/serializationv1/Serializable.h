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
//! # 🌺💜 `violet/serialization/Serializable.h`

#pragma once

#include "violet/container/Optional.h"
#include "violet/container/Result.h"
#include "violet/serialization/Error.h"
#include "violet/serialization/Serializer.h"

#include <optional>

namespace Noelware::Violet::Serialization {

inline auto Serialize(const int8& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeInt8(value);
}

inline auto Serialize(const int16& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeInt16(value);
}

inline auto Serialize(const int32& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeInt32(value);
}

inline auto Serialize(const int64& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeInt64(value);
}

inline auto Serialize(const uint8& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeUInt8(value);
}

inline auto Serialize(const uint16& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeUInt16(value);
}

inline auto Serialize(const uint32& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeUInt32(value);
}

inline auto Serialize(const uint64& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeUInt64(value);
}

inline auto Serialize(CStr value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeStr(value);
}

inline auto Serialize(const String& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeStr(value);
}

inline auto Serialize(const Str& value, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeStr(value);
}

inline auto Serialize(const std::nullopt_t&, Serializer& ser) -> Result<void, Error>
{
    return ser.SerializeNothing();
}

template<typename T>
inline auto Serialize(const Optional<T>& value, Serializer& ser) -> Result<void, Error>
{
    return value ? Serialize(value.Value(), ser) : Serialize(Nothing, ser);
}

template<typename Struct, typename FieldType>
auto StructField<Struct, FieldType>::Serialize(const void* object, Serializer& ser) const -> Result<void, Error>
{
    auto& typed = static_cast<const Struct*>(object);
    return Noelware::Violet::Serialization::Serialize(*typed.*n_member, ser);
}

template<typename T>
concept HasSerializeInstanceMember = requires(T ty, Serializer& serializer) {
    { ty.Serialize(serializer) } -> std::same_as<Result<void, Error>>;
};

template<typename T>
concept HasFreeMemberSerialize = requires(Serializer& serializer, const T& value) {
    { Noelware::Violet::Serialization::Serialize(value, serializer) } -> std::same_as<Result<void, Error>>;
};

template<typename T>
concept Serializable = HasSerializeInstanceMember<T> || HasFreeMemberSerialize<T>;

template<typename T>
inline auto Serialize(const T& value, Serializer& serializer) -> Result<void, Error>
{
    if constexpr (HasSerializeInstanceMember<T>) {
        value.Serialize(serializer);
    } else if constexpr (HasFreeMemberSerialize<T>) {
        Serialize(value, serializer);
    } else {
        static_assert(sizeof(T) == 0, "Type of `T` does not conform to `Serializable` concept");
    }
}

} // namespace Noelware::Violet::Serialization
