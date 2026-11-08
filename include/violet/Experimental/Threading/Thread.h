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
// # 🌺💜 `violet/Experimental/Threading/Thread.h`

#pragma once

#include <violet/Experimental/Own.h>
#include <violet/Experimental/Threading/CancellationToken.h>
#include <violet/Experimental/Unique.h>
#include <violet/IO/Experimental/Error.h>

namespace violet::experimental::threading {

/// Abstract unit of work that can be executed on a [thread][violet::experimental::threading::Thread].
///
/// `Runnable` is a minimal interface for anything that should be handed off to a thread (or thread pool) and
/// run there. Implementations must override [`Runnable::Execute(CancellationToken)`] with the work to perform; the
/// thread that receives the `Runnable` calls [`Execute(CancellationToken)`] exactly once.
///
/// ## Usage
/// ```cpp
/// #include <violet/Experimental/Threading/Thread.h>
/// #include <violet/Print.h>
///
/// using namespace violet::experimental::threading;
///
/// struct PrintTask final: Runnable {
///     VIOLET_EXPLICIT PrintTask(Str message) : n_message(message) {}
///
///     void Execute(CancellationToken& token) override {
///         violet::Println("[thread {}]: {}", Thread::Current().Id(), this->n_message);
///     }
///
/// private:
///     String n_message;
/// };
///
/// Thread thread = Thread::Builder()
///     .WithName("my-thread")
///     .Spawn<PrintTask>();
///
/// // join the thread explicitly
/// thread.Join();
/// ```
struct NOELDOC_EXPERIMENTAL_SINCE("current") Runnable {
    virtual ~Runnable() = default;

    /// Creates a [`Runnable`] object from a function.
    /// @param fun the function to execute.
    template<typename F>
        requires((callable<F> || callable<F, const CancellationToken&>)
            && (callable_returns<F, void> || callable_returns<F, void, const CancellationToken&>))
    static auto From(F&& fun) noexcept -> ptr::Unique<Runnable>
    {
        struct FnRunnable final: Runnable {
            std::decay_t<F> Function;

            VIOLET_EXPLICIT FnRunnable(F fun) noexcept
                : Function(VIOLET_MOVE(fun))
            {
            }

        protected:
            void Execute([[maybe_unused]] const CancellationToken& token) override
            {
                if constexpr (callable<F, const CancellationToken&>) {
                    std::invoke(this->Function, token);
                } else {
                    std::invoke(this->Function);
                }
            }
        };

        return ptr::Unique<Runnable>::New<FnRunnable>(VIOLET_FWD(F, fun));
    }

    /// Calls the execution for this [`Runnable`] object with a dummy, non-reactive cancellation token.
    void Run()
    {
        this->Execute(CancellationToken::None());
    }

    /// Calls the execution for this [`Runnable`] object with a cancellation token.
    /// @param token the cancellation token used to cancel work, if necessary.
    void Run(const CancellationToken& token)
    {
        this->Execute(token);
    }

protected:
    /// The main function to implement that performs the work. This is invoked once on the executing thread.
    /// @param token the cancellation token used to cancel work, if necessary.
    virtual void Execute([[maybe_unused]] const CancellationToken& token) = 0;
};

/// A unique identifier for a running thread.
///
/// ## Remarks
/// Like C++'s [`std::thread::id`], the value of a **ThreadId** may be reused by the operating system once the thread
/// it identifies has terminated and been joined (or detached).
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") ThreadId final {
    VIOLET_IMPLICIT_COPY_AND_MOVE(ThreadId);
    VIOLET_IMPLICIT ThreadId() noexcept = default;
    ~ThreadId() = default;

    /// Returns the thread ID of the current thread.
    static auto Current() noexcept -> ThreadId;

    /// Returns the thread ID of the main ("root") thread.
    static auto Main() noexcept -> ThreadId;

    [[nodiscard]] auto ToString() const -> String;
    friend auto operator<<(std::ostream& os, const ThreadId& self) noexcept -> std::ostream&
    {
        os << self.ToString();
        return os;
    }

    friend constexpr auto operator==(const ThreadId&, const ThreadId&) noexcept -> bool = default;
    friend constexpr auto operator<=>(const ThreadId&, const ThreadId&) noexcept = default;

private:
    friend struct Thread;
    friend struct std::hash<ThreadId>;

    constexpr VIOLET_EXPLICIT ThreadId(uintptr_t id) noexcept
        : n_value(id)
    {
    }

    uintptr_t n_value = 0;
};

/// A handle to an operating system thread.
///
/// `Thread` is a move-only owning handle. A thread is created through [`Thread::Builder`], which configures its name
/// and stack size before it is spawned. A handle to an existing thread can be obtained with [`Thread::Current()`] or
/// [`Thread::Main()`].
///
/// ## Cancellation
/// Each thread owns a `CancellationTokenSource`. Cancellation is cooperative: [`Cancel()`] only signals the token, and
/// the thread's body must observe the `const CancellationToken&` it was given and return on its own. Bodies that
/// take no token cannot be cancelled this way.
///
/// ## Joining and detaching
/// A spawned thread starts out *joinable*. It stops being joinable once it has been joined with [`Join()`] /
/// [`CancelAndJoin()`], or released with [`Detach()`]. Handles obtained from [`Current()`] or [`Main()`] are never
/// joinable.
///
/// ## Example
/// ```cpp
/// auto worker = Thread::Builder()
///     .WithName("worker")
///     .Spawn([](const CancellationToken& token) {
///         while (!token.IsCancellationRequested()) {
///             // ... do work ...
///         }
///     });
///
/// if (worker.Ok()) {
///     worker->CancelAndJoin();
/// }
/// ```
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Thread final {
    struct Builder;

    VIOLET_DISALLOW_CONSTRUCTOR(Thread);
    VIOLET_DISALLOW_COPY(Thread);

    /// Takes ownership of the thread referenced by `other`.
    /// After the move, `other` no longer refers to a thread and is not joinable.
    VIOLET_IMPLICIT Thread(Thread&& other) noexcept;

    /// Takes ownership of the thread referenced by `other`, releasing the
    /// thread this handle previously referred to.
    ///
    /// After the move, `other` no longer refers to a thread and is not joinable.
    auto operator=(Thread&& other) noexcept -> Thread&;

    /// Destroys the handle.
    ///
    /// Destroying a handle does not stop the underlying thread. If the thread
    /// is still joinable, it should be joined or detached first.
    ~Thread();

    /// Returns a handle to the calling thread.
    ///
    /// The returned handle is not joinable; it is only useful for inspecting
    /// the thread, e.g. its [`Id()`] or [`Name()`].
    static auto Current() noexcept -> Thread;

    /// Returns a handle to the process's main thread.
    /// The returned handle is not joinable.
    static auto Main() noexcept -> Thread;

    /// Returns this thread's unique identifier.
    /// Identifiers are unique among threads that exist at the same time.
    [[nodiscard]]
    auto Id() const noexcept -> ThreadId;

    /// Returns the name given to this thread with [`Builder::WithName()`].
    /// Returns an empty string if the thread was spawned without a name.
    [[nodiscard]]
    auto Name() const noexcept -> Str;

    /// Returns the name the operating system reports for this thread.
    ///
    /// This can differ from [`Name()`]: some platforms limit name length
    /// (Linux allows 15 bytes plus the terminator) and will truncate it.
    [[nodiscard]]
    auto OSName() const noexcept -> Str;

    /// Returns `true` if this thread can be joined with [`Join()`].
    ///
    /// A thread is joinable from the moment it is spawned until it is joined,
    /// detached, or moved from.
    [[nodiscard]]
    auto Joinable() const noexcept -> bool;

    void Cancel();
    void CancelAndJoin();

    void Join();
    void Detach();

private:
    friend struct ThreadId;

    struct State final {
        ThreadId ID;
        Optional<String> Name;
        CancellationTokenSource Cts;
    };

    struct start;

    VIOLET_EXPLICIT Thread(Own<State> state) noexcept
        : Thread(VIOLET_MOVE(state), nullptr)
    {
    }

    VIOLET_EXPLICIT Thread(Own<State> state, void* native) noexcept
        : n_nativeThread(native)
        , n_state(VIOLET_MOVE(state))
    {
    }

    static auto trampoline(void* arg) noexcept -> void*;
    static auto getMainThreadState() noexcept -> const Own<State>&;

    static thread_local Own<State> tCurrent;
    static const bool s_mainThreadInitialized;

    bool n_joinable = false;
    void* n_nativeThread = nullptr;
    Own<State> n_state;
};

struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Thread::Builder final {
    VIOLET_DISALLOW_COPY_AND_MOVE(Builder);
    VIOLET_IMPLICIT Builder() noexcept = default;
    ~Builder() = default;

    auto WithName(Str name) & noexcept -> Builder&;
    auto WithName(Str name) && noexcept -> Builder&&;

    auto WithStackSize(UInt bytes) & noexcept -> Builder&;
    auto WithStackSize(UInt bytes) && noexcept -> Builder&&;

    template<std::derived_from<Runnable> R, typename... Args>
        requires std::constructible_from<R, Args...>
    auto Spawn(Args&&... args) -> io::experimental::Result<Thread>
    {
        return this->spawn(ptr::Unique<Runnable>::New<R, Args...>(VIOLET_FWD(Args, args)...));
    }

    template<typename Fun>
        requires((callable<Fun> || callable<Fun, const CancellationToken&>)
            && (callable_returns<Fun, void> || callable_returns<Fun, void, const CancellationToken&>))
    auto Spawn(Fun&& fun) -> io::experimental::Result<Thread>
    {
        return this->spawn(Runnable::From(VIOLET_FWD(Fun, fun)));
    }

private:
    auto spawn(ptr::Unique<Runnable> runnable) -> io::experimental::Result<Thread>;

    Optional<String> n_name;
    Optional<UInt> n_stackSize;
};

} // namespace violet::experimental::threading

VIOLET_FORMATTER(violet::experimental::threading::ThreadId);

template<>
struct std::hash<violet::experimental::threading::ThreadId> {
    auto operator()(const violet::experimental::threading::ThreadId& self) noexcept -> violet::UInt
    {
        constexpr std::hash<uintptr_t> hasher{};
        return hasher(self.n_value);
    }
};
