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

#include <violet/Language/Panic.h>
#include <violet/Print.h>

#include <atomic>
#include <exception>

using violet::panic::Hook;
using violet::panic::Info;
using violet::panic::Strategy;

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
Hook n_panicHook = nullptr;
std::atomic<Strategy> n_strategy = Strategy::Unwind;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

VIOLET_COLD void defaultPanicHook(const Info& info)
{
    violet::PrintErr("[{}:{}:{}] panic reached", info.Location.File, info.Location.Line, info.Location.Column);
    if (info.Userland != nullptr) {
        violet::PrintErr("in userland [{}:{}:{}]", info.Userland->File, info.Userland->Line, info.Userland->Column);
    }

    violet::PrintErrln(": {}", info.Message);
}

} // namespace

void violet::panic::InstallDefaultPanicHook() noexcept
{
    violet::panic::SetHook(defaultPanicHook);
}

auto violet::panic::SetHook(Hook hook) noexcept -> Hook
{
    Hook oldHook = n_panicHook;
    n_panicHook = hook;

    return oldHook;
}

void violet::panic::SetStrategy(Strategy strategy) noexcept
{
    n_strategy.store(strategy, std::memory_order_relaxed);
}

void violet::detail::DoPanic(std::string_view message, SourceLocation* userland, SourceLocation loc)
{
    panic::Info info(message, loc, userland);
    if (n_panicHook != nullptr) {
        n_panicHook(info);
    }

    switch (n_strategy.load(std::memory_order_relaxed)) {
    case Strategy::Unwind:
        std::terminate();

    case Strategy::Abort:
        std::abort();
    }
}
