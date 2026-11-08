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
//! # 🌺💜 `violet/Violet.h`

#pragma once

#include <violet/Language/Macros.h>

#include <type_traits>

namespace violet {

/// Explicitly destroys `value`, ending its lifetime immediately, analogous to Rust's [`std::mem::drop`].
///
/// The argument is taken **by value**, so the caller's object is moved (or copied, if it is not movable)
/// into the parameter, and that parameter is destroyed when `Drop` returns, running `T`'s destructor. This
/// is useful for releasing a resource before the end of the enclosing scope, e.g. closing file handles held
/// by an iterator before performing follow-up work that depends on them being closed:
///
/// ## Example
/// ```cpp
/// auto iter = VIOLET_TRY(filesystem::WalkDir(path));
/// for (auto entry: iter) {
///     // ...
/// }
///
/// // release the iterator's open directory handles before we mutate the tree below.
/// Drop(VIOLET_MOVE(iter));
/// ```
///
/// ## Remarks
/// To actually end a value's lifetime the argument must be moved in (or be a copyable temporary); passing an
/// lvalue you still own merely drops a copy. Calling `Drop` on a trivially destructible value (e.g. an `Int`)
/// has no observable effect.
///
/// @param value the value to take ownership of and destroy.
///
/// [`std::mem::drop`]: https://doc.rust-lang.org/1.95.0/std/mem/fn.drop.html
template<typename T>
NOELDOC_SINCE("26.07")
constexpr void Drop(T value) noexcept(std::is_nothrow_destructible_v<T>)
{
    static_cast<void>(value);
}

} // namespace violet
