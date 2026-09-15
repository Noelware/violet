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

#include <violet/Language/Macros.h>

#if VIOLET_PLATFORM(UNIX)

#include <violet/Experimental/Logging/LogRecord.h>
#include <violet/Experimental/Logging/Sinks/Console.h>

#include <unistd.h>

namespace violet::experimental::log::sinks {
namespace io = violet::io::experimental;

Console::Console(Stream stream) noexcept
    : n_fd(std::in_place, io::Descriptor::Borrowed(stream == Stream::Stdout ? STDOUT_FILENO : STDERR_FILENO))
{
}

auto Console::WithStream(Stream stream) noexcept -> Console&
{
    // We still take the lock when we switch streams (stdout/stderr) so that a
    // concurrent `Emit` never observes a half-written descriptor.
    //
    // I know that the log factory doesn't stream records in a separate thread
    // and Console is a synchronous sink (which, will block) but it's better
    // than sorry in cases where it is multi-threaded.
    this->n_fd.With([stream](io::Descriptor::Borrowed& fd) -> void {
        fd = io::Descriptor::Borrowed(stream == Stream::Stdout ? STDOUT_FILENO : STDERR_FILENO);
    });

    return *this;
}

void Console::Emit(const Record& record)
{
    this->n_fd.With([this, &record](io::Descriptor::Borrowed& fd) -> void {
        if (!fd.Valid()) {
            return;
        }

        const auto* fmt = this->n_formatter.Get();
        if (fmt == nullptr) {
            return;
        }

        const auto message = fmt->Format(record);
        Vec<UInt8> data(message.begin(), message.end());

        if (auto res = fd.Write(data); res.Err()) {
            // In rare-case scenarios, `Write(data)` could fail at anytime for some
            // odd reason. A normal person would ignore it and forget about it,
            // but I am not a normal person, I am a god damn gay furry who cares
            // when shit hits the fan, so I will write this to `std::cerr`, and if
            // that fails... well... uh, it's not my problem. don't complain to me.
            std::cerr << "[violet/logging@fatal";
            if (!record.Location.File.empty()) {
                std::cerr << ' ' << record.Location.File;
            }

            if (record.Location.Line > 0) {
                std::cerr << ':' << record.Location.Line;
            }

            if (record.Location.Column > 0) {
                std::cerr << ':' << record.Location.Column;
            }

            std::cerr << "]: received fatal error when writing to stream: " << res.Error() << '\n';
        } else {
            // well, we know that it works! and I prefer to append newlines after each entry.
            // TODO(@auguwu/Noel): should this be configurable?
            constexpr static UInt8 newline = '\n';
            (void)fd.Write({&newline, 1});
        }
    });
}

void Console::Flush() noexcept
{
    this->n_fd.With([](io::Descriptor::Borrowed& fd) -> void { (void)fd.Flush(); });
}

} // namespace violet::experimental::log::sinks

#endif
