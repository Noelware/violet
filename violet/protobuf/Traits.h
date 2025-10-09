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

namespace Noelware::Violet::Protobuf {

/// C++20 concept to require the constraint that `T::FromProtobuf(const Proto&)` exists and
/// returns type `T`.
///
/// ## Example
/// ```cpp
/// #include "violet/protobuf/Traits.h"
/// #include "violet/violet.h"
/// #include "helloworld.pb.h"
///
/// using namespace Noelware::Violet;
///
/// struct User final {
///
/// };
/// ```
template<typename T, typename Proto>
concept From = requires(const Proto& proto) {
    { T::FromProtobuf(proto) } -> std::same_as<T>;
};

template<typename T, typename Proto>
concept Into = requires(T ty) {
    { ty.IntoProtobuf() } -> std::same_as<Proto>;
};

} // namespace Noelware::Violet::Protobuf

namespace Noelware::Violet {

template<typename T, typename Proto>
    requires Protobuf::From<T, Proto>
inline auto FromProtobuf(const Proto& proto) -> T
{
    return T::FromProtobuf(proto);
}

template<typename Proto, typename T>
    requires Protobuf::Into<T, Proto>
inline auto IntoProtobuf(const T& value) -> Proto
{
    return value.IntoProtobuf();
}

} // namespace Noelware::Violet
