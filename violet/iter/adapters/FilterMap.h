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

#include "violet/container/Optional.h"
#include "violet/iter/Iterator.h"
#include "violet/iter/adapters/Pipes.h"

namespace Noelware::Violet::Iterators {

template<Iterable Impl, typename Fun>
    requires Callable<Fun, IterableType<Impl>> && detail::is_optional_v<std::invoke_result_t<Fun, IterableType<Impl>>>
struct FilterMap final: public Noelware::Violet::Iterator<FilterMap<Impl, Fun>> {
    using Item = detail::optional_type_t<std::invoke_result_t<Fun, IterableType<Impl>>>;

    FilterMap() = delete;
    VIOLET_IMPLICIT FilterMap(Impl iter, Fun fun)
        : n_iter(VIOLET_MOVE(iter))
        , n_fun(VIOLET_MOVE(fun))
    {
    }

    auto Next() noexcept(noexcept(std::invoke(this->n_fun, std::declval<Item>()))) -> Optional<Item>
    {
        while (auto value = this->n_iter.Next()) {
            if (auto mapped = std::invoke(this->n_fun, *value)) {
                return mapped;
            }
        }

        return decltype(Optional<Item>())(Nothing);
    }

    auto NextBack() noexcept(noexcept(std::invoke(this->n_fun, std::declval<Item>()))) -> Optional<Item>
        requires DoubleEndedIterable<Impl>
    {
        while (auto value = this->n_iter.NextBack()) {
            if (auto mapped = std::invoke(this->n_fun, *value)) {
                return mapped;
            }
        }

        return decltype(Optional<Item>())(Nothing);
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
    Fun n_fun;
};

template<typename Fun>
struct FilterMapPipe final: PipeFactory<FilterMapPipe<Fun>> {
    FilterMapPipe() = delete;
    VIOLET_IMPLICIT FilterMapPipe(Fun fun)
        : n_fun(VIOLET_MOVE(fun))
    {
    }

    template<typename Inner>
    auto Apply(Inner&& inner) const noexcept
    {
        return FilterMap<Inner, Fun>(VIOLET_FWD(Inner, inner), this->n_fun);
    }

private:
    Fun n_fun;
};

} // namespace Noelware::Violet::Iterators

namespace Noelware::Violet {

template<typename Fun>
inline auto FilterMap(Fun fun) noexcept
{
    return Iterators::FilterMapPipe(fun);
}

template<class Impl>
template<typename Fun>
    requires Callable<Fun, IterableType<Impl>>
inline auto Iterator<Impl>::FilterMap(Fun fun) & noexcept
{
    return Iterators::FilterMap(getThisObject(), fun);
}

template<class Impl>
template<typename Fun>
    requires Callable<Fun, IterableType<Impl>>
inline auto Iterator<Impl>::FilterMap(Fun fun) && noexcept
{
    return Iterators::FilterMap(VIOLET_MOVE(getThisObject()), fun);
}

} // namespace Noelware::Violet
