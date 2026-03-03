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
#include <violet/Iterator/Map.h>
#include <violet/Support/Terminal.h>
#include <violet/Testing/CaptureStream.h>
#include <violet/anyhow.h>

// NOLINTBEGIN(google-build-using-namespace)
using namespace violet;
using namespace violet::testing;
// NOLINTEND(google-build-using-namespace)

struct dummy_t final {
    String Message;

    VIOLET_EXPLICIT dummy_t(String message)
        : Message(VIOLET_MOVE(message))
    {
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return this->Message;
    }
};

TEST(Anyhow, ContextWithString)
{
    terminal::SetColorChoice(terminal::ColorChoice::Never);

    anyhow::Error error = anyhow::Error(dummy_t("hello world")).Context("additional context");
    error.Print();

    {
        CaptureStream captured(std::cerr);
        error.Print();

        EXPECT_TRUE(captured.Contains("additional context"));
        EXPECT_TRUE(captured.Contains("hello world"));
        EXPECT_LT(captured.Find("hello world"), captured.Find("additional context"));
    }
}

TEST(Anyhow, DanglingContextLeak)
{
    {
        auto base = anyhow::Error("base error");
        auto wrapped = anyhow::Error("context").Context(VIOLET_MOVE(base));
    }
}

TEST(Anyhow, TempResultMove)
{
    auto pleaseNotFuckUp = []() -> anyhow::Result<int> { return Err(anyhow::Error("inner")); };
    auto test = [&]() -> auto {
        auto result = pleaseNotFuckUp();
        return anyhow::Error("outer context").Context(VIOLET_MOVE(result).Error());
    };

    auto error = test();
    error.Print();
}

TEST(Anyhow, ChainSingleNode)
{
    auto error = anyhow::Error("root cause");
    auto ch = anyhow::Chain(error);

    auto first = ch.Next();
    ASSERT_TRUE(first.HasValue());
    EXPECT_EQ(first->Message, "root cause");
    EXPECT_FALSE(ch.Next().HasValue());
}

TEST(Anyhow, ChainWithContext)
{
    auto error = anyhow::Error("root").Context("mid").Context("top");
    auto ch = anyhow::Chain(error);

    // head = most-recently-added context ("top"), tail = root cause
    auto f0 = ch.Next();
    ASSERT_TRUE(f0.HasValue());
    EXPECT_EQ(f0->Message, "top");

    auto f1 = ch.Next();
    ASSERT_TRUE(f1.HasValue());
    EXPECT_EQ(f1->Message, "mid");

    auto f2 = ch.Next();
    ASSERT_TRUE(f2.HasValue());
    EXPECT_EQ(f2->Message, "root");

    EXPECT_FALSE(ch.Next().HasValue());
}

TEST(Anyhow, ChainCount)
{
    auto error = anyhow::Error("a").Context("b").Context("c");
    EXPECT_EQ(anyhow::Chain(error).Count(), 3U);
}

TEST(Anyhow, ChainMessages)
{
    auto error = anyhow::Error("base").Context("mid").Context("top");
    Vec<String> messages = anyhow::Chain(error)
                               .Map([](const anyhow::Chain::Frame& frame) -> String { return frame.Message; })
                               .Collect<Vec<String>>();

    ASSERT_EQ(messages.size(), 3U);
    EXPECT_EQ(messages[0], "top");
    EXPECT_EQ(messages[1], "mid");
    EXPECT_EQ(messages[2], "base");
}

TEST(Anyhow, ChainSourceLocationsPopulated)
{
    auto error = anyhow::Error("root");
    auto frame = anyhow::Chain(error).Next();

    ASSERT_TRUE(frame.HasValue());
    EXPECT_NE(frame->Location.file_name(), nullptr);
    EXPECT_GT(frame->Location.line(), 0U);
}

TEST(Anyhow, ChainMovedFromError)
{
    anyhow::Error source("temp");
    anyhow::Error moved = VIOLET_MOVE(source);

    // source is now moved-from; its chain should be empty
    EXPECT_FALSE(anyhow::Chain(source).Next().HasValue());
}

TEST(Anyhow, ChainRangeFor)
{
    auto error = anyhow::Error("a").Context("b");
    Vec<String> messages;
    for (auto frame: anyhow::Chain(error)) {
        messages.push_back(frame.Message);
    }

    ASSERT_EQ(messages.size(), 2U);
    EXPECT_EQ(messages[0], "b");
    EXPECT_EQ(messages[1], "a");
}

#if VIOLET_USE_RTTI

struct other_error_t final {
    int Code;

    VIOLET_EXPLICIT other_error_t(int code)
        : Code(code)
    {
    }

    [[nodiscard]] auto ToString() const noexcept -> String
    {
        return std::format("error code {}", Code);
    }
};

TEST(Anyhow, DowncastSuccess)
{
    auto error = anyhow::Error(dummy_t("type-erased value"));
    auto frame = anyhow::Chain(error).Next();

    ASSERT_TRUE(frame.HasValue());

    auto result = frame->Downcast<dummy_t>();
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(result->Message, "type-erased value");
}

TEST(Anyhow, DowncastWrongType)
{
    auto error = anyhow::Error(dummy_t("some value"));
    auto frame = anyhow::Chain(error).Next();

    ASSERT_TRUE(frame.HasValue());
    EXPECT_FALSE(frame->Downcast<other_error_t>().HasValue());
}

TEST(Anyhow, DowncastMixedChain)
{
    auto error = anyhow::Error(dummy_t("root")).Context(other_error_t(42));
    auto ch = anyhow::Chain(error);

    auto f0 = ch.Next(); // other_error_t context frame
    ASSERT_TRUE(f0.HasValue());

    auto as_other = f0->Downcast<other_error_t>();
    ASSERT_TRUE(as_other.HasValue());
    EXPECT_EQ(as_other->Code, 42);
    EXPECT_FALSE(f0->Downcast<dummy_t>().HasValue());

    auto f1 = ch.Next(); // dummy_t root frame
    ASSERT_TRUE(f1.HasValue());

    auto as_dummy = f1->Downcast<dummy_t>();
    ASSERT_TRUE(as_dummy.HasValue());
    EXPECT_EQ(as_dummy->Message, "root");
    EXPECT_FALSE(f1->Downcast<other_error_t>().HasValue());
}

TEST(Anyhow, DowncastStringError)
{
    auto error = anyhow::Error(String("plain string error"));
    auto frame = anyhow::Chain(error).Next();

    ASSERT_TRUE(frame.HasValue());
    EXPECT_TRUE(frame->Downcast<String>().HasValue());
    EXPECT_FALSE(frame->Downcast<dummy_t>().HasValue());
}

#endif
