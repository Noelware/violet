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

#include <gtest/gtest.h>
#include <violet/Experimental/Coroutines/Runtime.h>

#if VIOLET_FEATURE(EXCEPTIONS)
#include <stdexcept>
#endif

namespace violet::experimental::coro {
namespace {

auto ReturnsFortyTwo() noexcept -> Task<Int32>
{
    co_return 42;
}

auto AddOne(Int32 value) noexcept -> Task<Int32>
{
    co_return value + 1;
}

auto SpawnAndJoin() noexcept -> Task<Int32>
{
    JoinHandle child = Handle::Current().Spawn(AddOne(41));
    co_return co_await VIOLET_MOVE(child);
}

auto SetsFlag(bool* flag) -> Task<void>
{
    *flag = true;
    co_return;
}

auto SpawnsDetached(bool* flag) -> Task<void>
{
    // Dropping the handle detaches; the child must still run before `BlockOn` returns.
    {
        [[maybe_unused]]
        JoinHandle detached = Handle::Current().Spawn(SetsFlag(flag));
    }

    co_return;
}

auto JoinsMany() -> Task<Int32>
{
    JoinHandle a = Handle::Current().Spawn(AddOne(0));
    JoinHandle b = Handle::Current().Spawn(AddOne(1));
    JoinHandle c = Handle::Current().Spawn(AddOne(2));

    Int32 sum = co_await VIOLET_MOVE(a);
    sum += co_await VIOLET_MOVE(b);
    sum += co_await VIOLET_MOVE(c);
    co_return sum;
}

#if VIOLET_FEATURE(EXCEPTIONS)
auto Throws() -> Task<Int32>
{
    throw std::runtime_error("boom");
    co_return 0;
}

auto JoinsThrowingChild() -> Task<Int32>
{
    co_return co_await Handle::Current().Spawn(Throws());
}
#endif

struct DropFlag final {
    bool* Dropped;

    VIOLET_EXPLICIT DropFlag(bool* dropped) noexcept
        : Dropped(dropped)
    {
    }
    DropFlag(DropFlag&& other) noexcept
        : Dropped(std::exchange(other.Dropped, nullptr))
    {
    }
    ~DropFlag()
    {
        if (this->Dropped != nullptr) {
            *this->Dropped = true;
        }
    }
};

// Parks and leaks a reference to itself, like a `Waker` stored somewhere that never fires.
struct ParkForever final {
    internals::RawTask** Slot;

    [[nodiscard]] static auto await_ready() noexcept -> bool
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<>) const noexcept
    {
        *this->Slot = internals::CurrentTask();
        internals::RetainRawTask(Unsafe("simulated waker"), *this->Slot);
    }

    static void await_resume() noexcept { }
};

auto ParksWithLocal(internals::RawTask** slot, DropFlag flag) -> Task<void>
{
    co_await ParkForever{slot};
}

auto ParksInChild(internals::RawTask** slot, bool* childDropped) -> Task<void>
{
    co_await ParksWithLocal(slot, DropFlag(childDropped)); // the child is the one parked
}

auto SpawnsParked(internals::RawTask** slot, bool* dropped) -> Task<void>
{
    {
        [[maybe_unused]]
        JoinHandle detached = Handle::Current().Spawn(ParksWithLocal(slot, DropFlag(dropped)));
    }

    co_return;
}

auto SpawnsParkedNested(internals::RawTask** slot, bool* dropped) -> Task<void>
{
    {
        [[maybe_unused]]
        JoinHandle detached = Handle::Current().Spawn(ParksInChild(slot, dropped));
    }

    co_return;
}

} // namespace

TEST(Runtime, BlockOnReturnsValue)
{
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_EQ(rt->BlockOn(ReturnsFortyTwo()), 42);
}

TEST(Runtime, BlockOnCanRunTwice)
{
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_EQ(rt->BlockOn(ReturnsFortyTwo()), 42);
    ASSERT_EQ(rt->BlockOn(AddOne(1)), 2);
}

TEST(Runtime, SpawnedTaskCanBeJoined)
{
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_EQ(rt->BlockOn(SpawnAndJoin()), 42);
}

TEST(Runtime, JoinsSeveralChildren)
{
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_EQ(rt->BlockOn(JoinsMany()), 1 + 2 + 3);
}

TEST(Runtime, DetachedTaskStillRuns)
{
    bool ran = false;
    auto rt = Runtime::Builder::CurrentThread().Build();
    rt->BlockOn(SpawnsDetached(&ran));

    ASSERT_TRUE(ran);
}

#if VIOLET_FEATURE(EXCEPTIONS)
TEST(Runtime, BlockOnRethrows)
{
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_THROW(rt->BlockOn(Throws()), std::runtime_error);
}

TEST(Runtime, JoinRethrowsChildException)
{
    auto rt = Runtime::Builder::CurrentThread().Build();
    ASSERT_THROW(rt->BlockOn(JoinsThrowingChild()), std::runtime_error);
}
#endif

TEST(Runtime, ShutdownDestroysParkedTaskFrames)
{
    internals::RawTask* parked = nullptr;
    bool dropped = false;
    {
        auto rt = Runtime::Builder::CurrentThread().Build();
        rt->BlockOn(SpawnsParked(&parked, &dropped));
        ASSERT_FALSE(dropped);
    }

    ASSERT_TRUE(dropped);
    internals::ReleaseRawTask(Unsafe("the simulated waker lets go"), parked);
}

TEST(Runtime, ShutdownDestroysNestedChildFrames)
{
    internals::RawTask* parked = nullptr;
    bool dropped = false;
    {
        auto rt = Runtime::Builder::CurrentThread().Build();
        rt->BlockOn(SpawnsParkedNested(&parked, &dropped));
    }

    ASSERT_TRUE(dropped);
    internals::ReleaseRawTask(Unsafe("the simulated waker lets go"), parked);
}

TEST(Runtime, HandleOutlivingRuntimeIsCancelled)
{
    internals::RawTask* parked = nullptr;
    bool dropped = false;
    Optional<JoinHandle<void>> handle;
    {
        auto rt = Runtime::Builder::CurrentThread().Build();
        handle = Some(rt->Spawn(ParksWithLocal(&parked, DropFlag(&dropped))));
        rt->BlockOn([]() -> Task<void> { co_return; }());
    }

    ASSERT_TRUE(handle.Value().Finished());
    ASSERT_TRUE(handle.Value().Cancelled());
    internals::ReleaseRawTask(Unsafe("the simulated waker lets go"), parked);
}

} // namespace violet::experimental::coro
