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
//! # 🌺💜 `violet/Experimental/Coroutines/Generator.h`

#include <violet/Experimental/OneOf.h>
#include <violet/Iterator.h>

#include <coroutine>
#include <exception>

namespace violet::experimental::coro {

template<typename T>
struct NOELDOC_EXPERIMENTAL_SINCE("current") [[nodiscard("coroutine generators are lazily-started")]] Generator
    : public Iterator<Generator<T>> {
    VIOLET_DISALLOW_CONSTRUCTOR(Generator);
    VIOLET_DISALLOW_COPY(Generator);

    struct promise_type final {
        std::conditional_t<std::is_void_v<T>, std::exception_ptr, OneOf<T, std::exception_ptr>> Value;

        auto get_return_object() noexcept -> Generator
        {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        constexpr auto initial_suspend() noexcept -> std::suspend_always
        {
            return {};
        }

        constexpr auto final_suspend() noexcept -> std::suspend_always
        {
            return {};
        }

        template<typename U>
            requires std::convertible_to<U&&, T>
        auto yield_value(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>) -> std::suspend_always
            requires(!std::is_void_v<T>)
        {
            this->Value = VIOLET_FWD(U, value);
            return {};
        }

        void return_void() noexcept { }

        void unhandled_exception() noexcept
        {
            this->Value = std::current_exception();
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    NOELDOC_HIDE VIOLET_EXPLICIT Generator(handle_type coro) noexcept
        : n_coro(coro)
    {
    }

    VIOLET_IMPLICIT Generator(Generator&& other) noexcept
        : n_coro(std::exchange(other.n_coro, {}))
    {
    }

    auto operator=(Generator&& other) noexcept -> Generator&
    {
        if (this != &other) {
            this->destroy();
            this->n_coro = std::exchange(other.n_coro, {});
        }

        return *this;
    }

    ~Generator()
    {
        this->destroy();
    }

    auto Next() -> Optional<T>
    {
        if (!this->n_coro || this->n_coro.done()) {
            return Nothing;
        }

        promise_type& promise = this->n_coro.promise();
        this->n_coro.resume();

        if constexpr (std::is_void_v<T>) {
            if (promise.Value != nullptr) {
                std::rethrow_exception(promise.Value);
            }
        } else {
            if (promise.Value.template Holds<std::exception_ptr>()) {
                auto ex
                    = promise.Value.template GetUnchecked<std::exception_ptr>(Unsafe("we're in the exception state"));

                std::rethrow_exception(ex);
            }
        }

        if (this->n_coro.done()) {
            return Nothing;
        }

        return promise.Value.template GetUnchecked<T>(Unsafe("we are not in the exception state"));
    }

private:
    void destroy()
    {
        if (this->n_coro) {
            this->n_coro.destroy();
        }
    }

    handle_type n_coro;
};

} // namespace violet::experimental::coro
