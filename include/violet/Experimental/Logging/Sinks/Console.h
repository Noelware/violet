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
//
//! # 🌺💜 `violet/Experimental/Logging/Sinks/Console.h`

#pragma once

#include <violet/Experimental/Logging/Formatter.h>
#include <violet/Experimental/Logging/Sink.h>
#include <violet/Experimental/Own.h>
#include <violet/Experimental/Synchronized.h>
#include <violet/IO/Experimental/Descriptor.h>

namespace violet::experimental::log::sinks {

/// Logging sink that writes formatted log records to a console stream.
///
/// **Console** is a concrete, synchronous [`Sink`] implementation that emits
/// log records to a process' standard input or error. Output formatting is
/// delegated to a userland [`console::Formatter`] implementation that allows
/// customization how log records are rendered before being written.
///
/// By default, the console writes nothing if no formatter is available.
///
/// ## Thread Safety
/// Emission is internally synchronized via a Mutex. Concurrent calls to [`Console::Emit`]
/// and [`Console::Flush`] are safe, through ordering between is not guaranteed.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Console final: public Sink {
    VIOLET_DISALLOW_COPY_AND_MOVE(Console);
    ~Console() override = default;

    /// What console stream to write formatted logs to.
    enum struct Stream : violet::UInt8 {
        Stdout, ///< process' standard input
        Stderr ///< process' standard error
    };

    /// Creates a new console sink with no formatter attached.
    /// @param stream The stream to use.
    VIOLET_IMPLICIT Console(Stream stream = Stream::Stdout) noexcept;

    template<io::experimental::AsFd Fd>
    VIOLET_IMPLICIT Console(Fd fd) noexcept
        : n_fd(std::in_place, fd.Borrow())
    {
    }

    template<typename F>
        requires std::derived_from<std::remove_cvref_t<F>, Formatter>
    VIOLET_IMPLICIT Console(F formatter) noexcept(std::is_nothrow_default_constructible_v<F>)
        : Console(Stream::Stdout)
    {
        this->n_formatter = Own<Formatter>::New<F>(formatter);
    }

    template<typename F>
        requires std::derived_from<std::remove_cvref_t<F>, Formatter>
    VIOLET_IMPLICIT Console(Stream stream, F formatter) noexcept(std::is_nothrow_default_constructible_v<F>)
        : Console(stream)
    {
        this->n_formatter = Own<Formatter>::New<F>(formatter);
    }

    template<io::experimental::AsFd Fd, typename F>
        requires std::derived_from<std::remove_cvref_t<F>, Formatter>
    VIOLET_IMPLICIT Console(Fd fd, F formatter) noexcept
        : n_formatter(Own<Formatter>::New<F>(formatter))
        , n_fd(std::in_place, fd.Borrow())
    {
    }

    template<typename F>
        requires((std::is_pointer_v<F> && std::derived_from<std::remove_pointer_t<F>, Formatter>)
            || std::derived_from<std::remove_cvref_t<F>, Formatter>)
    auto WithFormatter(F formatter) noexcept -> Console&
    {
        if constexpr (std::is_pointer_v<F>) {
            this->n_formatter = Own<Formatter>(formatter);
        } else {
            this->n_formatter = Own<Formatter>::New<F>(formatter);
        }

        return *this;
    }

    template<typename F, typename... Args>
        requires(!std::is_pointer_v<F> && std::derived_from<std::remove_cvref_t<F>, Formatter>)
    auto WithFormatter(Args&&... args) noexcept -> Console&
    {
        this->n_formatter = Own<Formatter>::New<F>(VIOLET_FWD(Args, args)...);
        return *this;
    }

    auto WithStream(Stream stream) noexcept -> Console&;

    NOELDOC_SEE("violet::experimental::log::Sink::Emit(const violet::experimental::log::Record&)")
    void Emit(const Record& record) override;

    NOELDOC_SEE("violet::experimental::log::Sink::Flush()")
    void Flush() noexcept override;

private:
    Own<Formatter> n_formatter;
    Synchronized<io::experimental::Descriptor::Borrowed> n_fd;
};

} // namespace violet::experimental::log::sinks
