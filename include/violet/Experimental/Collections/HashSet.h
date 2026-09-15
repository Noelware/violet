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
//
//! # 🌺💜 `violet/Experimental/Collections/HashSet.h`
//! This module provides `HashSet`: A Rust-inspired and flavoured hash set built on top of either
//! Abseil's `flat_hash_set` (SwissTable, more aligned with Rust's implementation) when the
//! [Abseil feature](#) is enabled, or `std::unordered_set` otherwise. Its API mirrors Rust's
//! `std::collections::HashSet`, including `Optional`-based fallible operations and set-algebra
//! helpers (`Disjointed`, `SubsetOf`, `SupersetOf`).

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Iterator.h>

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
#include "absl/container/flat_hash_set.h"
#else
#include <unordered_set>
#endif

#include <memory>

namespace violet::experimental {
namespace hashset {
#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
/// Default hasher used by [`HashSet`].
template<typename T>
using default_hasher = absl::DefaultHashContainerHash<T>;

/// Default key-equality predicate used by [`HashSet`].
template<typename T>
using default_equality = absl::DefaultHashContainerEq<T>;

/// Erases all elements from a container matching a predicate
using absl::erase_if;
#else
/// Default hasher used by [`HashSet`].
template<typename T>
using default_hasher = std::hash<T>;

/// Default key-equality predicate used by [`HashSet`].
template<typename T>
using default_equality = std::equal_to<T>;

/// Erases all elements from a container matching a predicate
using std::erase_if;
#endif
} // namespace hashset

/// A set-based type backed by Abseil's SwissTable or C++'s native [`std::unordered_set`] with the API of Rust's
/// [`std::collections::HashSet`] or the [`hashbrown` crate].
///
/// [`std::collections::HashSet`]: https://doc.rust-lang.org/stable/std/collections/struct.HashSet.html
/// [`hashbrown` crate]: https://docs.rs/hashbrown/latest/hashbrown/
///
/// ## Differences
/// - Using `violet::Optional<T>` for fallible operations instead of iterator-based signaling.
/// - Move-based insertion (`Insert` consumes).
/// - `Replace` returns the old element when one was present, mirroring Rust's `HashSet::replace`.
/// - `Take` removes and returns the stored element equivalent to a query value.
/// - Uses Violet's iterator framework for pluggable combinators for iteration.
//
/// ## Examples
/// ```cpp
/// #include <violet/Experimental/Collections/HashSet.h>
///
/// using namespace violet::experimental;
/// using namespace violet;
///
/// void ProcessFirstSight(Str word) {
///     /* ... */
/// }
///
/// HashSet<String> seen;
/// for (const auto& word: words) {
///     if (seen.Insert(word)) {
///         ProcessFirstSight(word);
///     }
/// }
/// ```
template<typename T, typename Hasher = hashset::default_hasher<T>, typename Equality = hashset::default_equality<T>,
    typename Alloc = std::allocator<T>>
struct NOELDOC_EXPERIMENTAL_SINCE("current") HashSet final {
#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
    /// Underlying implementation: `absl::flat_hash_map`
    using implementation = absl::flat_hash_set<T, Hasher, Equality, Alloc>;
#else
    /// Underlying implementation: `std::unordered_map`
    using implementation = std::unordered_set<T, Hasher, Equality, Alloc>;
#endif

    /// The value type, `T`.
    using value_type = T;

    /// The unsigned integer type used for sizes and capacities.
    using size_type = UInt;

    /// A mutable iterator over `(key, value)` pairs, delegating to [`implementation::iterator`].
    using iterator = typename implementation::iterator;

    /// A read-only iterator over `(key, value)` pairs, delegating to [`implementation::const_iterator`].
    using const_iterator = typename implementation::const_iterator;

    /// The type returned by wrapping [`implementation`] in Violet's iterator framework, used
    /// to expose combinator-based iteration over this map.
    using underlying_iterator = decltype(violet::MkIterable(std::declval<implementation&>()));

    /// Construct an empty `HashSet`.
    VIOLET_IMPLICIT HashSet() noexcept(std::is_nothrow_default_constructible_v<implementation>) = default;

    /// Construct a `HashSet` from an initializer list of value pairs.
    VIOLET_IMPLICIT HashSet(std::initializer_list<T> il) noexcept(
        std::is_nothrow_constructible_v<implementation, decltype(il)>)
        : n_impl(il)
    {
    }

    /// Constructor that forwards `args` directly to the underlying implementation's constructor.
    template<typename... Args>
        requires constructible<implementation, Args...>
    VIOLET_IMPLICIT HashSet(Args&&... args) noexcept(std::is_nothrow_constructible_v<implementation, Args...>)
        : n_impl(implementation(VIOLET_FWD(Args, args)...))
    {
    }

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
    /// Returns the number of elements the set can hold without triggering a rehash.
    auto Capacity() const noexcept(noexcept(std::declval<implementation>().capacity())) -> size_type
    {
        return this->n_impl.capacity();
    }
#endif

    /// Returns the number of elements currently stored in the set.
    [[nodiscard]] auto Size() const noexcept(noexcept(std::declval<implementation>().size())) -> size_type
    {
        return this->n_impl.size();
    }

    /// Returns **true** if the set contains no elements.
    [[nodiscard]] auto Empty() const noexcept(noexcept(std::declval<implementation>().empty())) -> bool
    {
        return this->n_impl.empty();
    }

    /// Reserves capacity for at least `additional` more elements without reallocating.
    void Reserve(size_type additional) noexcept(
        noexcept(std::declval<implementation>().reserve(std::declval<size_type>())))
    {
        this->n_impl.reserve(this->Size() + additional);
    }

    /// Shrinks the set's backing storage to fit its current contents as closely as possible.
    void ShrinkToFit() noexcept(noexcept(std::declval<implementation>().rehash(0)))
    {
        return this->n_impl.rehash(0);
    }

    /// Removes every element from the set.
    void Clear() noexcept(noexcept(std::declval<implementation>().clear()))
    {
        this->n_impl.clear();
    }

    /// Returns **true** if the set contains an element equivalent to `value`.
    /// @tparam Q any type comparable with `T`, which enables heterogeneous (transparent) lookup
    template<typename Q>
    auto Contains(const Q& value) const -> bool
    {
        return this->n_impl.contains(value);
    }

    /// Returns a reference to the stored element equivalent to `value`, or [`Nothing`] if no
    /// such element is present.
    /// @tparam Q any type equality-comparable with `T`, which enables heterogeneous (transparent) lookup
    template<std::equality_comparable_with<T> Q>
    auto Get(const Q& value) const -> Optional<std::reference_wrapper<const T>>
    {
        auto it = this->n_impl.find(value);
        if (it == this->n_impl.end()) {
            return Nothing;
        }

        return std::cref(*it);
    }

    /// Inserts `value` into the set, returning **true** if it was newly inserted, or **false**
    /// if an equivalent element was already present (in which case `value` is discarded).
    auto Insert(T value) -> bool
    {
        return this->n_impl.insert(VIOLET_MOVE(value)).second;
    }

    /// Constructs a `T` from `args` and inserts it, as if by `Insert(T(args...))`.
    template<typename... Args>
        requires constructible<T, Args...>
    auto Emplace(Args&&... args) -> bool
    {
        return this->Insert(T(VIOLET_FWD(Args, args)...));
    }

    /// Inserts `value` into the set, returning the element it replaced if an equivalent one was
    /// already present, or [`Nothing`] otherwise. Mirrors Rust's `HashSet::replace`.
    auto Replace(T value) -> Optional<T>
    {
        auto it = this->n_impl.find(value);
        if (it == this->n_impl.end()) {
            this->n_impl.insert(VIOLET_MOVE(value));
            return Nothing;
        }

        auto node = this->n_impl.extract(it);
        T old = VIOLET_MOVE(node.value());
        this->n_impl.insert(VIOLET_MOVE(value));

        return VIOLET_MOVE(old);
    }

    /// Removes and returns the stored element equivalent to `value`, or [`Nothing`] if no such
    /// element is present.
    /// @tparam Q any type comparable with `T`, which enables heterogeneous (transparent) lookup
    template<typename Q>
    auto Take(const Q& value) -> Optional<T>
    {
        auto it = this->n_impl.find(value);
        if (it == this->n_impl.end()) {
            return Nothing;
        }

        auto node = this->n_impl.extract(it);
        return VIOLET_MOVE(node.value());
    }

    /// Removes the element equivalent to `value` from the set, returning **true** if one was
    /// present.
    /// @tparam Q any type comparable with `T`, which enables heterogeneous (transparent) lookup
    template<typename Q>
    auto Remove(const Q& value) -> bool
    {
        return this->n_impl.erase(value) > 0;
    }

    /// Removes every element for which `pred(value)` returns `false`, keeping the rest.
    template<typename Pred>
        requires(violet::callable<Pred, const T&> && violet::callable_returns<Pred, bool, const T&>)
    void Retain(Pred&& pred)
    {
        hashset::erase_if(this->n_impl, [fun = VIOLET_FWD(Pred, pred)](const auto& value) -> bool {
            bool result = std::invoke(fun, value);
            return !result;
        });
    }

    /// Removes every element for which `predicate(value)` returns `true`, passing each removed
    /// element to `sink` before it is destroyed.
    template<typename Pred, typename Sink>
        requires(violet::callable<Pred, const T&> && violet::callable_returns<Pred, bool, const T&>
            && violet::callable<Sink, T &&>)
    void ExtractIf(Pred predicate, Sink sink)
    {
        for (auto it = this->n_impl.begin(); it != this->n_impl.end();) {
            if (std::invoke(predicate, *it)) {
                auto node = this->n_impl.extract(it++);
                std::invoke(sink, VIOLET_MOVE(node.value()));
            } else {
                ++it;
            }
        }
    }

    /// Returns **true** if `this` and `other` share no elements in common.
    template<typename H2, typename E2, typename A2>
    auto Disjointed(const HashSet<T, H2, E2, A2>& other) const -> bool
    {
        const auto& [smaller, larger] = this->Size() <= other.Size()
            ? Pair<const HashSet&, const HashSet<T, H2, E2, A2>&>(*this, other)
            : Pair<const HashSet&, const HashSet<T, H2, E2, A2>&>(other, *this);

        for (const auto& value: smaller.n_impl) {
            if (larger.n_impl.contains(value)) {
                return false;
            }
        }

        return true;
    }

    /// Returns **true** if every element of `this` is also present in `other`.
    template<typename H2, typename E2, typename A2>
    auto SubsetOf(const HashSet<T, H2, E2, A2>& other) const -> bool
    {
        if (this->Size() > other.Size()) {
            return false;
        }

        for (const auto& value: this->n_impl) {
            if (!other.n_impl.contains(value)) {
                return false;
            }
        }

        return true;
    }

    /// Returns **true** if every element of `other` is also present in `this`.
    template<typename H2, typename E2, typename A2>
    auto SupersetOf(const HashSet<T, H2, E2, A2>& other) const -> bool
    {
        return other.SubsetOf(*this);
    }

    [[nodiscard]] auto begin() noexcept -> iterator
    {
        return this->n_impl.begin();
    }

    [[nodiscard]] auto end() noexcept -> iterator
    {
        return this->n_impl.end();
    }

    [[nodiscard]] auto begin() const noexcept -> const_iterator
    {
        return this->n_impl.begin();
    }

    [[nodiscard]] auto end() const noexcept -> const_iterator
    {
        return this->n_impl.end();
    }

private:
    implementation n_impl{};
};

} // namespace violet::experimental
