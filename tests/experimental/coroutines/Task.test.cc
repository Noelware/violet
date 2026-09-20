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
#include <violet/Experimental/Coroutines/Task.h>

#include <stdexcept>

namespace violet::experimental::coro {
namespace {

// The tests in here use a rudimentary executor; none of the established
// executors available are used in this test, they should be in their respective
// test files instead.

template<typename T>
struct SyncWaitTask final {
    struct promise_type final: task_internal::PromiseStorage<T> {
        auto get_return_object() noexcept -> SyncWaitTask
        {
            return SyncWaitTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    VIOLET_EXPLICIT SyncWaitTask(handle_type coro) noexcept
        : n_coro(coro)
    {
    }

    VIOLET_DISALLOW_COPY(SyncWaitTask);

    ~SyncWaitTask()
    {
        if (this->n_coro) {
            this->n_coro.destroy();
        }
    }

    auto Run() -> T
    {
        this->n_coro.resume();
        VIOLET_ASSERT(this->n_coro.done(), "task chain did not complete synchronusly");

        promise_type& promise = this->n_coro.promise();
        if constexpr (std::is_void_v<T>) {
            if (promise.Exception != nullptr) {
                std::rethrow_exception(VIOLET_MOVE(promise.Exception));
            }
        } else {
            return promise.Value.Match(
                // clang-format off
                [](T value) -> T { return VIOLET_MOVE(value); },
                [](std::exception_ptr ex) -> T {
                    std::rethrow_exception(VIOLET_MOVE(ex));
                    VIOLET_UNREACHABLE();
                }
                // clang-format on
            );
        }
    }

private:
    handle_type n_coro;
};

template<typename T>
auto BlockOn(Task<T> task) -> T
{
    SyncWaitTask<T> driver
        = [](Task<T> task) -> SyncWaitTask<T> { co_return co_await VIOLET_MOVE(task); }(VIOLET_MOVE(task));

    return driver.Run();
}

inline void BlockOn(Task<void> task)
{
    SyncWaitTask<void> driver = [](Task<void> task) -> SyncWaitTask<void> {
        co_await VIOLET_MOVE(task);
        co_return;
    }(VIOLET_MOVE(task));

    driver.Run();
}

auto ReturnsFortyTwo() -> Task<Int32>
{
    co_return 42;
}

#if VIOLET_FEATURE(EXCEPTIONS)
auto ThrowsInsideTask() -> Task<Int32>
{
    throw std::runtime_error("task go BOOM");
    co_return 0;
}
#endif

auto AddOne(Int32 value) -> Task<Int32>
{
    co_return value + 1;
}

auto ChainThreeTasks() -> Task<Int32>
{
    auto a = co_await AddOne(1);
    auto b = co_await AddOne(a);
    auto c = co_await AddOne(b);
    co_return c;
}

auto RunsVoidTask(bool* flag) -> Task<void>
{
    *flag = true;
    co_return;
}

} // namespace

TEST(Task, ReturnsValue)
{
    ASSERT_EQ(BlockOn(ReturnsFortyTwo()), 42);
}

#if VIOLET_FEATURE(EXCEPTIONS)
TEST(Task, PropagatesException)
{
    ASSERT_THROW(BlockOn(ThrowsInsideTask()), std::runtime_error);
}
#endif

TEST(Task, ChainsNestedAwaits)
{
    ASSERT_EQ(BlockOn(ChainThreeTasks()), 4);
}

TEST(Task, VoidTaskRunsBody)
{
    bool ran = false;
    BlockOn(RunsVoidTask(&ran));
    ASSERT_TRUE(ran);
}

} // namespace violet::experimental::coro
