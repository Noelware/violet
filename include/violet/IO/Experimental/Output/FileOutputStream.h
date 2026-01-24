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

#pragma once

#include <violet/Filesystem/File.h>
#include <violet/Filesystem/Path.h>
#include <violet/IO/Descriptor.h>
#include <violet/IO/Error.h>
#include <violet/IO/Experimental/OutputStream.h>

namespace violet::io {

struct FileOutputStream final: public OutputStream {
    VIOLET_IMPLICIT FileOutputStream() noexcept;

    template<std::convertible_to<filesystem::PathRef> Path>
    static auto Open(Path&& path) noexcept -> Result<FileOutputStream>;

    auto Write(Span<const UInt8> buf) noexcept -> Result<UInt> override;
    auto Flush() noexcept -> Result<void> override;

private:
    VIOLET_EXPLICIT FileOutputStream(filesystem::File file) noexcept;

    filesystem::File n_file;
};

} // namespace violet::io
