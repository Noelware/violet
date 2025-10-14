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

#include <type_traits>

namespace Noelware::Violet::Memory {

/// Replaces `value` with a default value and returning the old value.
///
/// This is an analogous port of Rust's [`std::mem::take`] in C++. It moves out the contents
/// of `value` with its default constructor and returns the old value.
///
/// [`std::mem::take`]: https://doc.rust-lang.org/1.90.0/std/mem/fn.take.html
///
/// ## Remarks
/// * This will not allocate or copy beyond what `T`'s move and default constructors perform.
/// * If `T`'s default constructor throws, `Take` will also.
/// * Works at compile-time if both `T()` and `T(T&&)` are `constexpr`.
///
/// ## Example
/// ```cpp
/// #include "violet/memory/Memory.h"
///
/// usize thirtytwo = 32;
/// usize old = Noelware::Violet::Memory::Take(thirtytwo);
///
/// ASSERT_EQ(thirtytwo, 0);
/// ASSERT_EQ(old, 32);
/// ```
///
/// @tparam T Type that is both move and default constructible.
/// @param value A reference to the value to take from.
/// @returns the original value before replacing `value`.
template<typename T>
constexpr auto Take(T& value) noexcept(
    std::is_nothrow_move_constructible_v<T> && std::is_nothrow_default_constructible_v<T>) -> T
{
    T old = VIOLET_MOVE(value);
    value = T();

    return old;
}

} // namespace Noelware::Violet::Memory
