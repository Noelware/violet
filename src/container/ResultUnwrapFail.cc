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

#include <violet/Container/Result.h>
#include <violet/Violet.h>

#ifndef VIOLET_HAS_EXCEPTIONS
#include <source_location>
#endif

#ifdef VIOLET_HAS_EXCEPTIONS

void violet::resultUnwrapFail()
{
    throw std::logic_error("`Result<T, E>::Unwrap()` failed due to error variant");
}

void violet::resultUnwrapErrFail()
{
    throw std::logic_error("`Result<T, E>::UnwrapErr()` failed due to ok variant");
}

void violet::resultUnwrapFail(CStr message)
{
    throw std::logic_error(std::format("`Result<T, E>::Expect()`: {}", message));
}

#else

void violet::resultUnwrapFail(const std::source_location& loc)
{
    std::cerr << "[" << loc.file_name() << ":" << loc.line() << ":" << loc.column() << "; in " << loc.function_name()
              << "]: `Optional<T>::Unwrap()` failed due to error variant\n";

    std::abort();
}

void violet::resultUnwrapErrFail(const std::source_location& loc)
{
    std::cerr << "[" << loc.file_name() << ":" << loc.line() << ":" << loc.column() << "; in " << loc.function_name()
              << "]: `Optional<T>::UnwrapErr()` failed due to ok variant\n";

    std::abort();
}

void violet::resultUnwrapFail(CStr message, const std::source_location& loc)
{
    std::cerr << "[" << loc.file_name() << ":" << loc.line() << ":" << loc.column() << "; in " << loc.function_name()
              << "]: `Result<T, E>::Expect()`: " << message << '\n';

    std::abort();
}

#endif
