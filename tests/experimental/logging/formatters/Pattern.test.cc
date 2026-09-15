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
#include <violet/Experimental/Logging/Formatters/Pattern.h>
#include <violet/Experimental/Logging/LogRecord.h>

namespace violet::experimental::log::formatter {
namespace {
namespace pat = violet::experimental::log::formatter::pattern;

constexpr char kEsc = '\x1b';

auto makeRecord(LogLevel level = LogLevel::Info, Str message = "hello world") -> Record
{
    auto record = Record::Now(level, message);
    record.Location.File = "Pattern.test.cc";
    record.Location.Line = 42;
    record.Location.Column = 7;
    record.Location.Function = "TestFunction";
    record.Logger = "eousd";

    return record;
}

auto containsEscape(Str out) -> bool
{
    // NOLINTNEXTLINE(readability-container-contains)
    return out.find(kEsc) != Str::npos;
}

} // namespace

TEST(Pattern, ParsesAndFormatsLiteral)
{
    auto pattern = Pattern::Parse("hello");
    ASSERT_TRUE(pattern) << "failed to parse: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "hello");
}

TEST(Pattern, FormatsMessage)
{
    auto pattern = Pattern::Parse("[%m]");
    ASSERT_TRUE(pattern) << "failed to parse: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord(LogLevel::Info, "ur mom gay")), "[ur mom gay]");
}

TEST(Pattern, FormatsLoggerName)
{
    auto pattern = Pattern::Parse("[%n]");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "[eousd]");
}

TEST(Pattern, FormatsLoggerWithLeftPadding)
{
    auto pattern = Pattern::Parse("[%n{<10}]");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "[eousd     ]");
}

TEST(Pattern, FormatsLoggerWithRightPadding)
{
    auto pattern = Pattern::Parse("[%n{>10}]");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "[     eousd]");
}

TEST(Pattern, FormatsLoggerWithCenterPadding)
{
    auto pattern = Pattern::Parse("[%n{^9}]");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "[  eousd  ]");
}

TEST(Pattern, FormatsLoggerWithCustomFill)
{
    auto pattern = Pattern::Parse("[%n{*<8}]");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "[eousd***]");
}

TEST(Pattern, FormatsLoggerNoPaddingWhenWidthFits)
{
    auto pattern = Pattern::Parse("[%n{<3}]");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "[eousd]");
}

TEST(Pattern, FormatsLevelDefault)
{
    auto pattern = Pattern::Parse("%L");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord(LogLevel::Warning)), "warning");
}

TEST(Pattern, FormatsLevelWithPadding)
{
    auto pattern = Pattern::Parse("[%L{<8}]");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord(LogLevel::Info)), "[info    ]");
}

TEST(Pattern, FormatsFile)
{
    auto pattern = Pattern::Parse("%f");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "Pattern.test.cc");
}

TEST(Pattern, FormatsFunctionName)
{
    auto pattern = Pattern::Parse("%F");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "TestFunction");
}

TEST(Pattern, FormatsLineAndColumn)
{
    auto pattern = Pattern::Parse("%f:%l:%c");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "Pattern.test.cc:42:7");
}

TEST(Pattern, FormatsAttributeByKey)
{
    auto record = makeRecord().With("request.id", AttributeValue("abc-123"));
    auto pattern = Pattern::Parse("id=%X{request.id}");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(record), "id=abc-123");
}

TEST(Pattern, FormatsAttributeIntegerAndBool)
{
    auto record = makeRecord().With("retries", AttributeValue(static_cast<UInt64>(3))).With("ok", AttributeValue(true));

    auto pattern = Pattern::Parse("%X{retries}/%X{ok}");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(record), "3/true");
}

TEST(Pattern, AttributeMissingKeyEmitsNothing)
{
    auto pattern = Pattern::Parse("[%X{missing}]");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "[]");
}

TEST(Pattern, AllAttributesUsesDefaultMarkers)
{
    auto record = makeRecord().With("k", AttributeValue("v"));
    auto pattern = Pattern::Parse("%A");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(record), "[k=v]");
}

TEST(Pattern, AllAttributesEmptyEmitsNothing)
{
    auto pattern = Pattern::Parse(">%A<");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "><");
}

TEST(Pattern, AllAttributesCustomMarkersAndDelimiter)
{
    auto record = makeRecord();
    record.With("k", AttributeValue(Str("v")));

    auto pattern = Pattern::Parse("%A", Pattern::Config{.UseColours = false, .Markers = {"<", ">"}, .Delimiter = ":"});

    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(record), "<k:v>");
}

TEST(Pattern, EscapedPercentEmitsLiteral)
{
    auto pattern = Pattern::Parse("100%%");
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(makeRecord()), "100%");
}

TEST(Pattern, ColoursDisabledSuppressesAnsi)
{
    auto result = Pattern::Parse("%fg{red}%bold[%m]%style:end%style:end", Pattern::Config{.UseColours = false});
    ASSERT_TRUE(result) << "parse failed: " << result.Error().ToString();

    auto out = result.Value().Format(makeRecord(LogLevel::Info, "msg"));
    EXPECT_FALSE(containsEscape(out));
    EXPECT_EQ(out, "[msg]");
}

TEST(Pattern, ColoursEnabledEmitsAnsi)
{
    auto pattern = Pattern::Parse("%fg{red}[%m]%style:end", Pattern::Config{.UseColours = true});
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();

    auto out = pattern->Format(makeRecord(LogLevel::Info, "msg"));
    EXPECT_TRUE(containsEscape(out));
    EXPECT_NE(out.find("[msg]"), String::npos);
}

TEST(Pattern, LevelColourEnabledEmitsAnsi)
{
    auto pattern = Pattern::Parse("%levelColor[%L]%style:end", Pattern::Config{.UseColours = true});
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();

    auto out = pattern->Format(makeRecord(LogLevel::Error));
    EXPECT_TRUE(containsEscape(out));
    EXPECT_NE(out.find("[error]"), String::npos);
}

TEST(Pattern, LevelColourWithExplicitSpec)
{
    auto pattern = Pattern::Parse("%levelColor{#abcdef}[%L]%style:end", Pattern::Config{.UseColours = true});
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();

    auto out = pattern->Format(makeRecord(LogLevel::Trace));
    EXPECT_TRUE(containsEscape(out));
    EXPECT_NE(out.find("[trace]"), String::npos);
}

TEST(Pattern, RejectsBadHexColour)
{
    auto result = Pattern::Parse("%fg{#zzz}[x]%style:end");
    ASSERT_FALSE(result) << "expected parse failure for invalid hex";
}

TEST(Pattern, RejectsBadRgbColour)
{
    auto result = Pattern::Parse("%fg{300,0,0}[x]%style:end");
    ASSERT_FALSE(result) << "expected parse failure for out-of-range RGB";
}

TEST(Pattern, RejectsUnknownNamedColour)
{
    auto result = Pattern::Parse("%fg{notacolor}[x]%style:end");
    ASSERT_FALSE(result);
}

TEST(Pattern, AcceptsAnyCSSNamedColour)
{
    auto pattern = Pattern::Parse("%fg{rebeccapurple}[%m]%style:end", Pattern::Config{.UseColours = true});
    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();

    auto out = pattern->Format(makeRecord(LogLevel::Info, "x"));
    EXPECT_TRUE(containsEscape(out));
}

TEST(Pattern, MarkerPrefixSuffixApplyAroundFields)
{
    auto record = makeRecord().With("k", AttributeValue(Str("v")));

    auto pattern
        = Pattern::Parse("%A", Pattern::Config{.UseColours = false, .Markers = {"<<", ">>"}, .Delimiter = "="});

    ASSERT_TRUE(pattern) << "failed to parse pattern: " << pattern.Error();
    EXPECT_EQ(pattern->Format(record), "<<k=v>>");
}

TEST(Pattern, MarkerWithStyleDirectivesParsesAndStripsAnsiWhenDisabled)
{
    auto record = makeRecord().With("k", AttributeValue(Str("v")));
    auto result = Pattern::Parse(
        "%A", Pattern::Config{.UseColours = false, .Markers = {"%bold[", "]%style:end"}, .Delimiter = "="});
    ASSERT_TRUE(result) << "parse failed: " << result.Error().ToString();

    auto out = result->Format(record);
    EXPECT_FALSE(containsEscape(out));
    EXPECT_EQ(out, "[k=v]");
}

TEST(Pattern, MarkerRejectsForbiddenSegments)
{
    // `%m` is a data directive and must not be allowed inside marker patterns.
    auto result = Pattern::Parse("%A", Pattern::Config{.Markers = {"[%m]", "]"}, .Delimiter = "="});
    ASSERT_FALSE(result);
}

TEST(Pattern, AzaliaPresetIsConstructable)
{
    // Smoke test: this must not throw / abort.
    auto pattern = Pattern::Azalia();
    auto out = pattern.Format(makeRecord(LogLevel::Info, "hi"));
    EXPECT_FALSE(out.empty());
}

TEST(Parser, EmptyPatternHasNoSegments)
{
    auto parsed = pat::Parse("");
    ASSERT_TRUE(parsed) << "failed to parse pattern: " << parsed.Error();
    EXPECT_EQ(parsed->Count, 0u);
}

TEST(Parser, LiteralProducesSingleSegment)
{
    auto parsed = pat::Parse("hi");
    ASSERT_TRUE(parsed) << "failed to parse pattern: " << parsed.Error();
    ASSERT_EQ(parsed->Count, 1u);
    EXPECT_EQ(parsed->Segments[0].Kind, pat::SegmentKind::Literal);
}

TEST(Parser, ConsecutiveDirectivesProduceMultipleSegments)
{
    auto parsed = pat::Parse("%L %m");
    ASSERT_TRUE(parsed) << "failed to parse pattern: " << parsed.Error();

    ASSERT_EQ(parsed->Count, 3u);
    EXPECT_EQ(parsed->Segments[0].Kind, pat::SegmentKind::Level);
    EXPECT_EQ(parsed->Segments[1].Kind, pat::SegmentKind::Literal);
    EXPECT_EQ(parsed->Segments[2].Kind, pat::SegmentKind::Message);
}

TEST(Parser, EscapedPercentBecomesLiteralSegment)
{
    auto parsed = pat::Parse("%%");
    ASSERT_TRUE(parsed) << "failed to parse pattern: " << parsed.Error();
    ASSERT_EQ(parsed->Count, 1u);
    EXPECT_EQ(parsed->Segments[0].Kind, pat::SegmentKind::Literal);
}

TEST(Parser, RejectsUnknownDirective)
{
    auto result = pat::Parse("%q");
    ASSERT_FALSE(result);
}

TEST(Parser, RejectsUnterminatedBrace)
{
    auto result = pat::Parse("%fg{red");
    ASSERT_FALSE(result);
}

TEST(Parser, RejectsAttributeWithoutKey)
{
    auto result = pat::Parse("%X");
    ASSERT_FALSE(result);
}

TEST(Parser, RejectsForegroundWithoutSpec)
{
    auto result = pat::Parse("%fg");
    ASSERT_FALSE(result);
}

TEST(Parser, RejectsBackgroundWithoutSpec)
{
    auto result = pat::Parse("%bg");
    ASSERT_FALSE(result);
}

TEST(Parser, RejectsUnmatchedStyleEnd)
{
    auto result = pat::Parse("hello%style:end");
    ASSERT_FALSE(result);
}

TEST(Parser, RejectsUnclosedStyle)
{
    auto result = pat::Parse("%bold[hello]");
    ASSERT_FALSE(result);
}

TEST(Parser, ParsesStyleNestingDepth)
{
    auto parsed = pat::Parse("%bold%italic[x]%style:end%style:end");
    ASSERT_TRUE(parsed) << "failed to parse: " << parsed.Error();

    // Find the bold / italic / style:end segments and verify depths.
    UInt8 boldDepth = 0;
    UInt8 italicDepth = 0;
    UInt8 firstEndDepth = 0;
    UInt8 secondEndDepth = 255;
    bool sawFirstEnd = false;

    for (UInt i = 0; i < parsed->Count; ++i) {
        const auto& segment = parsed->Segments[i];
        switch (segment.Kind) {
        case pat::SegmentKind::Bold:
            boldDepth = segment.Depth;
            break;

        case pat::SegmentKind::Italic:
            italicDepth = segment.Depth;
            break;

        case pat::SegmentKind::StyleEnd:
            if (!sawFirstEnd) {
                firstEndDepth = segment.Depth;
                sawFirstEnd = true;
            } else {
                secondEndDepth = segment.Depth;
            }

            break;

        default:
            break;
        }
    }

    EXPECT_EQ(boldDepth, 1);
    EXPECT_EQ(italicDepth, 2);
    EXPECT_EQ(firstEndDepth, 1);
    EXPECT_EQ(secondEndDepth, 0);
}

TEST(Parser, AllowedSegmentsRestrictsKinds)
{
    pat::ParseConfig cfg{
        .AllowedSegments = {pat::SegmentKind::Literal, pat::SegmentKind::Bold, pat::SegmentKind::StyleEnd}};

    EXPECT_TRUE(pat::Parse("hello %bold[x]%style:end", cfg));
    EXPECT_FALSE(pat::Parse("%m", cfg));
}

TEST(Parser, LevelDirectiveSpecAndPaddingDisambiguates)
{
    // No `:` and starts with alignment char -> padding.
    auto a = pat::Parse("%L{<5}");
    ASSERT_TRUE(a) << "failed to parse: " << a.Error();

    ASSERT_EQ(a->Count, 1u);
    ASSERT_TRUE(a->Segments[0].Directive);
    EXPECT_TRUE(a->Segments[0].Directive->HasPadding());
    EXPECT_FALSE(a->Segments[0].Directive->HasSpecification());

    // No `:` and starts with letter -> spec.
    auto b = pat::Parse("%L{upper}");
    ASSERT_TRUE(b) << "failed to parse: " << b.Error();

    ASSERT_EQ(b->Count, 1u);
    ASSERT_TRUE(b->Segments[0].Directive);
    EXPECT_TRUE(b->Segments[0].Directive->HasSpecification());
    EXPECT_FALSE(b->Segments[0].Directive->HasPadding());

    // Explicit `:` separator -> both.
    auto c = pat::Parse("%L{upper:<5}");
    ASSERT_TRUE(c) << "failed to parse: " << c.Error();

    ASSERT_EQ(c->Count, 1u);
    ASSERT_TRUE(c->Segments[0].Directive);
    EXPECT_TRUE(c->Segments[0].Directive->HasSpecification());
    EXPECT_TRUE(c->Segments[0].Directive->HasPadding());
}

TEST(Parser, LoggerPaddingOnlyRejectsSpecSyntax)
{
    // `%n` capability is `PaddingOnly`, so a `:` (which would suggest spec) is rejected.
    auto result = pat::Parse("%n{upper:<5}");
    ASSERT_FALSE(result);
}

TEST(Parser, MessageRejectsBraceBlock)
{
    // `%m` capability is `None`, so `{...}` must be rejected.
    auto result = pat::Parse("%m{anything}");
    ASSERT_FALSE(result);
}

TEST(Parser, StoresDirectiveSpecOffsetAndLength)
{
    auto parsed = pat::Parse("%X{request.id}");
    ASSERT_TRUE(parsed) << "failed to parse: " << parsed.Error();

    ASSERT_EQ(parsed->Count, 1u);
    ASSERT_TRUE(parsed->Segments[0].Directive);

    const auto& dir = parsed->Segments[0].Directive.Value();
    auto spec = dir.Specification(parsed->Pattern);
    EXPECT_EQ(spec, "request.id");
}

TEST(Parser, KeywordWordBoundaryDistinguishesForegroundFromFile)
{
    // `%f` is `File` (no brace block); `%fg{...}` is the foreground-colour keyword.
    auto a = pat::Parse("%f");
    ASSERT_TRUE(a) << "failed to parse pattern: " << a.Error();
    ASSERT_EQ(a->Count, 1u);
    EXPECT_EQ(a->Segments[0].Kind, pat::SegmentKind::File);

    auto b = pat::Parse("%fg{red}[x]%style:end");
    ASSERT_TRUE(b) << "failed to parse pattern: " << b.Error();
    ASSERT_GE(b->Count, 1u);
    EXPECT_EQ(b->Segments[0].Kind, pat::SegmentKind::ForegroundColorStart);
}

} // namespace violet::experimental::log::formatter
