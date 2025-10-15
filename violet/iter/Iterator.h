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
//! # 🌺💜 `violet/iter/Iterator.h`

#pragma once

#include "violet/container/Optional.h"
#include "violet/container/Result.h"
#include "violet/violet.h"

#include <type_traits>
#include <utility>

namespace Noelware::Violet {

template<class Impl>
struct Iterator;

namespace detail {
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

    template<typename It>
    struct StlCompatibleIterator final: public Iterator<StlCompatibleIterator<It>> {
        using Item = std::remove_cv_t<std::remove_reference_t<typename std::iter_value_t<It>>>;

        StlCompatibleIterator(It begin, It end)
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
        It n_current, n_end;
    };
} // namespace detail

/// C++20 concept that ensures `T` implements the iteration protocol.
template<typename T>
concept Iterable = detail::ValidIterable<std::remove_cvref_t<T>>;

/// C++20 concept that ensures `T` implements the double-ended iteration protocol.
template<typename T>
concept DoubleEndedIterable = detail::ValidDoubleEndedIterable<std::remove_cvref_t<T>>;

/// Extracts the type that is returned of a iterator's `Next()` method.
template<typename T>
using IterableType = detail::optional_type_t<decltype(std::declval<T>().Next())>;

template<typename T, typename Item>
concept Collectable = requires(T& ty, Item value) {
    { ty.insert(ty.end(), value) } -> std::same_as<typename T::iterator>;
} || requires(T& cnt, Item value) {
    { cnt.push_back(value) } -> std::same_as<void>;
};

namespace detail {
    /// A sentinel object that is used to end iteration for a [`Iterator`](Noelware::Violet::Iterator)'s
    /// implementation of C++ range syntax.
    struct sentinel final {};

    template<class Impl>
    struct range_iter {
        range_iter() = delete;
        range_iter(Impl impl)
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
        Optional<IterableType<Impl>> n_current = Nothing;
    };
} // namespace detail

/// Provides a way to define lower and upper bound hints for iterator size.
struct SizeHint final {
    usize Low = 0; ///< Minimum number of elements that are expected.
    Optional<usize> High = Nothing; ///< An optional upper bound.

    constexpr SizeHint() = default;
    constexpr VIOLET_EXPLICIT SizeHint(usize lo, Optional<usize> hi = Nothing)
        : Low(lo)
        , High(VIOLET_MOVE(hi))
    {
    }
};

/// A CTRP-based trait class that allows ergnomic iteration of objects, inspired by Rust's
/// [`std::iter::Iterator`] trait.
///
/// [`std::iter::Iterator`]: https://doc.rust-lang.org/1.90.0/std/iter/trait.Iterator.html
///
/// This was made after learning the atrocious concept that is C++ iterators and ranges. They suck,
/// so I made my own, and I think it's beautiful.
///
/// @tparam Impl The implementation class.
template<class Impl>
struct Iterator {
    /// Extends this [`Iterator`] by allowing peeking through the elements
    /// of this iterator by using the [`Peekable::Peek`](Noelware::Violet::Iterators::Peekable#func-Peek) method.
    ///
    /// **NOTE**: You will need to include `violet/iter/adapters/Peekable.h` when using this method
    /// and add the `//violet/iter/adapters:peekable` dependency in your `cc_library`/`cc_binary`.
    ///
    /// ## Example
    /// ```cpp
    /// #include "violet/container/Optional.h"
    /// #include "violet/iter/adapters/Peekable.h"
    /// #include "violet/iter/Iterator.h"
    /// #include "violet/violet.h"
    ///
    /// # using namespace Noelware::Violet;
    /// #
    /// auto iter = MkIterable(Vec<usize> {1, 2, 3}).Peekable();
    ///
    /// // When peeking without advancing the iterator, it'll return the first
    /// // element right away.
    /// ASSERT_EQUALS(iter.Peek(), Some<usize>(1));
    /// ASSERT_EQUALS(iter.Next(), Some<usize>(1));
    /// ASSERT_EQUALS(iter.Next(), Some<usize>(2));
    ///
    /// // We can also use Peek() multiple times:
    ///
    /// // After the iterator is done producing elements, `Peek()`
    /// // will also return `Nothing`.
    /// ```
    auto Peekable() & noexcept;

    /// @doc{inherit=Noelware::Violet::Iterator::Peekable#lvalue-ref}
    auto Peekable() && noexcept;

    /// Produces a iterator that will include a index parameter to determine
    /// the position of the iterator.
    ///
    /// ## Notes
    /// - This iterator can use the pipe-style operation syntax, you don't need
    ///   to include any new headers.
    ///
    /// ## Remarks
    /// You will need to include the `violet/iter/adapters/Enumerate.h` header file and
    /// include the `//violet/iter/adapters:enumerate` dependency in your `cc_library`/`cc_binary`.
    ///
    /// ## Example
    /// ```cpp
    /// #include "violet/iter/adapters/Enumerate.h"
    /// #include "violet/iter/Iterator.h"
    /// #include "violet/violet.h"
    ///
    /// # using namespace Noelware::Violet;
    /// #
    /// auto iter = MkIterable(Vec<usize>{1, 2, 3}).Enumerate();
    /// ASSERT_EQUALS(iter.Next(), std::make_pair<usize, usize>(0, 1));
    /// ASSERT_EQUALS(iter.Next(), std::make_pair<usize, usize>(1, 2));
    /// ASSERT_EQUALS(iter.Next(), std::make_pair<usize, usize>(2, 3));
    /// ASSERT_EQUALS(iter.Next(), Nothing);
    /// ```
    auto Enumerate() & noexcept;

    /// @doc{inherit=Noelware::Violet::Iterator::Enumerate#lvalue-ref}
    auto Enumerate() && noexcept;

    template<typename Fun>
    auto Map(Fun fun) & noexcept;

    template<typename Fun>
    auto Map(Fun fun) && noexcept;

    template<typename Pred>
    auto Filter(Pred predicate) & noexcept;

    template<typename Pred>
    auto Filter(Pred predicate) && noexcept;

    template<typename Fun>
    auto FilterMap(Fun fun) & noexcept;

    template<typename Fun>
    auto FilterMap(Fun fun) && noexcept;

    auto Skip(usize skip) & noexcept;
    auto Skip(usize skip) && noexcept;

    auto Take(usize take) & noexcept;
    auto Take(usize take) && noexcept;

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Position(Pred pred) & noexcept -> Optional<usize>
    {
        usize pos = 0;
        while (auto value = getThisObject().Next()) {
            if (std::invoke(pred, *value)) {
                break;
            }

            pos++;
        }

        return pos == 0 ? Nothing : Some<usize>(pos);
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Position(Pred pred) && noexcept -> Optional<usize>
    {
        usize pos = 0;
        while (auto value = getThisObject().Next()) {
            if (std::invoke(pred, *value)) {
                break;
            }

            pos++;
        }

        return pos == 0 ? Nothing : Some<usize>(pos);
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Find(Pred pred) & noexcept
    {
        while (auto value = getThisObject().Next()) {
            if (std::invoke(pred, *value)) {
                return value;
            }
        }

        return Nothing;
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Find(Pred pred) && noexcept
    {
        while (auto value = getThisObject().Next()) {
            if (std::invoke(pred, *value)) {
                return value;
            }
        }

        return Nothing;
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Any(Pred pred) & noexcept
    {
        while (auto value = getThisObject().Next()) {
            if (std::invoke(pred, *value))
                return true;
        }

        return false;
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Any(Pred pred) && noexcept
    {
        while (auto value = getThisObject().Next()) {
            if (std::invoke(pred, *value))
                return true;
        }

        return false;
    }

    template<typename Fun>
        requires Callable<Fun, IterableType<Impl>> && CallableShouldReturn<Fun, void, IterableType<Impl>>
    auto Inspect(Fun fun) & noexcept -> Iterator<Impl>
    {
        while (auto value = getThisObject().Next()) {
            std::invoke(fun, *value);
        }

        return *this;
    }

    template<typename Fun>
        requires Callable<Fun, IterableType<Impl>> && CallableShouldReturn<Fun, void, IterableType<Impl>>
    auto Inspect(Fun fun) && noexcept -> Iterator<Impl>
    {
        while (auto value = getThisObject().Next()) {
            std::invoke(fun, *value);
        }

        return *this;
    }

    auto Count() & noexcept
    {
        usize counter = 0;
        while (auto _ = getThisObject().Next()) { // NOLINT(readability-identifier-length)
            counter++;
        }

        return counter;
    }

    auto Count() && noexcept
    {
        usize counter = 0;
        while (auto _ = getThisObject().Next()) { // NOLINT(readability-identifier-length)
            counter++;
        }

        return counter;
    }

    auto AdvanceBy(usize nth) & noexcept -> Result<void, usize>
    {
        for (usize i = 0; i < nth; i++) {
            if (!getThisObject().Next()) {
                return Err(i - 1);
            }
        }

        return {};
    }

    auto AdvanceBy(usize nth) && noexcept -> Result<void, usize>
    {
        for (usize i = 0; i < nth; i++) {
            if (!getThisObject().Next()) {
                return Err(i - 1);
            }
        }

        return {};
    }

    auto Nth(usize nth) & noexcept
    {
        if (this->AdvanceBy(nth)) {
            return getThisObject().Next();
        }

        return decltype(getThisObject().Next())(Nothing);
    }

    auto Nth(usize nth) && noexcept
    {
        if (this->AdvanceBy(nth)) {
            return getThisObject().Next();
        }

        return decltype(getThisObject().Next())(Nothing);
    }

    template<typename Container>
    auto Collect()
    {
        if constexpr (Collectable<Container, IterableType<Impl>>) {
            Container out;
            while (auto value = getThisObject().Next()) {
                if constexpr (requires { out.push_back(VIOLET_MOVE(*value)); }) {
                    out.push_back(VIOLET_MOVE(*value));
                } else {
                    out.insert(out.end(), VIOLET_MOVE(*value));
                }
            }
        } else if constexpr (requires { typename Container::value_type{}; }) {
            constexpr usize N = std::tuple_size_v<Container>; // NOLINT(readability-identifier-length)

            Container out{};
            usize idx = 0;
            while (auto value = getThisObject().Next()) {
                if (idx >= N) {
                    break;
                }

                out[idx++] = VIOLET_MOVE(*value);
            }

            return out;
        } else {
            static_assert([] { return false; }(), "unsupported container type");
        }
    }

    [[nodiscard]] constexpr auto SizeHint() const noexcept -> SizeHint
    {
        if constexpr (requires { getThisObject().SizeHint(); }) {
            return getThisObject().SizeHint();
        }

        return {};
    }

    auto begin()
    {
        return detail::range_iter(getThisObject());
    }

    auto end()
    {
        return detail::sentinel{};
    }

private:
    constexpr auto getThisObject() -> Impl&
    {
        return static_cast<Impl&>(*this);
    }
};

template<typename Container>
auto MkIterable(Container& cnt)
{
    return detail::StlCompatibleIterator(std::begin(cnt), std::end(cnt));
}

template<typename Container>
auto MkIterable(Container&& cnt)
{
    return detail::StlCompatibleIterator(std::begin(cnt), std::end(cnt));
}

} // namespace Noelware::Violet
