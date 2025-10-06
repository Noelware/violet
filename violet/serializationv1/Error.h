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
#include "violet/serialization/Type.h"
#include "violet/violet.h"

#include <any>

namespace Noelware::Violet::Serialization {

struct Expectation final {
    Expectation() = delete;

    explicit Expectation(Type expected, Type actual)
        : n_expected(expected)
        , n_actual(actual)
    {
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return std::format("expectation failed: expected type {}, but received {} instead",
            Noelware::Violet::ToString(n_expected), Noelware::Violet::ToString(n_actual));
    }

private:
    Type n_expected;
    Type n_actual;
};

struct Error final {
    Error() = default;

    template<typename T>
    explicit Error(T error)
        : n_inner(std::make_any<T>(VIOLET_MOVE(error)))
    {
    }

    template<typename U>
    [[nodiscard]] auto Cast() const noexcept -> Optional<std::decay_t<U>>
    {
        try {
            auto ptr = std::any_cast<std::decay_t<U>>(&this->n_inner);
            return Optional<std::decay_t<U>>(*ptr);
        } catch (const std::bad_any_cast&) {
            return Nothing;
        }
    }

    template<typename U>
    auto Cast() & noexcept -> Optional<std::decay_t<U>>
    {
        try {
            auto ptr = std::any_cast<std::decay_t<U>>(&this->n_inner);
            return Optional<std::decay_t<U>>(*ptr);
        } catch (const std::bad_any_cast&) {
            return Nothing;
        }
    }

    template<typename U>
    auto Cast() && noexcept -> Optional<std::decay_t<U>>
    {
        try {
            auto ptr = std::any_cast<std::decay_t<U>>(&this->n_inner);
            return Optional<std::decay_t<U>>(*ptr);
        } catch (const std::bad_any_cast&) {
            return Nothing;
        }
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        if (auto str = this->Cast<String>(); str.HasValue()) {
            return *str;
        }

        if (auto str = this->Cast<Str>(); str.HasValue()) {
            return String(*str);
        }

        if (auto expect = this->Cast<Expectation>(); expect.HasValue()) {
            return expect->ToString();
        }

        if (auto logic = this->Cast<std::logic_error>(); logic.HasValue()) {
            return std::format("logic error: {}", logic->what());
        }

        // If there is any other things that throw in this library and are not handled,
        // submit a PR and we might create a branch here.
        const auto& id = this->n_inner.type();
        return std::format("<unknown error with type {}@{}>", id.name(), id.hash_code());
    }

private:
    Any n_inner;
};

} // namespace Noelware::Violet::Serialization
