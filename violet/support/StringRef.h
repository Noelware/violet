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
#include "violet/violet.h"

#include <cstddef>
#include <ostream>

namespace Noelware::Violet {

/// Represents a reference to a non-owning string that extends the capabilities of [`std::string_view`].
struct StringRef final {
    /// Constructs a empty [`StringRef`].
    constexpr VIOLET_IMPLICIT StringRef() = default;

    /// Disallow constructing a [`StringRef`] with a null pointer.
    VIOLET_IMPLICIT StringRef(std::nullptr_t) = delete;

    /// Creates a new [`StringRef`] from an existing C string.
    /// @param str the C string that this string ref will hold into.
    constexpr VIOLET_IMPLICIT StringRef(CStr str)
        : n_data(str)
        , n_len(std::strlen(str))
    {
    }

    /// Creates a new [`StringRef`] from a copied [`std::string`].
    /// @param str the string that this will hold into.
    constexpr VIOLET_IMPLICIT StringRef(const String& str)
        : n_data(str.data())
        , n_len(str.length())
    {
    }

    /// Creates a new [`StringRef`] from a [`std::string_view`].
    /// @param str the string that this will hold into.
    constexpr VIOLET_IMPLICIT StringRef(Str str)
        : n_data(str.data())
        , n_len(str.size())
    {
    }

    constexpr VIOLET_EXPLICIT StringRef(CStr data, usize len)
        : n_data(data)
        , n_len(len)
    {
    }

    /// Returns **true** if this string contains data.
    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return !this->Empty();
    }

    /// Transitional operator from `StringRef` -> `std::string_view`. When using this operator,
    /// the data will not be owned by the new `std::string_view`.
    constexpr VIOLET_IMPLICIT operator std::string_view() const noexcept
    {
        return { this->n_data, this->n_len };
    }

    /// Transitional operator from `StringRef` -> `const char*`.
    constexpr VIOLET_IMPLICIT operator CStr() const noexcept
    {
        return this->n_data;
    }

    auto operator[](usize idx) const -> char
    {
        assert(idx < this->n_len && "invalid index");
        return this->n_data[idx];
    }

    /// Returns the C string data that this [`StringRef`] "techincally" owns for now.
    [[nodiscard]] constexpr auto Data() const noexcept -> CStr
    {
        return this->n_data;
    }

    /// Returns the length of this [`StringRef`].
    [[nodiscard]]
    constexpr auto Length() const noexcept -> usize
    {
        return this->n_len;
    }

    /// Returns **true** if this string is empty.
    [[nodiscard]] constexpr auto Empty() const noexcept -> bool
    {
        return this->n_len == 0;
    }

    [[nodiscard]] constexpr auto StartsWith(char ch) const noexcept -> bool
    {
        if (auto first = this->First()) {
            return ch == *first;
        }

        return false;
    }

    /// Returns **true** if the string starts with the given `prefix`.
    /// @param prefix the prefix to check if this string starts with
    [[nodiscard]] constexpr auto StartsWith(StringRef prefix) const noexcept -> bool
    {
        if (prefix.Length() > this->n_len)
            return false;

        for (usize i = 0; i < prefix.Length(); i++) {
            if (this->n_data[i] != prefix.n_data[i]) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr auto First() const noexcept -> Optional<char>
    {
        return (this->n_len != 0U) ? Some<char>(this->n_data[0]) : Nothing;
    }

    [[nodiscard]] constexpr auto Last() const noexcept -> Optional<char>
    {
        return this->n_len != 0U ? Some<char>(this->n_data[this->n_len - 1]) : Nothing;
    }

    [[nodiscard]] constexpr auto TrimStart() const noexcept -> StringRef
    {
        usize start = 0;
        while (start < this->n_len && isASCIIWS(this->n_data[start])) {
            start++;
        }

        return StringRef(this->n_data + start, this->n_len - start);
    }

    [[nodiscard]] constexpr auto TrimEnd() const noexcept -> StringRef
    {
        usize end = this->n_len;
        while (end > 0 && isASCIIWS(this->n_data[end - 1])) {
            end--;
        }

        return StringRef(this->n_data, end);
    }

    [[nodiscard]] constexpr auto Trim() const noexcept -> StringRef
    {
        usize start = 0;
        usize end = this->n_len;

        while (start < this->n_len && isASCIIWS(this->n_data[start])) {
            start++;
        }

        while (end > 0 && isASCIIWS(this->n_data[end - 1])) {
            end--;
        }

        return StringRef(this->n_data + start, end - start);
    }

    [[nodiscard]] constexpr auto StripPrefix(StringRef prefix) const noexcept -> Optional<StringRef>
    {
        if (!StartsWith(prefix))
            return Nothing;

        return Some<StringRef>(StringRef(this->n_data + prefix.Length(), this->n_len - prefix.Length()));
    }

    [[nodiscard]] constexpr auto AsBytes() const noexcept -> Span<const uint8>
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return { reinterpret_cast<const uint8*>(this->n_data), this->n_len };
    }

    [[nodiscard]] auto Split(char delim) const noexcept -> Vec<StringRef>
    {
        Vec<StringRef> parts;
        usize start = 0;

        for (usize i = 0; i < Length(); i++) {
            if (n_data[i] == delim) {
                parts.emplace_back(n_data + start, i - start);
                start = i + 1;
            }
        }

        parts.emplace_back(n_data + start, n_len - start);
        return parts;
    }

    [[nodiscard]] auto Split(StringRef delim) const noexcept -> Vec<StringRef>
    {
        Vec<StringRef> parts;
        usize start = 0;

        if (delim.Empty()) {
            return { *this };
        }

        for (usize i = 0; i < delim.Length(); i++) {
            bool match = true;
            for (usize j = 0; j < delim.Length(); j++) {
                if (this->n_data[i + j] != delim.n_data[j]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                parts.emplace_back(n_data + start, i - start);
                start = i + delim.Length();
                i = start;
            } else {
                i++;
            }
        }

        parts.emplace_back(n_data + start, n_len - start);
        return parts;
    }

    VIOLET_IMPL_EQUALITY_SINGLE(StringRef, lhs, rhs, {
        if (lhs.Length() != rhs.Length()) {
            return false;
        }

        for (usize i = 0; i < lhs.Length(); i++) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }

        return true;
    });

    VIOLET_IMPL_EQUALITY(const StringRef&, CStr, lhs, rhs, {
        if (!rhs) {
            return false;
        }

        usize idx = 0;
        for (; idx < lhs.Length(); idx++) {
            if (rhs[idx] == '\0' || lhs[idx] != rhs[idx]) {
                return false;
            }
        }

        return rhs[idx] == '\0';
    });

    VIOLET_IMPL_EQUALITY(const StringRef&, const String&, lhs, rhs, { return lhs == rhs.data(); });
    VIOLET_IMPL_EQUALITY(const StringRef&, const Str&, lhs, rhs, { return lhs == rhs.data(); });

    VIOLET_OSTREAM_IMPL(const StringRef&)
    {
        return os.write(self.Data(), static_cast<std::streamsize>(self.Length()));
    }

private:
    CStr n_data = nullptr;
    usize n_len = 0;

    [[nodiscard]] constexpr auto isASCIIWS(char ch) const noexcept -> bool
    {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v' || ch == '\f';
    }
};

} // namespace Noelware::Violet
