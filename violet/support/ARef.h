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
#include "violet/support/ref/Block.h"
#include "violet/violet.h"

#include <type_traits>

namespace Noelware::Violet {

template<typename T>
struct AWeak;

/// A thread-safe, reference counted object, analogous to Rust's [`std::sync::Arc`].
///
/// [`std::sync::Arc`]: https://doc.rust-lang.org/1.90.0/std/sync/struct.Arc.html
///
/// [`ARef`] uses atomic counters so that this can be passed through threads if `T` is
/// considered thread safe as well.
template<typename T>
struct ARef final {
    /// The type of this [`ARef`] uses.
    using value_type = T;

    ARef() = delete; // cannot create a [`ARef`] from empty data.

    constexpr VIOLET_IMPLICIT ARef(std::nullptr_t)
        : n_blk(nullptr)
    {
    }

    template<typename... Args>
        requires(!std::is_same_v<std::decay_t<Args>..., std::nullptr_t>)
    VIOLET_IMPLICIT ARef(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : ARef(std::in_place, VIOLET_FWD(Args, args)...)
    {
    }

    template<typename... Args>
    VIOLET_EXPLICIT ARef(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        blk_type* blk = blk_type::Alloc();
        try {
            ::new (blk->ValuePtr()) T(VIOLET_FWD(Args, args)...);
        } catch (...) {
            blk_type::Dealloc(blk);
            throw;
        }

        this->n_blk = blk;
    }

    ~ARef()
    {
        release();
    }

    ARef(const ARef& other) noexcept
        : n_blk(other.n_blk)
    {
        if (this->n_blk) {
            this->n_blk->Strong.fetch_add(1, MO_INC);
        }
    }

    auto operator=(const ARef& other) noexcept -> ARef&
    {
        if (this == &other) {
            return *this;
        }

        ARef tmp(other);
        std::swap(this->n_blk, tmp.n_blk);

        return *this;
    }

    constexpr ARef(ARef&& other) noexcept
        : n_blk(other.n_blk)
    {
        other.n_blk = nullptr;
    }

    constexpr auto operator=(ARef&& other) noexcept -> ARef&
    {
        if (this == &other) {
            return *this;
        }

        release();

        this->n_blk = other.n_blk;
        other.n_blk = nullptr;

        return *this;
    }

    auto Value() noexcept -> T*
    {
        return this->n_blk ? this->n_blk->ValuePtr() : nullptr;
    }

    auto Value() const noexcept -> const T*
    {
        return this->n_blk ? this->n_blk->ValuePtr() : nullptr;
    }

    auto operator*() noexcept -> T&
    {
        return *Value();
    }

    auto operator*() const noexcept -> const T&
    {
        return *Value();
    }

    auto operator->() noexcept -> T*
    {
        return Value();
    }

    auto operator->() const noexcept -> const T*
    {
        return Value();
    }

    [[nodiscard]] auto StrongCount() const noexcept -> usize
    {
        return this->n_blk ? this->n_blk->Strong.load(MO_LOAD) : 0;
    }

    [[nodiscard]] auto WeakCount() const noexcept -> usize
    {
        return this->n_blk ? this->n_blk->Weak.load(MO_LOAD) - 1 : 0;
    }

    auto Clone() const noexcept -> ARef
    {
        if (this->n_blk == nullptr) {
            return ARef<T>(nullptr);
        }

        auto oldSize = this->n_blk->Strong.load(std::memory_order_relaxed);
        if (oldSize > std::numeric_limits<usize>::max()) {
            // we should **never** be in this state, but if it ever does
            // then just coredump. i dont care. it's your fault.
            abort();
        }

        this->n_blk->Strong.fetch_add(1, MO_INC);
        return ARef<T>(this->n_blk);
    }

    auto Downgrade() const noexcept -> AWeak<T>;

    auto Release() noexcept(std::is_nothrow_destructible_v<T>)
    {
        release();
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_blk != nullptr;
    }

private:
    static constexpr auto MO_INC = std::memory_order_relaxed;
    static constexpr auto MO_LOAD = std::memory_order_acquire;
    static constexpr auto MO_DEC = std::memory_order_acq_rel;
    static constexpr auto MO_XCHG = std::memory_order_acq_rel;

    using blk_type = __detail::Block<T, true>;

    friend struct AWeak<T>;
    friend struct Optional<ARef<T>>;

    VIOLET_EXPLICIT ARef(blk_type* blk)
        : n_blk(blk)
    {
    }

    blk_type* n_blk;

    void release() noexcept(std::is_nothrow_destructible_v<T>)
    {
        if (this->n_blk == nullptr) {
            return;
        }

        if (this->n_blk->Strong.fetch_sub(1, MO_DEC) == 1) {
            this->n_blk->Destroy();
            if (this->n_blk->Weak.fetch_sub(1, MO_DEC) == 1) {
                blk_type::Dealloc(this->n_blk);
            }
        }

        this->n_blk = nullptr;
    }
};

template<typename T>
struct AWeak final {
    using value_type = T;

    AWeak() = delete; ///< cannot construct [`AWeak`] without data.

    constexpr AWeak(std::nullptr_t) noexcept
        : n_blk(nullptr)
    {
    }

    AWeak(const AWeak& other) noexcept
        : n_blk(other.n_blk)
    {
        if (this->n_blk) {
            this->n_blk->Weak.fetch_add(1, ARef<T>::MO_INC);
        }
    }

    AWeak(AWeak&& other) noexcept
        : n_blk(std::exchange(other.n_blk, nullptr))
    {
    }

    AWeak(const ARef<T>& ref) noexcept
        : n_blk(ref.n_blk)
    {
        if (this->n_blk) {
            this->n_blk->Weak.fetch_add(1, ARef<T>::MO_INC);
        }
    }

    ~AWeak() noexcept
    {
        reset();
    }

    auto operator=(const AWeak& other) noexcept -> AWeak&
    {
        if (this != &other) {
            AWeak tmp(other);
            std::swap(this->n_blk, tmp.n_blk);
        }

        return *this;
    }

    auto operator=(AWeak&& other) noexcept -> AWeak&
    {
        if (this != &other) {
            reset();

            this->n_blk = std::exchange(other.n_blk, nullptr);
        }

        return *this;
    }

    auto Upgrade() const noexcept -> Optional<ARef<T>>
    {
        if (this->n_blk == nullptr) {
            return Nothing;
        }

        usize cnt = this->n_blk->Strong.load(ARef<T>::MO_LOAD);
        while (cnt != 0) {
            if (this->n_blk->Strong.compare_exchange_weak(cnt, cnt + 1, ARef<T>::MO_XCHG, std::memory_order_relaxed)) {
                return Some<ARef<T>>(this->n_blk);
            }
        }

        return Nothing;
    }

    [[nodiscard]] auto StrongCount() const noexcept -> usize
    {
        return this->n_blk ? this->n_blk->Strong.load(ARef<T>::MO_INC) : 0;
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_blk != nullptr;
    }

private:
    friend struct ARef<T>;

    ARef<T>::blk_type* n_blk; ///< the allocation from `ARef`.

    VIOLET_EXPLICIT AWeak(ARef<T>::blk_type* blk)
        : n_blk(blk)
    {
    }

    void reset()
    {
        if (this->n_blk == nullptr) {
            return;
        }

        if (this->n_blk->Weak.fetch_sub(1, ARef<T>::MO_DEC) == 1) {
            ARef<T>::blk_type::Dealloc(this->n_blk);
        }

        this->n_blk = nullptr;
    }
};

template<typename T>
inline auto ARef<T>::Downgrade() const noexcept -> AWeak<T>
{
    if (this->n_blk) {
        this->n_blk->Weak.fetch_add(1, MO_INC);
    }

    return AWeak<T>(this->n_blk);
}

} // namespace Noelware::Violet
