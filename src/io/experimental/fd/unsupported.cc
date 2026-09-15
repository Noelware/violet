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

#include <violet/IO/Experimental/Descriptor.h>

namespace violet::io::experimental::fd_internal {

auto isFdValid(RawFd) noexcept -> bool
{
    return false;
}

#define impl(method, ret, ...)                                                                                         \
    auto method(__VA_ARGS__) noexcept -> ret                                                                           \
    {                                                                                                                  \
        return Err<Error>(ErrorKind::Unsupported, "unsupported operation: " #method);                                  \
    }

impl(read, Result<UInt>, RawFd, Span<UInt8>);
impl(write, Result<UInt>, RawFd, Span<const UInt8>);
impl(flush, Result<void>, RawFd);
impl(close, Result<void>, RawFd);

auto toString(RawFd) noexcept -> String
{
    return "fd(unsupported)";
}

} // namespace violet::io::experimental::fd_internal
