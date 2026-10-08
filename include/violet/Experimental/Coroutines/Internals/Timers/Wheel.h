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
//! # 🌺💜 `violet/Experimental/Coroutines/Internals/TimerWheel.h`
//! A hierarchical timer wheel: six levels of 64 slots each, with 1 tick (~1ms) per level-0 slot, covering 2^36 (~2.2
//! years). Later deadlines wait in an overflow list. All entries are intrusive, so insert and remove are of `O(1)`
//! complexity and allocation-free.

#pragma once

#include <violet/Container/Optional.h>

#include <algorithm>
#include <bit>

namespace NOELDOC_HIDE violet {
namespace experimental::coro::internals {

/// An intrusive node in a [`TimerWheel`]. This is meant to be derived in whatever owns the entry.
///
/// ## Remarks
/// The entry must not move or be destroyed while `Linked` is `true`.
struct VIOLET_API TimerEntry {
    using fire_function = void (*)(TimerEntry* ent) noexcept;

    UInt64 Deadline = 0; //< deadline, in ticks.
    fire_function Fire = nullptr;
    TimerEntry* Previous = nullptr;
    TimerEntry* Next = nullptr;
    UInt16 List = 0;
    bool Linked = false;
};

/// A hierarchical timer wheel over abstract ticks. This is not thread-safe; callers confine it to one thread
/// or lock around it.
struct TimerWheel final {
    static constexpr UInt32 kBits = 6;
    static constexpr UInt32 kSlots = 1U << kBits;
    static constexpr UInt32 kLevels = 6;
    static constexpr UInt64 kMaxDuration = UInt64{1} << (kBits * kLevels);

    VIOLET_DISALLOW_COPY_AND_MOVE(TimerWheel);
    ~TimerWheel() = default;

    VIOLET_EXPLICIT TimerWheel(UInt64 now = 0) noexcept
        : n_elapsed(now)
    {
    }

    [[nodiscard]]
    auto Elapsed() const noexcept -> UInt64
    {
        return this->n_elapsed;
    }

    [[nodiscard]]
    auto Size() const noexcept -> UInt
    {
        return this->n_size;
    }

    [[nodiscard]]
    auto Empty() const noexcept -> bool
    {
        return this->n_size == 0;
    }

    void Insert(TimerEntry& ent) noexcept
    {
        VIOLET_DEBUG_ASSERT(!ent.Linked, "timer entry is already linked");

        this->place(ent);
        this->n_size++;
    }

    void Remove(TimerEntry& ent) noexcept
    {
        VIOLET_DEBUG_ASSERT(ent.Linked, "timer entry was not linked");

        this->unlink(ent);
        this->n_size--;
    }

    [[nodiscard]]
    auto NextExpiration() const noexcept -> Optional<UInt64>
    {
        if (this->n_heads[kExpired] != nullptr) {
            return this->n_elapsed;
        }

        bool found = false;
        UInt64 best = 0;
        for (UInt32 level = 0; level < kLevels; level++) {
            if (this->n_occupied[level] == 0) {
                continue;
            }

            UInt64 deadline = this->getSlotDeadline(level, this->getNextSlot(level));
            if (!found || deadline < best) {
                found = true;
                best = deadline;
            }
        }

        if (this->n_heads[kOverflow] != nullptr) {
            UInt64 deadline = this->doOverflowRecheck();
            if (!found || deadline < best) {
                found = true;
                best = deadline;
            }
        }

        if (!found) {
            return Nothing;
        }

        return best;
    }

    template<typename F>
    void Advance(UInt64 now, F&& fire)
    {
        const auto fireFn = VIOLET_FWD(F, fire);
        while (true) {
            if (auto* ent = this->n_heads[kExpired]; ent != nullptr) {
                this->Remove(*ent);
                std::invoke(fireFn, ent);

                continue;
            }

            bool found = false;
            bool overflow = false;
            UInt32 bestLevel = 0;
            UInt32 bestSlot = 0;
            UInt64 best = 0;

            for (UInt32 level = 0; level < kLevels; level++) {
                if (this->n_occupied[level] == 0) {
                    continue;
                }

                UInt32 slot = this->getNextSlot(level);
                UInt64 deadline = this->getSlotDeadline(level, slot);
                if (!found || deadline < best) {
                    found = true;
                    best = deadline;
                    bestLevel = level;
                    bestSlot = slot;
                }
            }

            if (this->n_heads[kOverflow] != nullptr) {
                UInt64 deadline = this->doOverflowRecheck();
                if (!found || deadline < best) {
                    found = true;
                    overflow = true;
                    best = deadline;
                }
            }

            if (!found || best > now) {
                break;
            }

            this->n_elapsed = std::max(this->n_elapsed, best);

            if (overflow) {
                this->moveAll(kOverflow, kProcessing);
            } else {
                this->moveAll(static_cast<UInt16>((bestLevel * kSlots) + bestSlot), kProcessing);
                this->n_occupied[bestLevel] &= ~(UInt64{1} << bestSlot);
            }

            while (TimerEntry* entry = this->n_heads[kProcessing]) {
                this->unlink(*entry);
                this->n_size--;

                if (entry->Deadline <= this->n_elapsed) {
                    fire(entry);
                } else {
                    this->place(*entry);
                    this->n_size++;
                }
            }
        }

        this->n_elapsed = std::max(this->n_elapsed, now);
    }

    template<typename F>
    void Drain(F&& fn)
    {
        const auto fireFn = VIOLET_FWD(F, fn);
        for (UInt16 list = 0; list < kLists; list++) {
            while (TimerEntry* ent = this->n_heads[list]) {
                this->unlink(*ent);
                this->n_size--;

                std::invoke(fireFn, ent);
            }
        }
    }

private:
    static constexpr UInt16 kExpired = kLevels * kSlots;
    static constexpr UInt16 kOverflow = kExpired + 1;
    static constexpr UInt16 kProcessing = kExpired + 2;
    static constexpr UInt16 kLists = kExpired + 3;

    void place(TimerEntry& ent) noexcept
    {
        if (ent.Deadline <= this->n_elapsed) {
            this->link(ent, kExpired);
            return;
        }

        const UInt64 masked = (this->n_elapsed ^ ent.Deadline) | (kSlots - 1);
        if (masked >= kMaxDuration) {
            this->link(ent, kOverflow);
            return;
        }

        const UInt32 level = (63U - static_cast<UInt32>(std::countl_zero(masked))) / kBits;
        const UInt32 slot = static_cast<UInt32>(ent.Deadline >> (level * kBits)) & (kSlots - 1);

        this->link(ent, static_cast<UInt16>((level * kSlots) + slot));
        this->n_occupied[level] |= UInt64{1} << slot;
    }

    void link(TimerEntry& ent, UInt16 list) noexcept
    {
        ent.List = list;
        ent.Linked = true;
        ent.Previous = nullptr;
        ent.Next = this->n_heads[list];

        if (ent.Next != nullptr) {
            ent.Next->Previous = &ent;
        }

        this->n_heads[list] = &ent;
    }

    void unlink(TimerEntry& ent) noexcept
    {
        if (ent.Previous != nullptr) {
            ent.Previous->Next = ent.Next;
        } else {
            this->n_heads[ent.List] = ent.Next;
        }

        if (ent.Next != nullptr) {
            ent.Next->Previous = ent.Previous;
        }

        if (ent.List < kExpired && this->n_heads[ent.List] == nullptr) {
            this->n_occupied[ent.List / kSlots] &= ~(UInt64{1} << (ent.List % kSlots));
        }

        ent.Previous = nullptr;
        ent.Next = nullptr;
        ent.Linked = false;
    }

    void moveAll(UInt16 from, UInt16 to) noexcept
    {
        while (TimerEntry* entry = this->n_heads[from]) {
            this->n_heads[from] = entry->Next;
            this->link(*entry, to);
        }
    }

    [[nodiscard]]
    auto getNextSlot(UInt32 level) const noexcept -> UInt32
    {
        const UInt32 nowSlot = static_cast<UInt32>(this->n_elapsed >> (level * kBits)) & (kSlots - 1);
        const UInt64 rotated = std::rotr(this->n_occupied[level], static_cast<int>(nowSlot));

        return (nowSlot + static_cast<UInt32>(std::countr_zero(rotated))) & (kSlots - 1);
    }

    [[nodiscard]]
    auto getSlotDeadline(UInt32 level, UInt32 slot) const noexcept -> UInt64
    {
        const UInt64 slotRange = UInt64{1} << (level * kBits);
        const UInt64 levelRange = slotRange << kBits;
        const UInt32 nowSlot = static_cast<UInt32>(this->n_elapsed >> (level * kBits)) & (kSlots - 1);

        UInt64 deadline = (this->n_elapsed & ~(levelRange - 1)) + (slot * slotRange);
        if (slot < nowSlot) {
            deadline += levelRange;
        }

        return deadline < this->n_elapsed ? this->n_elapsed : deadline;
    }

    [[nodiscard]] auto doOverflowRecheck() const noexcept -> UInt64
    {
        UInt64 min = ~UInt64{0};
        for (const TimerEntry* entry = this->n_heads[kOverflow]; entry != nullptr; entry = entry->Next) {
            min = std::min(entry->Deadline, min);
        }

        const UInt64 epoch = min & ~(kMaxDuration - 1);
        return epoch > this->n_elapsed ? epoch : this->n_elapsed;
    }

    UInt64 n_elapsed;
    UInt n_size = 0;
    Array<UInt64, kLevels> n_occupied{};
    Array<TimerEntry*, kLists> n_heads{};
};

} // namespace experimental::coro::internals
} // namespace NOELDOC_HIDE violet
