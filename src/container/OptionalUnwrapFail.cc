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

#include <violet/Container/Optional.h>
#include <violet/Violet.h>

#ifndef VIOLET_HAS_EXCEPTIONS
#include <source_location>
#endif

#ifdef VIOLET_HAS_EXCEPTIONS

void violet::optionalUnwrapFail()
{
    throw std::logic_error("`Optional<T>::Unwrap()` failed due to nothing present");
}

void violet::optionalUnwrapFail(CStr message)
{
    throw std::logic_error(std::format("`Optional<T>::Expect()`: {}", message));
}

#else

void violet::optionalUnwrapFail(const std::source_location& loc)
{
    std::cerr << "[" << loc.file_name() << ":" << loc.line() << ":" << loc.column() << "; in " << loc.function_name()
              << "]: `Optional<T>::Unwrap()`failed due to nothing present\n";

    std::abort();
}

void violet::optionalUnwrapFail(CStr message, const std::source_location& loc)
{
    std::cerr << "[" << loc.file_name() << ":" << loc.line() << ":" << loc.column() << "; in " << loc.function_name()
              << "]: `Optional<T>::Expect()`: " << message << '\n';

    std::abort();
}

#endif
