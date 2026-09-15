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

#pragma once

#include <violet/IO/Experimental/Error.h>

namespace violet::io::experimental {

#if VIOLET_PLATFORM(UNIX)
/// This platform's raw file descriptor type.
/// @since current
using RawFd = Int32;
#elif VIOLET_PLATFORM(WINDOWS)
/// This platform's raw file descriptor type.
/// @since current
using RawFd = void*;
#else
/// This platform's raw file descriptor type.
using RawFd = void*;
#endif

#if VIOLET_PLATFORM(UNIX)
/// A sentinel value meaning "this descriptor doesn't refer to anything"
NOELDOC_EXPERIMENTAL_SINCE("current")
constexpr inline RawFd kInvalidRawFd = -1;
#elif VIOLET_PLATFORM(WINDOWS)
/// A sentinel value meaning "this descriptor doesn't refer to anything"
NOELDOC_EXPERIMENTAL_SINCE("current")
inline const RawFd kInvalidRawFd = reintepret_cast<RawFd>(static_cast<std::intptr_t>(-1));
#else
NOELDOC_EXPERIMENTAL_SINCE("current")
inline const RawFd kInvalidRawFd = nullptr;
#endif

namespace fd_internal {

VIOLET_API auto isFdValid(RawFd fd) noexcept -> bool;
VIOLET_API auto read(RawFd fd, Span<UInt8> buf) noexcept -> Result<UInt>;
VIOLET_API auto write(RawFd fd, Span<const UInt8> buf) noexcept -> Result<UInt>;
VIOLET_API auto flush(RawFd fd) noexcept -> Result<void>;
VIOLET_API auto close(RawFd fd) noexcept -> Result<void>;
VIOLET_API auto toString(RawFd fd) noexcept -> String;

template<typename Self>
struct Ops {
    /// Returns **true** if this descriptor points to a valid descriptor.
    [[nodiscard]] auto Valid() const noexcept -> bool
    {
        return isFdValid(this->asRawFd());
    }

    NOELDOC_SEE("violet::io::Readable")
    [[nodiscard]] auto Read(Span<UInt8> buf) const noexcept -> Result<UInt>
    {
        return read(this->asRawFd(), buf);
    }

    NOELDOC_SEE("violet::io::Writable")
    [[nodiscard]] auto Write(Span<const UInt8> buf) const noexcept -> Result<UInt>
    {
        return write(this->asRawFd(), buf);
    }

    NOELDOC_SEE("violet::io::Writable")
    [[nodiscard]] auto Flush() const noexcept -> Result<void>
    {
        return flush(this->asRawFd());
    }

    /// Returns a text representation of this descriptor.
    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return toString(this->asRawFd());
    }

    VIOLET_EXPLICIT operator bool() const noexcept
    {
        return this->Valid();
    }

    VIOLET_EXPLICIT operator RawFd() const noexcept
    {
        return this->asRawFd();
    }

    friend auto operator==(const Self& lhs, const Self& rhs) noexcept -> bool
    {
        return lhs.Get() == rhs.Get();
    }

    friend auto operator==(const Self& lhs, RawFd rhs) noexcept -> bool
    {
        return lhs.Get() == rhs;
    }

    friend auto operator<<(std::ostream& os, const Self& self) -> std::ostream&
    {
        return os << self.ToString();
    }

private:
    [[nodiscard]] auto asRawFd() const noexcept -> RawFd
    {
        return static_cast<const Self&>(*this).Get();
    }
};

} // namespace fd_internal

/// A zero-cost, tiny abstraction around OS-related file descriptors that owns
/// the handle and closes it on destruction.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Descriptor final: public fd_internal::Ops<Descriptor> {
    /// A non-owning view of a descriptor. Pass this by value wherever a function needs to
    /// read from or write to a descriptor it doesn't own.
    struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Borrowed final: public fd_internal::Ops<Borrowed> {
        using value_type = RawFd;

        VIOLET_IMPLICIT_COPY_AND_MOVE(Borrowed);

        constexpr VIOLET_IMPLICIT Borrowed() noexcept = default;

        /// Wraps a raw descriptor without taking full ownership of it.
        constexpr VIOLET_EXPLICIT Borrowed(value_type value) noexcept
            : n_fd(value)
        {
        }

        ~Borrowed() = default;

        /// Returns the raw value type of this descriptor.
        [[nodiscard]] auto Get() const noexcept -> value_type
        {
            return this->n_fd;
        }

        /// A borrow of a borrow is just copied. This allows us to pass in
        /// `Descriptor::Borrowed` values into any I/O code that might
        /// constrain [`AsFd`].
        [[nodiscard]] auto Borrow() const noexcept -> Borrowed
        {
            return *this;
        }

    private:
        value_type n_fd = kInvalidRawFd;
    };

    using value_type = RawFd;

    VIOLET_IMPLICIT Descriptor() noexcept = default;
    VIOLET_DISALLOW_COPY(Descriptor);

    /// This will clean up the resource that this file descriptor owns.
    ///
    /// This will ignore any errors that arise when closing the file. Call
    /// [`Descriptor::Close()`] yourself to observe.
    ~Descriptor();

    VIOLET_IMPLICIT Descriptor(Descriptor&& other) noexcept
        : n_fd(std::exchange(other.n_fd, kInvalidRawFd))
    {
    }

    auto operator=(Descriptor&& other) noexcept -> Descriptor&
    {
        if (this != &other) {
            this->Reset(std::exchange(other.n_fd, kInvalidRawFd));
        }

        return *this;
    }

    /// Takes ownership of a file descriptor and returns this structure.
    [[nodiscard("owned fds can be dropped")]] static auto FromRaw(value_type value) noexcept -> Descriptor
    {
        return Descriptor(value);
    }

    /// Returns the raw value type of this file descriptor.
    [[nodiscard]] auto Get() const noexcept -> value_type
    {
        return this->n_fd;
    }

    /// Returns a non-owning file descriptor. The borrowed fd must not outlive `*this`.
    [[nodiscard]] auto Borrow() const noexcept -> Borrowed
    {
        return Borrowed(this->n_fd);
    }

    /// Closes the file descriptor and reports what the platform said. Unlike the destructor
    /// for this structure, this will return the error if any were given.
    auto Close() noexcept -> Result<void>;

    /// Closes the current handle (if any) and adopts `fd` instead.
    void Reset(value_type fd = kInvalidRawFd) noexcept;

    /// Relinquishes ownership without closing, returning the raw handle. The
    /// caller becomes responsible for it.
    auto IntoRawFd() noexcept -> value_type
    {
        return std::exchange(this->n_fd, kInvalidRawFd);
    }

    VIOLET_IMPLICIT operator Borrowed() const noexcept
    {
        return this->Borrow();
    }

private:
    VIOLET_EXPLICIT Descriptor(value_type value) noexcept
        : n_fd(value)
    {
    }

    value_type n_fd = kInvalidRawFd;
};

/// Anything that can hand out a borrowed descriptor. This is analogous
/// to Rust's [`std::os::fd::AsFd`] trait.
///
/// [`std::os::fd::AsFd`]: https://doc.rust-lang.org/stable/std/os/fd/trait.AsFd.html
///
/// When building generic I/O code that is cross-platform, this is the most
/// preferable over using `Descriptor`/`Descriptor::Borrowed`. An example
/// of `File::Read` could look something like:
///
/// ```cpp
/// template<AsFd Fd>
/// auto Read(Fd desc, Span<UInt8> buf) -> io::Result<UInt> {
///     auto fd = desc.Borrow(); // => violet::io::experimental::Descriptor::Borrowed
///     /* use `fd` as a borrowed file descriptor */
///
///     return bytesRead;
/// }
/// ```
///
/// @since current
template<typename T>
concept AsFd = requires(const T& value) {
    { value.Borrow() } noexcept -> std::same_as<Descriptor::Borrowed>;
};

// static_assert(Readable<violet::io::experimental::Descriptor>,
//     "`violet::io::experimental::Descriptor` doesn't conform to the `violet::io::Readable` concept");

// static_assert(Writable<violet::io::experimental::Descriptor>,
//     "`violet::io::experimental::Descriptor` doesn't conform to the `violet::io::Writable` concept");

// static_assert(AsFd<violet::io::experimental::Descriptor>,
//     "`violet::io::experimental::Descriptor` doesn't conform to the `violet::io::experimental::AsFd` concept");

// static_assert(Readable<violet::io::experimental::Descriptor::Borrowed>,
//     "`violet::io::experimental::Descriptor::Borrowed` doesn't conform to the `violet::io::Readable` concept");

// static_assert(Writable<violet::io::experimental::Descriptor::Borrowed>,
//     "`violet::io::experimental::Descriptor::Borrowed` doesn't conform to the `violet::io::Writable` concept");

static_assert(AsFd<violet::io::experimental::Descriptor::Borrowed>,
    "`violet::io::experimental::Descriptor::Borrowed` doesn't conform to the `violet::io::experimental::AsFd` concept");

} // namespace violet::io::experimental

VIOLET_FORMATTER(violet::io::experimental::Descriptor);
VIOLET_FORMATTER(violet::io::experimental::Descriptor::Borrowed);
