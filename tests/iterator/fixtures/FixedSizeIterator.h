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

#include <violet/Iterator.h>

namespace violet::testing::fixtures {

template<typename T, violet::UInt N>
struct FixedSizeIterator final: public Iterator<FixedSizeIterator<T, N>> {
    using Item = T;

    VIOLET_DISALLOW_CONSTRUCTOR(FixedSizeIterator);
    ~FixedSizeIterator() = default;

    VIOLET_IMPLICIT FixedSizeIterator(Array<T, N> data)
        : n_data(data)
    {
    }

    auto Next() noexcept -> Optional<Item>
    {
        if (this->n_pos >= N) {
            return Nothing;
        }

        return this->n_data.at(this->n_pos++);
    }

    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        auto remaining = N - this->n_pos;
        return { remaining, remaining };
    }

private:
    Array<T, N> n_data;
    UInt n_pos = 0;
};

static_assert(Iterable<FixedSizeIterator<Int32, 25>>);

template<typename T, violet::UInt N>
struct DoubleEndedFixedSizeIterator final: public Iterator<DoubleEndedFixedSizeIterator<T, N>> {
    using Item = T;

    VIOLET_DISALLOW_CONSTRUCTOR(DoubleEndedFixedSizeIterator);
    ~DoubleEndedFixedSizeIterator() = default;

    VIOLET_IMPLICIT DoubleEndedFixedSizeIterator(Array<T, N> data)
        : n_data(data)
    {
    }

    auto Next() -> Optional<Item>
    {
        if (this->n_front > this->n_end) {
            return Nothing;
        }

        return this->n_data.at(this->n_front++);
    }

    auto NextBack() -> Optional<Item>
    {
        if (this->n_front >= this->n_end) {
            return Nothing;
        }

        if (this->n_end == 0) {
            return Nothing;
        }

        return this->n_data.at(this->n_end--);
    }

    [[nodiscard]] auto SizeHint() const noexcept -> violet::SizeHint
    {
        if (n_front >= n_end) {
            return { 0, 0 };
        }

        auto remaining = this->n_end - this->n_front;
        return { remaining, remaining };
    }

private:
    Array<T, N> n_data;
    UInt n_front = 0;
    UInt n_end = N - 1;
};

static_assert(Iterable<DoubleEndedFixedSizeIterator<Int32, 25>>);
static_assert(DoubleEndedIterable<DoubleEndedFixedSizeIterator<Int32, 25>>);

} // namespace violet::testing::fixtures
