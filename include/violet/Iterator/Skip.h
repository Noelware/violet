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

/// An iterator adapter that skips the first `n` elements of the underlying
/// iterator, then yields all remaining elements.
///
/// The skipped elements are consumed eagerly on the first call to [`Next`].
/// Once the skip count is exhausted, subsequent calls delegate directly to
/// the underlying iterator with no additional overhead.
///
/// ## Example
/// ```cpp
/// #include <violet/Iterator/Skip.h>
/// #include <violet/Print.h>
///
/// auto items = violet::MkIterable(violet::Vec<violet::Int32>{1, 2, 3, 4, 5}).Skip(2);
/// for (auto value: items) {
///     violet::Println("{}", value);
/// }
///
/// // => 3
/// // => 4
/// // => 5
/// ```
///
/// ## Size Hint
/// Both bounds are reduced by the skip count (clamped to zero). If the
/// underlying iterator does not provide a size hint, the default hint with
/// an unknown upper bound is returned.
///
/// @tparam Impl underlying iterator type.
template<Iterable Impl>
struct VIOLET_API Skip final: public Iterator<Skip<Impl>> {
    using Item = TypeOf<Impl>;
    using underlying_iterator = Impl;

    VIOLET_DISALLOW_CONSTRUCTOR(Skip);
    ~Skip() = default;

    /// Advances the iterator, skipping elements until the skip count is
    /// exhausted, then returns the next element from the underlying iterator.
    ///
    /// On the first call, up to `n` elements are consumed and discarded from
    /// the underlying iterator. If the underlying iterator is exhausted before
    /// the skip count is reached, `Nothing` is returned. Once all elements
    /// have been skipped, this method delegates directly to the underlying
    /// iterator's `Next`.
    auto Next() noexcept -> Optional<Item>
    {
        while (this->n_skip > 0) {
            if (!this->n_iter.Next()) {
                return Nothing;
            }

            this->n_skip--;
        }

        return this->n_iter.Next();
    }

    /// Returns a hint of the remaining length of the iterator.
    ///
    /// Both the lower and upper bounds of the underlying iterator's hint are
    /// reduced by the skip count, clamped to zero. If the underlying iterator
    /// does not provide a size hint, a default hint with unknown upper bound
    /// is returned.
    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        if constexpr (requires { this->n_iter.SizeHint(); }) {
            violet::SizeHint hint = this->n_iter.SizeHint();

            auto lo = hint.Low > this->n_skip ? (hint.Low - this->n_skip) : 0;
            Optional<UInt> hi;

            if (hint.High) {
                hi = *hint.High > this->n_skip ? (*hint.High - this->n_skip) : 0;
            }

            return { lo, hi };
        }

        return { };
    }

private:
    friend struct Iterator<Impl>;

    VIOLET_IMPLICIT Skip(Impl iter, UInt take)
        : n_iter(iter)
        , n_skip(take)
    {
    }

    Impl n_iter;
    UInt n_skip;
};

} // namespace violet::iter

namespace violet {

template<typename Impl>
inline auto Iterator<Impl>::Skip(UInt take) & noexcept -> decltype(auto)
{
    return iter::Skip(getThisObject(), take);
}

template<typename Impl>
inline auto Iterator<Impl>::Skip(UInt take) && noexcept -> decltype(auto)
{
    return iter::Skip(VIOLET_MOVE(getThisObject()), take);
}

} // namespace violet
