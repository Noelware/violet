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

#include "violet/support/StringRef.h"
#include "violet/violet.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace Noelware::Violet::Filesystem {

#ifdef _WIN32
constexpr static auto kSeparator = '\\';
#else
constexpr static auto kSeparator = '/';
#endif

struct Path final {
    constexpr static auto Separator = kSeparator;

    using value_type = char;

    constexpr Path() = default;

    /// Constructs a new [`Path`] from a C-style string.
    /// @param path cstr
    constexpr VIOLET_IMPLICIT Path(CStr path)
        : n_value(path)
    {
    }

    /// Constructs a new [`Path`] from a string reference.
    /// @param path the path to use.
    constexpr VIOLET_EXPLICIT Path(StringRef path)
        : n_value(path.Data(), path.Length())
    {
    }

    /// Constructs a new [`Path`] from an existing, owned string.
    /// @param path the path to use.
    constexpr VIOLET_EXPLICIT Path(String path)
        : n_value(VIOLET_MOVE(path))
    {
    }

    /// Conversion operator for **Path** -> **bool**.
    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return !this->Empty();
    }

    /// Returns **true** if this path is empty.
    [[nodiscard]] constexpr auto Empty() const noexcept -> bool
    {
        return this->n_value.empty();
    }

    /// Returns **true** if this path is absolute.
    ///
    /// ## Platform-specific Variants
    /// - Unix: Considered **true** if the path starts with `/`.
    /// - Windows: Considered **true** if it starts with a drive letter and colon (e.g. `"C:\\"`) or a UNC path
    /// (`"\\\\"`).
    [[nodiscard]] constexpr auto Absolute() const noexcept -> bool
    {
        if (Empty()) {
            return false;
        }

#ifdef _WIN32
        // For Windows, we ned to check if we are a drive letter or
        // UNC absolute path.
        if (this->n_value.Length() >= 2 && std::isalpha(static_cast<unsigned char>(this->n_value[0]))
            && this->n_value[1] == ':') {
            return true;
        }

        // UNC path: "\\server\share"
        if (this->n_value.Length() >= 2 && this->n_value[0] == '\\' && this->n_value[1] == '\\') {
            return true;
        }

        return false;
#else
        return this->n_value[0] == '/';
#endif
    }

    /// Returns **true** if this path is relative.
    [[nodiscard]] constexpr auto Relative() const noexcept -> bool
    {
        return !this->Absolute();
    }

    /// Returns the filename for this path.
    [[nodiscard]] constexpr auto Filename() const noexcept -> StringRef
    {
        if (Empty()) {
            return "";
        }

        auto [pos, val] = this->getTrailingSlashPosition();
        if (pos == -1) {
            return val;
        }

        return val.substr(pos + 1);
    }

    /// Returns the extension to the filename without the leading dot. `Nothing` is returned
    /// if this file doesn't include an extension.
    ///
    /// ## Example
    /// ```cpp
    /// Noelware::Violet::Filesystem::Path loveletter = "/home/noeltowa/Documents/loveletter.txt";
    /// if (auto ext = loveletter.Extension()) {
    ///     std::cout << "loveletter extension: " << ext.Value() << '\n';
    /// }
    /// ```
    [[nodiscard]] constexpr auto Extension() const noexcept -> Optional<StringRef>
    {
        if (Empty()) {
            return Nothing;
        }

        Str name = this->Filename();
        if (name.empty()) {
            return Nothing;
        }

        int64 pos = -1;
        for (int64 i = static_cast<int64>(name.length()) - 1; i >= 0; i--) {
            if (name[i] == '.') {
                pos = i;
                break;
            }
        }

        if (pos == -1 || pos == 0) {
            return Nothing;
        }

        return Some<StringRef>(name.substr(pos + 1));
    }

    /// Joins a component into this [`Path`].
    [[nodiscard]] constexpr auto Join(StringRef rhs) const -> Path
    {
        if (rhs.Empty())
            return *this;

        Path rhspath(rhs);
        if (rhspath.Absolute()) {
            return rhspath;
        }

        String result = this->n_value;
        if (!result.empty() && result.back() != kSeparator) {
            result.push_back(kSeparator);
        }

        usize start = 0;
        while (start < rhs.Length() && (rhs[start] == '/' || rhs[start] == '\\')) {
            start++;
        }

        Str view = rhs;
        result.append(view.substr(start));

        return Path(VIOLET_MOVE(result));
    }

    /// Canonicalizes this path by removing redundant `..` and `.` without resolving
    /// symlink metadata.
    ///
    /// ## Example
    /// ```cpp
    /// Noelware::Violet::Filesystem::Path clang = "/usr/./local/../bin/clang";
    /// if (auto path = clang.Canonicalize()) {
    ///     std::cout << "clang: " << path.Value() << '\n';
    ///     // "clang: /usr/bin/clang"
    /// }
    /// ```
    [[nodiscard]] auto Canonicalize() const noexcept -> Path // NOLINT(readability-function-cognitive-complexity)
    {
        if (Empty()) {
            return *this;
        }

        String result;
        usize idx = 0;
        bool consideredLastSep = false;

        while (idx < this->n_value.size()) {
            // Skip '/' or '\\'.
            if (this->n_value[idx] == '/' || this->n_value[idx] == '\\') {
                if (!consideredLastSep) {
                    result.push_back(kSeparator);
                }

                consideredLastSep = true;
                idx++;

                continue;
            }

            usize start = idx;
            while (idx < this->n_value.size() && this->n_value[idx] != '/' && this->n_value[idx] != '\\') {
                idx++;
            }

            Str part(&this->n_value[start], idx - start);
            if (part == ".") {
                // skip
            } else if (part == "..") {
                // remove last segment if possible
                usize pos = result.rfind(kSeparator);
                if (pos != String::npos) {
                    result.resize(pos);
                } else {
                    if (!result.empty()) {
                        result.push_back(kSeparator);
                    } else {
                        result.append(part);
                    }
                }

                consideredLastSep = false;
            } else {
                if (!result.empty() && result.back() != kSeparator) {
                    result.push_back(kSeparator);
                } else {
                    result.append(part);
                }

                consideredLastSep = false;
            }
        }

        if (result.size() > 1 && result.back() == kSeparator) {
            result.pop_back();
        }

        return Path(VIOLET_MOVE(result));
    }

    /// Returns the parent of this `Path`, returning `Nothing` if there is no parent or
    /// the path is invalid.
    [[nodiscard]] constexpr auto Parent() const -> Optional<Path>
    {
        if (Empty()) {
            return Nothing;
        }

        auto [pos, val] = this->getTrailingSlashPosition();
        if (pos == -1) {
            return Nothing;
        }

// edge case: if we are just the root path, return the root path as
// the parent.
#ifdef _WIN32
        if (pos == 2 && val[1] == ':') {
            return Some<Path>(Path(val.substr(0, pos + 1)));
        }
#else
        if (pos == 0) {
            return Some<Path>("/");
        }
#endif

        return Some<Path>(val.substr(0, pos));
    }

    [[nodiscard]] auto Stem() const -> StringRef
    {
        Str filename = this->Filename();
        if (filename.empty()) {
            return filename;
        }

        usize dotpos = filename.rfind('.');
        if (dotpos == Str::npos) {
            return filename;
        }

        // edge case: '.gitignore', .bazelignore, etc.
        //
        // if pos == 0 (only finds one `.`) and the first character
        // in `filename` == `.`
        if (dotpos == 0 && filename[0] == '.') {
            return filename;
        }

        return { filename.substr(0, dotpos) };
    }

    [[nodiscard]] auto WithFilename(StringRef filename) const noexcept -> Path
    {
        if (Empty()) {
            return Path(filename);
        }

        if (auto parent = this->Parent()) {
            return parent->Join(filename);
        }

#ifndef _WIN32
        // edge case: `n_value` == '/' (indicating that we are root on Unix) and
        // intentionally, `this->Parent()` will return `Nothing` if we are root
        // regardless.
        if (this->n_value == "/") {
            return Path(std::format("/{}", Str(filename)));
        }
#endif

        return Path(filename);
    }

    [[nodiscard]] auto WithExtension(StringRef ext) const noexcept -> Path
    {
        Str stem = this->Stem();
        if (stem.empty()) {
            return *this;
        }

        String final = String(ext);
        if (!final.empty() && final[0] != '.') {
            final = std::format(".{}", final);
        }

        return this->WithFilename(std::format("{}{}", stem, final));
    }

#ifdef _WIN32
    /// Allows converting this UTF-8 path into a UTF-16 wide string for Win32 APIs.
    ///
    /// ## Example
    /// ```cpp
    /// Noelware::Violet::Filesystem::Path path = "C:\\Users\\NoelTowa\\Workspaces\\Noelware\\secrets.txt";
    /// std::wstring w = path.AsWideStr();
    /// CreateFileW(w.c_str(), /* ... */);
    /// ```
    [[nodiscard]] auto AsWideStr() const -> std::wstring;

    /// Conversion operator for **Path** -> [`std::wstring`] for Win32 APIs.
    VIOLET_EXPLICIT operator std::wstring() const
    {
        return this->AsWideStr();
    }
#endif

    VIOLET_IMPL_EQUALITY_SINGLE(Path, lhs, rhs, { return lhs.n_value == rhs.n_value; });
    VIOLET_IMPL_EQUALITY(const Path&, CStr, lhs, rhs, { return lhs.n_value == rhs; });
    VIOLET_IMPL_EQUALITY(const Path&, const Str&, lhs, rhs, { return lhs.n_value == rhs; });
    VIOLET_IMPL_EQUALITY(const Path&, const String&, lhs, rhs, { return lhs.n_value == rhs; });
    VIOLET_IMPL_EQUALITY(const Path&, const StringRef&, lhs, rhs, { return lhs.n_value == rhs; });

    VIOLET_OSTREAM_IMPL(const Path&)
    {
        return os << self.n_value;
    }

private:
    String n_value; ///< Underlying UTF-8 string buffer representing the path.

    [[nodiscard]] constexpr auto getTrailingSlashPosition() const noexcept -> Pair<int64, Str>
    {
        Str val = this->n_value;
        while (!val.empty() && (val.back() == '/' || val.back() == '\\')) {
            val = val.substr(0, val.length() - 1);
        }

        int64 pos = -1;
        for (int64 i = static_cast<int64>(val.length()) - 1; i >= 0; i--) {
            char ch = val[i];
            if (ch == '/' || ch == '\\') {
                pos = i;
                break;
            }
        }

        return std::make_pair(pos, val);
    }
};

} // namespace Noelware::Violet::Filesystem
