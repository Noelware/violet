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

template<Iterable Impl, typename Fun>
    requires callable<Fun, TypeOf<Impl>>
struct Map final: public Iterator<Map<Impl, Fun>> {
    using Item = std::invoke_result_t<Fun, TypeOf<Impl>>;

    Map() = delete;

    VIOLET_IMPLICIT Map(Impl iter, Fun fun)
        : n_iter(iter)
        , n_fun(fun)
    {
    }

    auto Next() noexcept -> Optional<Item>
    {
        if (auto elem = this->n_iter.Next()) {
            return Some<Item>(std::invoke(this->n_fun, *elem));
        }

        return Nothing;
    }

private:
    Impl n_iter;
    Fun n_fun;
};

} // namespace violet::iter

template<typename Impl>
template<typename Fun>
    requires violet::callable<Fun, violet::iter::TypeOf<Impl>>
inline auto violet::Iterator<Impl>::Map(Fun&& fun) & noexcept
{
    return iter::Map(getThisObject(), VIOLET_FWD(Fun, fun));
}

template<typename Impl>
template<typename Fun>
    requires violet::callable<Fun, violet::iter::TypeOf<Impl>>
inline auto violet::Iterator<Impl>::Map(Fun&& fun) && noexcept
{
    return iter::Map(VIOLET_MOVE(getThisObject()), VIOLET_FWD(Fun, fun));
}
