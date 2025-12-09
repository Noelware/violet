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

#include "violet/Container/Optional.h"
#include "violet/Iterator.h"
#include "violet/Violet.h"

namespace violet::iter {

template<Iterable Impl>
struct Peekable final: public Iterator<Peekable<Impl>> {
    using Item = TypeOf<Impl>;

    VIOLET_IMPLICIT Peekable() = delete;

    VIOLET_IMPLICIT Peekable(Impl iter)
        : n_iter(iter)
    {
    }

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
    Impl n_iter;
    Optional<Item> n_peeked;
};

} // namespace violet::iter

template<typename Impl>
inline auto violet::Iterator<Impl>::Peekable() & noexcept
{
    return iter::Peekable(getThisObject());
}

template<typename Impl>
inline auto violet::Iterator<Impl>::Peekable() && noexcept
{
    return iter::Peekable(VIOLET_MOVE(getThisObject()));
}
