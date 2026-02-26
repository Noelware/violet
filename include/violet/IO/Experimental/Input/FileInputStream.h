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
#include <violet/IO/Error.h>
#include <violet/IO/Experimental/InputStream.h>

namespace violet::io::experimental {

/// An [`InputStream`] that reads data from a file on disk.
///
/// This stream wraps a [`violet::filesystem::File`] and provides a sequential,
/// read-only interface. It supports reading, skipping, and querying the number
/// of available bytes.
///
/// ## Example
/// ```cpp
/// #include <violet/IO/Experimental/Input/FileInputStream.h>
///
/// using namespace violet;
/// using namespace violet::io::experimental;
///
/// auto stream = FileInputStream::Open("config.toml");
/// if (stream.Err()) {
///     std::println(std::cerr, "failed to open file `config.toml': {}", stream.Error());
///     std::abort();
/// }
///
/// UInt8 buf[1024];
/// auto action = stream->Read(buf);
/// ```
struct FileInputStream final: public InputStream {
    VIOLET_DISALLOW_CONSTRUCTOR(FileInputStream);

    /// Constructs a `FileInputStream` from a [`filesystem::File`].
    /// @param file an already opened file, moved into the stream.
    VIOLET_IMPLICIT FileInputStream(filesystem::File&& file) noexcept
        : n_file(VIOLET_MOVE(file))
    {
    }

    /// Opens a file at the specified path for reading.
    /// @param path the file path to open.
    template<std::convertible_to<filesystem::PathRef> Path>
    static auto Open(Path&& path) noexcept -> Result<FileInputStream>
    {
        filesystem::File file = VIOLET_TRY(filesystem::OpenOptions{}.Read(true).Open(VIOLET_FWD(Path, path)));
        return FileInputStream(VIOLET_MOVE(file));
    }

    /// @inheritdoc violet::io::experimental::InputStream::Read(violet::Span<violet::UInt8>)
    auto Read(Span<UInt8> buf) noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Available()
    [[nodiscard]] auto Available() const noexcept -> Result<UInt> override;

    /// @inheritdoc violet::io::experimental::InputStream::Skip(violet::UInt8)
    auto Skip(UInt bytes) noexcept -> Result<void> override;

private:
    filesystem::File n_file;
};

} // namespace violet::io::experimental
