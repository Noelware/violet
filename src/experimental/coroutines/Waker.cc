// 🌺💜 Violet: Extended C standard library
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

#include <violet/Experimental/Coroutines/Waker.h>

namespace violet::experimental::coro {

void Waker::Wake() && noexcept
{
    if (const auto* vt = std::exchange(this->n_vtable, nullptr))
        vt->Wake(std::exchange(this->n_object, nullptr));
}

void Waker::WakeByRef() noexcept
{
    VIOLET_ASSUME(this->n_vtable != nullptr);
    this->n_vtable->WakeByRef(this->n_object);
}

auto internals::MkWakerFor(internals::ParkedNode& node) noexcept -> Waker
{
    static constexpr Waker::VTable vt{
        // clang-format off
        .Wake = [](void* data) noexcept -> void {
            auto* _ = static_cast<internals::ParkedNode*>(data);
            // node->Owner->WakeTask(&node->Task);
        },
        .WakeByRef = [](void* data) noexcept -> void {
            auto* _ = static_cast<internals::ParkedNode*>(data);
            // node->Owner->WakeTask(&node->Task);
        },
        .Clone = [](void* data, void** outData, const Waker::VTable** outVt) noexcept -> void {
            *outData = data;
            *outVt = &vt;
        },
        .Drop = [](void*) noexcept -> void {
            /* `node` lives inside the awaiter's coroutine frame */
        }
        // clang-format on
    };

    return Waker{&node, &vt};
}

} // namespace violet::experimental::coro
