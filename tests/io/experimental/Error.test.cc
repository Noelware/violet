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
#include <violet/IO/Experimental/Error.h>

namespace violet::io::experimental {
namespace {

auto contains(Str haystack, Str needle) noexcept -> bool
{
#if VIOLET_REQUIRE_STL(202302L)
    return haystack.contains(needle);
#else
    return haystack.find(needle) != Str::npos; // NOLINT(readability-container-contains)
#endif
}

} // namespace

TEST(ErrorKind, EveryKindHasANonEmptyMessage)
{
    for (UInt8 i = 0; i < error_internal::kErrorKindCount; i++) {
        const auto message = violet::ToString(static_cast<ErrorKind>(i));
        EXPECT_FALSE(message.empty()) << "ErrorKind(" << static_cast<UInt32>(i) << ") has an empty message";
    }
}

TEST(ErrorKind, MessagesAreDistinct)
{
    std::set<String> seen;
    for (UInt8 i = 0; i < error_internal::kErrorKindCount; i++) {
        const auto message = violet::ToString(static_cast<ErrorKind>(i));
        EXPECT_TRUE(seen.insert(message).second) << "duplicated message: " << message;
    }

    EXPECT_EQ(seen.size(), error_internal::kErrorKindCount);
}

TEST(ErrorKind, MessagesAreOrderedWithTheEnumerators)
{
    // ensures that the messages are the same from the `X`-macro
    EXPECT_EQ(violet::ToString(ErrorKind::NotFound), "entity not found");
    EXPECT_EQ(violet::ToString(ErrorKind::PermissionDenied), "permission denied");
    EXPECT_EQ(violet::ToString(ErrorKind::InvalidInput), "invalid input parameter");
    EXPECT_EQ(violet::ToString(ErrorKind::InProgress), "in progress");

    EXPECT_EQ(static_cast<UInt8>(ErrorKind::NotFound), 0);
    EXPECT_EQ(static_cast<UInt8>(ErrorKind::InProgress), error_internal::kErrorKindCount - 1);
}

TEST(Error, KindOnlyCarriesNoPayload)
{
    const Error error(ErrorKind::NotFound);

    EXPECT_EQ(error.Kind(), ErrorKind::NotFound);
    EXPECT_FALSE(error.RawOSError()) << "a bare kind must not report an OS error code";

    const auto text = error.ToString();
    EXPECT_TRUE(contains(text, "entity not found")) << text;
    EXPECT_TRUE(contains(text, "Error.test.cc")) << text;
}

TEST(Error, SimpleMessageKeepsBothKindAndMessage)
{
    const Error error(ErrorKind::InvalidData, "malformed header");

    EXPECT_EQ(error.Kind(), ErrorKind::InvalidData);
    EXPECT_FALSE(error.RawOSError());

    const auto text = error.ToString();
    EXPECT_TRUE(contains(text, "invalid data")) << text;
    EXPECT_TRUE(contains(text, "malformed header")) << text;
}

TEST(Error, CapturesTheCallSite)
{
    const auto line = static_cast<UInt32>(__LINE__) + 1;
    const Error error(ErrorKind::TimedOut);

    EXPECT_EQ(error.Location().Line, line);
    EXPECT_TRUE(contains(String(error.Location().File), "Error.test.cc")) << error.Location().File;
}

TEST(Error, ExplicitLocationIsNotOverwritten)
{
    const SourceLocation loc("Violet.cc", 42, 7, "someFunction");
    const Error error(ErrorKind::Unsupported, "nope", loc);

    EXPECT_EQ(error.Location().File, "Violet.cc");
    EXPECT_EQ(error.Location().Line, 42);
    EXPECT_EQ(error.Location().Column, 7);

    const auto text = error.ToString();
    EXPECT_TRUE(contains(text, "Violet.cc:42:7")) << text;
}

TEST(Error, ToStringMatchesTheStreamOperator)
{
    const Error error(ErrorKind::BrokenPipe, "the other end hung up");

    std::ostringstream os;
    os << error;

    EXPECT_EQ(os.str(), error.ToString());
}

TEST(Error, IsCopyableAndMovable)
{
    const Error error(ErrorKind::AlreadyExists, "already there");

    const Error copied = error; // NOLINT(performance-unnecessary-copy-initialization)
    EXPECT_EQ(copied.Kind(), ErrorKind::AlreadyExists);
    EXPECT_EQ(copied.Location().Line, error.Location().Line);
    EXPECT_EQ(copied.ToString(), error.ToString());

    Error source(ErrorKind::WriteZero);
    const Error moved = VIOLET_MOVE(source);
    EXPECT_EQ(moved.Kind(), ErrorKind::WriteZero);
}

TEST(Error, FitsInTheResultAlias)
{
    const auto failing = []() -> Result<UInt> { return Err(VIOLET_IO_ERROR(StorageFull, "disk is full")); };

    auto result = failing();
    ASSERT_FALSE(result);
    EXPECT_EQ(result.Error().Kind(), ErrorKind::StorageFull);
    EXPECT_TRUE(contains(result.Error().ToString(), "disk is full")) << result.Error();

    const auto succeeding = []() -> Result<UInt> { return 12; };
    auto ok = succeeding();
    ASSERT_TRUE(ok) << ok.Error();
    EXPECT_EQ(ok.Value(), 12);
}

#if VIOLET_PLATFORM(UNIX)

TEST(PlatformError, MapsWellKnownErrnosToKinds)
{
    struct {
        NativeErrorCode Code;
        ErrorKind Kind;
    } const cases[] = {
        {.Code = ENOENT, .Kind = ErrorKind::NotFound},
        {.Code = EACCES, .Kind = ErrorKind::PermissionDenied},
        {.Code = EPERM, .Kind = ErrorKind::PermissionDenied},
        {.Code = EEXIST, .Kind = ErrorKind::AlreadyExists},
        {.Code = EINVAL, .Kind = ErrorKind::InvalidInput},
        {.Code = EPIPE, .Kind = ErrorKind::BrokenPipe},
        {.Code = EISDIR, .Kind = ErrorKind::IsADirectory},
        {.Code = ENOTDIR, .Kind = ErrorKind::NotADirectory},
        {.Code = EINTR, .Kind = ErrorKind::Interrupted},
        {.Code = EAGAIN, .Kind = ErrorKind::WouldBlock},
        {.Code = ENOSYS, .Kind = ErrorKind::Unsupported},
        {.Code = EXDEV, .Kind = ErrorKind::CrossesDevices},
    };

    for (const auto& [code, kind]: cases) {
        const auto error = Error::FromOSError(code);

        EXPECT_EQ(error.Kind(), kind) << "errno " << code << " mapped to the wrong kind";
        ASSERT_TRUE(error.RawOSError()) << "errno " << code << " lost its raw code";
        EXPECT_EQ(error.RawOSError().Value(), code);
    }
}

TEST(PlatformError, OSErrorReadsTheAmbientErrno)
{
    errno = EPIPE;
    const auto error = Error::OSError();

    EXPECT_EQ(error.Kind(), ErrorKind::BrokenPipe);
    ASSERT_TRUE(error.RawOSError());
    EXPECT_EQ(error.RawOSError().Value(), EPIPE);

    errno = 0;
}

TEST(PlatformError, ToStringIncludesTheRawCodeAndTheSystemMessage)
{
    const auto error = Error::FromOSError(ENOENT);
    const auto text = error.ToString();

    EXPECT_TRUE(contains(text, "system error")) << text;
    EXPECT_TRUE(contains(text, std::to_string(ENOENT))) << text;
    EXPECT_TRUE(contains(text, ::strerror(ENOENT))) << text;
    EXPECT_TRUE(contains(text, "Error.test.cc")) << text;
}

TEST(PlatformError, ComparesAgainstItselfAndRawCodes)
{
    const auto lhs = Error::FromOSError(ENOENT);
    const auto rhs = Error::FromOSError(ENOENT);
    const auto other = Error::FromOSError(EACCES);

    ASSERT_TRUE(lhs.RawOSError());
    ASSERT_TRUE(rhs.RawOSError());
    ASSERT_TRUE(other.RawOSError());

    EXPECT_EQ(lhs.RawOSError().Value(), rhs.RawOSError().Value());
    EXPECT_NE(lhs.RawOSError().Value(), other.RawOSError().Value());
}

#endif

} // namespace violet::io::experimental
