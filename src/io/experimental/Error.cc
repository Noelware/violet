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

#include <violet/IO/Experimental/Error.h>

namespace violet::io::experimental {

auto Error::OSError(SourceLocation loc) -> Error
{
    return Error(PlatformError{}, loc);
}

auto Error::FromOSError(native_error_code ec, SourceLocation loc) -> Error
{
    return Error(PlatformError{ec}, loc);
}

auto Error::RawOSError() const noexcept -> Optional<native_error_code>
{
    return this->n_repr.Match(
        // clang-format off
        [](PlatformError error)  -> Optional<native_error_code> { return Some(error.Get()); },
        [](ErrorKind)            -> Optional<native_error_code> { return Nothing; },
        [](const SimpleMessage&) -> Optional<native_error_code> { return Nothing; }
        // clang-format on
    );
}

auto Error::Kind() const noexcept -> ErrorKind
{
    return this->n_repr.Visit([](const auto& value) -> ErrorKind {
        using ty = std::remove_cvref_t<decltype(value)>;

        if constexpr (std::same_as<ty, ErrorKind>) {
            return value;
        } else if constexpr (std::same_as<ty, SimpleMessage>) {
            return value.Kind;
        } else if constexpr (std::same_as<ty, PlatformError>) {
            return value.Kind();
        } else {
            VIOLET_UNREACHABLE();
        }
    });
}

auto Error::ToString() const -> String
{
    // clang-format off
    return std::format(
        "I/o error{}   [in {}:{}:{}]",
        this->n_repr.Match(
            [](PlatformError error) -> String { return std::format(" (system error «{}»): {}", error.Get(), error); },
            [](const SimpleMessage& msg) -> String { return std::format(" ({}): {}", msg.Kind, msg.Message); },
            [](ErrorKind kind) -> String { return std::format(": {}", kind); }
        ),
        this->n_loc.File,
        this->n_loc.Line,
        this->n_loc.Column
    );
    // clang-format on
}

} // namespace violet::io::experimental
