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

#include <violet/Experimental/Buf.h>

namespace violet::experimental {
namespace buf_internal {

static_assert(alignof(Storage) <= __STDCPP_DEFAULT_NEW_ALIGNMENT__);

auto AllocateStorage(UInt capacity) -> Storage*
{
    void* bytes = ::operator new(sizeof(Storage) + capacity);
    auto* storage = ::new (bytes) Storage{};

    storage->Capacity = capacity;
    return storage;
}

void RetainStorage(Storage* storage) noexcept
{
    storage->Refs.fetch_add(1, std::memory_order_relaxed);
}

void ReleaseStorage(Storage* storage) noexcept
{
    if (storage == nullptr || storage->Refs.fetch_sub(1, std::memory_order_acq_rel) != 1) {
        return;
    }

    const UInt capacity = storage->Capacity;
    storage->~Storage();
    ::operator delete(static_cast<void*>(storage), sizeof(Storage) + capacity);
}

const VTable kSharedBytesVTable{
    // clang-format off
    .Clone = [](void* data) noexcept -> void* {
        RetainStorage(static_cast<Storage*>(data));
        return data;
    },
    .Drop = [](void* data) noexcept -> void {
        ReleaseStorage(static_cast<Storage*>(data));
    },
    .Unique = [](void* data) noexcept -> bool {
        return static_cast<Storage*>(data)->Refs.load(std::memory_order_acquire) == 1;
    },
    // clang-format on
};

} // namespace buf_internal

auto Buf::CopyFrom(Span<const std::byte> data) -> Buf
{
    if (data.empty()) {
        return {};
    }

    Bytes buf(data.size());
    buf.Extend(data);

    return VIOLET_MOVE(buf).Freeze();
}

auto Buf::TryIntoMut() && noexcept -> Optional<Bytes>
{
    if (this->n_vtable != &buf_internal::kSharedBytesVTable || !this->Unique()) {
        return Nothing;
    }

    auto* storage = static_cast<buf_internal::Storage*>(std::exchange(this->n_handle, nullptr));
    auto* start = reinterpret_cast<std::byte*>(const_cast<char*>(this->n_data)); // NOLINT: storage is writable
    const auto offset = static_cast<UInt>(start - storage->Data());

    Bytes buf(storage, start, this->n_size, storage->Capacity - offset);
    this->n_vtable = &buf_internal::kStaticBytesVTable;
    this->n_data = nullptr;
    this->n_size = 0;

    return VIOLET_MOVE(buf);
}

void Bytes::Reserve(UInt additional)
{
    if (this->Spare() >= additional) {
        return;
    }

    const UInt capacity = std::max(this->n_size + additional, this->n_capacity * 2);
    buf_internal::Storage* next = buf_internal::AllocateStorage(capacity);

    if (this->n_size != 0) {
        ::memcpy(next->Data(), this->n_data, this->n_size);
    }

    buf_internal::ReleaseStorage(this->n_storage);
    this->n_storage = next;
    this->n_data = next->Data();
    this->n_capacity = capacity;
}

} // namespace violet::experimental
