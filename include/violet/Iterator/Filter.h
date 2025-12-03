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

template<Iterable Impl, typename Pred>
    requires(callable<Pred, TypeOf<Impl>> && callable_returns<Pred, bool, TypeOf<Impl>>)
struct Filter final: public Iterator<Filter<Impl, Pred>> {
    using Item = TypeOf<Impl>;

    Filter() = delete;

    VIOLET_IMPLICIT Filter(Impl iter, Pred predicate)
        : n_iter(iter)
        , n_pred(predicate)
    {
    }

    auto Next() noexcept -> Optional<Item>
    {
        while (auto value = this->n_iter.Next()) {
            if (std::invoke(this->n_pred, *value)) {
                return Some<Item>(*value);
            }
        }

        return Nothing;
    }

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

    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        if constexpr (requires { this->n_iter.SizeHint(); }) {
            struct SizeHint hint = this->n_iter.SizeHint();
            return { 0, hint.High };
        }

        return {};
    }

private:
    Impl n_iter;
    Pred n_pred;
};

} // namespace violet::iter

namespace violet {

template<class Impl>
template<typename Pred>
    requires callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>
inline auto Iterator<Impl>::Filter(Pred&& predicate) & noexcept
{
    return iter::Filter(getThisObject(), VIOLET_FWD(Pred, predicate));
}

template<class Impl>
template<typename Pred>
    requires callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>
inline auto Iterator<Impl>::Filter(Pred&& predicate) && noexcept
{
    return iter::Filter(getThisObject(), VIOLET_FWD(Pred, predicate));
}

} // namespace violet
