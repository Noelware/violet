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

#include "violet/iter/Iterator.h"
#include "violet/iter/adapters/Pipes.h"

namespace Noelware::Violet::Iterators {

template<Iterable Impl, typename Pred>
    requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
struct Filter final: public Noelware::Violet::Iterator<Impl> {
    using Item = IterableType<Impl>;

    Filter() = delete;
    VIOLET_IMPLICIT Filter(Impl iter, Pred predicate)
        : n_iter(VIOLET_MOVE(iter))
        , n_predicate(VIOLET_MOVE(predicate))
    {
    }

    auto Next() noexcept -> Optional<Item>
    {
        while (auto value = this->n_iter.Next()) {
            if (std::invoke(this->n_predicate, *value)) {
                return value;
            }
        }

        return Nothing;
    }

    auto NextBack() noexcept -> Optional<Item>
        requires DoubleEndedIterable<Impl>
    {
        while (auto value = this->n_iter.NextBack()) {
            if (std::invoke(this->n_predicate, *value)) {
                return value;
            }
        }

        return Nothing;
    }

    [[nodiscard]] auto SizeHint() const noexcept -> Noelware::Violet::SizeHint
    {
        if constexpr (requires { this->n_iter.SizeHint(); }) {
            Noelware::Violet::SizeHint hint = this->n_iter.SizeHint();
            return SizeHint(0, hint.High);
        }

        return {};
    }

private:
    Impl n_iter;
    Pred n_predicate;
};

template<typename Pred>
struct FilterPipe final: PipeFactory<FilterPipe<Pred>> {
    FilterPipe() = delete;
    VIOLET_IMPLICIT FilterPipe(Pred predicate)
        : n_predicate(VIOLET_MOVE(predicate))
    {
    }

    template<typename Inner>
    auto Apply(Inner&& inner) const noexcept
    {
        return Filter<Inner, Pred>(VIOLET_FWD(Inner, inner), this->n_predicate);
    }

private:
    Pred n_predicate;
};

} // namespace Noelware::Violet::Iterators

namespace Noelware::Violet {

template<typename Pred>
inline auto Filter(Pred predicate) noexcept
{
    return Iterators::FilterPipe(predicate);
}

template<class Impl>
template<typename Pred>
inline auto Iterator<Impl>::Filter(Pred predicate) & noexcept
{
    return Iterators::Filter(getThisObject(), predicate);
}

template<class Impl>
template<typename Pred>
inline auto Iterator<Impl>::Filter(Pred predicate) && noexcept
{
    return Iterators::Filter(VIOLET_MOVE(getThisObject()), predicate);
}

} // namespace Noelware::Violet
