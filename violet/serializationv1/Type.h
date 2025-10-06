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

#include "violet/violet.h"

namespace Noelware::Violet::Serialization {

/// Representation of a type that is expected of a descriptor's field.
enum struct Type : uint8 {
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    String,
    Array,
    Struct,
};

} // namespace Noelware::Violet::Serialization

namespace Noelware::Violet {

inline auto ToString(const Serialization::Type& ty) -> String
{
    switch (ty) {
    case Serialization::Type::Bool:
        return "boolean";

    case Serialization::Type::Int8:
        return "8-bit integer";

    case Serialization::Type::Int16:
        return "16-bit integer";

    case Serialization::Type::Int32:
        return "32-bit integer";

    case Serialization::Type::Int64:
        return "64-bit integer";

    case Serialization::Type::UInt8:
        return "unsigned 8-bit integer";

    case Serialization::Type::UInt16:
        return "unsigned 16-bit integer";

    case Serialization::Type::UInt32:
        return "unsigned 32-bit integer";

    case Serialization::Type::UInt64:
        return "unsigned 64-bit integer";

    case Serialization::Type::String:
        return "string";

    case Serialization::Type::Array:
        return "an array";

    case Serialization::Type::Struct:
        return "structure";
    }
}

} // namespace Noelware::Violet
