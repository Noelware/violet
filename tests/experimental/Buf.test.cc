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
#include <violet/Experimental/Buf.h>

#include <thread>

namespace violet::experimental {
namespace {

constexpr Buf kHttpOK = Buf::FromStatic("HTTP/1.1 200 OK");
static_assert(kHttpOK.Size() == 15);
static_assert(kHttpOK.AsStr() == "HTTP/1.1 200 OK");
static_assert(kHttpOK.Slice(9, 12) == "200");
static_assert(Buf{}.Empty());
static_assert(Buf{} == Buf::FromStatic(""));
static_assert([] -> bool {
    Buf buf = Buf::FromStatic("abcdef");
    Buf copy = buf;
    Buf moved = VIOLET_MOVE(copy);

    return moved == "abcdef" && copy.Empty();
}());

// Counts destructions of an owner, to prove `FromOwner` frees exactly once.
struct CountedOwner final {
    String Data;
    std::atomic<Int32>* Drops;

    CountedOwner(std::string data, std::atomic<Int32>* drops)
        : Data(VIOLET_MOVE(data))
        , Drops(drops)
    {
    }

    CountedOwner(CountedOwner&& other) noexcept
        : Data(VIOLET_MOVE(other.Data))
        , Drops(std::exchange(other.Drops, nullptr))
    {
    }

    ~CountedOwner()
    {
        if (this->Drops != nullptr) {
            this->Drops->fetch_add(1);
        }
    }

    // `byte_owner` needs a contiguous, sized range of bytes.
    [[nodiscard]]
    auto begin() const noexcept
    {
        return this->Data.begin();
    }

    [[nodiscard]]
    auto end() const noexcept
    {
        return this->Data.end();
    }

    [[nodiscard]] auto data() const noexcept -> decltype(this->Data.data())
    {
        return this->Data.data();
    }

    [[nodiscard]] auto size() const noexcept -> decltype(this->Data.size())
    {
        return this->Data.size();
    }
};

} // namespace

TEST(Buf, DefaultIsEmpty)
{
    Buf buf;
    ASSERT_TRUE(buf.Empty());
    ASSERT_EQ(buf.Size(), 0U);
    ASSERT_TRUE(buf.AsSpan().empty());
    ASSERT_FALSE(buf.Unique());
}

TEST(Buf, FromStaticDoesNotIncludeTerminator)
{
    ASSERT_EQ(Buf::FromStatic("hello").Size(), 5U);
}

TEST(Buf, CopyFromOwnsItsBytes)
{
    std::string source = "hello";
    Buf buf = Buf::CopyFrom(source);
    source[0] = 'J';

    ASSERT_EQ(buf, "hello");
    ASSERT_TRUE(buf.Unique());
}

TEST(Buf, CopyFromEmptyDoesNotAllocate)
{
    Buf buf = Buf::CopyFrom(Str{});
    ASSERT_TRUE(buf.Empty());
    ASSERT_FALSE(buf.Unique());
}

TEST(Buf, CopiesShareStorage)
{
    Buf a = Buf::CopyFrom("shared");
    ASSERT_TRUE(a.Unique());

    {
        Buf b = a;
        ASSERT_FALSE(a.Unique());
        ASSERT_EQ(a.AsStr(), b.AsStr());
    }

    ASSERT_TRUE(a.Unique());
}

TEST(Buf, SliceOutlivesOriginal)
{
    Buf slice;
    {
        Buf original = Buf::CopyFrom("HTTP/1.1 200 OK");
        slice = original.Slice(9, 12);
    }

    ASSERT_EQ(slice, "200");
    ASSERT_TRUE(slice.Unique());
}

TEST(Buf, EmptySliceDoesNotPinStorage)
{
    Buf original = Buf::CopyFrom("data");
    Buf empty = original.Slice(2, 2);

    ASSERT_TRUE(empty.Empty());
    ASSERT_TRUE(original.Unique());
}

TEST(Buf, SplitToAndSplitOff)
{
    Buf buf = Buf::CopyFrom("head:body:tail");
    Buf head = buf.SplitTo(5);
    Buf tail = buf.SplitOff(4);

    ASSERT_EQ(head, "head:");
    ASSERT_EQ(buf, "body");
    ASSERT_EQ(tail, ":tail");
}

TEST(Buf, AdvanceAndTruncate)
{
    Buf buf = Buf::FromStatic("0123456789");
    buf.Advance(2);
    buf.Truncate(5);
    ASSERT_EQ(buf, "23456");

    buf.Truncate(100);
    ASSERT_EQ(buf, "23456");
}

TEST(Buf, CopyAssignFromOwnSlice)
{
    Buf buf = Buf::CopyFrom("abcdef");
    Buf slice = buf.Slice(1, 3);
    buf = slice;

    ASSERT_EQ(buf, "bc");
}

TEST(Buf, MoveLeavesSourceEmpty)
{
    Buf a = Buf::CopyFrom("moved");
    Buf b = VIOLET_MOVE(a);

    ASSERT_EQ(b, "moved");
    ASSERT_TRUE(a.Empty());
}

TEST(Buf, FromOwnerVectorIsZeroCopy)
{
    Vec<char> data = {'a', 'b', 'c'};
    const char* original = data.data();

    Buf buf = Buf::FromOwner(VIOLET_MOVE(data));
    ASSERT_EQ(buf, "abc");
    ASSERT_EQ(buf.AsStr().data(), original);
}

TEST(Buf, FromOwnerSmallStringReadsPointerAfterMove)
{
    Buf buf = Buf::FromOwner<String>("hi");
    ASSERT_EQ(buf, "hi");
}

TEST(Buf, FromOwnerDropsOwnerExactlyOnce)
{
    std::atomic<Int32> drops{0};
    {
        Buf a = Buf::FromOwner(CountedOwner("owned", &drops));
        Buf b = a.Slice(1, 4);
        a.Clear();
        ASSERT_EQ(drops.load(), 0);
        ASSERT_EQ(b, "wne");
    }

    ASSERT_EQ(drops.load(), 1);
}

TEST(Buf, TryIntoMutRequiresUnique)
{
    Buf buf = Buf::CopyFrom("reuse");
    Buf other = buf;

    Buf copy = buf;
    ASSERT_FALSE(VIOLET_MOVE(copy).TryIntoMut());

    other.Clear();
    copy.Clear();
    Optional<Bytes> mut = VIOLET_MOVE(buf).TryIntoMut();
    ASSERT_TRUE(mut);
    ASSERT_EQ(mut.Value().Size(), 5U);
}

TEST(Buf, TryIntoMutRefusesStaticAndOwner)
{
    ASSERT_FALSE(Buf::FromStatic("static").TryIntoMut());
    ASSERT_FALSE(Buf::FromOwner(std::string("owner")).TryIntoMut());
}

TEST(Buf, HashMatchesContent)
{
    std::hash<Buf> hash;
    ASSERT_EQ(hash(Buf::CopyFrom("key")), hash(Buf::FromStatic("key")));
}

TEST(Buf, CrossThreadClonesFreeOnce)
{
    std::atomic<Int32> drops{0};
    Buf shared = Buf::FromOwner(CountedOwner("payload", &drops));

    Vec<std::thread> threads;
    for (Int32 t = 0; t < 8; t++) {
        threads.emplace_back([shared] {
            for (Int32 i = 0; i < 100'000; i++) {
                Buf local = shared;
                static_cast<void>(local.Size());
            }
        });
    }

    for (auto& thread: threads) {
        thread.join();
    }

    shared.Clear();
    ASSERT_EQ(drops.load(), 1);
}

TEST(Bytes, DefaultDoesNotAllocate)
{
    Bytes bytes;
    ASSERT_EQ(bytes.Capacity(), 0U);
    ASSERT_TRUE(bytes.Empty());
}

TEST(Bytes, ExtendGrows)
{
    Bytes bytes(4);
    bytes.Extend("hello, world");

    ASSERT_EQ(bytes.Size(), 12U);
    ASSERT_GE(bytes.Capacity(), 12U);
    ASSERT_EQ(VIOLET_MOVE(bytes).Freeze(), "hello, world");
}

TEST(Bytes, AssumeInitCommitsSpare)
{
    Bytes bytes(8);
    std::memcpy(bytes.SpareData(), "abc", 3);
    bytes.AssumeInit(Unsafe("we wrote three bytes into the spare tail"), 3);

    ASSERT_EQ(bytes.Size(), 3U);
    ASSERT_EQ(bytes.Spare(), 5U);
}

TEST(Bytes, FreezeIsZeroCopy)
{
    Bytes bytes(16);
    bytes.Extend("frozen");
    std::byte* before = bytes.Data();

    Buf buf = VIOLET_MOVE(bytes).Freeze();
    ASSERT_EQ(buf, "frozen");
    ASSERT_EQ(buf.Data().Get(), before);
}

TEST(Bytes, FreezeEmptyReleasesStorage)
{
    Bytes bytes(64);
    Buf buf = VIOLET_MOVE(bytes).Freeze();

    ASSERT_TRUE(buf.Empty());
    ASSERT_FALSE(buf.Unique());
}

TEST(Bytes, SplitToThenKeepWriting)
{
    Bytes bytes(8);
    bytes.Extend("frame1");

    Buf frame = bytes.SplitTo(6).Freeze();
    bytes.Extend("this write reallocates past the old capacity");

    ASSERT_EQ(frame, "frame1");
    ASSERT_EQ(VIOLET_MOVE(bytes).Freeze(), "this write reallocates past the old capacity");
}

TEST(Bytes, SplitToSharesStorageWithoutOverlap)
{
    Bytes bytes(10);
    bytes.Extend("aaaa");
    Bytes head = bytes.SplitTo(2);

    head.Extend("X");
    ASSERT_EQ(VIOLET_MOVE(head).Freeze(), "aaX");
    ASSERT_EQ(VIOLET_MOVE(bytes).Freeze(), "aa");
}

TEST(Bytes, TryIntoMutReclaimsTailCapacity)
{
    Bytes bytes(32);
    bytes.Extend("abc");
    Buf buf = VIOLET_MOVE(bytes).Freeze();

    Optional<Bytes> mut = VIOLET_MOVE(buf).TryIntoMut();
    ASSERT_TRUE(mut);
    ASSERT_EQ(mut.Value().Size(), 3U);
    ASSERT_EQ(mut.Value().Capacity(), 32U);
}

} // namespace violet::experimental
