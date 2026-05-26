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

namespace violet::experimental {
namespace slice {
    template<typename T, UInt N>
    struct Iter;
}

/// A fixed-capacity, stack-allocated container akin to [`std::array<T, N>`] or Rust's [`&[T; N]`][rust-slice].
///
/// [rust-slice]: https://doc.rust-lang.org/std/primitive.slice.html
///
/// Unlike [`std::array<T, N>`], **Slice** leaves its storage uninitialized until elements are explicitly
/// emplaced. This makes it suitable for types that are not default-constructible while providing the cache-friendly,
/// stack-allocated layout of a fixed-sized array.
///
/// Elements are constructed in-place via [`Slice::Emplace`] or [`Slice::Push`] and destroyed
/// in reverse-order on destruction.
///
/// ## Example
/// ```cpp
/// #include <violet/Experimental/Slice.h>
/// #include <thread>
///
/// using namespace violet::experimental;
///
/// Slice<std::thread, 4> threads;
/// threads.Emplace([&] -> void { doWork(0); });
/// threads.Emplace([&] -> void { doWork(1); });
///
/// for (auto& t: threads) { t.join(); }
///
/// // constexpr usage for literal types
/// constexpr auto makeSlice() -> Slice<Int32, 4> {
///     Slice<Int32, 4> slice;
///     slice.Push(1);
///     slice.Push(2);
///     slice.Push(3);
///
///     return slice;
/// }
///
/// static_assert(makeSlice().Elements() == 3);
/// static_assert(makeSlice().Size() == 4);
/// ```
///
/// @tparam T element type. must be either move or copy constructible.
/// @tparam N maximum number of elements
template<typename T, UInt N>
struct NOELDOC_EXPERIMENTAL_SINCE("26.07") Slice final {
    static_assert(N > 0, "`N` must non-zero");
    static_assert(!std::is_void_v<T>, "`T` = `void` is ill-formed");
    static_assert(!std::is_reference_v<T>, "`T&` is ill-formed");
    static_assert(!std::is_const_v<T>, "`const T` is ill-formed");
    static_assert(std::is_destructible_v<T>, "`T` must be destructible");

    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using size_type = UInt;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using underlying_iterator = slice::Iter<T, N>;

    VIOLET_DISALLOW_COPY(Slice);

    /// Constructs an empty `Slice` with no initialized elements.
    constexpr VIOLET_IMPLICIT Slice() noexcept = default;

    /// Constructs a [`Slice`] from an initializer list.
    ///
    /// This method will panic if the initializer list exceeds the capacity `N`.
    constexpr VIOLET_IMPLICIT Slice(std::initializer_list<T> init)
        : n_storage({ })
    {
        for (auto& element: init) {
            this->Push(element);
        }
    }

    /// Move-constructs a `Slice`, transferring ownership of all elements.
    constexpr VIOLET_IMPLICIT Slice(Slice&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_storage({ })
    {
        for (size_type i = 0; i < other.n_size; i++) {
            this->Emplace(VIOLET_MOVE(other.at(i)));
        }

        other.Clear();
    }

    /// Move-assigns a `Slice`, destroying existing elements and transferring
    /// ownership from `other`.
    constexpr auto operator=(Slice&& other) noexcept(std::is_nothrow_move_constructible_v<T>) -> Slice&
    {
        if (this != &other) {
            this->Clear();
            for (size_type i = 0; i < other.n_size; i++) {
                this->Emplace(VIOLET_MOVE(other.at(i)));
            }

            other.Clear();
        }

        return *this;
    }

    constexpr ~Slice()
    {
        this->destroyAll();
    }

    /// Returns a reference to the first element.
    constexpr auto Front() -> Optional<std::reference_wrapper<value_type>>
    {
        if (this->n_size > 0) {
            return this->at(0);
        }

        return Nothing;
    }

    /// Returns a reference to the first element.
    constexpr auto Front() const -> Optional<std::reference_wrapper<const value_type>>
    {
        if (this->n_size > 0) {
            return this->at(0);
        }

        return Nothing;
    }

    /// Returns a reference to the first element.
    constexpr auto Back() -> Optional<std::reference_wrapper<value_type>>
    {
        if (this->n_size > 0) {
            return std::ref(this->at(this->n_size - 1));
        }

        return Nothing;
    }

    /// Returns a reference to the first element.
    constexpr auto Back() const -> Optional<std::reference_wrapper<const value_type>>
    {
        if (this->n_size > 0) {
            return std::cref(this->at(this->n_size - 1));
        }

        return Nothing;
    }

    /// Returns a pointer to the underlying storage. Only the first `Size()` elements are valid.
    constexpr auto Data() noexcept -> pointer
    {
        return std::addressof(this->at(0));
    }

    /// Returns a pointer to the underlying storage. Only the first `Size()` elements are valid.
    constexpr auto Data() const noexcept -> const_pointer
    {
        return std::addressof(this->at(0));
    }

    /// Returns `true` if the `Slice` contains no elements.
    [[nodiscard]] constexpr auto Empty() const noexcept -> bool
    {
        return this->n_size == 0;
    }

    /// Returns `true` if the `Slice` has reached its maximum capacity.
    [[nodiscard]] constexpr auto Full() const noexcept -> bool
    {
        return this->n_size == N;
    }

    /// Returns the fixed capacity `N`.
    [[nodiscard]] constexpr auto Size() const noexcept -> UInt
    {
        return N;
    }

    /// Returns the number of initialized elements.
    [[nodiscard]] constexpr auto Elements() const noexcept -> size_type
    {
        return this->n_size;
    }

    /// Constructs an element in-place at the end of the `Slice`.
    ///
    /// This method will panic if the slice is full.
    template<typename... Args>
    constexpr auto Emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T>) -> reference
    {
        VIOLET_ASSERT(this->Elements() < N, "`violet::experimental::Slice`: Emplace() called on a full slice");
        std::construct_at(std::addressof(this->at(this->n_size)), VIOLET_FWD(Args, args)...);
        return this->at(this->n_size++);
    }

    /// Copies an element to the end of the `Slice`.
    ///
    /// This method will panic if the slice is full.
    constexpr auto Push(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>) -> reference
        requires(std::is_copy_constructible_v<T>)
    {
        return this->Emplace(value);
    }

    /// Moves an element to the end of the `Slice`.
    ///
    /// This method will panic if the slice is full.
    constexpr auto Push(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>) -> reference
        requires(std::is_move_constructible_v<T>)
    {
        return this->Emplace(VIOLET_MOVE(value));
    }

    /// Removes and destroys the last element.
    ///
    /// This method will panic if the slice is empty.
    constexpr void Pop()
    {
        VIOLET_ASSERT(this->Elements() > 0, "`violet::experimental::Slice`: Pop() called on an empty Slice");

        this->n_size--;
        std::destroy_at(std::addressof(this->at(this->n_size)));
    }

    /// Removes, destroys and returns the last element.
    ///
    /// This method will panic if the slice is empty.
    constexpr auto PopBack() -> value_type
    {
        VIOLET_ASSERT(this->Elements() > 0, "`violet::experimental::Slice`: Pop() called on an empty Slice");

        this->n_size--;
        value_type value = VIOLET_MOVE(this->at(this->n_size));

        std::destroy_at(std::addressof(this->at(this->n_size)));
        return value;
    }

    /// Destroys all elements, leaving the `Slice` empty.
    constexpr void Clear()
    {
        this->destroyAll();
        this->n_size = 0;
    }

    [[nodiscard("violet iterators are lazily-evaluated")]] auto Iter() -> slice::Iter<T, N>;
    [[nodiscard("violet iterators are lazily-evaluated")]] auto Iter() const -> slice::Iter<const T, N>;

    [[nodiscard]] constexpr auto begin() noexcept -> iterator
    {
        return this->Data();
    }

    [[nodiscard]] constexpr auto end() noexcept -> iterator
    {
        return this->Data() + this->n_size;
    }

    [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator
    {
        return this->Data();
    }

    [[nodiscard]] constexpr auto end() const noexcept -> const_iterator
    {
        return this->Data() + this->n_size;
    }

    [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator
    {
        return this->Data();
    }

    [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator
    {
        return this->Data() + this->n_size;
    }

    [[nodiscard]] constexpr auto rbegin() noexcept -> reverse_iterator
    {
        return std::reverse_iterator(this->end());
    }

    [[nodiscard]] constexpr auto rend() noexcept -> reverse_iterator
    {
        return std::reverse_iterator(this->begin());
    }

    [[nodiscard]] constexpr auto rbegin() const noexcept -> reverse_iterator
    {
        return std::reverse_iterator(this->end());
    }

    [[nodiscard]] constexpr auto rend() const noexcept -> reverse_iterator
    {
        return std::reverse_iterator(this->begin());
    }

    [[nodiscard]] constexpr auto crbegin() const noexcept -> const_reverse_iterator
    {
        return std::reverse_iterator(this->cend());
    }

    [[nodiscard]] constexpr auto crend() const noexcept -> const_reverse_iterator
    {
        return std::reverse_iterator(this->cbegin());
    }

    constexpr VIOLET_IMPLICIT operator bool() const noexcept
    {
        return !this->Empty();
    }

    constexpr VIOLET_IMPLICIT operator Span<T>() noexcept
    {
        return { this->Data(), this->Elements() };
    }

    constexpr VIOLET_IMPLICIT operator Span<const T>() const noexcept
    {
        return { this->Data(), this->Elements() };
    }

    template<UInt M>
        requires(M <= N)
    constexpr VIOLET_IMPLICIT operator Span<T, M>() noexcept
    {
        VIOLET_ASSERT_FMT(this->Elements() == M,
            "`violet::experimental::Slice`: size mismatch when converting to `violet::Span<T, M = {}>`", M);

        return Span<T, M>(this->Data(), M);
    }

    template<UInt M>
        requires(M <= N)
    constexpr VIOLET_IMPLICIT operator Span<const T, M>() const noexcept
    {
        VIOLET_ASSERT_FMT(this->Elements() == M,
            "`violet::experimental::Slice`: size mismatch when converting to `violet::Span<T, M = {}>`", M);

        return Span<T, M>(this->Data(), M);
    }

    constexpr auto operator[](size_type index) -> reference
    {
        VIOLET_ASSERT(index < this->Elements(), "`violet::experimental::Slice`: index out of bounds");
        return this->at(index);
    }

    constexpr auto operator[](size_type index) const -> const_reference
    {
        VIOLET_ASSERT(index < this->Elements(), "`violet::experimental::Slice`: index out of bounds");
        return this->at(index);
    }

    constexpr auto operator==(const Slice& other) const -> bool
        requires(std::equality_comparable<T>)
    {
        if (this->n_size != other.n_size) {
            return false;
        }

        for (size_type i = 0; i < this->n_size; i++) {
            if (this->at(i) != other.at(i)) {
                return false;
            }
        }

        return true;
    }

    constexpr auto operator<=>(const Slice& other) const
        requires(std::three_way_comparable<T>)
    {
        for (size_type i = 0; i < std::min(this->n_size, other.n_size); i++) {
            if (auto cmp = this->at(i) <=> other.at(i); cmp != 0) {
                return cmp;
            }
        }

        return this->n_size <=> other.n_size;
    }

private:
    /// Storage cell that avoids default-constructing `T`.
    ///
    /// The `dummy` member makes the union trivially default-constructible,
    /// while `value` holds the actual element once constructed via
    /// `std::construct_at`.
    union cell {
        char dummy;
        T value;

        constexpr cell() noexcept
            : dummy('\0')
        {
        }

        constexpr ~cell()
            requires(std::is_trivially_destructible_v<T>)
        = default;

        constexpr ~cell()
            requires(!std::is_trivially_destructible_v<T>)
        {
        }
    };

    cell n_storage[N];
    size_type n_size = 0;

    constexpr auto at(size_type index) -> reference
    {
        return this->n_storage[index].value;
    }

    constexpr auto at(size_type index) const -> const_reference
    {
        return this->n_storage[index].value;
    }

    constexpr void destroyAll() noexcept
    {
        for (size_type i = this->n_size; i > 0; i--) {
            std::destroy_at(std::addressof(this->at(i - 1)));
        }
    }
};

namespace slice {

    /// An iterator over the elements of `Slice`.
    ///
    /// Yields each element as `Optional<T&>`, returning `Nothing` when
    /// exhausted. Supports both forward and reverse iteration via
    /// `Next()` and `NextBack()`.
    ///
    /// ## Example
    /// ```cpp
    /// #include <violet/Experimental/Slice.h>
    /// #include <violet/Iterator/Map.h>
    /// #include <violet/Print.h>
    ///
    /// using namespace violet::experimental;
    /// using namespace violet;
    ///
    /// Slice<Int32, 4> slice;
    /// slice.Push(1);
    /// slice.Push(2);
    /// slice.Push(3);
    ///
    /// for (auto item: slice.Iter().Map([](Int32 value) -> Int32 { return value * 2; })) {
    ///     violet::Println("{}", item);
    /// }
    /// ```
    template<typename T, UInt N>
    struct Iter final: public Iterator<Iter<T, N>> {
        static_assert(std::is_reference_v<T&>, "`T` must be a referenceable type");
        static_assert(!std::is_void_v<T>, "`T` = `void` is ill-formed");

        using Item = std::reference_wrapper<T>;

        VIOLET_DISALLOW_CONSTRUCTOR(Iter);
        constexpr VIOLET_IMPLICIT Iter(T* data, UInt size)
            : n_data(data)
            , n_front(0)
            , n_back(size)
        {
        }

        /// Advances the iterator and returns the next element, or `Nothing`
        /// if all elements have been yielded.
        auto Next() -> Optional<Item>
        {
            if (this->n_front >= this->n_back) {
                return Nothing;
            }

            if constexpr (std::is_const_v<T>) {
                return std::cref(this->n_data[this->n_front++]);
            } else {
                return std::ref(this->n_data[this->n_front++]);
            }
        }

        /// Advances the iterator from the back and returns the last
        /// unconsumed element, or `Nothing` if all elements have been yielded.
        auto NextBack() -> Optional<Item>
        {
            if (this->n_front >= this->n_back) {
                return Nothing;
            }

            if constexpr (std::is_const_v<T>) {
                return std::cref(this->n_data[--this->n_back]);
            } else {
                return std::ref(this->n_data[--this->n_back]);
            }
        }

        /// Returns the number of remaining elements.
        [[nodiscard]] constexpr auto SizeHint() const noexcept -> violet::SizeHint
        {
            auto remaining = this->n_back - this->n_front;
            return { remaining, Some(remaining) };
        }

    private:
        T* n_data;
        UInt n_front;
        UInt n_back;
    };

    static_assert(Iterable<Iter<Int32, 3>>);
    static_assert(Iterable<Iter<const Int32, 3>>);
    static_assert(DoubleEndedIterable<Iter<Int32, 3>>);
    static_assert(DoubleEndedIterable<Iter<const Int32, 3>>);

} // namespace slice

template<typename T, UInt N>
inline auto Slice<T, N>::Iter() -> slice::Iter<T, N>
{
    return slice::Iter<T, N>(this->Data(), this->Elements());
}

template<typename T, UInt N>
auto Slice<T, N>::Iter() const -> slice::Iter<const T, N>
{
    return slice::Iter<const T, N>(this->Data(), this->Elements());
}

} // namespace violet::experimental
