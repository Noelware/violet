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
#include <violet/RefCnt.h>

// NOLINTBEGIN(google-build-using-namespace,readability-identifier-length)
using namespace violet;

namespace {
struct Counter final {
    Int32 References = 0; ///< live reference count
    Int32 Bumps = 0; ///< total `BumpRef` calls (retains)
    Int32 Decrements = 0; ///< total `DecRef` calls (releases)
};

using Ref = RefCnt<Counter*>;
} // namespace

template<>
struct violet::RefCntTraits<Counter*> final {
    static auto Default() noexcept -> Counter*
    {
        return nullptr;
    }

    static auto Valid(Counter* handle) noexcept -> bool
    {
        return handle != nullptr;
    }

    static void BumpRef(Counter* handle) noexcept
    {
        if (handle != nullptr) {
            ++handle->References;
            ++handle->Bumps;
        }
    }

    static void DecRef(Counter* handle) noexcept
    {
        if (handle != nullptr) {
            --handle->References;
            ++handle->Decrements;
        }
    }
};

TEST(RefCnt, DefaultOwnsNothing)
{
    Ref r;
    EXPECT_FALSE(r.Valid());
    EXPECT_FALSE(static_cast<bool>(r));
    EXPECT_EQ(r.Get(), nullptr);
    // Destroying an empty RefCnt must not touch any count: DecRef is a no-op on Default().
}

TEST(RefCnt, AdoptTakesOverExistingReference)
{
    Counter c;
    c.References = 1; // pretend the caller already holds a reference
    {
        Ref r(&c);
        EXPECT_EQ(c.Bumps, 0); // adopt does not retain
        EXPECT_TRUE(r.Valid());
    }

    EXPECT_EQ(c.Decrements, 1);
    EXPECT_EQ(c.References, 0); // released on destruction
}

TEST(RefCnt, RetainAcquiresAReference)
{
    Counter c;
    {
        auto r = Ref::Retain(&c);
        EXPECT_EQ(c.References, 1);
        EXPECT_EQ(c.Bumps, 1);
        EXPECT_TRUE(r.Valid());
        EXPECT_EQ(r.Get(), &c);
    }

    EXPECT_EQ(c.References, 0);
    EXPECT_EQ(c.Decrements, 1);
}

TEST(RefCnt, CopyRetains)
{
    Counter c;
    {
        auto r = Ref::Retain(&c);
        EXPECT_EQ(c.References, 1);
        {
            auto r2 = r; // NOLINT(performance-unnecessary-copy-initialization)
            EXPECT_EQ(c.References, 2);
            EXPECT_EQ(r2.Get(), &c);
        }

        EXPECT_EQ(c.References, 1); // r2 released
    }

    EXPECT_EQ(c.References, 0);
}

TEST(RefCnt, CopyAssignRetainsNewReleasesOld)
{
    Counter a;
    Counter b;
    {
        auto ra = Ref::Retain(&a);
        auto rb = Ref::Retain(&b);
        EXPECT_EQ(a.References, 1);
        EXPECT_EQ(b.References, 1);

        ra = rb;
        EXPECT_EQ(b.References, 2); // retained
        EXPECT_EQ(a.References, 0); // released
        EXPECT_EQ(ra.Get(), &b);
    }

    EXPECT_EQ(a.References, 0);
    EXPECT_EQ(b.References, 0);
}

TEST(RefCnt, SelfCopyAssignIsStable)
{
    Counter c;
    {
        auto r = Ref::Retain(&c);
        EXPECT_EQ(c.References, 1);

        Ref& alias = r;
        r = alias;
        EXPECT_EQ(c.References, 1); // retain-before-release keeps the count balanced
        EXPECT_TRUE(r.Valid());
    }

    EXPECT_EQ(c.References, 0);
}

TEST(RefCnt, MoveTransfersWithoutRetaining)
{
    Counter c;
    {
        auto r = Ref::Retain(&c);
        EXPECT_EQ(c.Bumps, 1);

        auto r2 = VIOLET_MOVE(r);
        EXPECT_EQ(c.References, 1); // no extra retain
        EXPECT_EQ(c.Bumps, 1);
        EXPECT_FALSE(r.Valid()); // source owns nothing
        EXPECT_EQ(r.Get(), nullptr);
        EXPECT_TRUE(r2.Valid());
        EXPECT_EQ(r2.Get(), &c);
    }

    EXPECT_EQ(c.References, 0);
    EXPECT_EQ(c.Decrements, 1);
}

TEST(RefCnt, MoveAssignReleasesOldAndTransfers)
{
    Counter a;
    Counter b;
    {
        auto ra = Ref::Retain(&a);
        auto rb = Ref::Retain(&b);

        ra = VIOLET_MOVE(rb);
        EXPECT_EQ(a.References, 0); // old released
        EXPECT_EQ(b.References, 1); // transferred, not retained
        EXPECT_FALSE(rb.Valid());
        EXPECT_EQ(ra.Get(), &b);
    }

    EXPECT_EQ(a.References, 0);
    EXPECT_EQ(b.References, 0);
}

TEST(RefCnt, SelfMoveAssignIsStable)
{
    Counter c;
    {
        auto r = Ref::Retain(&c);
        Ref& alias = r; // dodge -Wself-move
        r = VIOLET_MOVE(alias);
        EXPECT_TRUE(r.Valid()); // guarded self-move leaves it intact
        EXPECT_EQ(c.References, 1);
    }

    EXPECT_EQ(c.References, 0);
}

TEST(RefCnt, GetDoesNotRetain)
{
    Counter c;
    auto r = Ref::Retain(&c);
    EXPECT_EQ(r.Get(), &c);
    EXPECT_EQ(c.Bumps, 1); // Get borrows; no extra bump
}

// NOLINTEND(google-build-using-namespace,readability-identifier-length)
