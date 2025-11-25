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

#include <violet/Events/EventEmitter.h>
#include <violet/Violet.h>

using violet::events::Emitter;
using violet::events::Event;

template<typename... Args>
using fun = std::function<void(Args...)>;

template<typename... Args>
Event<Args...>::Event(Emitter<Args...>* ptr)
    : n_emitter(ptr)
{
}

template<typename... Args>
auto Event<Args...>::operator()(fun<Args...> fn) const noexcept -> Emitter<Args...>::Guard
{
    return this->n_emitter->On(fn);
}

template<typename... Args>
Emitter<Args...>::Emitter()
    : n_event(this)
{
}

template<typename... Args>
Emitter<Args...>::Guard::Guard(Emitter* ptr, UInt64 id)
    : n_emitter(ptr)
    , n_id(id)
{
}

template<typename... Args>
Emitter<Args...>::Guard::~Guard()
{
    if (this->n_emitter != nullptr && this->n_id != 0U) {
        this->n_emitter->removeListener(this->n_id);
    }
}

template<typename... Args>
auto Emitter<Args...>::Guard::ID() const noexcept -> UInt64
{
    return this->n_id;
}

template<typename... Args>
auto Emitter<Args...>::On(fun<Args...> fun) noexcept -> Guard
{
    return this->addListener(fun);
}

template<typename... Args>
auto Emitter<Args...>::Once(fun<Args...> fun) noexcept -> Guard
{
    return this->addListener(fun, true);
}

template<typename... Args>
void Emitter<Args...>::Unsubscribe(UInt64 id) noexcept
{
    this->removeListener(id);
}

template<typename... Args>
void Emitter<Args...>::Fire(Args&&... args) noexcept
{
    Vec<entry> snapshot = this->snapshot();
    for (entry& entry: snapshot) {
        std::invoke(entry.Callback, VIOLET_FWD(Args, args)...);

        if (entry.Once) {
            this->removeListener(entry.ID);
        }
    }
}

template<typename... Args>
auto Emitter<Args...>::Event() const noexcept -> event_t
{
    return this->n_event;
}

template<typename... Args>
auto Emitter<Args...>::getNextId() noexcept -> UInt64
{
    return this->n_nextId.fetch_add(1, std::memory_order_relaxed);
}

template<typename... Args>
auto Emitter<Args...>::addListener(fun<Args...> fn, bool once) noexcept -> Guard
{
    std::lock_guard lock(this->n_mu);

    auto id = this->getNextId();
    this->n_listeners.push_back(entry{ .ID = id, .Once = once, .Callback = fn });

    return Guard(this, id);
}

template<typename... Args>
void Emitter<Args...>::removeListener(UInt64 id) noexcept
{
    std::lock_guard lock(this->n_mu);

    auto it = std::find_if(
        this->n_listeners.begin(), this->n_listeners.end(), [id](auto const& entry) -> bool { return entry.ID == id; });

    if (it != this->n_listeners.end()) {
        this->n_listeners.erase(it);
    }
}

template<typename... Args>
auto Emitter<Args...>::snapshot() const noexcept -> Vec<entry>
{
    std::lock_guard(this->n_mu);
    return this->n_listeners;
}
