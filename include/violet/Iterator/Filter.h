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

/// An iterator adapter that yields only the elements for which a predicate returns `true`
///
/// `Filter` wraps an existing iterator and a callable predicate, advancing the underlying
/// iterator on each call to [`Filter::Next`] and forwarding only those elements that satisfy
/// the predicate. Elements that do not match are silently discarded.
///
/// If the underlying iterator implements [`violet::DoubleEndedIterable`], this iterator
/// adapter also supports reverse iteration via [`Filter::NextBack`].
///
/// ## Example
/// ```cpp
/// #include <violet/Iterator/Filter.h>
/// #include <violet/Iterator.h>
/// #include <violet/Print.h>
///
/// auto items = violet::MkIterable(violet::Vec<violet::Int32>{1, 2, 3, 4, 5, 6});
/// while (auto num: items.Filter([](violet::Int32 num) -> bool { return num % 2 == 0; })) {
///     violet::Println("number {} is a even number", num);
/// }
/// ```
///
/// ## Size Hint
/// Because the predicate may reject an arbitrary number of elements, the lower bound
/// is always zero. The upper bound is inherited from the underlying iterator's size
/// hint, since at most every element could match.
///
/// @tparam Impl underlying iterator
/// @tparam Pred the predicate that accepts the iterator type.
template<Iterable Impl, typename Pred>
    requires(callable<Pred, TypeOf<Impl>> && callable_returns<Pred, bool, TypeOf<Impl>>)
struct VIOLET_API Filter final: public Iterator<Filter<Impl, Pred>> {
    using Item = TypeOf<Impl>;
    using underlying_iterator = Impl;

    VIOLET_DISALLOW_CONSTRUCTOR(Filter);
    ~Filter() = default;

    /// Advances the underlying iterator, returning the next element that
    /// satisfies the predicate, or `Nothing` if no matching elements remain.
    ///
    /// Elements that do not satisfy the predicate are consumed and discarded.
    /// This means a single call to `Next` may advance the underlying iterator
    /// by more than one position.
    auto Next() noexcept -> Optional<Item>
    {
        while (auto value = this->n_iter.Next()) {
            if (std::invoke(this->n_pred, *value)) {
                return Some<Item>(*value);
            }
        }

        return Nothing;
    }

    /// Advances the underlying iterator from the back, returning the next
    /// trailing element that satisfies the predicate, or `Nothing` if no
    /// matching elements remain.
    ///
    /// This method is only available when the underlying iterator satisfies
    /// `DoubleEndedIterable`. Elements that do not match are consumed from
    /// the back and discarded.
    auto NextBack() noexcept -> Optional<Item>
        requires DoubleEndedIterable<Impl>
    {
        while (auto value = this->n_iter.NextBack()) {
            if (std::invoke(this->n_pred, *value)) {
                return Some<Item>(*value);
            }
        }

        return Nothing;
    }

    /// Returns a hint of the remaining length of the iterator.
    ///
    /// The lower bound is always `0` because the predicate could reject every
    /// remaining element. The upper bound is taken from the underlying
    /// iterator's size hint, representing the case where every element matches.
    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        if constexpr (requires { this->n_iter.SizeHint(); }) {
            violet::SizeHint hint = this->n_iter.SizeHint();
            return { 0, hint.High };
        }

        return { };
    }

private:
    friend struct Iterator<Impl>;

    VIOLET_IMPLICIT Filter(Impl iter, Pred predicate)
        : n_iter(iter)
        , n_pred(predicate)
    {
    }

    Impl n_iter;
    Pred n_pred;
};

} // namespace violet::iter

namespace violet {

template<class Impl>
template<typename Pred>
    requires callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>
inline auto Iterator<Impl>::Filter(Pred&& predicate) & noexcept -> decltype(auto)
{
    return iter::Filter(getThisObject(), VIOLET_FWD(Pred, predicate));
}

template<class Impl>
template<typename Pred>
    requires callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>
inline auto Iterator<Impl>::Filter(Pred&& predicate) && noexcept -> decltype(auto)
{
    return iter::Filter(getThisObject(), VIOLET_FWD(Pred, predicate));
}

} // namespace violet
