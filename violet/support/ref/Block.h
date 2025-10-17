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

#include "violet/violet.h"

#include <atomic>
#include <type_traits>

/// @internal
namespace Noelware::Violet::__detail {

/// A managed, heap-allocated block to a value of `T` that is owned
/// by [`ARef`](Noelware::Violet::ARef) or [`Ref`](Noelware::Violet::Ref).
///
/// @tparam Atomic whether if the block should hold atomic references or regular numerical references.
template<typename T, bool Atomic = false>
struct Block final {
    /// Counter type for strong and weak references.
    using ref_count_type = std::conditional_t<Atomic, std::atomic<usize>, usize>;

    /// amount of strong references in this controlled block. if there is only one, then `T` can be dropped safely.
    ref_count_type Strong;

    /// amount of weak references in this controlled block.
    ref_count_type Weak;

    /// Allocates a new [`Block`] without in-place allocation of `T`.
    static auto Alloc() -> Block*
    {
        usize alignment = std::max(alignof(Block), alignof(T));
        void* blk = ::operator new(sizeof(Block) + sizeof(T), std::align_val_t(alignment));

        return new (blk) Block();
    }

    /// Deallocates a `blk` out of existence.
    /// @param blk pointer to the block
    static auto Dealloc(Block* blk)
    {
        blk->~Block(); // call the destructor of `blk` before deallocating.
        ::operator delete(blk, std::align_val_t(alignof(Block))); // free the memory
    }

    ~Block() = default;
    Block()
        : Strong(1)
        , Weak(1)
    {
    }

    /// Returns the pointer where the value of `T` is stored.
    auto ValuePtr() noexcept
    {
        // Safety: We allocated `sizeof(Block)+sizeof(T)` bytes, aligned to `max(alignof(Block), alignof(T)).
        //         The memory immediately after this block is to be suitably aligned for `T`, hopefully.
        //
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<T*>(reinterpret_cast<uint8*>(std::addressof(*this)) + sizeof(Block));
    }

    /// Returns the pointer where the value of `T` is stored.
    auto ValuePtr() const noexcept -> const T*
    {
        // Safety: read above ^^
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<const T*>(reinterpret_cast<const uint8*>(std::addressof(*this)) + sizeof(Block));
    }

    void Destroy() noexcept(std::is_nothrow_destructible_v<T>)
    {
        std::destroy_at(ValuePtr());
    }
};

} // namespace Noelware::Violet::__detail
