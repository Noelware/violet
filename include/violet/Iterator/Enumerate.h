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

#include <violet/Container/Optional.h>
#include <violet/Iterator.h>
#include <violet/Violet.h>

namespace violet::iter {

/// Iterator adapter that yields the current index alongside each element from the
/// underlying iterator.
///
/// `Enumerate` wraps an [existing iterator][violet::Iterator] and produces pairs of `(index, element)`,
/// where the index starts at zero and increments by one for each successive call to [`Enumerate::Next`].
/// This is useful when positional information is needed during iteration without maintaining a separate
/// counter.
///
/// ## Example
/// ```cpp
/// #include <violet/Iterator/Enumerate.h>
/// #include <violet/Print.h>
///
/// auto items = violet::MkIterable(violet::Vec<violet::String>({"hello", "world"}));
/// for (auto [index, str]: items.Enumerate()) {
///     violet::Println("#{:02}.  {}", index, str);
/// }
/// ```
///
/// @tparam Impl underlying iterator type. Must satisfy the [`violet::Iterable`] concept.
template<Iterable Impl>
struct VIOLET_API Enumerate final: public Iterator<Enumerate<Impl>> {
    /// Yields `Pair<UInt, Iterator::Type>`, where the first element is the zero-based
    /// index and the second is the value produced by the inner iterator.
    using Item = Pair<UInt, TypeOf<Impl>>;
    using underlying_iterator = Impl;

    VIOLET_DISALLOW_CONSTRUCTOR(Enumerate);
    ~Enumerate() = default;

    /// Advances the iterator and returns the next index-element pair, or
    /// [`violet::Nothing`] if the underlying iterator is exhausted.
    ///
    /// Each call delegates to the inner iterator's [`Next`][violet::Iterator::Next()]. If it produces
    /// a value, that value is paired with the current index and the index is incremented. Once the inner
    /// iterator returns [`violet::Nothing`], all subsequent calls return [`violet::Nothing`] as well.
    auto Next() noexcept -> Optional<Item>
    {
        if (auto elem = this->n_iter.Next()) {
            return Some<Item>(Pair<UInt, TypeOf<Impl>>(this->n_index++, *elem));
        }

        return Nothing;
    }

private:
    friend struct Iterator<Impl>;

    VIOLET_IMPLICIT Enumerate(Impl iter)
        : n_iter(iter)
    {
    }

    Impl n_iter;
    UInt n_index = 0;
};

} // namespace violet::iter

namespace violet {

template<class Impl>
inline auto Iterator<Impl>::Enumerate() & noexcept -> decltype(auto)
{
    return iter::Enumerate<Impl>(getThisObject());
}

template<class Impl>
inline auto Iterator<Impl>::Enumerate() && noexcept -> decltype(auto)
{
    return iter::Enumerate<Impl>(VIOLET_MOVE(getThisObject()));
}

} // namespace violet
