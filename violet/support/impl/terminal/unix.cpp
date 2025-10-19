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

#ifdef VIOLET_UNIX

// clang-format off
#include "violet/support/Terminal.h"

#include <sys/ioctl.h>
#include <unistd.h>
// clang-format on

using Noelware::Violet::Terminal;
using Noelware::Violet::TerminalStreamSource;
using Noelware::Violet::TermWindow;

auto Noelware::Violet::Terminal::TTY(TerminalStreamSource source) noexcept -> bool
{
    switch (source) {
    case TerminalStreamSource::Stdout:
        return static_cast<bool>(isatty(fileno(stdout)));

    case TerminalStreamSource::Stderr:
        return static_cast<bool>(isatty(fileno(stderr)));
    }
}

auto Terminal::Window(TerminalStreamSource source) noexcept -> IO::Result<TermWindow>
{
    struct winsize win{};
    switch (source) {
    case TerminalStreamSource::Stdout:
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
        break;

    case TerminalStreamSource::Stderr:
        ioctl(STDERR_FILENO, TIOCGWINSZ, &win);
        break;
    }

    if (errno != 0) {
        return Err(IO::Error::Platform(IO::ErrorKind::Other));
    }

    return TermWindow{ .Columns = win.ws_col, .Rows = win.ws_row };
}

#endif
