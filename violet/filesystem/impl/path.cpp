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

#include "violet/filesystem/Path.h"
#include "violet/container/Optional.h"
#include "violet/support/StringRef.h"
#include "violet/violet.h"

using Noelware::Violet::CStr;
using Noelware::Violet::int64;
using Noelware::Violet::Optional;
using Noelware::Violet::Pair;
using Noelware::Violet::String;
using Noelware::Violet::StringRef;
using Noelware::Violet::Filesystem::Path;
using Noelware::Violet::Filesystem::PathRef;

static constexpr auto getTrailingSlashPos(const String& value) noexcept -> Pair<int64, String>
{
    auto val = value;
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

Path::Path(CStr data) noexcept
    : n_path(data)
{
}

Path::Path(String path) noexcept
    : n_path(VIOLET_MOVE(path))
{
}

Path::Path(StringRef path) noexcept
    : n_path(path.Data(), path.Length())
{
}

Path::Path(const PathRef& path) noexcept
    : n_path(path.n_path)
{
}

Path::Path(PathRef&& path) noexcept
    : n_path(VIOLET_MOVE(path).n_path)
{
}

auto Path::Empty() const noexcept -> bool
{
    return this->n_path.empty();
}

auto Path::Relative() const noexcept -> bool
{
    return !this->Absolute();
}

auto Path::Stem() const noexcept -> String
{
    return static_cast<PathRef>(this->n_path).Stem();
}

auto Path::Filename() const noexcept -> String
{
    return static_cast<PathRef>(this->n_path).Filename();
}

auto Path::Extension() const noexcept -> Optional<String>
{
    return static_cast<PathRef>(this->n_path).Extension();
}

auto Path::Join(StringRef rhs) const noexcept -> Path
{
    return static_cast<PathRef>(this->n_path).Join(rhs);
}

void Path::Canonicalize() noexcept
{
    *this = static_cast<PathRef>(this->n_path).Canonicalize();
}

auto Path::WithFilename(StringRef piece) noexcept -> Path
{
    auto parent = this->Parent();
    if (parent.HasValue()) {
        return parent->Join(piece);
    }

    if (Root()) {
        return Path(n_path).Join(piece);
    }

    return Path(piece);
}

auto Path::WithExtension(StringRef piece) noexcept -> Path
{
    String stem = this->Stem();
    if (stem.empty()) {
        return *this;
    }

    String final = String(piece);
    if (!final.empty() && final[0] != '.') {
        final = std::format(".{}", final);
    }

    return this->WithFilename(std::format("{}{}", stem, final));
}

Path::operator bool() const noexcept
{
    return !this->Empty();
}

Path::operator String() const noexcept
{
    return this->n_path;
}

Path::operator PathRef() const noexcept
{
    return this->n_path;
}

Path::operator StringRef() const noexcept
{
    return this->n_path;
}

PathRef::PathRef(CStr path) noexcept
    : n_path(path)
{
}

PathRef::PathRef(const String& path) noexcept
    : n_path(path)
{
}

PathRef::PathRef(const Path& path) noexcept
    : n_path(path.n_path)
{
}

PathRef::PathRef(StringRef path) noexcept
    : n_path(path)
{
}

PathRef::PathRef(Path&& path) noexcept
    : n_path(VIOLET_MOVE(path).n_path)
{
}

auto PathRef::Empty() const noexcept -> bool
{
    return this->n_path.Empty();
}

auto PathRef::Relative() const noexcept -> bool
{
    return !this->Absolute();
}

auto PathRef::Stem() const noexcept -> String
{
    auto name = this->Filename();
    if (name.empty()) {
        return name;
    }

    usize pos = name.rfind('.');
    if (pos == String::npos) {
        return name;
    }

    // edge case: '.gitignore', .bazelignore, etc.
    //
    // if pos == 0 (only finds one `.`) and the first character
    // in `filename` == `.`
    if (pos == 0 && name[0] == '.') {
        return name;
    }

    return { name.substr(0, pos) };
}

auto PathRef::Filename() const noexcept -> String
{
    if (Empty()) {
        return "";
    }

    auto [pos, path] = getTrailingSlashPos(this->n_path);
    if (pos == -1) {
        return this->n_path;
    }

    return this->n_path;
}

auto PathRef::Extension() const noexcept -> Optional<String>
{
    if (Empty()) {
        return Nothing;
    }

    String name = this->Filename();
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

    return pos >= 1 ? Some<String>(name.substr(pos + 1)) : Nothing;
}

auto PathRef::Join(StringRef rhs) const noexcept -> Path
{
    if (rhs.Empty())
        return {};

    Path rhspath(rhs);
    if (rhspath.Absolute()) {
        return rhspath;
    }

    String result = this->n_path;
    if (!result.empty() && result.back() != PathSeparator) {
        result.push_back(PathSeparator);
    }

    usize start = 0;
    while (start < rhs.Length() && (rhs[start] == '/' || rhs[start] == '\\')) {
        start++;
    }

    Str view = rhs;
    result.append(view.substr(start));

    return VIOLET_MOVE(result);
}

auto PathRef::Canonicalize() noexcept -> Path
{
    if (Empty()) {
        return {};
    }

    String result;
    String path = this->n_path;
    usize idx = 0;
    bool consideredLastSep = false;

    while (idx < path.size()) {
        // Skip '/' or '\\'.
        if (this->n_path[idx] == '/' || this->n_path[idx] == '\\') {
            if (!consideredLastSep) {
                result.push_back(PathSeparator);
            }

            consideredLastSep = true;
            idx++;

            continue;
        }

        usize start = idx;
        while (idx < path.size() && this->n_path[idx] != '/' && this->n_path[idx] != '\\') {
            idx++;
        }

        Str part(&path[start], idx - start);
        if (part == ".") {
            // skip
        } else if (part == "..") {
            // remove last segment if possible
            usize pos = result.rfind(PathSeparator);
            if (pos != String::npos) {
                result.resize(pos);
            } else {
                if (!result.empty()) {
                    result.push_back(PathSeparator);
                } else {
                    result.append(part);
                }
            }

            consideredLastSep = false;
        } else {
            if (!result.empty() && result.back() != PathSeparator) {
                result.push_back(PathSeparator);
            } else {
                result.append(part);
            }

            consideredLastSep = false;
        }
    }

    if (result.size() > 1 && result.back() == PathSeparator) {
        result.pop_back();
    }

    return VIOLET_MOVE(result);
}

PathRef::operator bool() const noexcept
{
    return !this->Empty();
}

PathRef::operator Path() const noexcept
{
    return VIOLET_MOVE(this->n_path);
}

PathRef::operator StringRef() const noexcept
{
    return this->n_path;
}
