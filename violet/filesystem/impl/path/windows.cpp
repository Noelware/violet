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

#include "violet/violet.h"

#ifdef VIOLET_WINDOWS

// clang-format off
#include "violet/filesystem/Path.h"
// clang-format on

using Noelware::Violet::Filesystem::Path;
using Noelware::Violet::Filesystem::PathRef;

constexpr auto Path::Root() const noexcept -> bool
{
    if (Empty()) {
        return true;
    }

    return this->n_value.length() == 2 && std::isalpha(this->n_value[0]) && this->n_value[1] == ':';
}

constexpr auto Path::Absolute() const noexcept -> bool
{
    if (Empty()) {
        return false;
    }

    // For Windows, we ned to check if we are a drive letter or
    // UNC absolute path.
    if (this->n_value.size() >= 2 && std::isalpha(static_cast<unsigned char>(this->n_value[0]))
        && this->n_value[1] == ':') {
        return true;
    }

    // UNC path: "\\server\share"
    if (this->n_value.size() >= 2 && this->n_value[0] == '\\' && this->n_value[1] == '\\') {
        return true;
    }

    return false;
}

constexpr auto Path::Parent() const noexcept -> Optional<Path>
{
    if (Empty()) {
        return Nothing;
    }

    String path = this->n_path;
    int64 pos = detail::getTrailingSlashPos(path);
    if (pos == -1) {
        return Nothing;
    }

    if (pos == 2 && val[1] == ':') {
        return Some<Path>(Path(val.substr(0, pos + 1)));
    }

    return Some<Path>(this->n_path.substr(pos + 1));
}

constexpr auto PathRef::Root() const noexcept -> bool
{
    return Empty() || this->n_path == "/";
}

constexpr auto PathRef::Absolute() const noexcept -> bool
{
    if (Empty()) {
        return false;
    }

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
}

constexpr auto PathRef::Parent() const noexcept -> Optional<Path>
{
    if (Empty()) {
        return Nothing;
    }

    String path = this->n_path;
    int64 pos = detail::getTrailingSlashPos(path);
    if (pos == -1) {
        return Nothing;
    }

    if (pos == 2 && val[1] == ':') {
        return Some<Path>(Path(val.substr(0, pos + 1)));
    }

    return Some<Path>(static_cast<Str>(this->n_path).substr(pos + 1));
}

#endif
