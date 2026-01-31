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
//! # 🌺💜 `violet/Events/EventEmitter.h`
//! This module provides a event-driven system similar to Visual Studio Code's `events` API
//! or Node.js' `node:events` module.

#pragma once

#include <violet/Violet.h>

#include <functional>

namespace violet::events {

template<typename... Args>
struct Event;

/// Emitter represents a way to fire event objects to functions in a thread-safe context. [`Emitter<Args...>`]
/// is modeled after Visual Studio Code's `events` API ([`vscode.events.Emitter`]).
///
/// [`vscode.events.Emitter`]: https://code.visualstudio.com/api/references/vscode-api#EventEmitter&lt;T&gt;
///
/// ## Example
/// ```cpp
/// /* TODO(@auguwu): example here lol */
/// ```
///
/// @tparam Args arguments that the listener will receive.
template<typename... Args>
class Emitter final: std::enable_shared_from_this<Emitter<Args...>> {
    using event_t = struct Event<Args...>;

public:
    /// Type alias for listener callbacks.
    using func_t = std::function<void(Args...)>;

    /// A RAII-style guard for managing the lifetime of a event listener. This will deregister
    /// the event from a [`Emitter<Args...>`] after the guard being destroyed or manually
    /// via [`Guard::Dispose`].
    struct Guard final {
        VIOLET_DISALLOW_CONSTRUCTOR(Guard);
        VIOLET_DISALLOW_COPY(Guard);

        ~Guard()
        {
            this->Dispose();
        }

        Guard(Guard&& other) noexcept
            : n_emitter(VIOLET_MOVE(other.n_emitter))
            , n_id(other.n_id)
            , n_persist(other.n_persist)
        {

            other.n_id = -1;
            other.n_persist = false;
        }

        auto operator=(Guard&& other) noexcept -> Guard&
        {
            if (this != &other) {
                Dispose();

                this->n_emitter = VIOLET_MOVE(other.n_emitter);
                this->n_persist = other.n_persist;
                this->n_id = other.n_id;

                other.n_persist = false;
                other.n_id = -1;
            }

            return *this;
        }

        /// Returns the opaque, unique identifier of the listener itself.
        [[nodiscard]] auto ID() const noexcept -> Int64
        {
            return this->n_id;
        }

        /// When called, this listener will be persisted through the lifetime
        /// of a [`Emitter`].
        ///
        /// ## Remarks
        /// It is up to the caller to deregister the listener.
        void Persist() noexcept
        {
            this->n_persist = true;
        }

        /// Manually deregister the listener from the emitter that the RAII guard
        /// was constructed from.
        void Dispose() noexcept
        {
            if (this->n_emitter == nullptr || this->n_id == -1 || this->n_persist) {
                return;
            }

            this->n_emitter->removeListener(this->n_id);

            this->n_emitter = nullptr;
            this->n_persist = false;
            this->n_id = -1;
        }

    private:
        friend class Emitter;

        VIOLET_EXPLICIT Guard(Emitter* emitter, Int64 id)
            : n_emitter(emitter)
            , n_id(id)
        {
        }

        Emitter* n_emitter;
        bool n_persist = false;
        Int64 n_id;
    };

    /// Construct a new [`Emitter`].
    VIOLET_IMPLICIT Emitter()
        : n_event(this)
    {
    }

    /// Subscribes a listener to the emitter and returns a RAII guard.
    /// @param fun callback function to invoke when the event fires.
    /// @param persist whether if the listener should persist through the lifetime of this emitter.
    auto On(func_t fun, bool persist = false) noexcept -> Guard
    {
        auto id = this->addListener(fun);
        auto guard = Guard(this, id);

        if (persist) {
            guard.Persist();
        }

        return guard;
    }

    /// Subscribes a one-time listener to this emitter.
    /// @param fun callback function to invoke once when the event fires.
    auto Once(func_t fun) noexcept -> Guard
    {
        auto id = this->addListener(fun, true);
        return Guard(this, id);
    }

    /// Deregister a listener manually
    /// @param id the listener id (usually from [`Guard::ID`])
    void Unsubscribe(Int64 id) noexcept
    {
        this->removeListener(id);
    }

    /// Fires a new event that all listeners will react to
    /// @param args the arguments the listener will receive
    void Fire(Args&&... args) noexcept
    {
        for (entry& ent: this->getSnapshot()) {
            std::invoke(ent.Callback, VIOLET_FWD(Args, args)...);

            if (ent.Once) {
                this->removeListener(ent.ID);
            }
        }
    }

    /// Returns the event handle for this emitter.
    auto Event() const noexcept -> event_t
    {
        return this->n_event;
    }

private:
    struct entry final {
        Int64 ID;
        func_t Callback;
        bool Once;
    };

    auto getNextId() noexcept -> UInt64
    {
        return this->n_nextId.fetch_add(1, std::memory_order_relaxed);
    }

    auto addListener(func_t fun, bool once = false) noexcept -> Int64
    {
        std::lock_guard lock(this->n_mu);

        auto id = this->getNextId();
        this->n_listeners.push_back(entry{ .ID = id, .Callback = fun, .Once = once });

        return id;
    }

    auto getSnapshot() const noexcept -> Vec<entry>
    {
        std::lock_guard lock(this->n_mu);
        return this->n_listeners;
    }

    void removeListener(Int64 id) noexcept
    {
        std::lock_guard lock(this->n_mu);

        auto it = std::find_if(this->n_listeners.begin(), this->n_listeners.end(),
            [id](auto const& entry) -> bool { return entry.ID == id; });

        if (it != this->n_listeners.end()) {
            this->n_listeners.erase(it);
        }
    }

    event_t n_event;
    mutable std::atomic<Int64> n_nextId = 0;
    mutable Mutex n_mu;
    mutable Vec<entry> n_listeners;
};

/// A handle that allows subscribing listeners from a [`Emitter<Args...>`].
/// @tparam Args argument types that the event carries (must match [`Emitter<Args...>`])
template<typename... Args>
struct Event final {
    /// Deleted default constructor to enforce association with an `Emitter`.
    VIOLET_DISALLOW_CONSTEXPR_CONSTRUCTOR(Event);

    /// Returns a [`Guard`][Emitter<Args...>::Guard] RAII object managing the listener's
    /// lifetime.
    ///
    /// ## Remarks
    /// The event will keep track of the guard so you can "forget" it exists when
    /// registering a new callback.
    ///
    /// @param fun callback function to attach.
    auto operator()(Emitter<Args...>::func_t fun, bool persist = false) const noexcept -> Emitter<Args...>::Guard
    {
        return this->n_emitter->On(fun, persist);
    }

    /// Returns a [`Guard`][Emitter<Args...>::Guard] RAII object managing the listener's
    /// lifetime. Once the event is invoked, the listener will be deregistered afterwards.
    ///
    /// ## Remarks
    /// The event will keep track of the guard so you can "forget" it exists when
    /// registering a new callback.
    ///
    /// @param fun callback function to attach.
    auto Once(Emitter<Args...>::func_t fun) const noexcept -> Emitter<Args...>::Guard
    {
        return this->n_emitter->Once(fun);
    }

private:
    using guard_t = Emitter<Args...>::Guard;

    friend class Emitter<Args...>;

    VIOLET_EXPLICIT Event(Emitter<Args...>* emitter)
        : n_emitter(emitter)
    {
    }

    Emitter<Args...>* n_emitter;
};

} // namespace violet::events
