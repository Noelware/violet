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

#include <violet/Violet.h>

#if VIOLET_PLATFORM(UNIX)

#include <violet/Subprocess/PID.h>

#include <unistd.h>

using violet::subprocess::PID;

auto PID::Current() noexcept -> PID
{
    return ::getpid();
}

auto PID::Parent() noexcept -> Optional<PID>
{
    return ::getppid();
}

auto PID::ToString() const noexcept -> String
{
    return std::format("PID({})", this->n_value);
}

PID::operator bool() const noexcept
{
    return this->n_value > 0;
}

PID::operator value_type() const noexcept
{
    return this->Get();
}

auto PID::operator<=>(const PID& other) const noexcept -> std::strong_ordering
{
    return this->n_value <=> other.n_value;
}

auto PID::operator<=>(value_type other) const noexcept -> std::strong_ordering
{
    return this->n_value <=> other;
}

auto PID::operator==(const PID& other) const noexcept -> bool
{
    return this->n_value == other.n_value;
}

auto PID::operator!=(const PID& other) const noexcept -> bool
{
    return !(*this == other);
}

auto PID::operator==(value_type other) const noexcept -> bool
{
    return this->n_value == other;
}

auto PID::operator!=(value_type other) const noexcept -> bool
{
    return !(*this == other);
}

#endif
