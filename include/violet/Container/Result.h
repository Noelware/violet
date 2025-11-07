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

#include "violet/Violet.h"
#include <type_traits>

#if VIOLET_USE_RTTI
#include "violet/Support/Demangle.h"
#endif

#include <expected>

namespace violet {

template<typename T>
struct Optional;

template<typename T, typename E>
struct Result;

/// Newtype for [`std::unexpected`]
template<typename E>
using Err = std::unexpected<E>;

/// Creates a new `Result<T, E>` that contains a successful value.
///
/// @tparam T The underlying success type to construct.
/// @tparam E The underlying error type to pass.
/// @tparam Args The arguments to pass to `T`'s constructor.
/// @param args The arguments to pass to `T`'s constructor.
/// @return An `Optional<T>` with the value constructed.
template<typename T, typename E, typename... Args>
constexpr static auto Ok(Args&&... args) -> Result<T, E>
{
    static_assert(!std::is_void_v<T>, "`void` types cannot be constructed. use `{}` for void-like results");

    return Result<T, E>(std::in_place_index<0>, VIOLET_FWD(Args, args)...);
}

/// Representation of a successful or failed state, analgous of Rust's [`std::result::Result`].
///
/// [`std::result::Result`]: https://doc.rust-lang.org/1.90.0/std/result/enum.Result.html
template<typename T, typename E>
struct [[nodiscard("always check the error state")]] VIOLET_API Result final {
    using value_type = T;
    using error_type = E;

    constexpr VIOLET_IMPLICIT Result() = delete;

    constexpr VIOLET_IMPLICIT Result(const T& ok)
        : n_ok(true)
    {
        ::new (&this->n_storage.ok) T(ok);
    }

    constexpr VIOLET_IMPLICIT Result(T&& ok) noexcept(std::is_nothrow_move_constructible_v<T>)
        : n_ok(true)
    {
        ::new (&this->n_storage.ok) T(VIOLET_MOVE(ok));
    }

    /// @internal
    template<typename... Args>
    constexpr VIOLET_EXPLICIT Result(std::in_place_index_t<0L>, Args&&... args)
        : n_ok(true)
    {
        ::new (&this->n_storage.ok) T(VIOLET_FWD(Args, args)...);
    }

    constexpr VIOLET_IMPLICIT Result(const Err<E>& err)
    {
        ::new (&this->n_storage.err) violet::Err<E>(err);
    }

    constexpr VIOLET_IMPLICIT Result(Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<E>)
    {
        ::new (&this->n_storage.err) violet::Err<E>(VIOLET_MOVE(err));
    }

    ~Result()
    {
        destroy();
    }

    constexpr VIOLET_IMPLICIT Result(const Result& other)
        : n_ok(other.n_ok)
    {
        if (other.n_ok) {
            ::new (&this->n_storage.ok) T(other.n_storage.ok);
        } else {
            ::new (&this->n_storage.err) violet::Err<E>(other.n_storage.err);
        }
    }

    constexpr auto operator=(const Result& other) noexcept -> Result&
    {
        if (this != &other) {
            destroy();

            this->n_ok = other.n_ok;
            if (other.n_ok) {
                ::new (&this->n_storage.ok) T(other.n_storage.ok);
            } else {
                ::new (&this->n_storage.err) violet::Err<E>(other.n_storage.err);
            }
        }

        return *this;
    }

    constexpr Result(Result&& other) noexcept(
        std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>)
        : n_ok(other.n_ok)
    {
        if (other.n_ok) {
            ::new (&this->n_storage.ok) T(VIOLET_MOVE(other.n_storage.ok));
        } else {
            ::new (&this->n_storage.err) violet::Err<E>(VIOLET_MOVE(other.n_storage.err));
        }
    }

    constexpr auto operator=(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>
        && std::is_nothrow_move_constructible_v<E> && std::is_nothrow_move_assignable_v<T>
        && std::is_nothrow_move_assignable_v<E>) -> Result&
    {
        if (this != &other) {
            // If both are same variant, move-assign; else destroy + reconstruct.
            if (this->n_ok && other.n_ok) {
                this->n_storage.ok = VIOLET_MOVE(other.n_storage.ok);
            } else if (!this->n_ok && !other.n_ok) {
                this->n_storage.err = VIOLET_MOVE(other.n_storage.err);
            } else {
                destroy();
                this->n_ok = other.n_ok;
                if (other.n_ok) {
                    ::new (&this->n_storage.ok) T(VIOLET_MOVE(other.n_storage.ok));
                } else {
                    ::new (&this->n_storage.err) violet::Err<E>(VIOLET_MOVE(other.n_storage.err));
                }
            }
        }

        return *this;
    }

    constexpr auto Ok() const noexcept -> bool
    {
        return this->n_ok;
    }

    constexpr auto Err() const noexcept -> bool
    {
        return !this->Ok();
    }

    constexpr auto IntoOpt() const noexcept -> Optional<T>
    {
        return Ok() ? Optional<T>(Value()) : std::nullopt;
    }

    constexpr auto Value() noexcept -> T&
    {
        VIOLET_DEBUG_ASSERT(Ok());
        return this->n_storage.ok;
    }

    constexpr auto Value() const noexcept -> const T&
    {
        VIOLET_DEBUG_ASSERT(Ok());
        return this->n_storage.ok;
    }

    constexpr auto Error() noexcept -> E&
    {
        VIOLET_DEBUG_ASSERT(Err());
        return this->n_storage.err.error();
    }

    constexpr auto Error() const noexcept -> const E&
    {
        VIOLET_DEBUG_ASSERT(Err());
        return this->n_storage.err.error();
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->Ok();
    }

    constexpr VIOLET_EXPLICIT operator std::expected<T, E>() const noexcept
    {
        return Ok() ? std::expected<T, E>(Value()) : violet::Err<E>(Error());
    }

    constexpr auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        if (Ok()) {
            if constexpr (Stringify<T>) {
                return os << violet::ToString(Value());
            }

#if VIOLET_USE_RTTI
            const auto& type = typeid(T);
            return os << "ok variant type `" << util::DemangleCXXName(type.name()) << '@' << type.hash_code()
                      << "` not streamable»";
#else
            return os << "«type `T` not streamable»";
#endif
        } else {
            if constexpr (Stringify<T>) {
                return os << violet::ToString(Value());
            }

#if VIOLET_USE_RTTI
            const auto& type = typeid(E);
            return os << "error variant type `" << util::DemangleCXXName(type.name()) << '@' << type.hash_code()
                      << "` not streamable»";
#else
            return os << "«type `E` not streamable»";
#endif
        }
    }

private:
    mutable bool n_ok = false;
    union storage_t { // NOLINT(cppcoreguidelines-special-member-functions)
        T ok;
        violet::Err<E> err;

        storage_t() {}
        ~storage_t() {}
    } n_storage;

    void destroy()
    {
        if (this->n_ok) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                this->n_storage.ok.~T();
            }
        } else {
            if constexpr (!std::is_trivially_destructible_v<E>) {
                this->n_storage.err.~Err<E>();
            }
        }
    }
};

/// Specialization for `void`-returned types of a [`Result`].
template<typename E>
struct [[nodiscard("always check the error state before discard")]] VIOLET_API Result<void, E> final {
    using value_type = void;
    using error_type = E;

    constexpr Result() noexcept = default;
    constexpr VIOLET_IMPLICIT Result(const Err<E>& err)
        : n_err(new violet::Err<E>(err))
    {
    }

    constexpr VIOLET_IMPLICIT Result(Err<E>&& err) noexcept(std::is_nothrow_move_constructible_v<E>)
        : n_err(new violet::Err<E>(VIOLET_MOVE(err)))
    {
    }

    template<typename U = E>
        requires std::convertible_to<U, E>
    constexpr VIOLET_IMPLICIT Result(const U& err)
        : n_err(new violet::Err<U>(err))
    {
    }

    template<typename U = E>
        requires std::convertible_to<U, E>
    constexpr VIOLET_IMPLICIT Result(U&& err)
        : n_err(new violet::Err<U>(VIOLET_FWD(U, err)))
    {
    }

    ~Result()
    {
        delete this->n_err;
    }

    constexpr VIOLET_IMPLICIT Result(const Result& other)
        : n_err(other.n_err ? new violet::Err<E>(*other.n_err->error()) : nullptr)
    {
    }

    constexpr auto operator=(const Result& other) noexcept -> Result&
    {
        if (this != &other) {
            delete this->n_err;
            this->n_err = other.n_err ? new violet::Err<E>(*other.n_err) : nullptr;
        }

        return *this;
    }

    constexpr VIOLET_IMPLICIT Result(Result&& other) noexcept
        : n_err(std::exchange(other.n_err, nullptr))
    {
    }

    constexpr auto operator=(Result&& other) noexcept -> Result&
    {
        if (this != &other) {
            delete this->n_err;
            this->n_err = std::exchange(other.n_err, nullptr);
        }

        return *this;
    }

    [[nodiscard]] constexpr auto Ok() const noexcept -> bool
    {
        return this->n_err == nullptr;
    }

    [[nodiscard]] constexpr auto Err() const noexcept -> bool
    {
        return !this->Ok();
    }

    [[nodiscard]] constexpr auto Error() const -> const E&
    {
        assert(Err());
        return this->n_err->error();
    }

    [[nodiscard]] constexpr auto Error() -> E&
    {
        assert(Err());
        return this->n_err->error();
    }

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->n_err == nullptr;
    }

    constexpr VIOLET_IMPLICIT operator std::expected<void, E>() noexcept
    {
        return Ok() ? std::expected<void, E>({}) : violet::Err(Error());
    }

    constexpr VIOLET_IMPLICIT operator std::unexpected<E>() noexcept
    {
        assert(Err());
        return Err(Error());
    }

    constexpr auto operator<<(std::ostream& os) noexcept -> std::ostream&
    {
        if (Ok()) {
            return os << "«result of 'void' type»";
        }

        if constexpr (Stringify<E>) {
            return os << violet::ToString(Error());
        }

#if VIOLET_USE_RTTI
        const auto& type = typeid(E);
        return os << "error variant type `" << util::DemangleCXXName(type.name()) << '@' << type.hash_code()
                  << "` not streamable»";
#else
        return os << "«type `E` not streamable»";
#endif
    }

private:
    violet::Err<E>* n_err = nullptr;
};

} // namespace violet
