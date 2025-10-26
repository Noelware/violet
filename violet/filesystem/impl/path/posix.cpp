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

using Noelware::Violet::Filesystem::Path;
using Noelware::Violet::Filesystem::PathRef;

auto Path::Root() const noexcept -> bool
{
    return Empty() || this->n_path == "/";
}

auto Path::Absolute() const noexcept -> bool
{
    if (Empty()) {
        return false;
    }

    return this->n_path[0] == '/';
}

auto Path::Parent() const noexcept -> Optional<Path>
{
    if (Empty()) {
        return Nothing;
    }

    String path = this->n_path;
    int64 pos = detail::getTrailingSlashPos(path);
    if (pos == -1) {
        return Nothing;
    }

    if (pos == 0) {
        return Some<Path>("/");
    }

    return Some<Path>(this->n_path.substr(0, pos));
}

auto PathRef::Root() const noexcept -> bool
{
    return Empty() || this->n_path == "/";
}

auto PathRef::Absolute() const noexcept -> bool
{
    if (Empty()) {
        return false;
    }

    return this->n_path[0] == '/';
}

auto PathRef::Parent() const noexcept -> Optional<Path>
{
    if (Empty()) {
        return Nothing;
    }

    String path = this->n_path;
    int64 pos = detail::getTrailingSlashPos(path);
    if (pos == -1) {
        return Nothing;
    }

    if (pos == 0) {
        return Some<Path>("/");
    }

    return Some<Path>(static_cast<Str>(this->n_path).substr(0, pos));
}
