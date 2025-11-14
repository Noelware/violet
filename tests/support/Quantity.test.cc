// 🌺💜 Violet: Extended standard library for C++26
// Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors
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
#include <violet/Support/Quantity.h>

using namespace violet; // NOLINT(google-build-using-namespace)
using namespace violet::resource; // NOLINT(google-build-using-namespace)

TEST(CPU, ParseSimpleCores)
{
    auto cpu = violet::resource::CPU::Parse("1");
    ASSERT_TRUE(cpu);

    EXPECT_DOUBLE_EQ(cpu.Value().AsCores(), 1.0);
    EXPECT_EQ(cpu.Value().ToString(), "1.000");

    auto cpuMilli = violet::resource::CPU::Parse("250m");
    ASSERT_TRUE(cpuMilli);

    EXPECT_DOUBLE_EQ(cpuMilli.Value().AsCores(), 0.25);
    EXPECT_EQ(cpuMilli.Value().ToString(), "250m");
}

TEST(CPU, ParseScientific)
{
    auto cpu = violet::resource::CPU::Parse("1e3m");
    ASSERT_TRUE(cpu);
    EXPECT_DOUBLE_EQ(cpu.Value().AsCores(), 1.0); // 1000m = 1 core
}

TEST(Memory, ParseBinaryUnits)
{
    auto mem = violet::resource::Memory::Parse("512Ki");
    ASSERT_TRUE(mem);

    EXPECT_DOUBLE_EQ(mem.Value().ToDouble(), 512 * 1024);
    EXPECT_EQ(mem.Value().ToString(), "512Ki");

    auto memMi = violet::resource::Memory::Parse("1.5Mi");
    ASSERT_TRUE(memMi);

    EXPECT_DOUBLE_EQ(memMi.Value().ToDouble(), 1.5 * 1024 * 1024);
    EXPECT_EQ(memMi.Value().ToString(), "1.5Mi");
}

TEST(Memory, ParseDecimalUnits)
{
    auto mem = violet::resource::Memory::Parse("2M");
    ASSERT_TRUE(mem);

    EXPECT_DOUBLE_EQ(mem.Value().ToDouble(), 2e6);
    EXPECT_EQ(mem.Value().ToString(), "2M"); // assuming your table has decimal units
}

TEST(Memory, AutoUnitSelection)
{
    auto mem = violet::resource::Memory::Parse("1536Mi");
    ASSERT_TRUE(mem);

    // Should pick Gi if using largest unit that fits
    EXPECT_EQ(mem.Value().ToString(), "1.5Gi");
}

/*
// Scaling Tests
TEST(QuantityScaling, ScaleMemory) {
    auto mem = violet::resource::MemoryQuantity::parse("512Mi");
    auto scaled = mem * 1.5; // returns new Quantity
    EXPECT_DOUBLE_EQ(scaled.asBytes(), 512 * 1024 * 1.5);
    EXPECT_EQ(scaled.toString(), "768Mi");
}

TEST(QuantityScaling, ScaleCPU) {
    auto cpu = violet::resource::CPUQuantity::parse("250m");
    auto scaled = cpu * 2.0;
    EXPECT_DOUBLE_EQ(scaled.asCores(), 0.5);
    EXPECT_EQ(scaled.toString(), "500m");
}

// Edge Cases
TEST(MemoryQuantity, ZeroAndOneByte) {
    auto zero = violet::resource::MemoryQuantity::parse("0B");
    EXPECT_EQ(zero.toString(), "0B");

    auto one = violet::resource::MemoryQuantity::parse("1B");
    EXPECT_EQ(one.toString(), "1B");
}

TEST(CPUQuantity, InvalidSuffix) {
    EXPECT_THROW(violet::resource::CPUQuantity::parse("1x"), std::invalid_argument);
}

TEST(MemoryQuantity, InvalidSuffix) {
    EXPECT_THROW(violet::resource::MemoryQuantity::parse("1Zi"), std::invalid_argument);
}
*/
