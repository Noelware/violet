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

#include <violet/Subprocess/ExitStatus.h>

#include <sys/wait.h>

using violet::Int32;
using violet::subprocess::ExitStatus;

auto ExitStatus::Exited() const noexcept -> bool
{
    return WIFEXITED(this->n_code);
}

auto ExitStatus::Signaled() const noexcept -> bool
{
    return WIFSIGNALED(this->n_code);
}

auto ExitStatus::Stopped() const noexcept -> bool
{
    return WIFSTOPPED(this->n_code);
}

auto ExitStatus::CoreDumped() const noexcept -> bool
{
#ifdef WCOREDUMP
    return this->Signaled() && WCOREDUMP(this->n_code);
#else
    return false;
#endif
}

auto ExitStatus::Code() const noexcept -> Optional<value_type>
{
    if (!this->Exited()) {
        return Nothing;
    }

    return WEXITSTATUS(this->n_code);
}

auto ExitStatus::Signal() const noexcept -> Optional<value_type>
{
    if (!this->Signaled()) {
        return Nothing;
    }

    if (this->Stopped()) {
        return WSTOPSIG(this->n_code);
    }

    return WTERMSIG(this->n_code);
}

auto ExitStatus::ToString() const noexcept -> String
{
    return std::format("ExitStatus(code={}, exited={}, signaled={}, stopped={}, coreDumped={})",
        this->Code().UnwrapOr(0), this->Exited(), this->Signaled(), this->Stopped(), this->CoreDumped());
}

auto ExitStatus::operator==(const ExitStatus& other) const noexcept -> bool
{
    auto lhs = this->AsNative();
    auto rhs = other.AsNative();

    if (this->Exited() && other.Exited()) {
        return WEXITSTATUS(lhs) == WEXITSTATUS(rhs);
    }

    if (this->Signaled() && other.Signaled()) {
        return WTERMSIG(lhs) == WTERMSIG(rhs);
    }

    if (this->Stopped() && other.Stopped()) {
        return WSTOPSIG(lhs) == WSTOPSIG(rhs);
    }

    return false;
}

auto ExitStatus::operator!=(const ExitStatus& other) const noexcept -> bool
{
    return !(*this == other);
}

auto ExitStatus::operator==(value_type other) const noexcept -> bool
{
    auto me = this->AsNative();
    return this->Exited() && WEXITSTATUS(me) == other;
}

auto ExitStatus::operator!=(value_type other) const noexcept -> bool
{
    return !(*this == other);
}

#endif
