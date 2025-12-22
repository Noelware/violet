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

#include "violet/Container/Optional.h"
#include "violet/Iterator.h"
#include "violet/Violet.h"

namespace violet::iter {

template<Iterable Impl>
struct Take final: public Iterator<Take<Impl>> {
    using Item = TypeOf<Impl>;

    Take() = delete;

    VIOLET_IMPLICIT Take(Impl iter, UInt take)
        : n_iter(iter)
        , n_remaining(take)
    {
    }

    auto Next() noexcept -> Optional<Item>
    {
        if (this->n_remaining == 0) {
            return Nothing;
        }

        this->n_remaining--;
        return this->n_iter.Next();
    }

    auto NextBack() noexcept -> Optional<Item>
        requires DoubleEndedIterable<Impl>
    {
        if (this->n_remaining == 0) {
            return Nothing;
        }

        this->n_remaining--;
        return this->n_iter.NextBack();
    }

    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        if constexpr (requires { this->n_iter.SizeHint(); }) {
            struct SizeHint hint = this->n_iter.SizeHint();

            UInt lo = std::min(hint.Low, this->n_remaining);
            Optional<UInt> hi;

            if (hint.High.HasValue()) {
                hi = Some<UInt>(std::min(*hint.High, this->n_remaining));
            } else {
                hi = Some<UInt>(this->n_remaining);
            }

            return { lo, hi };
        }

        return {};
    }

private:
    Impl n_iter;
    UInt n_remaining;
};

} // namespace violet::iter

namespace violet {

template<typename Impl>
inline auto Iterator<Impl>::Take(UInt take) & noexcept
{
    return iter::Take(getThisObject(), take);
}

template<typename Impl>
inline auto Iterator<Impl>::Take(UInt take) && noexcept
{
    return iter::Take(VIOLET_MOVE(getThisObject()), take);
}

} // namespace violet
