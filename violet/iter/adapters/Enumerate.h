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
#include "violet/violet.h"

namespace Noelware::Violet::Iterators {

/// A iterator that holds a index of the iteration point.
///
/// - Supports the **Iteration Pipe Syntax** and **Declarative Syntax**.
template<Iterable Impl>
struct Enumerate final: public Noelware::Violet::Iterator<Impl> {
    using Item = Pair<usize, IterableType<Impl>>;

    Enumerate(Impl iter)
        : n_iter(iter)
    {
    }

    auto Next() noexcept -> Optional<Item>
    {
        if (auto value = this->n_iter.Next()) {
            return Some<Item>(std::pair<usize, IterableType<Impl>>(n_idx++, *value));
        }

        return Nothing;
    }

private:
    Impl n_iter;
    usize n_idx = 0;
};

struct EnumeratePipe final: PipeFactory<EnumeratePipe> {
    template<typename Inner>
    auto Apply(Inner&& inner) const noexcept
    {
        return Enumerate<std::decay_t<Inner>>(VIOLET_FWD(Inner, inner));
    }
};

} // namespace Noelware::Violet::Iterators

namespace Noelware::Violet {

inline constexpr Iterators::EnumeratePipe Enumerate;

template<class Impl>
inline auto Iterator<Impl>::Enumerate() & noexcept
{
    return Iterators::Enumerate<Impl>(getThisObject());
}

template<class Impl>
inline auto Iterator<Impl>::Enumerate() && noexcept
{
    return Iterators::Enumerate<Impl>(VIOLET_MOVE(getThisObject()));
}

} // namespace Noelware::Violet
