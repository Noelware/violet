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

#include <violet/Experimental/Any.h>

using violet::String;
using violet::experimental::Any;

Any::~Any()
{
    this->destructObject();
}

Any::Any(const Any& other) noexcept
    : n_vtable(other.n_vtable)
    , n_size(other.n_size)
    , n_type(other.n_type)
{
    // TODO(@auguwu/Noel): is there a way to do this at compile-time?
    VIOLET_DEBUG_ASSERT(this->n_vtable.Clone != nullptr, "`Any` cannot be copyable if `T` was not non-copyable");

    void* self = ::operator new(this->n_size);
    this->n_self = this->n_vtable.Clone(other.n_self, self);
}

auto Any::operator=(const Any& other) noexcept -> Any&
{
    if (this != &other) {
        Any tmp(other);
        *this = VIOLET_MOVE(tmp);
    }

    return *this;
}

Any::Any(Any&& other) noexcept
    : n_self(std::exchange(other.n_self, nullptr))
    , n_vtable(other.n_vtable)
    , n_size(other.n_size)
    , n_type(other.n_type)
{
}

auto Any::operator=(Any&& other) noexcept -> Any&
{
    if (this != &other) {
        this->destructObject();

        this->n_self = std::exchange(other.n_self, nullptr);
        this->n_vtable = other.n_vtable;
        this->n_size = other.n_size;
        this->n_type = other.n_type;
    }

    return *this;
}

#if VIOLET_USE_RTTI
auto Any::TypeName() const noexcept -> String
{
    return util::DemangleCXXName(this->n_type.name());
}
#endif

auto Any::ToString() const noexcept -> String
{
    if (this->n_vtable.ToString != nullptr) {
        return this->n_vtable.ToString(this->n_self);
    }

#if VIOLET_USE_RTTI
    return std::format("<type {}@{}>", this->TypeName(), this->n_type.hash_code());
#else
    return "<type not stringifiable>";
#endif
}

void Any::destructObject()
{
    if (this->n_self != nullptr) {
        if (this->n_vtable.Destruct != nullptr) {
            this->n_vtable.Destruct(this->n_self);
        }

        this->n_self = nullptr;
    }
}
