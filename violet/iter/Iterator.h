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
#include "violet/container/Result.h"
#include "violet/violet.h"

#include <concepts>
#include <type_traits>

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

    template<typename It>
    struct STLCompatIterable final: public Iterator<STLCompatIterable<It>> {
        using Item = std::remove_cv_t<std::remove_reference_t<typename std::iter_value_t<It>>>;

        explicit(false) STLCompatIterable(It begin, It end)
            : n_current(begin)
            , n_end(end)
        {
        }

        auto Next() -> Optional<Item>
        {
            return this->n_current == this->n_end ? Nothing : *this->n_current++;
        }

    private:
        It n_current, n_end;
    };

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

} // namespace detail

template<typename T>
concept Iterable = detail::ValidIterable<std::remove_cvref_t<T>>;

template<typename T>
concept DoubleEndedIterable = detail::ValidDoubleEndedIterable<std::remove_cvref_t<T>>;

template<typename T>
using IterableType = detail::optional_type_t<decltype(std::declval<T>().Next())>;

namespace Iterators::Adapters {

    template<Iterable Impl, typename Fun>
        requires Callable<Fun, IterableType<Impl>>
    struct Map;

    template<Iterable Impl, typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    struct Filter;

    template<Iterable Impl>
    struct Peekable;

    template<Iterable Impl>
    struct Enumerate;

} // namespace Iterators::Adapters

/// The bound on the remaining length of a [`Iterator`].
struct SizeHint final {
    usize Low = 0;
    Optional<usize> High = Nothing;

    SizeHint() = default;
    explicit(false) SizeHint(usize low, const Optional<usize>& hi = {})
        : Low(low)
        , High(hi)
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
    /// Returns **true** if this iterator is considered "double-ended"
    constexpr static auto IsDoubleEnded = DoubleEndedIterable<Impl>;

    auto Next() & noexcept
        requires Iterable<Impl>
    {
        return getThisObject().Next();
    }

    auto Next() && noexcept
        requires Iterable<Impl>
    {
        return getThisObject().Next();
    }

    auto NextBack() & noexcept
        requires DoubleEndedIterable<Impl>
    {
        return getThisObject().NextBack();
    }

    auto NextBack() && noexcept
        requires DoubleEndedIterable<Impl>
    {
        return getThisObject().NextBack();
    }

    [[nodiscard]] auto SizeHint() const& noexcept -> Noelware::Violet::SizeHint
    {
        // clang-format off
        if constexpr (requires(Impl impl) {
            { impl.SizeHint() } -> std::same_as<Noelware::Violet::SizeHint>;
        }) {
            // clang-format on
            return getThisObject().SizeHint();
        }

        return {};
    }

    [[nodiscard]] auto SizeHint() const&& noexcept -> Noelware::Violet::SizeHint
    {
        // clang-format off
        if constexpr (requires(Impl impl) {
            { impl.SizeHint() } -> std::same_as<Noelware::Violet::SizeHint>;
        }) {
            // clang-format on
            return getThisObject().SizeHint();
        }

        return {};
    }

    auto Peekable() & noexcept
    {
        return Iterators::Adapters::Peekable<Impl>(getThisObject());
    }

    auto Peekable() && noexcept
    {
        return Iterators::Adapters::Peekable<Impl>(VIOLET_MOVE(getThisObject()));
    }

    template<typename Fun>
        requires Callable<Fun, IterableType<Impl>>
    auto Map(Fun&& fun) & noexcept
    {
        return Iterators::Adapters::Map<Impl, Fun>(getThisObject(), VIOLET_FWD(Fun, fun));
    }

    template<typename Fun>
        requires Callable<Fun, IterableType<Impl>>
    auto Map(Fun&& fun) && noexcept
    {
        return Iterators::Adapters::Map<Impl, Fun>(VIOLET_MOVE(getThisObject()), VIOLET_FWD(Fun, fun));
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Filter(Pred pred) & noexcept
    {
        return Iterators::Adapters::Filter<Impl, Pred>(getThisObject(), VIOLET_MOVE(pred));
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Filter(Pred pred) && noexcept
    {
        return Iterators::Adapters::Filter<Impl, Pred>(VIOLET_MOVE(getThisObject()), VIOLET_MOVE(pred));
    }

    auto Enumerate() & noexcept
    {
        return Iterators::Adapters::Enumerate<Impl>(getThisObject());
    }

    auto Enumerate() && noexcept
    {
        return Iterators::Adapters::Enumerate<Impl>(VIOLET_MOVE(getThisObject()));
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Any(Pred pred) & -> bool
    {
        while (auto value = this->Next()) {
            if (std::invoke(pred, *value)) {
                return true;
            }
        }

        return false;
    }

    template<typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    auto Any(Pred pred) && -> bool
    {
        while (auto value = this->Next()) {
            if (std::invoke(pred, *value)) {
                return true;
            }
        }

        return false;
    }

    auto Count() & -> usize
    {
        usize counter = 0;
        while (auto _ = this->Next()) // NOLINT(readability-identifier-length)
            counter++;

        return counter;
    }

    auto Count() && -> usize
    {
        usize counter = 0;
        while (auto _ = this->Next()) // NOLINT(readability-identifier-length)
            counter++;

        return counter;
    }

    auto AdvanceBy(usize nth) & -> Result<void, usize>
    {
        for (usize i = 0; i < nth; i++) {
            if (!this->Next()) {
                return Err(nth - 1);
            }
        }

        return {};
    }

    auto AdvanceBy(usize nth) && -> Result<void, usize>
    {
        for (usize i = 0; i < nth; i++) {
            if (!this->Next()) {
                return Err(nth - 1);
            }
        }

        return {};
    }

    auto Nth(usize nth) &
    {
        return this->AdvanceBy(nth) ? this->Next() : decltype(this->Next())(Nothing);
    }

    auto Nth(usize nth) &&
    {
        return this->AdvanceBy(nth) ? this->Next() : decltype(this->Next())(Nothing);
    }

private:
    auto getThisObject() -> Impl&
    {
        return static_cast<Impl&>(*this);
    }
};

namespace Iterators::Adapters {

    template<Iterable Impl, typename Fun>
        requires Callable<Fun, IterableType<Impl>>
    struct Map final: public Iterator<Map<Impl, Fun>> {
        using Item = std::invoke_result_t<Fun, IterableType<Impl>>;

        auto Next() -> Optional<Item>
        {
            if (auto value = this->n_iter.Next()) {
                return std::invoke(this->n_fun, *value);
            }

            return Nothing;
        }

        auto NextBack() -> Optional<Item>
            requires DoubleEndedIterable<Impl>
        {
            if (auto value = this->n_iter.NextBack()) {
                return std::invoke(this->n_fun, *value);
            }

            return Nothing;
        }

        [[nodiscard]] auto SizeHint() const noexcept -> Noelware::Violet::SizeHint
        {
            // clang-format off
            if constexpr (requires(Impl impl) {
                { impl.SizeHint() } -> std::same_as<Noelware::Violet::SizeHint>;
            }) {
                // clang-format on
                return this->n_iter.SizeHint();
            }

            return {};
        }

    private:
        friend struct Noelware::Violet::Iterator<Impl>;

        explicit Map(Impl iterator, Fun func)
            : n_iter(iterator)
            , n_fun(func)
        {
        }

        Impl n_iter;
        Fun n_fun;
    };

    template<Iterable Impl, typename Pred>
        requires Callable<Pred, IterableType<Impl>> && CallableShouldReturn<Pred, bool, IterableType<Impl>>
    struct Filter final: public Iterator<Filter<Impl, Pred>> {
        using Item = IterableType<Impl>;

        auto Next() -> Optional<Item>
        {
            while (auto value = this->n_iter.Next()) {
                if (std::invoke(this->n_predicate, *value)) {
                    return value;
                }
            }

            return Nothing;
        }

        auto NextBack() -> Optional<Item>
            requires DoubleEndedIterable<Impl>
        {
            while (auto value = this->n_iter.Next()) {
                if (std::invoke(this->n_predicate, *value)) {
                    return value;
                }
            }

            return Nothing;
        }

        [[nodiscard]] auto SizeHint() const noexcept -> Noelware::Violet::SizeHint
        {
            // clang-format off
            if constexpr (requires(Impl impl) {
                { impl.SizeHint() } -> std::same_as<Noelware::Violet::SizeHint>;
            }) {
                // clang-format on
                Noelware::Violet::SizeHint hint = this->n_iter.SizeHint();
                return { 0, hint.High };
            }

            return {};
        }

    private:
        friend struct Noelware::Violet::Iterator<Impl>;

        explicit(false) Filter(Impl iterator, Pred pred)
            : n_iter(iterator)
            , n_predicate(pred)
        {
        }

        Impl n_iter;
        Pred n_predicate;
    };

    template<Iterable Impl>
    struct Peekable final: public Iterator<Peekable<Impl>> {
        using Item = IterableType<Impl>;

        auto Next() -> Optional<Item>
        {
            if (this->n_peeked) {
                return this->n_peeked.Take();
            }

            return this->n_iter.Next();
        }

        auto Count() & -> usize
        {
            auto val = this->n_peeked.Take();
            if (!val)
                return this->n_iter.Count();

            Optional<Item>& value = val.Value();
            if (!value) {
                return 0;
            }

            return 1 + this->n_iter.Count();
        }

        auto Count() && -> usize
        {
            auto val = this->n_peeked.Take();
            if (!val)
                return this->n_iter.Count();

            Optional<Item>& value = val.Value();
            if (!value) {
                return 0;
            }

            return 1 + this->n_iter.Count();
        }

        auto Nth(usize nth) &
        {
            auto val = this->n_peeked.Take();
            if (!val)
                return this->n_iter.Nth(nth);

            Optional<Item>& value = val.Value();
            if (!value) {
                return decltype(this->n_iter.Next())(Nothing);
            }

            const usize& elem = value.Value();
            return elem == 0 ? Some<usize>(elem) : this->n_iter.Nth(nth - 1);
        }

        auto Nth(usize nth) &&
        {
            auto val = this->n_peeked.Take();
            if (!val)
                return this->n_iter.Nth(nth);

            Optional<Item>& value = val.Value();
            if (!value) {
                return decltype(this->n_iter.Next())(Nothing);
            }

            const usize& elem = value.Value();
            return elem == 0 ? Some<usize>(elem) : this->n_iter.Nth(nth - 1);
        }

    private:
        friend struct Noelware::Violet::Iterator<Impl>;

        explicit Peekable(Impl iterator)
            : n_iter(iterator)
        {
        }

        Impl n_iter;
        Optional<Optional<Item>> n_peeked = Nothing;
    };

    template<Iterable Impl>
    struct Enumerate final: public Iterator<Enumerate<Impl>> {
        using Item = Pair<usize, IterableType<Impl>>;

        auto Next() -> Optional<Item>
        {
            if (auto item = this->n_iter.Next()) {
                usize count = this->n_idx;
                this->n_idx++;

                return std::make_pair(count, *item);
            }

            return Nothing;
        }

        auto Nth(usize nth) &
        {
            if (auto item = this->n_iter.Nth(nth)) {
                usize count = this->n_idx + nth;
                this->n_idx = count + 1;

                return std::make_pair(count, *item);
            }

            return Nothing;
        }

        auto Nth(usize nth) &&
        {
            if (auto item = this->n_iter.Nth(nth)) {
                usize count = this->n_idx + nth;
                this->n_idx = count + 1;

                return std::make_pair(count, *item);
            }

            return Nothing;
        }

        [[nodiscard]] auto SizeHint() const noexcept -> Noelware::Violet::SizeHint
        {
            // clang-format off
            if constexpr (requires(Impl impl) {
                { impl.SizeHint() } -> std::same_as<Noelware::Violet::SizeHint>;
            }) {
                // clang-format on
                Noelware::Violet::SizeHint hint = this->n_iter.SizeHint();
                return { 0, hint.High };
            }

            return {};
        }

    private:
        friend struct Noelware::Violet::Iterator<Impl>;

        explicit Enumerate(Impl iterator)
            : n_iter(iterator)
        {
        }

        Impl n_iter;
        usize n_idx = 0;
    };

} // namespace Iterators::Adapters

template<typename Container>
auto MkIterable(Container& cnt)
{
    return detail::STLCompatIterable(std::begin(cnt), std::end(cnt));
}

template<typename Container>
auto MkIterable(Container&& cnt)
{
    return detail::STLCompatIterable(std::begin(cnt), std::end(cnt));
}

} // namespace Noelware::Violet
