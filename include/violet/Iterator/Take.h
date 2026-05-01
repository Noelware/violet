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

/// An iterator adapter that yields at most the first `n` elements from the
/// underlying iterator.
///
/// `Take` wraps an existing iterator and enforces a cap on the number of
/// elements produced. Once the limit is reached, all subsequent calls to
/// [`Next`] return `Nothing`, regardless of whether the underlying iterator
/// has remaining elements.
///
/// If the underlying iterator implements `DoubleEndedIterable`, `Take` also
/// supports reverse iteration via [`NextBack`], consuming from the back
/// against the same shared remaining count.
///
/// ## Example
/// ```cpp
/// #include <violet/Iterator/Take.h>
/// #include <violet/Print.h>
///
/// auto items = violet::MkIterable(violet::Vec<violet::Int32>{1, 2, 3, 4, 5}).Take(3);
/// for (auto value: items) {
///     violet::Println("{}", value);
/// }
///
/// // => 1
/// // => 2
/// // => 3
/// ```
///
/// ## Size Hint
/// Both bounds are capped at the remaining take count. If the underlying
/// iterator does not provide a size hint, the upper bound is set to the
/// remaining count.
///
/// @tparam Impl underlying iterator type
template<Iterable Impl>
struct VIOLET_API Take final: public Iterator<Take<Impl>> {
    using Item = TypeOf<Impl>;
    using underlying_iterator = Impl;

    VIOLET_DISALLOW_CONSTRUCTOR(Take);
    ~Take() = default;

    /// Returns the next element from the underlying iterator, or `Nothing`
    /// if the take limit has been reached or the underlying iterator is
    /// exhausted.
    ///
    /// Each call decrements the remaining count. Once the count reaches
    /// zero, all subsequent calls return `Nothing`.
    auto Next() noexcept -> Optional<Item>
    {
        if (this->n_remaining == 0) {
            return Nothing;
        }

        this->n_remaining--;
        return this->n_iter.Next();
    }

    /// Returns the next element from the back of the underlying iterator,
    /// or `Nothing` if the take limit has been reached or the underlying
    /// iterator is exhausted.
    ///
    /// This method is only available when the underlying iterator satisfies
    /// `DoubleEndedIterable`. Each call decrements the same shared remaining
    /// count used by [`Next`].
    auto NextBack() noexcept -> Optional<Item>
        requires DoubleEndedIterable<Impl>
    {
        if (this->n_remaining == 0) {
            return Nothing;
        }

        this->n_remaining--;
        return this->n_iter.NextBack();
    }

    /// Returns a hint of the remaining length of the iterator.
    ///
    /// Both bounds are capped at the remaining take count. If the underlying
    /// iterator does not provide a size hint, the lower bound is `0` and the
    /// upper bound is set to the remaining count.
    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        if constexpr (requires { this->n_iter.SizeHint(); }) {
            violet::SizeHint hint = this->n_iter.SizeHint();

            UInt lo = std::min(hint.Low, this->n_remaining);
            Optional<UInt> hi;

            if (hint.High.HasValue()) {
                hi = Some<UInt>(std::min(*hint.High, this->n_remaining));
            } else {
                hi = Some<UInt>(this->n_remaining);
            }

            return { lo, hi };
        }

        return { };
    }

private:
    friend struct Iterator<Impl>;

    VIOLET_IMPLICIT Take(Impl iter, UInt take)
        : n_iter(iter)
        , n_remaining(take)
    {
    }

    Impl n_iter;
    UInt n_remaining;
};

} // namespace violet::iter

namespace violet {

template<typename Impl>
inline auto Iterator<Impl>::Take(UInt take) & noexcept -> decltype(auto)
{
    return iter::Take(getThisObject(), take);
}

template<typename Impl>
inline auto Iterator<Impl>::Take(UInt take) && noexcept -> decltype(auto)
{
    return iter::Take(VIOLET_MOVE(getThisObject()), take);
}

} // namespace violet
