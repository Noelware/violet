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
#include <violet/IO/Experimental/Descriptor.h>

#if VIOLET_PLATFORM(UNIX)
#include <fcntl.h>
#endif

namespace violet::io::experimental {

static_assert(AsFd<Descriptor>, "`Descriptor` should hand out a borrowed descriptor");
static_assert(AsFd<Descriptor::Borrowed>, "`Descriptor::Borrowed` should hand out a borrowed descriptor");
static_assert(!std::is_copy_constructible_v<Descriptor>, "an owning descriptor must not be copyable");
static_assert(std::is_move_constructible_v<Descriptor>, "an owning descriptor must be movable");
static_assert(std::is_copy_constructible_v<Descriptor::Borrowed>, "a borrowed descriptor must be copyable");

namespace {

auto toBytes(Str value) -> Span<const UInt8>
{
    return {reinterpret_cast<const UInt8*>(value.data()), value.size()};
}

#if VIOLET_PLATFORM(UNIX)
auto isStillOpen(RawFd fd) -> bool
{
    return ::fcntl(fd, F_GETFD) != -1;
}
#endif

} // namespace

TEST(Descriptor, DefaultConstructedIsInvalid)
{
    const Descriptor descriptor;

    EXPECT_EQ(descriptor.Get(), kInvalidRawFd);
    EXPECT_FALSE(descriptor.Valid());
    EXPECT_FALSE(static_cast<bool>(descriptor));
    EXPECT_EQ(descriptor.ToString(), "fd(invalid)");
}

TEST(Descriptor, BorrowOfAnInvalidDescriptorIsAlsoInvalid)
{
    const Descriptor descriptor;
    const auto borrowed = descriptor.Borrow();

    EXPECT_EQ(borrowed.Get(), kInvalidRawFd);
    EXPECT_FALSE(borrowed.Valid());
    EXPECT_EQ(borrowed.ToString(), "fd(invalid)");
}

TEST(Descriptor, ReadOnAnInvalidDescriptorYieldsZeroBytes)
{
    const Descriptor descriptor;

    std::array<UInt8, 8> buffer{};
    auto read = descriptor.Read(Span<UInt8>(buffer));

    ASSERT_TRUE(read) << read.Error();
    EXPECT_EQ(read.Value(), 0);
}

TEST(Descriptor, WriteOnAnInvalidDescriptorIsRejected)
{
    const Descriptor descriptor;
    auto written = descriptor.Write(toBytes("violet"));

    ASSERT_FALSE(written) << "writing to an invalid descriptor must not succeed";
    EXPECT_EQ(written.Error().Kind(), ErrorKind::InvalidInput);
}

TEST(Descriptor, ClosingAnInvalidDescriptorIsANoOp)
{
    Descriptor descriptor;

    auto closed = descriptor.Close();
    EXPECT_TRUE(closed) << closed.Error();
    EXPECT_FALSE(descriptor.Valid());
}

#if VIOLET_PLATFORM(UNIX)
namespace {

struct DescriptorPipe: public ::testing::Test {
    Descriptor Reader;
    Descriptor Writer;

protected:
    void SetUp() override
    {
        Int32 fds[2] = {-1, -1};
        ASSERT_EQ(::pipe(fds), 0) << "failed to create a pipe: " << ::strerror(errno);

        this->Reader = Descriptor::FromRaw(fds[0]);
        this->Writer = Descriptor::FromRaw(fds[1]);
    }
};

TEST_F(DescriptorPipe, FromRawTakesOwnershipOfALiveHandle)
{
    EXPECT_TRUE(this->Reader.Valid());
    EXPECT_TRUE(static_cast<bool>(this->Writer));
    EXPECT_NE(this->Reader.Get(), kInvalidRawFd);
    EXPECT_NE(this->Reader.Get(), this->Writer.Get());
    EXPECT_TRUE(isStillOpen(this->Reader.Get()));
}

TEST_F(DescriptorPipe, ToStringPrintsTheUnderlyingNumber)
{
    EXPECT_EQ(this->Reader.ToString(), std::format("fd({})", this->Reader.Get()));

    std::ostringstream os;
    os << this->Reader;
    EXPECT_EQ(os.str(), this->Reader.ToString());
}

TEST_F(DescriptorPipe, RoundTripsBytesThroughTheKernel)
{
    constexpr Str kPayload = "violet is a flower";

    auto written = this->Writer.Write(toBytes(kPayload));
    ASSERT_TRUE(written) << written.Error();
    EXPECT_EQ(written.Value(), kPayload.size());

    Array<UInt8, 64> buffer{};
    auto read = this->Reader.Read(Span<UInt8>(buffer));
    ASSERT_TRUE(read) << read.Error();
    ASSERT_EQ(read.Value(), kPayload.size());

    EXPECT_EQ(String(buffer.data(), buffer.data() + read.Value()), kPayload);
}

TEST_F(DescriptorPipe, ReadingIntoAnEmptyBufferDoesNotBlock)
{
    auto read = this->Reader.Read(Span<UInt8>());

    ASSERT_TRUE(read) << read.Error();
    EXPECT_EQ(read.Value(), 0);
}

TEST_F(DescriptorPipe, FlushIgnoresDescriptorsThatCannotBeSynced)
{
    // `fsync(2)` answers `EINVAL` for a pipe, which the implementation folds into a success.
    auto flushed = this->Writer.Flush();
    EXPECT_TRUE(flushed) << flushed.Error();
}

TEST_F(DescriptorPipe, WritingToAClosedReadEndIsAnError)
{
    // ignore SIGPIPE so the write reports `EPIPE` instead of killing the test binary.
    ::signal(SIGPIPE, SIG_IGN);

    ASSERT_TRUE(this->Reader.Close());

    auto written = this->Writer.Write(toBytes("nobody is listening"));
    ASSERT_FALSE(written) << "writing into a pipe with no reader must fail";
    EXPECT_EQ(written.Error().Kind(), ErrorKind::BrokenPipe);
}

TEST_F(DescriptorPipe, BorrowSharesTheHandleWithoutOwningIt)
{
    const auto borrowed = this->Writer.Borrow();
    EXPECT_EQ(borrowed.Get(), this->Writer.Get());
    EXPECT_TRUE(borrowed.Valid());

    // a borrow of a borrow is just a copy.
    EXPECT_EQ(borrowed.Borrow(), borrowed);
    EXPECT_EQ(borrowed, this->Writer.Get());

    auto written = borrowed.Write(toBytes("hi"));
    ASSERT_TRUE(written) << written.Error();
    EXPECT_EQ(written.Value(), 2);

    // dropping the borrow must leave the owner's handle alone.
    {
#if VIOLET_COMPILER(GCC) || VIOLET_COMPILER(CLANG)
        VIOLET_DIAGNOSTIC_PUSH
        VIOLET_DIAGNOSTIC_IGNORE("-Wunused-variable")
#endif

        const auto _scoped = this->Writer.Borrow();

#if VIOLET_COMPILER(GCC) || VIOLET_COMPILER(CLANG)
        VIOLET_DIAGNOSTIC_POP
#endif
    }

    EXPECT_TRUE(isStillOpen(this->Writer.Get()));
}

TEST_F(DescriptorPipe, ImplicitlyConvertsToABorrow)
{
    const auto take = [](Descriptor::Borrowed fd) -> Descriptor::value_type { return fd.Get(); };
    EXPECT_EQ(take(this->Reader), this->Reader.Get());
}

TEST_F(DescriptorPipe, MoveConstructionLeavesTheSourceEmpty)
{
    const auto raw = this->Reader.Get();

    Descriptor moved(VIOLET_MOVE(this->Reader));
    EXPECT_EQ(moved.Get(), raw);
    EXPECT_FALSE(this->Reader.Valid());
    EXPECT_EQ(this->Reader.Get(), kInvalidRawFd);
    EXPECT_TRUE(isStillOpen(raw)) << "moving must not close the handle";
}

TEST_F(DescriptorPipe, MoveAssignmentClosesTheOverwrittenHandle)
{
    const auto overwritten = this->Reader.Get();
    const auto surviving = this->Writer.Get();

    this->Reader = VIOLET_MOVE(this->Writer);

    EXPECT_EQ(this->Reader.Get(), surviving);
    EXPECT_FALSE(this->Writer.Valid());
    EXPECT_FALSE(isStillOpen(overwritten)) << "the replaced handle should have been closed";
    EXPECT_TRUE(isStillOpen(surviving));
}

TEST_F(DescriptorPipe, SelfMoveAssignmentKeepsTheHandle)
{
    const auto raw = this->Reader.Get();
    auto& alias = this->Reader;

    this->Reader = VIOLET_MOVE(alias);

    EXPECT_EQ(this->Reader.Get(), raw);
    EXPECT_TRUE(isStillOpen(raw));
}

TEST_F(DescriptorPipe, ResetClosesThePreviousHandle)
{
    const auto raw = this->Reader.Get();

    this->Reader.Reset();
    EXPECT_FALSE(this->Reader.Valid());
    EXPECT_FALSE(isStillOpen(raw));

    const auto adopted = this->Writer.IntoRawFd();
    this->Reader.Reset(adopted);
    EXPECT_EQ(this->Reader.Get(), adopted);
    EXPECT_TRUE(isStillOpen(adopted));
}

TEST_F(DescriptorPipe, IntoRawFdRelinquishesWithoutClosing)
{
    const auto raw = this->Reader.IntoRawFd();

    EXPECT_FALSE(this->Reader.Valid());
    EXPECT_EQ(this->Reader.Get(), kInvalidRawFd);
    ASSERT_TRUE(isStillOpen(raw)) << "`IntoRawFd` must hand the handle to the caller, not close it";

    EXPECT_EQ(::close(raw), 0);
}

TEST_F(DescriptorPipe, CloseReportsSuccessAndIsIdempotent)
{
    const auto raw = this->Reader.Get();

    auto closed = this->Reader.Close();
    EXPECT_TRUE(closed) << closed.Error();
    EXPECT_FALSE(this->Reader.Valid());
    EXPECT_FALSE(isStillOpen(raw));

    // a second close must not touch (and possibly reuse) the handle again.
    EXPECT_TRUE(this->Reader.Close());
}

TEST_F(DescriptorPipe, DestructorClosesTheHandle)
{
    const auto raw = this->Writer.IntoRawFd();
    {
        auto owned = Descriptor::FromRaw(raw);
        EXPECT_TRUE(owned.Valid());
    }

    EXPECT_FALSE(isStillOpen(raw)) << "the destructor should have closed the handle";
}

TEST(Descriptor, BorrowedComparesByRawValue)
{
    const Descriptor::Borrowed lhs(3);
    const Descriptor::Borrowed rhs(3);
    const Descriptor::Borrowed other(4);

    EXPECT_EQ(lhs, rhs);
    EXPECT_EQ(lhs, RawFd{3});
    EXPECT_FALSE(lhs == other);
}

} // namespace
#endif

} // namespace violet::io::experimental
