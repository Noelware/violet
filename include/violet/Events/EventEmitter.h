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

#pragma once

#include "violet/Violet.h"

namespace violet::events {

template<typename... Args>
struct Event;

template<typename... Args>
struct Emitter final {
private:
    using event_t = struct Event<Args...>;

public:
    struct Guard final {
        ~Guard();

        Guard(const Guard&) = delete;
        Guard(Guard&&) noexcept = default;

        auto operator=(const Guard&) -> Guard& = delete;
        auto operator=(Guard&&) noexcept -> Guard& = default;

        [[nodiscard]] auto ID() const noexcept -> UInt64;

    private:
        VIOLET_EXPLICIT Guard(Emitter*, UInt64);

        Emitter* n_emitter;
        UInt64 n_id;
    };

    VIOLET_IMPLICIT Emitter();

    auto On(std::function<void(Args...)> fun) noexcept -> Guard;
    auto Once(std::function<void(Args...)> fun) noexcept -> Guard;
    void Unsubscribe(UInt64 id) noexcept;
    void Fire(Args&&... args) noexcept;

    auto Event() const noexcept -> event_t;

private:
    struct entry final {
        UInt64 ID;
        std::function<void(Args...)> Callback;
        bool Once;
    };

    auto getNextId() noexcept -> UInt64;
    auto addListener(std::function<void(Args...)> fun, bool once = false) noexcept -> Guard;
    auto snapshot() const noexcept -> Vec<entry>;
    void removeListener(UInt64 id) noexcept;

    event_t n_event;
    mutable std::atomic<UInt64> n_nextId = 0;
    mutable Mutex n_mu;
    mutable Vec<entry> n_listeners ABSL_GUARDED_BY(n_mu);
};

template<typename... Args>
struct Event final {
    auto operator()(std::function<void(Args...)>) const noexcept -> Emitter<Args...>::Guard;

private:
    friend struct Emitter<Args...>;

    VIOLET_EXPLICIT Event(Emitter<Args...>*);

    Emitter<Args...>* n_emitter;
};

} // namespace violet::events
