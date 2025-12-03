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
struct Skip final: public Iterator<Skip<Impl>> {
    using Item = TypeOf<Impl>;

    Skip() = delete;

    VIOLET_IMPLICIT Skip(Impl iter, UInt take)
        : n_iter(iter)
        , n_skip(take)
    {
    }

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

    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        if constexpr (requires { this->n_iter.SizeHint(); }) {
            struct SizeHint hint = this->n_iter.SizeHint();

            auto lo = hint.Low > this->n_skip ? (hint.Low - this->n_skip) : 0;
            Optional<UInt> hi;

            if (hint.High) {
                hi = *hint.High > this->n_skip ? Some<UInt>(*hint.High - this->n_skip) : Some<UInt>();
            }

            return { lo, hi };
        }

        return {};
    }

private:
    Impl n_iter;
    UInt n_skip;
};

} // namespace violet::iter

namespace violet {

template<typename Impl>
inline auto Iterator<Impl>::Skip(UInt take) & noexcept
{
    return iter::Skip(getThisObject(), take);
}

template<typename Impl>
inline auto Iterator<Impl>::Skip(UInt take) && noexcept
{
    return iter::Skip(VIOLET_MOVE(getThisObject()), take);
}

} // namespace violet
