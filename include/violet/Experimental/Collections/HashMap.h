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
//! # 🌺💜 `violet/Experimental/Collections/HashMap.h`
//! This module provides `HashMap`: A Rust-inspired and flavoured hash map built on top of either
//! Abseil's `flat_hash_map` (SwissTable, more aligned with Rust's implementation) when the
//! [Abseil feature](#) is enabled, or `std::unordered_map` otherwise. The entry API (`hashmap::Entry`,
//! `hashmap::OccupiedEntry`, `hashmap::VacantEntry`) mirrors `hashbrown`'s entry API for single-lookup insert-or-modify
//! operations.

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Experimental/OneOf.h>
#include <violet/Iterator.h>

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
#include "absl/container/flat_hash_map.h"
#else
#include <unordered_map>
#endif

#include <memory>

namespace violet::experimental {
namespace hashmap {

template<typename K, typename V, typename Hasher, typename Equality, typename Alloc>
struct Entry;

template<typename K, typename V, typename Hasher, typename Equality, typename Alloc>
struct OccupiedEntry;

template<typename K, typename V, typename Hasher, typename Equality, typename Alloc>
struct VacantEntry;

template<typename K, typename V, typename Hasher, typename Equality, typename Alloc>
struct Iter;

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
/// Default hasher used by [`HashMap`].
template<typename K>
using default_hasher = absl::DefaultHashContainerHash<K>;

/// Default key-equality predicate used by [`HashMap`].
template<typename K>
using default_equality = absl::DefaultHashContainerEq<K>;

/// Erases all elements from a container matching a predicate
using absl::erase_if;
#else
/// Default hasher used by [`HashMap`].
template<typename K>
using default_hasher = std::hash<K>;

/// Default key-equality predicate used by [`HashMap`].
template<typename K>
using default_equality = std::equal_to<K>;

/// Erases all elements from a container matching a predicate
using std::erase_if;
#endif

} // namespace hashmap

/// A map-based type backed by Abseil's SwissTable or C++'s native [`std::unordered_map`] with the API of Rust's
/// [`std::collections::HashMap`] or the [`hashbrown` crate].
///
/// [`std::collections::HashMap`]: https://doc.rust-lang.org/stable/std/collections/struct.HashMap.html
/// [`hashbrown` crate]: https://docs.rs/hashbrown/latest/hashbrown/
///
/// ## Differences
/// - Entry API for single-lookup insert-or-modify, borrowed from `hashbrown`
/// - Using `violet::Optional<V>` for fetch instead of iterator+bool
/// - Move-based insertion (`insert` consumes, returns old value)
/// - Uses Violet's iterator framework for pluggable combinators for iteration.
template<typename K, typename V, typename Hasher = hashmap::default_hasher<K>,
    typename Equality = hashmap::default_equality<K>, typename Alloc = std::allocator<Pair<const K, V>>>
struct NOELDOC_EXPERIMENTAL_SINCE("current") HashMap final {
#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
    /// Underlying implementation: `absl::flat_hash_map`
    using implementation = absl::flat_hash_map<K, V, Hasher, Equality, Alloc>;
#else
    /// Underlying implementation: `std::unordered_map`
    using implementation = std::unordered_map<K, V, Hasher, Equality, Alloc>;
#endif

    /// The key type, `K`.
    using key_type = K;

    /// The mapped value type, `V`.
    using value_type = V;

    /// The unsigned integer type used for sizes and capacities.
    using size_type = UInt;

    /// A mutable iterator over `(key, value)` pairs, delegating to [`implementation::iterator`].
    using iterator = typename implementation::iterator;

    /// A read-only iterator over `(key, value)` pairs, delegating to [`implementation::const_iterator`].
    using const_iterator = typename implementation::const_iterator;

    /// The type returned by wrapping [`implementation`] in Violet's iterator framework, used
    /// to expose combinator-based iteration over this map.
    using underlying_iterator = decltype(violet::MkIterable(std::declval<implementation&>()));

    /// Construct a empty `HashMap`.
    VIOLET_IMPLICIT HashMap() noexcept(std::is_nothrow_default_constructible_v<implementation>) = default;

    /// Construct a `HashMap` from an initializer list of `(key, value)` pairs.
    VIOLET_IMPLICIT HashMap(std::initializer_list<Pair<const K, V>> il) noexcept(
        std::is_nothrow_constructible_v<implementation, decltype(il)>)
        : n_impl(il)
    {
    }

    /// Constructor that forwards `args` directly to the underlying implementation's constructor.
    template<typename... Args>
        requires constructible<implementation, Args...>
    VIOLET_IMPLICIT HashMap(Args&&... args) noexcept(std::is_nothrow_constructible_v<implementation, Args...>)
        : n_impl(implementation(VIOLET_FWD(Args, args)...))
    {
    }

#if defined(VIOLET_FEATURE_ABSEIL) && VIOLET_FEATURE(ABSEIL)
    /// Returns the number of elements the map can hold without triggering a rehash.
    [[nodiscard]] auto Capacity() const noexcept -> size_type
    {
        return this->n_impl.capacity();
    }
#endif

    /// Returns the number of elements currently stored in the map.
    [[nodiscard]] auto Size() const noexcept(noexcept(std::declval<implementation>().size())) -> size_type
    {
        return this->n_impl.size();
    }

    /// Returns **true** if the map contains no elements.
    [[nodiscard]] auto Empty() const noexcept(noexcept(std::declval<implementation>().empty())) -> bool
    {
        return this->n_impl.empty();
    }

    /// Reserves capacity for at least `additional` more elements without reallocating.
    void Reserve(size_type additional) noexcept(
        noexcept(std::declval<implementation>().reserve(std::declval<size_type>())))
    {
        this->n_impl.reserve(additional);
    }

    /// Shrinks the map's backing storage to fit its current contents as closely as possible.
    void ShrinkToFit() noexcept(noexcept(std::declval<implementation>().rehash(0)))
    {
        this->n_impl.rehash(0);
    }

    /// Returns a reference to the value for `key`, or [`Nothing`] if the key is not present.
    /// @tparam Q any type comparable with `K`, which enables heterogeneous (transparent) lookup
    template<std::equality_comparable_with<K> Q>
    auto Get(const Q& key) -> Optional<std::reference_wrapper<V>>
    {
        auto it = this->n_impl.find(K(key));
        if (it == this->n_impl.end()) {
            return Nothing;
        }

        return std::ref(it->second);
    }

    /// Returns a const reference to the value for `key`, or [`Nothing`] if the key is not present.
    /// @tparam Q any type comparable with `K`, which enables heterogeneous (transparent) lookup
    template<std::equality_comparable_with<K> Q>
    auto Get(const Q& key) const -> Optional<std::reference_wrapper<const V>>
    {
        auto it = this->n_impl.find(K(key));
        if (it == this->n_impl.end()) {
            return Nothing;
        }

        return std::cref(it->second);
    }

    /// Returns **true** if the map contains an entry for `key`.
    template<std::equality_comparable_with<K> Q>
    auto Contains(const Q& key) const -> bool
    {
        return this->n_impl.contains(K(key));
    }

    /// Inserts `key`/`value` into the map, returning the previous value if `key` was already
    /// present, or [`Nothing`] otherwise.
    ///
    /// Unlike `std::unordered_map::insert`, this always overwrites an existing value.
    template<typename Q, typename Tp>
        requires(std::is_constructible_v<K, Q> && std::is_constructible_v<V, Tp>)
    auto Insert(Q key, Tp value) -> Optional<V>
    {
        auto [it, inserted] = this->n_impl.try_emplace(VIOLET_MOVE(key), VIOLET_MOVE(value));
        if (inserted) {
            return Nothing;
        }

        V old = std::exchange(it->second, VIOLET_MOVE(value));
        return VIOLET_MOVE(old);
    }

    /// Inserts every `(key, value)` pair in `il` into the map, overwriting existing entries.
    void BulkInsert(std::initializer_list<Pair<K, V>> il)
    {
        this->n_impl.insert(il);
    }

    /// Removes `key` from the map, returning its value, or [`Nothing`] if `key` was not present.
    template<std::equality_comparable_with<K> Q>
    auto Remove(const Q& key) -> Optional<V>
    {
        auto it = this->n_impl.find(K(key));
        if (it == this->n_impl.end()) {
            return Nothing;
        }

        V value = VIOLET_MOVE(it->second);
        this->n_impl.erase(it);

        return VIOLET_MOVE(value);
    }

    /// Removes `key` from the map, returning the full `(key, value)` pair, or [`Nothing`] if
    /// `key` was not present.
    template<std::equality_comparable_with<K> Q>
    auto RemoveEntry(const Q& key) -> Optional<Pair<K, V>>
    {
        auto it = this->n_impl.find(K(key));
        if (it == this->n_impl.end()) {
            return Nothing;
        }

        auto node = this->n_impl.extract(it);
        return Pair(VIOLET_MOVE(const_cast<K&>(node.key())), VIOLET_MOVE(node.mapped()));
    }

    /// Moves every entry of `map` into `this`, leaving `map` with only the entries whose keys
    /// collided with an existing key in `this`.
    void Merge(HashMap&& map)
    {
        this->n_impl.merge(VIOLET_MOVE(map).n_impl);
    }

    /// Overload of [`HashMap::Merge`] that merges directly from the underlying [`implementation`] type.
    void Merge(implementation&& impl)
    {
        this->n_impl.merge(VIOLET_MOVE(impl));
    }

    /// Removes every entry for which `pred(key, value)` returns `false`, keeping the rest.
    template<typename Fun>
        requires(callable<Fun, const K&, V&> && callable_returns<Fun, bool, const K&, V&>)
    void Retain(Fun&& pred)
    {
        hashmap::erase_if(this->n_impl, [fun = VIOLET_FWD(Fun, pred)](auto& kv) -> bool {
            bool result = std::invoke(fun, kv.first, kv.second);
            return !result;
        });
    }

    /// Removes every entry for which `predicate(key, value)` returns `true`, passing each
    /// removed `(key, value)` pair to `sink` before it is destroyed.
    template<typename Pred, typename Sink>
        requires(
            callable<Pred, const K&, V&> && callable_returns<Pred, bool, const K&, V&> && callable<Sink, K &&, V &&>)
    void ExtractIf(Pred predicate, Sink sink)
    {
        for (auto it = this->n_impl.begin(); it != this->n_impl.end();) {
            if (std::invoke(predicate, it->first, it->second)) {
                auto node = this->n_impl.extract(it++);
                std::invoke(sink, VIOLET_MOVE(const_cast<K&>(node.key())), VIOLET_MOVE(node.mapped()));
            } else {
                ++it;
            }
        }
    }

    /// Returns a reference to the value for `key`, inserting a value constructed from `args`
    /// first if `key` is not already present.
    template<typename... Args>
    auto GetOrInsert(const K& key, Args&&... args) -> V&
    {
        auto [it, _] = this->n_impl.try_emplace(key, VIOLET_FWD(Args, args)...);
        return it->second;
    }

    /// Overload of [`GetOrInsert`] that takes `key` by rvalue reference to avoid a copy when
    /// the key is not already present.
    template<typename... Args>
    auto GetOrInsert(K&& key, Args&&... args) -> V&
    {
        auto [it, _] = this->n_impl.try_emplace(VIOLET_MOVE(key), VIOLET_FWD(Args, args)...);
        return it->second;
    }

    /// Removes every entry from the map.
    void Clear()
    {
        this->n_impl.clear();
    }

    /// Performs a single lookup for `key` and returns a [`hashmap::Entry`] representing either
    /// the occupied slot found or the vacant slot where `key` would be inserted.
    [[nodiscard]] auto Entry(K key) -> hashmap::Entry<K, V, Hasher, Equality, Alloc>;

    auto Iter() -> decltype(auto)
    {
        return MkIterable(*this);
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

    auto operator[](const key_type& key) -> V&
    {
        return this->n_impl[key];
    }

    auto operator[](key_type&& key) -> V&
    {
        return this->n_impl[VIOLET_MOVE(key)];
    }

    auto operator[](const K& key) const -> Optional<std::reference_wrapper<const V>>
    {
        return this->Get(key);
    }

private:
    friend struct hashmap::Entry<K, V, Hasher, Equality, Alloc>;
    friend struct hashmap::OccupiedEntry<K, V, Hasher, Equality, Alloc>;
    friend struct hashmap::VacantEntry<K, V, Hasher, Equality, Alloc>;

    implementation n_impl{};
};

namespace hashmap {

/// A view into an occupied entry that can be obtained by [`Entry::Occupied()`].
///
/// This holds an iterator into the map; invalidated by any rehash.
template<typename K, typename V, typename Hasher, typename Equality, typename Alloc>
struct OccupiedEntry final {
    /// Returns a reference to this entry's key.
    auto Key() const noexcept -> const K&
    {
        return this->n_it->first;
    }

    /// Returns a mutable reference to this entry's value.
    auto Value() noexcept -> V&
    {
        return this->n_it->second;
    }

    /// Returns a read-only reference to this entry's value.
    auto Value() const noexcept -> const V&
    {
        return this->n_it->second;
    }

    /// Replaces this entry's value with `value`, returning the old one.
    auto Insert(V value) -> V
    {
        return std::exchange(this->n_it->second, VIOLET_MOVE(value));
    }

    /// Removes this entry from the map, returning the full `(key, value)` pair.
    auto RemoveEntry() && -> Pair<K, V>
    {
        auto node = this->n_map->n_impl.extract(this->n_it);
        return {VIOLET_MOVE(const_cast<K&>(node.key())), VIOLET_MOVE(node.mapped())};
    }

    /// Removes this entry from the map, returning just its value.
    auto Remove() && -> V
    {
        return VIOLET_MOVE(*this).RemoveEntry().second;
    }

private:
    friend struct Entry<K, V, Hasher, Equality, Alloc>;

    using map = HashMap<K, V, Hasher, Equality, Alloc>;

    VIOLET_EXPLICIT OccupiedEntry(map* map, typename map::iterator it) noexcept
        : n_map(map)
        , n_it(it)
    {
    }

    map* n_map;
    typename map::iterator n_it;
};

/// A view into an vacant entry, obtained via [`Entry::Vacant()`]
template<typename K, typename V, typename Hasher, typename Equality, typename Alloc>
struct VacantEntry final {
    /// Returns a reference to the key that would be inserted.
    auto Key() const noexcept -> const K&
    {
        return this->n_key;
    }

    /// Consumes this entry, returning the key that would have been inserted.
    auto IntoKey() && -> K
    {
        return VIOLET_MOVE(this->n_key);
    }

    /// Inserts `value` for this entry's key, returning a reference to the newly inserted value.
    auto Insert(V value) && -> V&
    {
        auto [it, _] = this->n_map->n_impl.try_emplace(VIOLET_MOVE(this->n_key), VIOLET_MOVE(value));
        return it->second;
    }

private:
    friend struct Entry<K, V, Hasher, Equality, Alloc>;
    using map = HashMap<K, V, Hasher, Equality, Alloc>;

    VIOLET_EXPLICIT VacantEntry(map* map, K key) noexcept
        : n_map(map)
        , n_key(key)
    {
    }

    HashMap<K, V, Hasher, Equality, Alloc>* n_map;
    K n_key;
};

/// A view into a single map entry, which may be vacant or occupied.
///
/// The entry is obtained via [`HashMap::Entry()`] and performs a single lookup. Both the
/// "modify if present" and "insert if absent" use the same probe.
template<typename K, typename V, typename Hasher, typename Equality, typename Alloc>
struct Entry final {
    /// If this entry is occupied, invokes `fun` with a mutable reference to its value. Has no
    /// effect on a vacant entry. Returns `*this` to allow chaining into [`OrInsert`] or
    /// [`OrInsertWith`].
    ///
    /// [`OrInsert`]: OrInsert
    /// [`OrInsertWith`]: OrInsertWith
    template<typename Fun>
        requires(callable<Fun, V&> && callable_returns<Fun, void, V&>)
    auto AndModify(Fun&& fun) && -> Entry&&
    {
        if (auto occupied = this->n_state.template Get<OccupiedEntry<K, V, Hasher, Equality, Alloc>>();
            occupied.HasValue()) {
            std::invoke(VIOLET_FWD(Fun, fun), occupied->Value());
        }

        return VIOLET_MOVE(*this);
    }

    /// Returns a reference to the value, inserting `defaultValue` first if the entry is vacant.
    auto OrInsert(V defaultValue) && -> V&
    {
        return this->n_state.Visit([value = VIOLET_MOVE(defaultValue)](auto&& entry) -> V& {
            using T = std::decay_t<decltype(entry)>;

            if constexpr (std::same_as<T, OccupiedEntry<K, V, Hasher, Equality, Alloc>>) {
                return entry.Value();
            } else {
                return VIOLET_MOVE(entry).Insert(value);
            }
        });
    }

    /// Returns a reference to the value, inserting the result of calling `fun` first if the
    /// entry is vacant. Unlike [`OrInsert`], `fun` is only invoked when needed.
    ///
    /// [`OrInsert`]: OrInsert
    template<typename Fun>
        requires(callable<Fun> && callable_returns<Fun, V>)
    auto OrInsertWith(Fun&& fun) && -> V&
    {
        return this->n_state.Visit([fun = VIOLET_FWD(Fun, fun)](auto&& entry) -> V& {
            using T = std::decay_t<decltype(entry)>;

            if constexpr (std::same_as<T, OccupiedEntry<K, V, Hasher, Equality, Alloc>>) {
                return entry.Value();
            } else {
                return VIOLET_MOVE(entry).Insert(std::invoke(fun));
            }
        });
    }

    /// Returns a reference to the value, inserting a default-constructed `V` first if the
    /// entry is vacant.
    auto OrDefault() && -> V&
    {
        return VIOLET_MOVE(*this).OrInsertWith([] -> V { return V{}; });
    }

    /// Returns `true` if this entry is occupied.
    [[nodiscard]] auto Occupied() const noexcept -> bool
    {
        return this->n_state.template Holds<OccupiedEntry<K, V, Hasher, Equality, Alloc>>();
    }
    /// Returns `true` if this entry is vacant.
    [[nodiscard]] auto Vacant() const noexcept -> bool
    {
        return this->n_state.template Holds<VacantEntry<K, V, Hasher, Equality, Alloc>>();
    }

    /// Returns a reference to the underlying [`OccupiedEntry`], or [`Nothing`] if this entry
    /// is vacant.
    auto AsOccupied() noexcept -> Optional<std::reference_wrapper<OccupiedEntry<K, V, Hasher, Equality, Alloc>>>
    {
        return this->n_state.template Get<OccupiedEntry<K, V, Hasher, Equality, Alloc>>();
    }

    /// A `const`-qualified overload of [`AsOccupied`].
    ///
    /// [`AsOccupied`]: AsOccupied
    auto AsOccupied() const noexcept
        -> Optional<const std::reference_wrapper<OccupiedEntry<K, V, Hasher, Equality, Alloc>>>
    {
        return this->n_state.template Get<OccupiedEntry<K, V, Hasher, Equality, Alloc>>();
    }

    /// Returns a reference to the underlying [`VacantEntry`], or [`Nothing`] if this entry is
    /// occupied.
    auto AsVacant() noexcept -> Optional<std::reference_wrapper<VacantEntry<K, V, Hasher, Equality, Alloc>>>
    {
        return this->n_state.template Get<VacantEntry<K, V, Hasher, Equality, Alloc>>();
    }

    /// A `const`-qualified overload of [`AsVacant`].
    ///
    /// [`AsVacant`]: AsVacant
    auto AsVacant() const noexcept -> Optional<const std::reference_wrapper<VacantEntry<K, V, Hasher, Equality, Alloc>>>
    {
        return this->n_state.template Get<VacantEntry<K, V, Hasher, Equality, Alloc>>();
    }

    /// Consumes this entry and returns the underlying [`OccupiedEntry`].
    ///
    /// Asserts that the entry is occupied; use [`Occupied`] to check first.
    ///
    /// [`Occupied`]: Occupied
    auto IntoOccupied() && -> OccupiedEntry<K, V, Hasher, Equality, Alloc>
    {
        auto opt = this->AsOccupied();
        VIOLET_ASSERT0(opt.HasValue());

        return opt.UnwrapUnchecked(Unsafe("checked via assert"));
    }

    /// Consumes this entry and returns the underlying [`VacantEntry`].
    ///
    /// Asserts that the entry is vacant; use [`Vacant`] to check first.
    ///
    /// [`Vacant`]: Vacant
    auto IntoVacant() && -> VacantEntry<K, V, Hasher, Equality, Alloc>
    {
        auto opt = this->AsVacant();
        VIOLET_ASSERT0(opt.HasValue());

        return opt.UnwrapUnchecked(Unsafe("checked via assert"));
    }

    /// Returns a reference to this entry's key, whether occupied or vacant.
    auto Key() const noexcept -> const K&
    {
        return this->n_state.Visit([](const auto& ent) -> const K& { return ent.Key(); });
    }

private:
    friend struct HashMap<K, V, Hasher, Equality, Alloc>;

    using map = HashMap<K, V, Hasher, Equality, Alloc>;

    static auto make(map* map, K key)
        -> OneOf<OccupiedEntry<K, V, Hasher, Equality, Alloc>, VacantEntry<K, V, Hasher, Equality, Alloc>>
    {
        auto it = map->n_impl.find(key);
        if (it != map->end()) {
            return OneOf<OccupiedEntry<K, V, Hasher, Equality, Alloc>, VacantEntry<K, V, Hasher, Equality, Alloc>>::
                template New<OccupiedEntry<K, V, Hasher, Equality, Alloc>>(OccupiedEntry(map, it));
        }

        return OneOf<OccupiedEntry<K, V, Hasher, Equality, Alloc>, VacantEntry<K, V, Hasher, Equality, Alloc>>::
            template New<VacantEntry<K, V, Hasher, Equality, Alloc>>(VacantEntry(map, VIOLET_MOVE(key)));
    }

    VIOLET_EXPLICIT Entry(map* map, K key) noexcept
        : n_state(make(map, key))
    {
    }

    OneOf<OccupiedEntry<K, V, Hasher, Equality, Alloc>, VacantEntry<K, V, Hasher, Equality, Alloc>> n_state;
};

} // namespace hashmap

template<typename K, typename V, typename Hasher, typename Equality, typename Alloc>
auto HashMap<K, V, Hasher, Equality, Alloc>::Entry(K key) -> hashmap::Entry<K, V, Hasher, Equality, Alloc>
{
    return hashmap::Entry(this, VIOLET_MOVE(key));
}

} // namespace violet::experimental
