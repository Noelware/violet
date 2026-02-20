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
#include <violet/Support/Terminal.h>
#include <violet/anyhow.h>

using namespace violet; // NOLINT(google-build-using-namespace)

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

    std::ostringstream os;
    auto* oldOs = std::cerr.rdbuf(os.rdbuf());
    error.Print();
    std::cerr.rdbuf(oldOs);

    String output = os.str();
    EXPECT_NE(output.find("additional context"), String::npos);
    EXPECT_NE(output.find("hello world"), String::npos);
    EXPECT_LT(output.find("hello world"), output.find("additional context"));
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
