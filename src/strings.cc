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

#include <violet/Strings.h>

using violet::strings::Split;

auto violet::strings::SplitOnce(Str input, char delim) noexcept -> Pair<Str, Optional<Str>>
{
    auto pos = input.find(delim);
    if (pos == Str::npos) {
        return { input, Nothing };
    }

    return { input.substr(0, pos), input.substr(pos + 1) };
}

Split::Split(Str input) noexcept
    : Split(input, ' ')
{
}

Split::Split(Str input, char delim) noexcept
    : n_input(input)
    , n_delim(delim)
    , n_revPos(input.size())
{
}

auto Split::Next() noexcept -> Optional<Str>
{
    if (this->n_pos >= this->n_revPos) {
        return Nothing;
    }

    UInt start = this->n_pos;
    while (this->n_pos < this->n_revPos) {
        char ch = this->n_input[this->n_pos];
        if (ch == this->n_delim) {
            break;
        }

        ++this->n_pos;
    }

    UInt size = this->n_pos - start;
    if (this->n_pos < this->n_revPos && this->n_input[this->n_pos] == this->n_delim) {
        ++this->n_pos;
    }

    return Str(this->n_input.data() + start, size);
}
