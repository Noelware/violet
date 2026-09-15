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

#pragma once

#include <violet/Language/Macros.h>
#include <violet/Language/Policy.h>

#include <type_traits>

namespace violet::experimental {

/// A stateless deleter that does nothing when invoked.
///
/// Pass this as the deleter to [`Own`] or [`Unique`]'s raw-pointer constructor when the handle
/// should participate in shared ownership without ever freeing the pointee. For
/// example when the managed object has automatic or static storage duration, or
/// is owned elsewhere:
///
/// ```cpp
/// Int32 y = 32;
/// Own<Int32> ref(&y, NoOpDeleter());   // shares `&y`, never deletes it
/// Unique<Int32, NoOpDeleter> ref2(&y); // unique pointer
/// ```
struct NOELDOC_EXPERIMENTAL_SINCE("26.07.03") NoOpDeleter final {
    /// Constructs a [`NoOpDeleter`]. Stateless, so this is trivial.
    constexpr VIOLET_IMPLICIT NoOpDeleter() = default;

    /// Does nothing. The pointee is intentionally left untouched.
    template<typename T>
    constexpr void operator()(T*) const noexcept
    {
        static_assert(!std::is_function_v<T>, "`NoOpDeleter` cannot be instantiated for function types");
        static_assert(sizeof(T) >= 0 && !std::is_void_v<T>, "cannot delete an incomplete type");
    }
};

} // namespace violet::experimental
