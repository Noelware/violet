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
//! # 🌺💜 `violet/Experimental/Buf.h`
//! Cheaply cloneable, sliceable byte buffers, modeled after the [`bytes`] Rust crate.
//! [`bytes`]: https://docs.rs/bytes
//!
//! * [`Buf`] is an immutable view `(vtable, data, ptr, size)`. The vtable decides how the memory
//!   is kept alive (static, shared allocation, or an arbitrary owner), so clones and slices never copy.
//! * [`Bytes`] is a unique, growable buffer with an uninitialized tail that I/O reads fill. It
//!   [freezes][Bytes::Freeze] into `Buf` without copying.
//!
//! ## Thread safety
//! `Bytes` and `BytesMut` are values: a single object isn't internally synchronized, but distinct
//! objects sharing memory may be used from different threads freely (refcounts are atomic and the
//! bytes behind a `Bytes` never change).

#pragma once

#include <violet/Experimental/Memory/NonNull.h>

#include <algorithm>

namespace violet::experimental {

struct Buf;
struct Bytes;

namespace NOELDOC_HIDE buf_internal {

struct VTable final {
    void* (*Clone)(void* data) noexcept = nullptr;
    void (*Drop)(void* data) noexcept = nullptr;
    bool (*Unique)(void* data) noexcept = nullptr;
};

struct Storage final {
    std::atomic<UInt> Refs{1};
    UInt Capacity = 0;

    [[nodiscard]]
    auto Data() noexcept -> std::byte*
    {
        return reinterpret_cast<std::byte*>(this + 1);
    }
};

VIOLET_API
auto AllocateStorage(UInt capacity) -> Storage*;

VIOLET_API
void RetainStorage(Storage* storage) noexcept;

VIOLET_API
void ReleaseStorage(Storage* storage) noexcept;

inline constexpr VTable kStaticBytesVTable{
    // clang-format off
    .Clone = [](void* data) noexcept -> void* {
        return data;
    },
    .Drop = [](void*) noexcept -> void {},
    .Unique = [](void*) noexcept -> bool {
        return false;
    }
    // clang-format on
};

VIOLET_API
extern const VTable kSharedBytesVTable;

template<typename T>
struct Box final {
    std::atomic<UInt> Refs{1};
    T Owner;
};

template<typename T>
inline constexpr VTable kOwnerBytesVTable{
    // clang-format off
    .Clone = [](void* data) noexcept -> void* {
        auto* self = static_cast<Box<T>*>(data);
        self->Refs.fetch_add(1, std::memory_order_relaxed);

        return data;
    },
    .Drop = [](void* data) noexcept -> void {
        auto* self = static_cast<Box<T>*>(data);
        if (self->Refs.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete self;
        }
    },
    .Unique = [](void* data) noexcept -> bool {
        return static_cast<Box<T>*>(data)->Refs.load(std::memory_order_acquire) == 1;
    }
    // clang-format on
};

} // namespace NOELDOC_HIDE buf_internal

/// Satisfied by types that own a contiguous run of byte-sized elements, e.g. `std::vector<char>`,
/// `std::string`, `std::vector<std::byte>`.
///
/// ## Remarks
/// The owner's data pointer must stay valid while the owner itself isn't modified (true for every
/// standard container), because `Buf` never modifies it.
template<typename T>
concept byte_owner = std::move_constructible<T> && std::ranges::contiguous_range<const T>
    && std::ranges::sized_range<const T> && sizeof(std::ranges::range_value_t<const T>) == 1
    && std::is_trivially_copyable_v<std::ranges::range_value_t<const T>>;

/// An immutable, cheaply-cloneable, sliceable view of bytes, analogous to [`bytes::Bytes`].
/// [`bytes::Bytes`]: https://docs.rs/bytes/latest/bytes/struct.Bytes.html
///
/// ## Remarks
/// * Copying and slicing never copy bytes; at most they bump an atomic refcount.
/// * Static data (including the default and moved-from states) is usable in constant expressions.
/// * An empty slice doesn't keep its source's memory alive.
///
/// ## Example
/// ```cpp
/// constexpr Buf kHttpOK = Buf::FromStatic("HTTP/1.1 200 OK\r\n");
/// static_assert(kHttpOK.Slice(9, 12) == "200");
///
/// Buf body = Buf::FromOwner(std::string{"hello"});
/// Buf copy = body;
/// ```
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Buf final {
    constexpr VIOLET_IMPLICIT Buf() noexcept = default;

    constexpr VIOLET_IMPLICIT Buf(const Buf& other) noexcept
        : n_vtable(other.n_vtable)
        , n_handle(other.n_vtable->Clone(other.n_handle))
        , n_data(other.n_data)
        , n_size(other.n_size)
    {
    }

    constexpr auto operator=(const Buf& other) noexcept -> Buf&
    {
        if (this != &other) {
            Buf copy(other);
            this->swap(copy);
        }

        return *this;
    }

    constexpr VIOLET_IMPLICIT Buf(Buf&& other) noexcept
        : n_vtable(std::exchange(other.n_vtable, &buf_internal::kStaticBytesVTable))
        , n_handle(std::exchange(other.n_handle, nullptr))
        , n_data(std::exchange(other.n_data, nullptr))
        , n_size(std::exchange(other.n_size, 0))
    {
    }

    constexpr auto operator=(Buf&& other) noexcept -> Buf&
    {
        if (this != &other) {
            Buf moved(VIOLET_MOVE(other));
            this->swap(moved);
        }

        return *this;
    }

    constexpr ~Buf()
    {
        if (this->n_vtable != nullptr) {
            this->n_vtable->Drop(this->n_handle);
        }
    }

    template<size_t N>
    [[nodiscard]]
    constexpr static auto FromStatic(const char (&str)[N]) noexcept
    {
        return Buf(&buf_internal::kStaticBytesVTable, nullptr, str, N - 1);
    }

    [[nodiscard]]
    constexpr static auto FromStatic(Str data) noexcept -> Buf
    {
        return Buf(&buf_internal::kStaticBytesVTable, nullptr, data.data(), data.size());
    }

    [[nodiscard]]
    static auto FromStatic(Span<const std::byte> data) noexcept -> Buf
    {
        return FromStatic(Str{reinterpret_cast<const char*>(data.data()), data.size()});
    }

    [[nodiscard]]
    static auto CopyFrom(Span<const std::byte> data) -> Buf;

    [[nodiscard]]
    static auto CopyFrom(Str data) -> Buf
    {
        return Buf::CopyFrom(std::as_bytes(std::span{data}));
    }

    template<byte_owner T>
    [[nodiscard]]
    static auto FromOwner(T owner) -> Buf
    {
        auto* box = new buf_internal::Box<T>{.Owner = VIOLET_MOVE(owner)};

        const auto* begin = reinterpret_cast<const char*>(std::ranges::data(box->Owner));
        return Buf(&buf_internal::kOwnerBytesVTable<T>, box, begin, std::ranges::size(box->Owner));
    }

    [[nodiscard]]
    constexpr auto Size() const noexcept -> UInt
    {
        return this->n_size;
    }

    [[nodiscard]]
    constexpr auto Empty() const noexcept -> bool
    {
        return this->n_size == 0;
    }

    [[nodiscard]]
    constexpr auto AsStr() const noexcept -> Str
    {
        return {this->n_data, this->n_size};
    }

    [[nodiscard]]
    auto AsSpan() const noexcept -> Span<const std::byte>
    {
        return {this->Data().Get(), this->Size()};
    }

    [[nodiscard]]
    auto Data() const noexcept -> ptr::NonNull<const std::byte>
    {
        return ptr::NonNull<const std::byte>::NewUnchecked(
            Unsafe("safe to reinterpret; if `data` is nil, then it's moved from, which is invalid to begin with"),
            reinterpret_cast<const std::byte*>(this->n_data));
    }

    [[nodiscard]]
    auto Unique() const noexcept -> bool
    {
        return this->n_vtable->Unique(this->n_handle);
    }

    [[nodiscard]]
    constexpr auto Slice(UInt begin, UInt end) const noexcept -> Buf
    {
        VIOLET_DEBUG_ASSERT(begin <= end && end <= this->n_size, "`Buf::Slice`: range is out of bounds");
        if (begin == end) {
            return {};
        }

        return Buf(this->n_vtable, this->n_vtable->Clone(this->n_handle), this->n_data + begin, end - begin);
    }

    [[nodiscard]]
    constexpr auto SplitTo(UInt at) noexcept -> Buf
    {
        Buf head = this->Slice(0, at);
        this->Advance(at);

        return head;
    }

    [[nodiscard]]
    constexpr auto SplitOff(UInt at) noexcept -> Buf
    {
        Buf tail = this->Slice(at, this->n_size);
        this->Truncate(at);

        return tail;
    }

    constexpr void Advance(UInt count) noexcept
    {
        VIOLET_DEBUG_ASSERT(count <= this->n_size, "`Buf::Advance`: count exceeds size");

        this->n_data += count;
        this->n_size -= count;
    }

    constexpr void Truncate(UInt size) noexcept
    {
        this->n_size = std::min(size, this->n_size);
    }

    constexpr void Clear() noexcept
    {
        *this = Buf{};
    }

    [[nodiscard]]
    auto TryIntoMut() && noexcept -> Optional<Bytes>;

    constexpr void swap(Buf& other) noexcept
    {
        std::swap(this->n_vtable, other.n_vtable);
        std::swap(this->n_handle, other.n_handle);
        std::swap(this->n_data, other.n_data);
        std::swap(this->n_size, other.n_size);
    }

    constexpr friend void swap(Buf& lhs, Buf& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    constexpr VIOLET_IMPLICIT operator bool() const noexcept
    {
        return !this->Empty();
    }

    constexpr VIOLET_EXPLICIT operator Str() const noexcept
    {
        return this->AsStr();
    }

    constexpr VIOLET_EXPLICIT operator Span<const std::byte>() const noexcept
    {
        return this->AsSpan();
    }

    constexpr friend auto operator<=>(const Buf& lhs, const Buf& rhs) noexcept
        -> decltype(std::declval<Str>() <=> std::declval<Str>())
    {
        return lhs.AsStr() <=> rhs.AsStr();
    }

    constexpr friend auto operator==(const Buf& lhs, const Buf& rhs) noexcept -> bool
    {
        return lhs.AsStr() == rhs.AsStr();
    }

    constexpr friend auto operator!=(const Buf& lhs, const Buf& rhs) noexcept -> bool
    {
        return lhs.AsStr() != rhs.AsStr();
    }

    template<UInt N>
    constexpr friend auto operator==(const Buf& lhs, const char (&rhs)[N]) noexcept -> bool
    {
        return lhs.AsStr() == Str(rhs, N - 1);
    }

    constexpr friend auto operator==(const Buf& lhs, Str rhs) noexcept -> bool
    {
        return lhs.AsStr() == rhs;
    }

    template<UInt N>
    constexpr friend auto operator!=(const Buf& lhs, const char (&rhs)[N]) noexcept -> bool
    {
        return lhs.AsStr() != Str(rhs, N - 1);
    }

    constexpr friend auto operator!=(const Buf& lhs, Str rhs) noexcept -> bool
    {
        return lhs.AsStr() != rhs;
    }

private:
    friend struct Bytes;

    constexpr VIOLET_EXPLICIT Buf(const buf_internal::VTable* vt, void* handle, const char* data, UInt size) noexcept
        : n_vtable(vt)
        , n_handle(handle)
        , n_data(data)
        , n_size(size)
    {
    }

    const buf_internal::VTable* n_vtable = &buf_internal::kStaticBytesVTable;
    void* n_handle = nullptr;
    const char* n_data = nullptr;
    UInt n_size = 0;
};

/// A unique, growable byte buffer with an uninitialized tail, analogous to [`bytes::BytesMut`].
/// [`bytes::BytesMut`]: https://docs.rs/bytes/latest/bytes/struct.BytesMut.html
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Bytes final {
    VIOLET_DISALLOW_COPY(Bytes);

    constexpr VIOLET_IMPLICIT Bytes() noexcept = default;

    VIOLET_EXPLICIT Bytes(UInt capacity)
    {
        this->Reserve(capacity);
    }

    VIOLET_IMPLICIT Bytes(Bytes&& other) noexcept
        : n_storage(std::exchange(other.n_storage, nullptr))
        , n_data(std::exchange(other.n_data, nullptr))
        , n_size(std::exchange(other.n_size, 0))
        , n_capacity(std::exchange(other.n_capacity, 0))
    {
    }

    auto operator=(Bytes&& other) noexcept -> Bytes&
    {
        if (this != &other) {
            buf_internal::ReleaseStorage(this->n_storage);

            this->n_storage = std::exchange(other.n_storage, nullptr);
            this->n_data = std::exchange(other.n_data, nullptr);
            this->n_size = std::exchange(other.n_size, 0);
            this->n_capacity = std::exchange(other.n_capacity, 0);
        }

        return *this;
    }

    ~Bytes()
    {
        buf_internal::ReleaseStorage(this->n_storage);
    }

    [[nodiscard]]
    auto Size() const noexcept -> UInt
    {
        return this->n_size;
    }

    [[nodiscard]]
    auto Capacity() const noexcept -> UInt
    {
        return this->n_capacity;
    }

    [[nodiscard]]
    auto Empty() const noexcept -> bool
    {
        return this->n_size == 0;
    }

    [[nodiscard]]
    auto Data() noexcept -> std::byte*
    {
        return this->n_data;
    }

    [[nodiscard]]
    auto Data() const noexcept -> ptr::NonNull<const std::byte>
    {
        return ptr::NonNull<const std::byte>::NewUnchecked(Unsafe("data is never non-nil"), this->n_data);
    }

    [[nodiscard]]
    auto Spare() const noexcept -> UInt
    {
        return this->n_capacity - this->n_size;
    }

    [[nodiscard]]
    auto SpareData() noexcept -> std::byte*
    {
        return this->n_data + this->n_size;
    }

    [[nodiscard]]
    auto AsSpan() const noexcept -> Span<const std::byte>
    {
        return {this->Data().Get(), this->Size()};
    }

    void AssumeInit(Unsafe, UInt count) noexcept
    {
        VIOLET_DEBUG_ASSERT(count <= this->Spare(), "`Bytes::AssumeInit`: count exceeds spare capacity");
        this->n_size += count;
    }

    void Extend(Span<const std::byte> data)
    {
        this->Reserve(data.size());
        if (!data.empty()) {
            ::memcpy(this->SpareData(), data.data(), data.size());
        }

        this->n_size += data.size();
    }

    void Extend(Str data)
    {
        this->Extend(std::as_bytes(std::span{data}));
    }

    void Clear() noexcept
    {
        this->n_size = 0;
    }

    void Reserve(UInt additional);

    [[nodiscard]]
    auto SplitTo(UInt at) noexcept -> Bytes
    {
        VIOLET_DEBUG_ASSERT(at <= this->n_size, "`Bytes::SplitTo`: index is past the initialized bytes");
        if (at == 0) {
            return {};
        }

        buf_internal::RetainStorage(this->n_storage);
        Bytes head(this->n_storage, this->n_data, at, at);

        this->n_data += at;
        this->n_size -= at;
        this->n_capacity -= at;

        return head;
    }

    [[nodiscard]]
    auto Freeze() && noexcept -> Buf
    {
        if (this->n_size == 0) {
            *this = Bytes{};
            return {};
        }

        this->n_capacity = 0;
        return Buf(&buf_internal::kSharedBytesVTable, std::exchange(this->n_storage, nullptr),
            reinterpret_cast<const char*>(std::exchange(this->n_data, nullptr)), std::exchange(this->n_size, 0));
    }

private:
    friend struct Buf;

    VIOLET_EXPLICIT Bytes(buf_internal::Storage* storage, std::byte* data, UInt size, UInt capacity) noexcept
        : n_storage(storage)
        , n_data(data)
        , n_size(size)
        , n_capacity(capacity)
    {
    }

    buf_internal::Storage* n_storage = nullptr;
    std::byte* n_data = nullptr;
    UInt n_size = 0;
    UInt n_capacity = 0;
};

} // namespace violet::experimental

template<>
struct std::hash<violet::experimental::Buf> {
    auto operator()(const violet::experimental::Buf& buf) const noexcept -> violet::UInt
    {
        constexpr std::hash<violet::Str> hasher{};
        return hasher(buf.AsStr());
    }
};
