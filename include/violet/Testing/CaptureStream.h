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
//! # 🌺💜 `violet/Testing/CaptureStream.h`
//!
//! Provides [`violet::testing::CaptureStream`], a RAII utility for redirecting
//! and capturing the output of a [`std::ostream`] during tests.

#include <violet/Language/Macros.h>
#include <violet/Language/Policy.h>

#include <iostream>
#include <sstream>

namespace violet::testing {

/// RAII guard that redirects any [`std::ostream`] to an internal [`std::stringstream`] buffer
/// for the duration of its lifetime.
///
/// This is useful for tests where capturing standard output/error is necessary for validation.
///
/// ## Example
/// ```cpp
/// #include <violet/Testing/CaptureStream.h>
///
/// using violet::testing::CaptureStream;
/// {
///     CaptureStream stream;
///     std::cout << "hello, world!";
///     assert(stream.Get() == "hello, world!");
/// } // original `std::cout` buffer is restored
/// ```
struct VIOLET_API CaptureStream final {
    VIOLET_DISALLOW_COPY_AND_MOVE(CaptureStream);

    /// Constructs a `CaptureStream` that redirects `stream` to an internal buffer.
    ///
    /// The original buffer of `stream` is saved and will be restored when this
    /// object is destroyed.
    ///
    /// @param stream output stream to capture, defaults to [`std::cout`].
    VIOLET_IMPLICIT CaptureStream(std::ostream& stream = std::cout);

    /// Destroys the `CaptureStream` and restores the original stream buffer.
    ~CaptureStream();

    /// Returns the full contents captured from the stream so far as a [`std::string`].
    VIOLET_API auto Get() const noexcept(noexcept(std::declval<std::stringstream>().str())) -> std::string;

    /// Returns `true` if nothing has been written to the captured stream.
    VIOLET_API auto Empty() const noexcept(noexcept(std::declval<std::stringstream>().str())) -> bool;

    /// Returns `true` if the captured output contains `needle` as a substring.
    ///
    /// # Parameters
    /// - `needle`: The substring to search for within the captured output.
    ///
    /// # Example
    /// ```cpp
    /// violet::testing::CaptureStream cap(std::cout);
    /// std::cout << "hello, world";
    ///
    /// assert(cap.Contains("world"));
    /// assert(!cap.Contains("goodbye"));
    /// ```
    VIOLET_API auto Contains(std::string_view needle) const noexcept(noexcept(std::declval<std::stringstream>().str()))
        -> bool;

    VIOLET_API auto Find(std::string_view needle) const noexcept(noexcept(std::declval<std::stringstream>().str()))
        -> size_t;

private:
    std::ostream& n_stream;
    std::streambuf* n_original;
    std::stringstream n_captured;
};

} // namespace violet::testing
