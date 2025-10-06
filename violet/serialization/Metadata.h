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

#include "violet/container/Optional.h"
#include "violet/violet.h"

namespace Noelware::Violet::Serialization {

/// Used for specialization per-type.
template<typename T>
struct Meta;

/// All the avaliable options for customizing the serialization and/or deserialization
/// of [`Field`]s.
template<typename FieldType>
struct FieldOptions {
    Optional<FieldType> Default = Nothing;
    bool Skip = false;
};

template<typename Struct, typename FieldType>
struct Field {
    explicit Field(String field, FieldType Struct::* member, const FieldOptions<FieldType>& options = {})
        : n_field(VIOLET_MOVE(field))
        , n_member(member)
        , n_options(options)
    {
    }

    [[nodiscard]]
    constexpr auto Name() const noexcept -> String
    {
        return this->n_field;
    }

    [[nodiscard]]
    constexpr auto DefaultValue() const noexcept -> Optional<FieldType>
    {
        return this->n_options.Default;
    }

    [[nodiscard]] constexpr auto ShouldSkip() const noexcept -> bool
    {
        return this->n_options.Skip;
    }

private:
    String n_field;
    FieldType Struct::* n_member;
    FieldOptions<FieldType> n_options = {};
};

} // namespace Noelware::Violet::Serialization

#define MARK_SERIALIZABLE(TYPE, NAME, ...)                                                                             \
    namespace Noelware::Violet::Serialization {                                                                        \
        template<>                                                                                                     \
        struct Meta<TYPE> {                                                                                            \
            static inline const auto fields = ::std::make_tuple(__VA_ARGS__);                                          \
            static constexpr ::Noelware::Violet::CStr name = NAME;                                                     \
        };                                                                                                             \
    }
