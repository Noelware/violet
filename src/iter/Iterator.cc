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

#include <violet/Container/Optional.h>
#include <violet/Iterator.h>
#include <violet/Violet.h>

using violet::Iterator;
using violet::iter::detail::STLCompatibleIterator;
using violet::iter::detail::STLRangeIterator;

template<typename It>
STLCompatibleIterator<It>::STLCompatibleIterator(It begin, It end)
    : n_current(begin)
    , n_end(end)
{
}

template<typename It>
auto STLCompatibleIterator<It>::Next() noexcept -> violet::Optional<Item>
{
    if (this->n_current != this->n_end) {
        return Some<Item>(*n_current++);
    }

    return Nothing;
}

template<class Impl>
STLRangeIterator<Impl>::STLRangeIterator(Impl impl)
    : n_iter(VIOLET_MOVE(impl))
{
}

template<class Impl>
auto STLRangeIterator<Impl>::operator!=(detail::sentinel) -> bool
{
    if (!this->n_current) {
        this->n_current = this->n_iter.Next();
    }

    return this->n_current.HasValue();
}

template<class Impl>
auto STLRangeIterator<Impl>::operator++()
{
    this->n_current = this->n_iter.Next();
}

template<class Impl>
auto STLRangeIterator<Impl>::operator*()
{
    return *this->n_current;
}

// template<class Impl>
// template<typename Acc, typename Fun>
// auto Iterator<Impl>::Fold(Acc init, Fun&& fun) & noexcept(
//     std::invoke(VIOLET_FWD(Fun, fun), init, *std::declval<iter::TypeOf<Impl>>()))
// {
//     auto acc = init;
//     while (auto value = this->getThisObject().Next()) {
//         acc = std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(acc), *value);
//     }

//     return acc;
// }

/*
    template<typename Acc, typename Fun>
        requires Callable<Fun, Acc, IterableType<Impl>>
        && std::convertible_to<std::invoke_result_t<Fun, Acc, IterableType<Impl>>, Acc>
    auto Fold(Acc init, Fun&& fun) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), init, *std::declval<IterableType<Impl>>())))
    {
        auto acc = VIOLET_MOVE(init);

        while (auto value = getThisObject().Next()) {
            acc = std::invoke(VIOLET_FWD(Fun, fun), VIOLET_MOVE(acc), *value);
        }

        return acc;
    }
*/

// template<class Impl>
// struct Iterator {
//     /// Reduces the iterator to a single accumulated value.
//     ///
//     /// Equivalent to Rust's [`Iterator::fold()`].
//     /// [`Iterator::fold()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.fold
//     ///
//     /// @param init The initial accumulator value
//     /// @param fun callable that combines the accumulator with each iterator item
//     /// @returns the final accumulated value.
//     template<typename Acc, typename Fun>
//         requires(callable<Fun, Acc, iter::TypeOf<Impl>>
//             && std::convertible_to<std::invoke_result_t<Fun, Acc, iter::TypeOf<Impl>>, Acc>)
//     auto Fold(Acc init, Fun&& fun) & noexcept(
//         std::invoke(VIOLET_FWD(Fun, fun), init, *std::declval<iter::TypeOf<Impl>>()));

//     /// Reduces the iterator to a single accumulated value.
//     ///
//     /// Equivalent to Rust's [`Iterator::fold()`].
//     /// [`Iterator::fold()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.fold
//     ///
//     /// @param init The initial accumulator value
//     /// @param fun callable that combines the accumulator with each iterator item
//     /// @returns the final accumulated value.
//     template<typename Acc, typename Fun>
//         requires(callable<Fun, Acc, iter::TypeOf<Impl>>
//             && std::convertible_to<std::invoke_result_t<Fun, Acc, iter::TypeOf<Impl>>, Acc>)
//     auto Fold(Acc init, Fun&& fun) && noexcept(
//         std::invoke(VIOLET_FWD(Fun, fun), init, *std::declval<iter::TypeOf<Impl>>()));

//     /// Reduces a double-ended iterator from the back.
//     ///
//     /// Equivalent to Rust's [`Iterator::rfold()`].
//     /// [`Iterator::rfold()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.rfold
//     template<typename Acc, typename Fun>
//         requires(DoubleEndedIterable<Impl> && callable<Fun, Acc, iter::TypeOf<Impl>>
//             && std::convertible_to<std::invoke_result_t<Fun, Acc, iter::TypeOf<Impl>>, Acc>)
//     auto RFold(Acc init, Fun&& fun) & noexcept(
//         std::invoke(VIOLET_FWD(Fun, fun), init, *std::declval<iter::TypeOf<Impl>>()));

//     /// Reduces a double-ended iterator from the back.
//     ///
//     /// Equivalent to Rust's [`Iterator::rfold()`].
//     /// [`Iterator::rfold()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.rfold
//     template<typename Acc, typename Fun>
//         requires(DoubleEndedIterable<Impl> && callable<Fun, Acc, iter::TypeOf<Impl>>
//             && std::convertible_to<std::invoke_result_t<Fun, Acc, iter::TypeOf<Impl>>, Acc>)
//     auto RFold(Acc init, Fun&& fun) && noexcept(
//         std::invoke(VIOLET_FWD(Fun, fun), init, *std::declval<iter::TypeOf<Impl>>()));

//     /// Returns the position of the first item satisfying a predicate.
//     ///
//     /// Equivalent to Rust's [`Iterator::position()`].
//     /// [`Iterator::position()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.position
//     ///
//     /// @returns Optional index of the item, or `Nothing` if no item matches.
//     template<typename Pred>
//     auto Position(Pred&& pred) & noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>()))
//         -> Optional<UInt>;

//     /// Returns the position of the first item satisfying a predicate.
//     ///
//     /// Equivalent to Rust's [`Iterator::position()`].
//     /// [`Iterator::position()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.position
//     ///
//     /// @returns Optional index of the item, or `Nothing` if no item matches.
//     template<typename Pred>
//     auto Position(Pred&& pred) && noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>()))
//         -> Optional<UInt>;

//     /// Finds the first item that satisfies the given predicate.
//     ///
//     /// Equivalent to Rust's [`Iterator::find()`].
//     /// [`Iterator::find()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.find
//     template<typename Pred>
//         requires(callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>)
//     auto Find(Pred&& pred) & noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>()));

//     /// Finds the first item that satisfies the given predicate.
//     ///
//     /// Equivalent to Rust's [`Iterator::find()`].
//     /// [`Iterator::find()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.find
//     template<typename Pred>
//         requires(callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>)
//     auto Find(Pred&& pred) && noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>()));

//     /// Applies a function that returns an `Optional` and returns the first `Some` value.
//     ///
//     /// Equivalent to Rust's [`Iterator::find_map()`].
//     /// [`Iterator::find_map()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.find_map
//     template<typename Fun>
//         requires(callable<Fun, iter::TypeOf<Impl>>
//             && iter::detail::is_optional_v<std::invoke_result_t<Fun, iter::TypeOf<Impl>>>)
//     auto FindMap(Fun&& fun) & noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<iter::TypeOf<Impl>>()));

//     /// Applies a function that returns an `Optional` and returns the first `Some` value.
//     ///
//     /// Equivalent to Rust's [`Iterator::find_map()`].
//     /// [`Iterator::find_map()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.find_map
//     template<typename Fun>
//         requires(callable<Fun, iter::TypeOf<Impl>>
//             && iter::detail::is_optional_v<std::invoke_result_t<Fun, iter::TypeOf<Impl>>>)
//     auto FindMap(Fun&& fun) && noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<iter::TypeOf<Impl>>()));

//     /// Returns true if any element matches the predicate.
//     ///
//     /// Equivalent to Rust's [`Iterator::any()`].
//     /// [`Iterator::any()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.any
//     template<typename Pred>
//         requires(callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>)
//     auto Any(Pred&& pred) & noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>()));

//     /// Returns true if any element matches the predicate.
//     ///
//     /// Equivalent to Rust's [`Iterator::any()`].
//     /// [`Iterator::any()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.any
//     template<typename Pred>
//         requires(callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>)
//     auto Any(Pred&& pred) && noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>()));

//     /// Inspects each item of the iterator by calling a function with a side effect.
//     ///
//     /// Equivalent to Rust's [`Iterator::inspect()`].
//     /// [`Iterator::inspect()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.inspect
//     template<typename Fun>
//         requires(callable<Fun, iter::TypeOf<Impl>> && callable_returns<Fun, void, iter::TypeOf<Impl>>)
//     auto Inspect(Fun&& fun) & noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<iter::TypeOf<Impl>>()));

//     /// Inspects each item of the iterator by calling a function with a side effect.
//     ///
//     /// Equivalent to Rust's [`Iterator::inspect()`].
//     /// [`Iterator::inspect()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.inspect
//     template<typename Fun>
//         requires(callable<Fun, iter::TypeOf<Impl>> && callable_returns<Fun, void, iter::TypeOf<Impl>>)
//     auto Inspect(Fun&& fun) && noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<iter::TypeOf<Impl>>()));

//     /// Counts the number of remaining elements.
//     ///
//     /// Equivalent to Rust's [`Iterator::count()`].
//     /// [`Iterator::count()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.count
//     auto Count() & noexcept -> UInt;

//     /// Counts the number of remaining elements.
//     ///
//     /// Equivalent to Rust's [`Iterator::count()`].
//     /// [`Iterator::count()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.count
//     auto Count() && noexcept -> UInt;

//     /// Advances the iterator by `nth` elements.
//     ///
//     /// Equivalent to Rust's [`Iterator::advance_by()`].
//     ///
//     /// [`Iterator::advance_by()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.advance_by
//     auto AdvanceBy(UInt nth) & noexcept -> Result<void, UInt>;

//     /// Advances the iterator by `nth` elements.
//     ///
//     /// Equivalent to Rust's [`Iterator::advance_by()`].
//     ///
//     /// [`Iterator::advance_by()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.advance_by
//     auto AdvanceBy(UInt nth) && noexcept -> Result<void, UInt>;

//     /// Returns the `nth` element of the iterator.
//     ///
//     /// Equivalent to Rust's [`Iterator::nth()`].
//     ///
//     /// [`Iterator::nth()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.nth
//     auto Nth(UInt nth) & noexcept -> Optional<iter::TypeOf<Impl>>;

//     /// Returns the `nth` element of the iterator.
//     ///
//     /// Equivalent to Rust's [`Iterator::nth()`].
//     ///
//     /// [`Iterator::nth()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.nth
//     auto Nth(UInt nth) && noexcept -> Optional<iter::TypeOf<Impl>>;

//     /// Collects the iterator elements into a container.
//     template<typename Container>
//     auto Collect();

//     /// Returns the lower and upper bound size hints for the iterator.
//     [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint;

//     auto begin();
//     auto end();

// private:
//     constexpr auto getThisObject() & noexcept -> Impl&
//     {
//         return static_cast<Impl&>(*this);
//     }

//     constexpr auto getThisObject() && noexcept -> Impl&&
//     {
//         return static_cast<Impl&&>(*this);
//     }
// };

// /// Adapts a container into a Violet iterator.
// ///
// /// Backed by a `STLCompatibleIterator`, this allows bridging existing STL
// /// containers into Violet’s iterator API.
// ///
// /// # Examples
// ///
// /// ```cpp
// /// std::vector<int> v{1,2,3};
// /// auto it = MkIterable(v);
// ///
// /// for (auto x : it) { /* ... */ }
// /// ```
// template<typename Container>
// auto MkIterable(Container& cnt);

// /// Adapts a container into a Violet iterator.
// ///
// /// Backed by a `STLCompatibleIterator`, this allows bridging existing STL
// /// containers into Violet’s iterator API.
// ///
// /// # Examples
// ///
// /// ```cpp
// /// std::vector<int> v{1,2,3};
// /// auto it = MkIterable(v);
// ///
// /// for (auto x : it) { /* ... */ }
// /// ```
// template<typename Container>
// auto MkIterable(Container&& cnt);

// } // namespace violet

// */
