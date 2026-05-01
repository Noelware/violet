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

/// An iterator adapter that transforms each element by applying a function.
///
/// `Map` wraps an existing iterator and callable, producing a new iterator whose
/// elements are the results of applying the function to each element of the underlying
/// iterator. The output type is determined by the return type of the callable (`Fun`).
///
/// ## Example
/// ```cpp
/// #include <violet/Iterator/Map.h>
/// #include <violet/Print.h>
///
/// auto items = violet::MkIterable(violet::Vec<violet::Int32>{1, 2, 3});
/// for (auto num: items.Map([](violet::Int32 num) -> violet::Int32 { return num * 2; })) {
///     violet::Println("{}", num);
/// }
///
/// // `Map`'s return type can also be a different type
/// for (auto str: items.Map([](violet::Int32 num) -> violet::String { return violet::ToString(num); })) {
///     violet::Println("\"{}\"", str);
/// }
/// ```
///
/// @tparam Impl underlying iterator
/// @tparam Fun functor to call on each iteration
template<Iterable Impl, typename Fun>
    requires callable<Fun, TypeOf<Impl>>
struct VIOLET_API Map final: public Iterator<Map<Impl, Fun>> {
    /// Yields the return type of the mapping function (`Fun`) applied to each element.
    using Item = std::invoke_result_t<Fun, TypeOf<Impl>>;
    using underlying_iterator = Impl;

    VIOLET_DISALLOW_CONSTRUCTOR(Map);
    ~Map() = default;

    /// Advances the underlying iterator and applies the mapping function to
    /// the next element, or returns [`violet::Nothing`] if the iterator is exhausted.
    ///
    /// Each call delegates to the inner iterator's `Next`. If it produces a
    /// value, the mapping function is invoked on that value and the result
    /// is returned. Once the inner iterator returns `Nothing`, all subsequent
    /// calls return `Nothing` as well.
    auto Next() noexcept -> Optional<Item>
    {
        if (auto elem = this->n_iter.Next()) {
            return Some<Item>(std::invoke(this->n_fun, *elem));
        }

        return Nothing;
    }

private:
    friend struct Iterator<Impl>;

    VIOLET_IMPLICIT Map(Impl iter, Fun fun)
        : n_iter(iter)
        , n_fun(fun)
    {
    }

    Impl n_iter;
    Fun n_fun;
};

} // namespace violet::iter

template<typename Impl>
template<typename Fun>
    requires violet::callable<Fun, violet::iter::TypeOf<Impl>>
inline auto violet::Iterator<Impl>::Map(Fun&& fun) & noexcept -> decltype(auto)
{
    return iter::Map(getThisObject(), VIOLET_FWD(Fun, fun));
}

template<typename Impl>
template<typename Fun>
    requires violet::callable<Fun, violet::iter::TypeOf<Impl>>
inline auto violet::Iterator<Impl>::Map(Fun&& fun) && noexcept -> decltype(auto)
{
    return iter::Map(VIOLET_MOVE(getThisObject()), VIOLET_FWD(Fun, fun));
}
