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

#include "violet/subprocess/Stdio.h"
#include "violet/io/Descriptor.h"

using Noelware::Violet::Process::Stderr;
using Noelware::Violet::Process::Stdin;
using Noelware::Violet::Process::Stdio;
using Noelware::Violet::Process::Stdout;

constexpr auto Stdio::Piped() noexcept -> Stdio
{
    Stdio stdio;
    stdio.n_kind = kind::kPiped;

    return stdio;
}

constexpr auto Stdio::Inherit() noexcept -> Stdio
{
    Stdio stdio;
    stdio.n_kind = kind::kInherit;

    return stdio;
}

constexpr auto Stdio::Null() noexcept -> Stdio
{
    Stdio stdio;
    stdio.n_kind = kind::kNull;

    return stdio;
}

constexpr auto Stdio::IsPiped() const noexcept -> bool
{
    return this->n_kind == kind::kPiped;
}

constexpr auto Stdio::IsInherited() const noexcept -> bool
{
    return this->n_kind == kind::kInherit;
}

constexpr auto Stdio::IsNull() const noexcept -> bool
{
    return this->n_kind == kind::kNull;
}

Stdin::Stdin(IO::Descriptor descriptor, Stdio stdio) noexcept
    : n_descriptor(descriptor)
    , n_stdio(stdio)
{
}

auto Stdin::Valid() const noexcept -> bool
{
    return this->n_stdio.IsPiped() && this->n_descriptor.Valid();
}

auto Stdin::ToString() const noexcept -> String
{
    return std::format("Stdin({})", this->n_descriptor.ToString());
}

Stdout::Stdout(IO::Descriptor descriptor, Stdio stdio) noexcept
    : n_descriptor(descriptor)
    , n_stdio(stdio)
{
}

auto Stdout::Valid() const noexcept -> bool
{
    return this->n_stdio.IsPiped() && this->n_descriptor.Valid();
}

auto Stdout::ToString() const noexcept -> String
{
    return std::format("Stdout({})", this->n_descriptor.ToString());
}

Stderr::Stderr(IO::Descriptor descriptor, Stdio stdio) noexcept
    : n_descriptor(descriptor)
    , n_stdio(stdio)
{
}

auto Stderr::Valid() const noexcept -> bool
{
    return this->n_stdio.IsPiped() && this->n_descriptor.Valid();
}

auto Stderr::ToString() const noexcept -> String
{
    return std::format("Stderr({})", this->n_descriptor.ToString());
}
