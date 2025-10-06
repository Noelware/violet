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
//! # 🌺💜 `violet/serialization/Descriptor.h`
//! This header contains the main interface for building descriptors, a schema-like concept
//! for structures that the library uses.

#pragma once

#include "violet/container/Result.h"
#include "violet/serialization/Error.h"
#include "violet/serialization/Type.h"
#include "violet/violet.h"

namespace Noelware::Violet::Serialization {

struct Serializer;
struct Descriptor;

/// Concept to determine if `T` is describable.
template<typename T>
concept Describable = requires(T ty) {
    { T::Descriptor() } noexcept -> std::same_as<Descriptor>;
};

/// @internal
/// @hide
struct BaseField {
    virtual ~BaseField() = default;

    [[nodiscard]] virtual auto Name() const noexcept -> Str = 0;
    [[nodiscard]] virtual auto Repr() const noexcept -> Type = 0;
    [[nodiscard]] virtual auto IsOptional() const noexcept -> bool = 0;
    [[nodiscard]] virtual auto Serialize(const void* object, Serializer& serializer) const -> Result<void, Error> = 0;
    [[nodiscard]] virtual auto Of() const noexcept -> Optional<Descriptor> = 0;
};

template<typename Struct, typename FieldType>
struct StructField;

struct Descriptor final {
    Descriptor() = delete;

    /// Returns the name of this descriptor.
    [[nodiscard]] auto Name() const noexcept -> Str
    {
        return this->n_descriptor;
    }

    [[nodiscard]] auto Fields() const noexcept -> Span<const SharedPtr<BaseField>>
    {
        return this->n_fields;
    }

    struct Builder final {
        Builder() = delete;

        explicit Builder(Str name)
            : n_descriptor(VIOLET_MOVE(name))
        {
        }

        template<typename Struct, typename FieldType>
        auto Field(Str field, Type type, FieldType Struct::* member, bool optional = false) noexcept -> Builder&
        {
            auto field_ = std::make_shared<StructField<Struct, FieldType>>(StructField(field, type, member, optional));
            this->n_fields.push_back(field_);

            return *this;
        }

        auto Build() -> Descriptor
        {
            return Descriptor{ n_descriptor, n_fields };
        }

    private:
        Str n_descriptor;
        Vec<SharedPtr<BaseField>> n_fields;
    };

private:
    friend struct Builder;

    explicit Descriptor(Str descriptor, Vec<SharedPtr<BaseField>> fields)
        : n_descriptor(descriptor)
        , n_fields(VIOLET_MOVE(fields))
    {
    }

    Str n_descriptor;
    Vec<SharedPtr<BaseField>> n_fields;
};

/// Representation of a struct field that can be described.
template<typename Struct, typename FieldType>
struct StructField final: public BaseField {
    /// Constructs a new [`Field`] object.
    /// @param name The field's name
    /// @param type The expected type this field expects.
    /// @param member A pointer to the member that should be initialize when deserializing / the data when serializing
    /// @param optional whether this field is required
    explicit StructField(Str name, Type type, FieldType Struct::* member, bool optional = false,
        const Optional<Descriptor>& descriptor = {}) noexcept
        : n_field(name)
        , n_type(type)
        , n_member(member)
        , n_optional(optional)
        , n_descriptor(descriptor)
    {
    }

    [[nodiscard]] auto Name() const noexcept -> Str override
    {
        return this->n_field;
    }

    [[nodiscard]] auto Repr() const noexcept -> Type override
    {
        return this->n_type;
    }

    [[nodiscard]] auto IsOptional() const noexcept -> bool override
    {
        return this->n_optional;
    }

    [[nodiscard]] auto Of() const noexcept -> Optional<Descriptor> override
    {
        return this->n_descriptor;
    }

    [[nodiscard]] auto Serialize(const void* object, Serializer& serializer) const -> Result<void, Error> override;

private:
    Str n_field;
    Type n_type;
    FieldType Struct::* n_member;
    bool n_optional;
    Optional<Descriptor> n_descriptor = Nothing;
};

} // namespace Noelware::Violet::Serialization
