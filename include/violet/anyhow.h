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
//! # 🌺💜 `violet/anyhow.h` ¯\_(°ペ)_/¯
//! The **violet::anyhow** library is a recreation of [`anyhow::Error`], [`eyre::Report`] but in C++26
//! using the same constraints and benefits.
//!
//! [`anyhow::Error`]: https://docs.rs/anyhow/latest/anyhow/struct.Error.html
//! [`eyre::Report`]: https://docs.rs/eyre/latest/eyre/struct.Report.html

#pragma once

#include <violet/Container/Result.h>
#include <violet/Violet.h>

#if VIOLET_USE_RTTI
#include <violet/Support/Demangle.h>
#endif

#include <source_location>
#include <sstream>

namespace violet::anyhow {

/// A type-erased error container which stores an arbitrary object that represents
/// a error and an optional chain of contextual frames.
///
/// ## Notes
/// * `Error` is conceptually similar to [`anyhow::Error`] or [`eyre::Report`].
///
/// [`anyhow::Error`]: https://docs.rs/anyhow/latest/anyhow/struct.Error.html
/// [`eyre::Report`]: https://docs.rs/eyre/latest/eyre/struct.Report.html
///
/// ## Example
/// ```cpp
/// #include <violet/anyhow.h>
/// #include <print>
///
/// using Error = violet::anyhow::Error;
///
/// Error error("disk full [/dev/sda1]");
/// std::println("error reached: {}", error);
///
/// auto ctx = error.Context("while trying to save `user_data.json`");
/// std::println("{}", ctx);
/// ```
struct Error final {
    VIOLET_DISALLOW_COPY(Error);

    VIOLET_IMPLICIT Error(Error&& other) noexcept;
    auto operator=(Error&& other) noexcept -> Error&;

    ~Error() noexcept;

    template<typename T>
    VIOLET_IMPLICIT Error(T object, std::source_location loc = std::source_location::current()) noexcept
        : n_node(node_t::New(object, loc))
    {
    }

    auto Context(Error&& error) && noexcept -> Error;

    /// Produce a new `Error` that adds an additional context frame on top of the
    /// existing chain.
    ///
    /// `Context` accepts any `object` (string, custom error type, etc.) and captures
    /// its `std::source_location`. The original error chain is preserved as the
    /// `Next` link of the newly-created node.
    ///
    /// Example:
    /// ```cpp
    /// auto base = violet::anyhow::Error(violet::io::Error::OSError());
    /// auto with_ctx = base.Context("reading config");
    /// ```
    template<typename T>
        requires(!std::same_as<std::decay_t<T>, Error>)
    auto Context(T object, std::source_location loc = std::source_location::current()) && noexcept -> Error
    {
        node_t* next = std::exchange(this->n_node, nullptr);

        Error error{};
        error.n_node = node_t::New(VIOLET_MOVE(object), loc, next);
        return error;
    }

    /// Print a human-readable representation of the error to the process' standard error.
    void Print() const noexcept;

private:
    VIOLET_IMPLICIT Error() noexcept = default;

    struct vtable_t final {
        String (*Message)(const void*) = nullptr;
        void* (*Clone)(const void*, void*) = nullptr;
        void (*Destruct)(void*) = nullptr;
    };

    struct node_t final {
        VIOLET_DISALLOW_COPY(node_t);

        void* Object;
        vtable_t VTable;
        UInt Size;
        std::source_location Location;
        node_t* Next;

        VIOLET_IMPLICIT node_t() noexcept;
        ~node_t() noexcept;

        VIOLET_IMPLICIT node_t(node_t&& other) noexcept;
        auto operator=(node_t&& other) noexcept -> node_t&;

        template<typename T>
        static auto New(T object, std::source_location loc, node_t* next = nullptr) noexcept(
            std::is_move_constructible_v<T>) -> node_t*
        {
            auto* node = new node_t();
            node->Location = loc;
            node->Object = new T(VIOLET_MOVE(object));
            node->Size = sizeof(T);
            node->Next = next;

            constexpr static vtable_t __vtable_for_object{
                [](const void* ptr) -> String {
                    const auto* src = static_cast<const T*>(ptr);
                    VIOLET_DEBUG_ASSERT(src != nullptr, "assumption failed");

                    if constexpr (Stringify<T>) {
                        return ToString(*src);
                    }

                    if constexpr (requires(std::ostream& os) { os << *src; }) {
                        std::ostringstream os;
                        os << *src;

                        return os.str();
                    }

#if VIOLET_USE_RTTI
                    const auto& type = typeid(T);
                    return std::format("<type {}@{}>", violet::util::DemangleCXXName(type.name()), type.hash_code());
#else
                    return "";
#endif
                },

                [](const void* src, void* dst) -> void* { return new (dst) T(*static_cast<const T*>(src)); },
                [](void* src) -> void {
                    T* ptr = static_cast<T*>(src);

                    std::destroy_at(ptr);
                    ::operator delete(ptr);
                }
            };

            node->VTable = __vtable_for_object;
            return node;
        }
    };

    node_t* n_node;
};

template<typename T = void>
using Result = violet::Result<T, Error>;

} // namespace violet::anyhow
