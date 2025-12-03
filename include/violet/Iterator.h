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
//
//! # 🌺💜 `violet/Iterator.h`
//! This header file provides the high-level framework for iteration, analogous to Rust's [`std::iter`]
//!
//! [`std::iter`]: https://doc.rust-lang.org/1.90.0/std/iter/index.html
//!
//! This module provides a full-featured, composable iteration API centered
//! around `Iterator<Impl>`, a CRTP-based trait class that mirrors Rust's
//! `Iterator` and `DoubleEndedIterator`.
//!
//! The design goal is to offer:
//! - Stateless, chainable iterator adapters,
//! - Lazy, pull-based iteration semantics,
//! - Strong typing via concepts (`Iterable`, `DoubleEndedIterable`),
//! - Integration with standard C++ containers via `MkIterable`,
//! - Optional C++ range-for compatibility (`begin()`, `end()`),
//! - APIs that closely resemble Rust's ergonomics and guarantees.
//!
//! ## Overview
//! A type becomes a Violet iterator by implementing the following methods:
//!
//! ```cpp
//! Optional<Item> Next();
//! Optional<Item> NextBack(); // optional, enables double-ended iteration
//! ```
//!
//! and inheriting from <code>[`Iterator`]\<Impl\></code>:
//!
//! ```cpp
//! struct MyIter final: public Iterator<MyIter> {
//!     auto Next() -> Optional<int> { /* ... */ }
//! };
//! ```
//!
//! All adapters like `Map`, `Filter`, `Take`, etc. are inherited automatically! Complex
//! iteration pipelines can be constructed by chaining methods, like Rust:
//!
//! ```cpp
//! auto it = MkIterable(vec)
//!     .Map([](auto x) { return x + 1; })
//!     .Filter([](auto x) { return x % 2 == 0; })
//!     .Take(10);
//!
//! for (auto v : it) {
//!     // ...
//! }
//! ```
//!
//! ## Interoperability
//! ### C++ Ranges
//! Violet iterators that implement <code>[`Iterator`]\<Impl\></code> support standard C++ iteration:
//!
//! ```cpp
//! for (auto value: MyIter()) { /* ... */ }
//! ```
//!
//! ### STL Containers
//! Any container that supports [`std::begin`] and [`std::end`] can be adapted as well with
//! the [`violet::MkIterable`] function.
//!
//! ## Size Information
//! The [`violet::SizeHint`] class provides `(lo, hi]` estimates for the remaining items on an
//! iterator. These hints are popagated through all adapters whenever possible and feasible
//! that enable optimizations such as reserving capacity during collection.

#pragma once

#include "violet/Container/Optional.h"
#include "violet/Container/Result.h"
#include "violet/Violet.h"

#include <concepts>
#include <type_traits>
#include <utility>

namespace violet {

template<class Impl>
struct Iterator;
struct SizeHint;

namespace iter::detail {

    template<class T>
    struct is_optional: std::false_type {};

    template<class U>
    struct is_optional<Optional<U>>: std::true_type {};

    template<class T>
    inline constexpr bool is_optional_v = is_optional<T>::value;

    template<class T>
    struct optional_type;

    template<class U>
    struct optional_type<Optional<U>> {
        using type = U;
    };

    template<class T>
    using optional_type_t = typename optional_type<T>::type;

    template<typename T>
    concept ValidIterable = requires(T ty) {
        // Constraint that requires `ty.Next()` to be callable.
        { ty.Next() } -> std::same_as<decltype(ty.Next())>;

        // Constraint that the type that is returned by `ty.Next()` is a
        // `Noelware::Violet::Optional`.
        requires detail::is_optional_v<std::remove_cvref_t<decltype(ty.Next())>>;
    };

    template<typename T>
    concept ValidDoubleEndedIterable = ValidIterable<T> && requires(T ty) {
        // Constraint that requires `ty.NextBack()` to be callable.
        { ty.NextBack() } -> std::same_as<decltype(ty.Next())>;

        // Constraint that the type that is returned by `ty.NextBack()` is a
        // `Noelware::Violet::Optional`.
        requires detail::is_optional_v<std::remove_cvref_t<decltype(ty.NextBack())>>;
    };

} // namespace iter::detail

/// Concept that identifies types implementing the Violet iteration protocol.
///
/// A type `T` is considered iterable when:
///
/// - It provides a `Next()` member function,
/// - `Next()` returns `Optional<Item>`,
/// - The returned `Optional` matches `violet::Optional`.
///
/// This mirrors the constraints of Rust's `Iterator` trait. Any type meeting
/// these requirements can be adapted into the Violet iterator ecosystem.
template<typename T>
concept Iterable = iter::detail::ValidIterable<std::remove_cvref_t<T>>;

/// Concept for types that support double-ended iteration.
///
/// Extends `Iterable` by requiring:
/// - A `NextBack()` member function,
/// - `NextBack()` returns the same `Optional<Item>` type as `Next()`.
///
/// This corresponds to Rust's `DoubleEndedIterator`, enabling operations
/// that pull items from both the front and back of a sequence.
template<typename T>
concept DoubleEndedIterable = iter::detail::ValidDoubleEndedIterable<std::remove_cvref_t<T>>;

namespace iter {

    /// Extracts the type that is returned of a iterator's `Next()` method.
    template<typename T>
    using TypeOf = detail::optional_type_t<decltype(std::declval<T>().Next())>;

    namespace detail {
        /// Sentinel object used to mark the end of iteration when integrating a
        /// Violet iterator with C++ range-based for loops.
        ///
        /// Works in conjunction with [`STLRangeIterator`] to provide:
        ///
        /// ```cpp
        /// for (auto x : my_iter) { ... }
        /// ```
        struct sentinel final {};

        /// Adapter that exposes a Violet iterator as a C++ range iterator.
        ///
        /// Wraps an iterator implementation `Impl` and provides:
        /// - `operator!=` comparison with `sentinel`,
        /// - `operator++` advancing the iterator,
        /// - `operator*` dereferencing the current item.
        ///
        /// This enables idiomatic C++ `for` loops over Violet iterators.
        template<class Impl>
        struct STLRangeIterator final {
            STLRangeIterator() = delete;
            STLRangeIterator(Impl impl)
                : n_iter(VIOLET_MOVE(impl))
            {
            }

            auto operator!=(detail::sentinel) -> bool
            {
                if (!this->n_current) {
                    this->n_current = this->n_iter.Next();
                }

                return this->n_current.HasValue();
            }

            void operator++()
            {
                this->n_current = this->n_iter.Next();
            }

            auto operator*()
            {
                return *this->n_current;
            }

        private:
            Impl n_iter;
            Optional<violet::iter::TypeOf<Impl>> n_current;
        };

        /// Iterator adapter that wraps standard-library iterators (`std::begin`/`std::end`)
        /// and exposes them as Violet iterators.
        ///
        /// @tparam It any valid STL input iterator.
        template<typename It>
        struct STLCompatibleIterator final: public Iterator<STLCompatibleIterator<It>> {
            using Item = std::remove_cv_t<std::remove_reference_t<typename std::iter_value_t<It>>>;

            STLCompatibleIterator(It begin, It end)
                : n_current(begin)
                , n_end(end)
            {
            }

            auto Next() noexcept -> Optional<Item>
            {
                if (this->n_current != this->n_end) {
                    return Some<Item>(*n_current++);
                }

                return Nothing;
            }

        private:
            It n_current;
            It n_end;
        };
    } // namespace detail

} // namespace iter

/// Size hint information for iterators.
///
/// Provides lower and optional upper bounds describing how many items an
/// iterator may yield. These hints are used to optimize allocation strategies
/// or pre-sizing of collections.
///
/// ## Example
/// ```cpp
/// violet::SizeHint h1; // 0..None
/// violet::SizeHint h2(5, 10); // 5..Some(10)
/// violet::SizeHint h3(3, Nothing); // 3..None
/// ```
struct SizeHint final {
    UInt Low = 0; ///< Minimum number of elements that are expected.
    Optional<UInt> High = Nothing; ///< An optional upper bound.

    constexpr SizeHint() = default;
    constexpr VIOLET_IMPLICIT SizeHint(UInt lo, Optional<UInt> hi = Nothing)
        : Low(lo)
        , High(VIOLET_MOVE(hi))
    {
    }
};

/// The core trait-like base class for building Violet iterator types.
///
/// This CRTP class defines a large suite of iterator adapters and consumers,
/// closely modeled after Rust's `Iterator` API. Each adapter returns a new
/// iterator type that composes with others, enabling fluent, functional
/// iteration pipelines.
///
/// Implementors must define:
///
/// ```cpp
/// Optional<Item> Impl::Next();
/// Optional<Item> Impl::NextBack();     // required only for double-ended iteration
/// ```
///
/// ## Example
/// ```cpp
/// auto it = MkIterable(vec)
///     .FilterMap([](int x) -> Optional<int> {
///         int y = x * 2;
///         return y > 10 ? Some<int>(y) : Nothing;
///     });
///
/// for (auto v: it) { /* ... */ }
/// ```
///
/// @tparam Impl The implementation class.
template<class Impl>
struct Iterator {
    /// Adapter that allows inspecting the next element without consuming it.
    ///
    /// Calling `Peek()` returns an `Optional<Item>` representing the next value,
    /// but does not advance the iterator. Subsequent calls to `Next()` will still
    /// return this value.
    ///
    /// Equivalent to Rust's [`Iterator::peekable()`].
    ///
    /// [`Iterator::peekable()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.peekable
    auto Peekable() & noexcept;

    /// Adapter that allows inspecting the next element without consuming it.
    ///
    /// Calling `Peek()` returns an `Optional<Item>` representing the next value,
    /// but does not advance the iterator. Subsequent calls to `Next()` will still
    /// return this value.
    ///
    /// Equivalent to Rust's [`Iterator::peekable()`].
    ///
    /// [`Iterator::peekable()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.peekable
    auto Peekable() && noexcept;

    /// Adapter that yields `(index, item)` pairs.
    ///
    /// The index starts at zero and increments for each value produced.
    /// Equivalent to Rust's `Iterator::enumerate()`.
    auto Enumerate() & noexcept;

    /// Adapter that yields `(index, item)` pairs.
    ///
    /// The index starts at zero and increments for each value produced.
    /// Equivalent to Rust's `Iterator::enumerate()`.
    ///
    /// [`Iterator::enumerate()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.enumerate
    auto Enumerate() && noexcept;

    /// Adapter that transforms items using a mapping function.
    ///
    /// Each yielded item becomes:
    ///
    /// ```cpp
    /// f(item)
    /// ```
    ///
    /// The resulting iterator yields values of the function's return type.
    ///
    /// Equivalent to Rust's [`Iterator::map()`].
    ///
    /// [`Iterator::map()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.map
    auto Map() & noexcept;

    /// Adapter that transforms items using a mapping function.
    ///
    /// Each yielded item becomes:
    ///
    /// ```cpp
    /// f(item)
    /// ```
    ///
    /// The resulting iterator yields values of the function's return type.
    ///
    /// Equivalent to Rust's [`Iterator::map()`].
    ///
    /// [`Iterator::map()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.map
    auto Map() && noexcept;

    /// Adapter that yields only items for which the predicate returns `true`.
    ///
    /// Equivalent to Rust's [`Iterator::filter()`].
    ///
    /// [`Iterator::filter()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.filter
    template<typename Pred>
        requires callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>
    auto Filter(Pred&& pred) & noexcept;

    /// Adapter that yields only items for which the predicate returns `true`.
    ///
    /// Equivalent to Rust's [`Iterator::filter()`].
    ///
    /// [`Iterator::filter()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.filter
    template<typename Pred>
        requires callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>
    auto Filter(Pred&& pred) && noexcept;

    /// Skips the first `skip` items.
    ///
    /// Equivalent to Rust's [`Iterator::skip()`].
    /// [`Iterator::skip()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.skip
    auto Skip(UInt skip) & noexcept;

    /// Skips the first `skip` items.
    ///
    /// Equivalent to Rust's [`Iterator::skip()`].
    /// [`Iterator::skip()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.skip
    auto Skip(UInt skip) && noexcept;

    /// Yields at most `take` items from the iterator.
    ///
    /// Equivalent to Rust's [`Iterator::take()`].
    /// [`Iterator::take()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.take
    auto Take(UInt take) & noexcept;

    /// Yields at most `take` items from the iterator.
    ///
    /// Equivalent to Rust's [`Iterator::take()`].
    /// [`Iterator::take()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.take
    auto Take(UInt take) && noexcept;

    /// Reduces the iterator to a single accumulated value.
    ///
    /// Equivalent to Rust's [`Iterator::fold()`].
    /// [`Iterator::fold()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.fold
    ///
    /// @param init The initial accumulator value
    /// @param fun callable that combines the accumulator with each iterator item
    /// @returns the final accumulated value.
    template<typename Acc, typename Fun>
        requires(callable<Fun, const Acc&, iter::TypeOf<Impl>>
            && std::convertible_to<std::invoke_result_t<Fun, const Acc&, iter::TypeOf<Impl>>, Acc>)
    auto Fold(Acc init, Fun&& fun) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), init, std::declval<iter::TypeOf<Impl>>())))
    {
        auto acc = VIOLET_MOVE(init);
        while (auto elem = getThisObject().Next()) {
            acc = std::invoke(VIOLET_FWD(Fun, fun), acc, *elem);
        }

        return acc;
    }

    /// Reduces the iterator to a single accumulated value.
    ///
    /// Equivalent to Rust's [`Iterator::fold()`].
    /// [`Iterator::fold()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.fold
    ///
    /// @param init The initial accumulator value
    /// @param fun callable that combines the accumulator with each iterator item
    /// @returns the final accumulated value.
    template<typename Acc, typename Fun>
        requires(callable<Fun, const Acc&, iter::TypeOf<Impl>>
            && std::convertible_to<std::invoke_result_t<Fun, const Acc&, iter::TypeOf<Impl>>, Acc>)
    auto Fold(Acc init, Fun&& fun) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), init, std::declval<iter::TypeOf<Impl>>())))
    {
        auto acc = VIOLET_MOVE(init);
        while (auto elem = getThisObject().Next()) {
            acc = std::invoke(VIOLET_FWD(Fun, fun), acc, *elem);
        }

        return acc;
    }

    /// Reduces a double-ended iterator from the back.
    ///
    /// Equivalent to Rust's [`Iterator::rfold()`].
    /// [`Iterator::rfold()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.rfold
    template<typename Acc, typename Fun>
        requires(DoubleEndedIterable<Impl> && callable<Fun, const Acc&, iter::TypeOf<Impl>>
            && std::convertible_to<std::invoke_result_t<Fun, const Acc&, iter::TypeOf<Impl>>, Acc>)
    auto RFold(Acc init, Fun&& fun) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), init, std::declval<iter::TypeOf<Impl>>())))
    {
        auto acc = VIOLET_MOVE(init);
        while (auto elem = getThisObject().NextBack()) {
            acc = std::invoke(VIOLET_FWD(Fun, fun), acc, *elem);
        }

        return acc;
    }

    /// Reduces a double-ended iterator from the back.
    ///
    /// Equivalent to Rust's [`Iterator::rfold()`].
    /// [`Iterator::rfold()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.rfold
    template<typename Acc, typename Fun>
        requires(DoubleEndedIterable<Impl> && callable<Fun, const Acc&, iter::TypeOf<Impl>>
            && std::convertible_to<std::invoke_result_t<Fun, const Acc&, iter::TypeOf<Impl>>, Acc>)
    auto RFold(Acc init, Fun&& fun) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Fun, fun), init, std::declval<iter::TypeOf<Impl>>())))
    {
        auto acc = VIOLET_MOVE(init);
        while (auto elem = getThisObject().NextBack()) {
            acc = std::invoke(VIOLET_FWD(Fun, fun), acc, *elem);
        }

        return acc;
    }

    /// Returns the position of the first item satisfying a predicate.
    ///
    /// Equivalent to Rust's [`Iterator::position()`].
    /// [`Iterator::position()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.position
    ///
    /// @returns Optional index of the item, or `Nothing` if no item matches.
    template<typename Pred>
    auto Position(Pred&& pred) & noexcept(
        noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>()))) -> Optional<UInt>
    {
        UInt pos = 0;
        while (auto value = getThisObject().Next()) {
            if (std::invoke(VIOLET_FWD(Pred, pred), *value)) {
                return Some<UInt>(pos);
            }

            pos++;
        }

        return Nothing;
    }

    /// Returns the position of the first item satisfying a predicate.
    ///
    /// Equivalent to Rust's [`Iterator::position()`].
    /// [`Iterator::position()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.position
    ///
    /// @returns Optional index of the item, or `Nothing` if no item matches.
    template<typename Pred>
    auto Position(Pred&& pred) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>()))) -> Optional<UInt>
    {
        UInt pos = 0;
        while (auto value = getThisObject().Next()) {
            if (std::invoke(VIOLET_FWD(Pred, pred), *value)) {
                return Some<UInt>(pos);
            }

            pos++;
        }

        return Nothing;
    }

    /// Finds the first item that satisfies the given predicate.
    ///
    /// Equivalent to Rust's [`Iterator::find()`].
    /// [`Iterator::find()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.find
    template<typename Pred>
        requires(callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>)
    auto Find(Pred&& pred) & noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>())))
    {
        while (auto value = getThisObject().Next()) {
            if (std::invoke(VIOLET_FWD(Pred, pred), *value)) {
                return value;
            }
        }

        return decltype(Optional<iter::TypeOf<Impl>>{})(Nothing);
    }

    /// Finds the first item that satisfies the given predicate.
    ///
    /// Equivalent to Rust's [`Iterator::find()`].
    /// [`Iterator::find()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.find
    template<typename Pred>
        requires(callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>)
    auto Find(Pred&& pred) && noexcept(
        noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>())))
    {
        while (auto value = getThisObject().Next()) {
            if (std::invoke(VIOLET_FWD(Pred, pred), *value)) {
                return value;
            }
        }

        return decltype(Optional<iter::TypeOf<Impl>>{})(Nothing);
    }

    /// Applies a function that returns an `Optional` and returns the first `Some` value.
    ///
    /// Equivalent to Rust's [`Iterator::find_map()`].
    /// [`Iterator::find_map()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.find_map
    template<typename Fun>
        requires(callable<Fun, iter::TypeOf<Impl>>
            && iter::detail::is_optional_v<std::invoke_result_t<Fun, iter::TypeOf<Impl>>>)
    auto FindMap(Fun&& fun) & noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<iter::TypeOf<Impl>>())))
    {
        using Ret = std::invoke_result_t<Fun, iter::TypeOf<Impl>>;
        using U = iter::detail::optional_type_t<Ret>;

        while (auto value = getThisObject().Next()) {
            if (Optional<U> mapped = std::invoke(VIOLET_FWD(Fun, fun), *value)) {
                return Some<U>(*mapped);
            }
        }

        return decltype(Optional<U>())(Nothing);
    }

    /// Applies a function that returns an `Optional` and returns the first `Some` value.
    ///
    /// Equivalent to Rust's [`Iterator::find_map()`].
    /// [`Iterator::find_map()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.find_map
    template<typename Fun>
        requires(callable<Fun, iter::TypeOf<Impl>>
            && iter::detail::is_optional_v<std::invoke_result_t<Fun, iter::TypeOf<Impl>>>)
    auto FindMap(Fun&& fun) && noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<iter::TypeOf<Impl>>())))
    {
        using Ret = std::invoke_result_t<Fun, iter::TypeOf<Impl>>;
        using U = iter::detail::optional_type_t<Ret>;

        while (auto value = getThisObject().Next()) {
            if (Optional<U> mapped = std::invoke(VIOLET_FWD(Fun, fun), *value)) {
                return Some<U>(*mapped);
            }
        }

        return decltype(Optional<U>())(Nothing);
    }

    /// Returns true if any element matches the predicate.
    ///
    /// Equivalent to Rust's [`Iterator::any()`].
    /// [`Iterator::any()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.any
    template<typename Pred>
        requires(callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>)
    auto Any(Pred&& pred) & noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>())))
    {
        while (auto value = getThisObject().Next()) {
            if (std::invoke(VIOLET_FWD(Pred, pred), *value))
                return true;
        }

        return false;
    }

    /// Returns true if any element matches the predicate.
    ///
    /// Equivalent to Rust's [`Iterator::any()`].
    /// [`Iterator::any()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.any
    template<typename Pred>
        requires(callable<Pred, iter::TypeOf<Impl>> && callable_returns<Pred, bool, iter::TypeOf<Impl>>)
    auto Any(Pred&& pred) && noexcept(noexcept(std::invoke(VIOLET_FWD(Pred, pred), std::declval<iter::TypeOf<Impl>>())))
    {
        while (auto value = getThisObject().Next()) {
            if (std::invoke(VIOLET_FWD(Pred, pred), *value))
                return true;
        }

        return false;
    }

    /// Inspects each item of the iterator by calling a function with a side effect.
    ///
    /// Equivalent to Rust's [`Iterator::inspect()`].
    /// [`Iterator::inspect()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.inspect
    template<typename Fun>
        requires(callable<Fun, iter::TypeOf<Impl>> && callable_returns<Fun, void, iter::TypeOf<Impl>>)
    auto Inspect(Fun&& fun) & noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<iter::TypeOf<Impl>>())))
    {
        while (auto value = getThisObject().Next()) {
            std::invoke(VIOLET_FWD(Fun, fun), *value);
        }

        return *this;
    }

    /// Inspects each item of the iterator by calling a function with a side effect.
    ///
    /// Equivalent to Rust's [`Iterator::inspect()`].
    /// [`Iterator::inspect()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.inspect
    template<typename Fun>
        requires(callable<Fun, iter::TypeOf<Impl>> && callable_returns<Fun, void, iter::TypeOf<Impl>>)
    auto Inspect(Fun&& fun) && noexcept(noexcept(std::invoke(VIOLET_FWD(Fun, fun), std::declval<iter::TypeOf<Impl>>())))
    {
        while (auto value = getThisObject().Next()) {
            std::invoke(VIOLET_FWD(Fun, fun), *value);
        }

        return *this;
    }

    /// Counts the number of remaining elements.
    ///
    /// Equivalent to Rust's [`Iterator::count()`].
    ///
    /// [`Iterator::count()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.count
    auto Count() & noexcept -> UInt
    {
        UInt counter = 0;
        while (auto _ = getThisObject().Next()) { // NOLINT(readability-identifier-length)
            counter++;
        }

        return counter;
    }

    /// Counts the number of remaining elements.
    ///
    /// Equivalent to Rust's [`Iterator::count()`].
    ///
    /// [`Iterator::count()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.count
    auto Count() && noexcept -> UInt
    {
        UInt counter = 0;
        while (auto _ = getThisObject().Next()) { // NOLINT(readability-identifier-length)
            counter++;
        }

        return counter;
    }

    /// Advances the iterator by `nth` elements.
    ///
    /// Equivalent to Rust's [`Iterator::advance_by()`].
    ///
    /// [`Iterator::advance_by()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.advance_by
    auto AdvanceBy(UInt nth) & noexcept -> Result<void, UInt>
    {
        for (UInt i = 0; i < nth; i++) {
            if (!getThisObject().Next()) {
                return Err(i);
            }
        }

        return {};
    }

    /// Advances the iterator by `nth` elements.
    ///
    /// Equivalent to Rust's [`Iterator::advance_by()`].
    ///
    /// [`Iterator::advance_by()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.advance_by
    auto AdvanceBy(UInt nth) && noexcept -> Result<void, UInt>
    {
        for (UInt i = 0; i < nth; i++) {
            if (!getThisObject().Next()) {
                return Err(i);
            }
        }

        return {};
    }

    /// Returns the `nth` element of the iterator.
    ///
    /// Equivalent to Rust's [`Iterator::nth()`].
    ///
    /// [`Iterator::nth()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.nth
    auto Nth(UInt nth) & noexcept
    {
        if (this->AdvanceBy(nth)) {
            return getThisObject().Next();
        }

        return decltype(getThisObject().Next())(Nothing);
    }

    /// Returns the `nth` element of the iterator.
    ///
    /// Equivalent to Rust's [`Iterator::nth()`].
    ///
    /// [`Iterator::nth()`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html#method.nth
    auto Nth(UInt nth) && noexcept
    {
        if (this->AdvanceBy(nth)) {
            return getThisObject().Next();
        }

        return decltype(getThisObject().Next())(Nothing);
    }

    /// Collects the iterator elements into a container.
    template<typename Container>
    auto Collect()
    {
        if constexpr (collectable<Container, iter::TypeOf<Impl>>) {
            Container out;
            while (auto value = getThisObject().Next()) {
                if constexpr (requires { out.push_back(VIOLET_MOVE(*value)); }) {
                    out.push_back(VIOLET_MOVE(*value));
                } else {
                    out.insert(out.end(), VIOLET_MOVE(*value));
                }
            }

            return out;
        } else if constexpr (requires { typename Container::value_type{}; } && (std::tuple_size_v<Container>) > 0) {
            constexpr UInt N = std::tuple_size_v<Container>; // NOLINT(readability-identifier-length)

            Container out{};
            UInt idx = 0;
            while (auto value = getThisObject().Next()) {
                if (idx >= N) {
                    break;
                }

                out[idx++] = VIOLET_MOVE(*value);
            }

            return out;
        } else {
            static_assert([] -> auto { return false; }(), "unsupported container type");
        }
    }

    /// Returns the lower and upper bound size hints for the iterator.
    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        if constexpr (requires { getThisObject().SizeHint(); }) {
            return getThisObject().SizeHint();
        }

        return {};
    }

    auto begin()
    {
        return iter::detail::STLRangeIterator(getThisObject());
    }

    auto end()
    {
        return iter::detail::sentinel{};
    }

private:
    constexpr auto getThisObject() & noexcept -> Impl&
    {
        return static_cast<Impl&>(*this);
    }

    constexpr auto getThisObject() && noexcept -> Impl&&
    {
        return static_cast<Impl&&>(*this);
    }
};

/// Adapts a container into a Violet iterator.
///
/// Backed by a `STLCompatibleIterator`, this allows bridging existing STL
/// containers into Violet's iterator API.
///
/// # Examples
///
/// ```cpp
/// std::vector<int> v{1,2,3};
/// auto it = MkIterable(v);
///
/// for (auto x : it) { /* ... */ }
/// ```
template<typename Container>
auto MkIterable(const Container& cnt)
{
    return iter::detail::STLCompatibleIterator(std::begin(cnt), std::end(cnt));
}

/// Adapts a container into a Violet iterator.
///
/// Backed by a `STLCompatibleIterator`, this allows bridging existing STL
/// containers into Violet's iterator API.
///
/// # Examples
///
/// ```cpp
/// std::vector<int> v{1,2,3};
/// auto it = MkIterable(v);
///
/// for (auto x : it) { /* ... */ }
/// ```
template<typename Container>
auto MkIterable(Container&& cnt) // NOLINT
{
    return iter::detail::STLCompatibleIterator(std::begin(cnt), std::end(cnt));
}

} // namespace violet
