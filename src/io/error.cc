// 🌺💜 Violet: Extended C++ standard library
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

#include <violet/IO/Error.h>
#include <violet/Violet.h>

using violet::io::Error;

auto Error::OSError() -> Error
{
    Error error;
    error.n_repr = PlatformError();

    return error;
}

auto Error::RawOSError() const noexcept -> Optional<PlatformError::error_type>
{
    if (const auto* pat = std::get_if<PlatformError>(&this->n_repr)) {
        return Some<PlatformError::error_type>(pat->Get());
    }

    return {};
}

auto Error::Kind() const noexcept -> ErrorKind
{
    if (const auto* pat = std::get_if<PlatformError>(&this->n_repr)) {
        return pat->AsErrorKind();
    }

    if (const auto* msg = std::get_if<Error::simple_message>(&this->n_repr)) {
        return msg->Kind();
    }

    if (const auto* kind = std::get_if<ErrorKind>(&this->n_repr)) {
        return *kind;
    }

#if VIOLET_USE_RTTI
    if (const auto* pair = std::get_if<Pair<ErrorKind, Any>>(&this->n_repr)) {
        return pair->first;
    }
#endif

    return ErrorKind::__other;
}

auto Error::ToString() const noexcept -> String
{
    std::ostringstream os("I/o error");
    if (const auto* pat = std::get_if<PlatformError>(&this->n_repr)) {
        os << " (system error «" << pat->Get() << "»): " << pat->ToString();
        return os.str();
    }

    if (const auto* msg = std::get_if<simple_message>(&this->n_repr)) {
        os << " (" << violet::ToString(msg->Kind()) << "): " << msg->Message();
        return os.str();
    }

    if (const auto* kind = std::get_if<ErrorKind>(&this->n_repr)) {
        os << ": " << violet::ToString(*kind);
        return os.str();
    }

#if VIOLET_USE_RTTI
    // Find the most common ways that users could possibly used:
    // - String, Str, CStr (use the `ErrorKind(Kind, Message)` constructor instead of this)
    if (auto msg = this->Downcast<String>()) {
        os << ": " << *msg;
    }

    if (auto msg = this->Downcast<Str>()) {
        os << ": " << *msg;
    }

    if (auto msg = this->Downcast<CStr>()) {
        os << ": " << *msg;
    }
#endif

    return os.str();
}

#if VIOLET_USE_RTTI
template<typename T>
auto Error::Downcast() const noexcept -> Optional<T>
{
    if (const auto* pair = std::get_if<Pair<ErrorKind, Any>>(&this->n_repr)) {
        try {
            auto downcasted = std::any_cast<T>(pair->second);
            return Some<T>(downcasted);
        } catch (const std::bad_any_cast&) {
            return Nothing;
        }
    }

    return Nothing;
}
#endif
