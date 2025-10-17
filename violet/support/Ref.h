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
struct Weak;

/// A reference counted object, analogous to Rust's [`std::rc::Rc`].
///
/// [`std::rc::Rc`]: https://doc.rust-lang.org/1.90.0/std/rc/struct.Rc.html
template<typename T>
struct Ref final {
    using value_type = T;

    Ref() = delete;

    constexpr VIOLET_IMPLICIT Ref(std::nullptr_t)
        : n_blk(nullptr)
    {
    }

    template<typename... Args>
        requires(!std::is_same_v<std::decay_t<Args>..., std::nullptr_t>)
    VIOLET_IMPLICIT Ref(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : Ref(std::in_place, VIOLET_FWD(Args, args)...)
    {
    }

    template<typename... Args>
    VIOLET_EXPLICIT Ref(std::in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        blk_type* blk = blk_type::Alloc();
        try {
            ::new (blk->ValuePtr()) T(VIOLET_FWD(Args, args)...);
        } catch (...) {
            blk_type::Dealloc(this->n_blk);
            throw;
        }

        this->n_blk = blk;
    }

    constexpr VIOLET_IMPLICIT Ref(const Ref& other) noexcept
        : n_blk(other.n_blk)
    {
        if (this->n_blk) {
            this->n_blk->Strong++;
        }
    }

    constexpr VIOLET_IMPLICIT Ref(Ref&& other) noexcept
        : n_blk(std::exchange(other.n_blk, nullptr))
    {
    }

    ~Ref()
    {
        release();
    }

    constexpr auto operator=(const Ref& other) noexcept -> Ref&
    {
        if (this != &other) {
            Ref tmp(other);
            std::swap(this->n_blk, tmp.n_blk);
        }

        return *this;
    }

    constexpr auto operator=(Ref&& other) noexcept -> Ref&
    {
        if (this == &other) {
            return *this;
        }

        release();

        this->n_blk = other.n_blk;
        other.n_blk = nullptr;

        return *this;
    }

    constexpr auto Value() noexcept -> T*
    {
        return this->n_blk ? this->n_blk->ValuePtr() : nullptr;
    }

    constexpr auto Value() const noexcept -> const T*
    {
        return this->n_blk ? this->n_blk->ValuePtr() : nullptr;
    }

    constexpr auto operator*() noexcept -> T&
    {
        return *Value();
    }

    constexpr auto operator*() const noexcept -> const T&
    {
        return *Value();
    }

    constexpr auto operator->() noexcept -> T*
    {
        return Value();
    }

    constexpr auto operator->() const noexcept -> const T*
    {
        return Value();
    }

    [[nodiscard]] constexpr auto StrongCount() const noexcept -> usize
    {
        return this->n_blk ? this->n_blk->Strong : 0;
    }

    [[nodiscard]] constexpr auto WeakCount() const noexcept -> usize
    {
        return this->n_blk ? this->n_blk->Weak - 1 : 0;
    }

    constexpr auto Clone() const noexcept -> Ref
    {
        if (this->n_blk == nullptr) {
            return Ref<T>(nullptr);
        }

        if (this->n_blk->Strong > std::numeric_limits<usize>::max()) {
            // we should **never** be in this state, but if it ever does
            // then just coredump. i dont care. it's your fault.
            abort();
        }

        this->n_blk->Strong++;
        return Ref<T>(this->n_blk);
    }

    constexpr auto Downgrade() const noexcept -> Weak<T>;

    auto Release() noexcept(std::is_nothrow_destructible_v<T>)
    {
        release();
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_blk != nullptr;
    }

private:
    using blk_type = __detail::Block<T, false>;

    friend struct Weak<T>;
    friend struct Optional<Ref<T>>;

    blk_type* n_blk;

    VIOLET_EXPLICIT Ref(blk_type* blk)
        : n_blk(blk)
    {
    }

    void release() noexcept(std::is_nothrow_destructible_v<T>)
    {
        if (this->n_blk == nullptr) {
            return;
        }

        if (this->n_blk->Strong-- == 0) {
            this->n_blk->Destroy();
            if (this->n_blk->Weak-- == 0) {
                blk_type::Dealloc(this->n_blk);
            }
        }

        this->n_blk = nullptr;
    }
};

template<typename T>
struct Weak final {
    using value_type = T;

    Weak() = delete; ///< cannot construct [`Weak`] without data.

    constexpr Weak(std::nullptr_t) noexcept
        : n_blk(nullptr)
    {
    }

    constexpr Weak(const Weak& other) noexcept
        : n_blk(other.n_blk)
    {
        if (this->n_blk) {
            this->n_blk->Weak++;
        }
    }

    constexpr Weak(Weak&& other) noexcept
        : n_blk(std::exchange(other.n_blk, nullptr))
    {
    }

    constexpr Weak(const Ref<T>& ref) noexcept
        : n_blk(ref.n_blk)
    {
        if (this->n_blk) {
            this->n_blk->Weak++;
        }
    }

    ~Weak() noexcept
    {
        reset();
    }

    auto operator=(const Weak& other) noexcept -> Weak&
    {
        if (this != &other) {
            Weak tmp(other);
            std::swap(this->n_blk, tmp.n_blk);
        }

        return *this;
    }

    constexpr auto operator=(Weak&& other) noexcept -> Weak&
    {
        if (this != &other) {
            reset();

            this->n_blk = std::exchange(other.n_blk, nullptr);
        }

        return *this;
    }

    auto Upgrade() const noexcept -> Optional<Ref<T>>
    {
        if (this->n_blk == nullptr || this->n_blk->Strong == 0) {
            return Nothing;
        }

        this->n_blk->Strong++;
        return Some<Ref<T>>(this->n_blk);
    }

    [[nodiscard]] constexpr auto StrongCount() const noexcept -> usize
    {
        return this->n_blk ? this->n_blk->Strong : 0;
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_blk != nullptr;
    }

private:
    friend struct Ref<T>;

    Ref<T>::blk_type* n_blk; ///< the allocation from `Ref`.

    VIOLET_EXPLICIT Weak(Ref<T>::blk_type* blk)
        : n_blk(blk)
    {
    }

    void reset()
    {
        if (this->n_blk == nullptr) {
            return;
        }

        if (this->n_blk->Weak-- == 1) {
            Ref<T>::blk_type::Dealloc(this->n_blk);
        }

        this->n_blk = nullptr;
    }
};

template<typename T>
constexpr auto Ref<T>::Downgrade() const noexcept -> Weak<T>
{
    if (this->n_blk) {
        this->n_blk->Weak++;
    }

    return Weak<T>(this->n_blk);
}

} // namespace Noelware::Violet
