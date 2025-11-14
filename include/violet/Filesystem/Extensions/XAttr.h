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
//! # 🌺💜 `violet/Filesystem/Extensions/XAttr.h`
//! This header file provides extended attributes (`xattr`) support for files.
//!
//! Extended attributes are arbitrary key-value metadata that can be attached
//! to a file, beyond the standard permissions and timestamps. They are
//! supported on most Unix filesystems (via `getxattr`, `setxattr`, `listxattr`)
//! and can be emulated on Windows using NTFS Alternate Data Streams (ADS).
//!
//! `xattr` allows applications to store structured data such as:
//! - Custom metadata (e.g., tags, hashes)
//! - Application-specific attributes
//! - Security or access control information
//!
//! ## Example
//! ```cpp
//! /* TODO: this */
//! ```
//!
//! ## Notes
//! * Extended attributes are optional and may not be supported on all filesystems.
//! * Keys are typically prefixed (i.e, `user.`) to avoid conflicts with system attributes.
//! * For cross-platform portability, prefer storing application data in a standard file format,
//!   falling back to `xattr` when supported.

#pragma once

#include "violet/IO/Descriptor.h"
#include "violet/IO/Error.h"
#include "violet/Violet.h"

// #include <type_traits>

namespace violet::filesystem::xattr {

struct Iter;

/// Sets an extended attribute for the file. This will overwrite the attribute if it exists
/// depending on platform semantics.
///
/// @param fd the raw file descriptor
/// @param key the key to append
/// @param value the byte array that presents the value of this extended attribute
auto Set(io::FileDescriptor::value_type fd, Str key, Span<const UInt8> value) noexcept -> io::Result<void>;

/// Retrieves the value of a file's extended attribute. Returns an error if the attribute
/// doesn't exist depending on platform semantics.
///
/// @param fd the raw file descriptor
/// @param key the key to append
auto Get(io::FileDescriptor::value_type fd, Str key) noexcept -> io::Result<Vec<UInt8>>;

// /// Returns a iterator that lists all extended attributes present on this file.
// /// @param fd the raw file descriptor
// auto List(io::FileDescriptor::value_type fd) noexcept -> io::Result<Iter>;

// /// An iterator that lists all a file's extended attributes.
// struct Iter final {
//     /// The item that is returned from this iterator.
//     using Item = io::Result<Pair<String, Vec<UInt8>>>;

//     Iter() = delete;

//     /// Returns the next entry in this iterator. `Nothing` is returned if we exhausted
//     /// the amount of entries present.
//     auto Next() noexcept -> Optional<Item>;

// private:
//     friend auto violet::filesystem::xattr::List(io::FileDescriptor::value_type) noexcept -> io::Result<Iter>;

//     /// The platform-implementation of the iterator itself.
//     struct Impl;

//     template<typename... Args>
//         requires(std::is_constructible_v<Impl, Args...>)
//     VIOLET_EXPLICIT Iter(Args&&... args) noexcept(std::is_nothrow_constructible_v<Impl, Args...>);
// };

} // namespace violet::filesystem::xattr
