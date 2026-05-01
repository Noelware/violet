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

/// An iterator adapter that allows peeking at the next element without
/// consuming it.
///
/// `Peekable` wraps an existing iterator and adds a [`Peekable::Peek`] method that
/// returns a reference to the next element without advancing the iterator.
/// The peeked value is cached internally and returned on the subsequent
/// call to [`Peekable::Next`].
///
/// This is useful for lookahead-based parsing, conditional consumption, and
/// any situation where a decision depends on the next element before
/// committing to consume it.
///
/// ## Examples
/// ```cpp
/// #include <violet/Iterator/Peekable.h>
/// #include <violet/Print.h>
///
/// auto items = violet::MkIterable(violet::Vec<violet::Int32>({1, 2, 3})).Peekable();
/// auto peeked = items.Peek(); // Some(1)
/// assert(peeked == items.Peek());
///
/// auto first = items.Next(); // consume
/// assert(first == peeked);
/// assert(first != items.Peek());
/// ```
template<Iterable Impl>
struct VIOLET_API Peekable final: public Iterator<Peekable<Impl>> {
    using Item = TypeOf<Impl>;
    using underlying_iterator = Impl;

    VIOLET_DISALLOW_CONSTRUCTOR(Peekable);
    ~Peekable() = default;

    /// Returns the next element without consuming it, or `Nothing` if the
    /// iterator is exhausted.
    ///
    /// The first call to `Peek` advances the underlying iterator by one
    /// position and caches the result. Subsequent calls to `Peek` return
    /// the same cached value until [`Next`] is called to consume it.
    auto Peek() noexcept -> Optional<Item>
    {
        if (!this->n_peeked.HasValue()) {
            this->n_peeked = this->n_iter.Next();
        }

        if (auto peeked = this->n_peeked) {
            return Some<Item>(*peeked);
        }

        return Nothing;
    }

    /// Advances the iterator and returns the next element, or `Nothing` if
    /// the iterator is exhausted.
    ///
    /// If a value was previously cached by [`Peek`], that value is returned
    /// and the cache is cleared. Otherwise, the underlying iterator is
    /// advanced directly.
    auto Next() noexcept -> Optional<Item>
    {
        if (this->n_peeked.HasValue()) {
            auto out = VIOLET_MOVE(*this->n_peeked);
            this->n_peeked.Reset();

            return Some<Item>(out);
        }

        return this->n_iter.Next();
    }

private:
    friend struct Iterator<Impl>;

    VIOLET_IMPLICIT Peekable(Impl iter)
        : n_iter(iter)
    {
    }

    Impl n_iter;
    Optional<Item> n_peeked;
};

} // namespace violet::iter

template<typename Impl>
inline auto violet::Iterator<Impl>::Peekable() & noexcept -> decltype(auto)
{
    return iter::Peekable(getThisObject());
}

template<typename Impl>
inline auto violet::Iterator<Impl>::Peekable() && noexcept -> decltype(auto)
{
    return iter::Peekable(VIOLET_MOVE(getThisObject()));
}
