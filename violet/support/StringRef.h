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
//
//! # 🌺💜 `violet/support/StringRef.h`
//! This header file contains the blueprint of the [`Noelware::Violet::StringRef`] type, which is
//! an immutable, UTF-8 encoded, non-owning string akin to Rust's [`str`].
//!
//! [`str`]: https://doc.rust-lang.org/1.90.0/std/primitive.str.html

#pragma once

#include "violet/container/Optional.h"
#include "violet/violet.h"

#include <cstddef>
#include <limits>
#include <ostream>

namespace Noelware::Violet {

/// An immutable, UTF-8 encoded, non-owning string akin to Rust's [`str`] or C++'s
/// [`std::string_view`].
///
/// [`str`]: https://doc.rust-lang.org/1.90.0/std/primitive.str.html
struct StringRef final {
    using value_type = char;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = usize;
    using difference_type = std::ptrdiff_t;

    static constexpr size_type npos = Str::npos;

    VIOLET_IMPLICIT StringRef() noexcept = default;
    VIOLET_IMPLICIT StringRef(std::nullptr_t) noexcept = delete;
    VIOLET_IMPLICIT StringRef(CStr str) noexcept;
    VIOLET_IMPLICIT StringRef(const String&) noexcept;
    VIOLET_IMPLICIT StringRef(Str) noexcept;
    VIOLET_EXPLICIT StringRef(CStr data, usize len) noexcept;

    VIOLET_IMPLICIT operator Str() const noexcept;
    VIOLET_EXPLICIT operator bool() const noexcept;
    VIOLET_EXPLICIT operator const_pointer() const noexcept;

    auto operator[](size_type) const noexcept -> value_type;
    auto At(size_type) const noexcept -> Optional<value_type>;
    auto First() const noexcept -> Optional<value_type>;
    auto Last() const noexcept -> Optional<value_type>;
    auto Data() const noexcept -> const_pointer;

    auto Empty() const noexcept -> bool;
    auto Size() const noexcept -> size_type;

    auto StartsWith(value_type) const noexcept -> bool;
    auto StartsWith(StringRef) const noexcept -> bool;
    auto EndsWith(value_type) const noexcept -> bool;
    auto EndsWith(StringRef) const noexcept -> bool;
    auto TrimStart() const noexcept -> StringRef;
    auto TrimEnd() const noexcept -> StringRef;
    auto Trim() const noexcept -> StringRef;
    auto StripPrefix(StringRef) const noexcept -> Optional<StringRef>;
    auto StripSuffix(StringRef) const noexcept -> Optional<StringRef>;
    auto AsBytes() const noexcept -> Span<const uint8>;
    auto Split(value_type) const noexcept -> Vec<StringRef>;
    auto Split(StringRef) const noexcept -> Vec<StringRef>;

    VIOLET_IMPL_EQUALITY_SINGLE(StringRef, lhs, rhs, {
        if (lhs.Size() != rhs.Size()) {
            return false;
        }

        for (usize i = 0; i < lhs.Size(); i++) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }

        return true;
    });

    VIOLET_IMPL_EQUALITY(StringRef, CStr, lhs, rhs, {
        if (!rhs) {
            return false;
        }

        usize idx = 0;
        for (; idx < lhs.Size(); idx++) {
            if (rhs[idx] == '\0' || lhs[idx] != rhs[idx]) {
                return false;
            }
        }

        return rhs[idx] == '\0';
    });

    VIOLET_IMPL_EQUALITY(StringRef, Str, lhs, rhs, { return lhs == rhs.data(); });
    VIOLET_IMPL_EQUALITY(StringRef, const String&, lhs, rhs, { return lhs == rhs.data(); });

    VIOLET_OSTREAM_IMPL(StringRef)
    {
        assert(self.Size() < std::numeric_limits<std::streamsize>::max());
        return os.write(self.Data(), static_cast<std::streamsize>(self.Size()));
    }

private:
    size_type n_len;
    const_pointer n_data;
};

} // namespace Noelware::Violet
