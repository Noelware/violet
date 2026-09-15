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

#include <violet/Experimental/Logging/Formatters/Pattern.h>
#include <violet/Experimental/Slice.h>
#include <violet/Strings.h>
#include <violet/Support/Terminal.h>

#if VIOLET_REQUIRE_STL(202302L)
#define contains(input, ch) (input).contains(ch)
#else
#define contains(input, ch) (input).find(ch) != ::violet::Str::npos
#endif

namespace violet::experimental::log::formatter {
namespace {

using LevelColourPalette = Slice<Pair<LogLevel, terminal::RGB>, 6>;

inline constexpr LevelColourPalette kDefaultLevelColorPalette = {
    // clang-format off
    LevelColourPalette::value_type
    {LogLevel::Trace, {102, 178, 178}},   // rgb(102,178,178); #66b2b2
    {LogLevel::Debug,   {162, 224, 162}}, // rgb(162, 224, 162); #a2e0a2
    {LogLevel::Info,    {48, 48, 105}},   // rgb(48, 48, 105); #303069
    {LogLevel::Warning, {239, 239, 184}}, // rgb(239, 239, 184); #efefb8
    {LogLevel::Error,   {193, 50, 50}},   // rgb(193, 50, 50); #c13232
    {LogLevel::Fatal,   {193, 50, 50}},   // rgb(193, 50, 50); #c13232
    // clang-format on
};

inline constexpr LevelColourPalette kAzaliaLevelColorPalette = {
    // clang-format off
    LevelColourPalette::value_type
    {LogLevel::Trace, {163, 182, 138}},   // rgb(163, 182, 138); #a3b68a
    {LogLevel::Debug,   {148, 224, 232}}, // rgb(148, 224, 232); #94e0e8
    {LogLevel::Info,    {178, 157, 243}},   // rgb(178, 157, 243); #b29df3
    {LogLevel::Warning, {243, 243, 134}}, // rgb(243, 243, 134); #f3f386
    {LogLevel::Error,   {153, 75, 104}},  // rgb(153, 75, 104); #994b68
    {LogLevel::Fatal,   {153, 75, 104}},  // rgb(153, 75, 104); #994b68
    // clang-format on
};

using CSSColorMap = Slice<Pair<Str, terminal::RGB>, 148>;

inline constexpr CSSColorMap kCSSColours = {
    // clang-format off
    CSSColorMap::value_type
    {"aliceblue", {240, 248, 255}},
    {"antiquewhite", {250, 235, 215}},
    {"aqua", {0, 255, 255}},
    {"aquamarine", {127, 255, 212}},
    {"azure", {240, 255, 255}},
    {"beige", {245, 245, 220}},
    {"bisque", {255, 228, 196}},
    {"black", {0, 0, 0}},
    {"blanchedalmond", {255, 235, 205}},
    {"blue", {0, 0, 255}},
    {"blueviolet", {138, 43, 226}},
    {"brown", {165, 42, 42}},
    {"burlywood", {222, 184, 135}},
    {"cadetblue", {95, 158, 160}},
    {"chartreuse", {127, 255, 0}},
    {"chocolate", {210, 105, 30}},
    {"coral", {255, 127, 80}},
    {"cornflowerblue", {100, 149, 237}},
    {"cornsilk", {255, 248, 220}},
    {"crimson", {220, 20, 60}},
    {"cyan", {0, 255, 255}},
    {"darkblue", {0, 0, 139}},
    {"darkcyan", {0, 139, 139}},
    {"darkgoldenrod", {184, 134, 11}},
    {"darkgray", {169, 169, 169}},
    {"darkgreen", {0, 100, 0}},
    {"darkgrey", {169, 169, 169}},
    {"darkkhaki", {189, 183, 107}},
    {"darkmagenta", {139, 0, 139}},
    {"darkolivegreen", {85, 107, 47}},
    {"darkorange", {255, 140, 0}},
    {"darkorchid", {153, 50, 204}},
    {"darkred", {139, 0, 0}},
    {"darksalmon", {233, 150, 122}},
    {"darkseagreen", {143, 188, 143}},
    {"darkslateblue", {72, 61, 139}},
    {"darkslategray", {47, 79, 79}},
    {"darkslategrey", {47, 79, 79}},
    {"darkturquoise", {0, 206, 209}},
    {"darkviolet", {148, 0, 211}},
    {"deeppink", {255, 20, 147}},
    {"deepskyblue", {0, 191, 255}},
    {"dimgray", {105, 105, 105}},
    {"dimgrey", {105, 105, 105}},
    {"dodgerblue", {30, 144, 255}},
    {"firebrick", {178, 34, 34}},
    {"floralwhite", {255, 250, 240}},
    {"forestgreen", {34, 139, 34}},
    {"fuchsia", {255, 0, 255}},
    {"gainsboro", {220, 220, 220}},
    {"ghostwhite", {248, 248, 255}},
    {"gold", {255, 215, 0}},
    {"goldenrod", {218, 165, 32}},
    {"gray", {128, 128, 128}},
    {"green", {0, 128, 0}},
    {"greenyellow", {173, 255, 47}},
    {"grey", {128, 128, 128}},
    {"honeydew", {240, 255, 240}},
    {"hotpink", {255, 105, 180}},
    {"indianred", {205, 92, 92}},
    {"indigo", {75, 0, 130}},
    {"ivory", {255, 255, 240}},
    {"khaki", {240, 230, 140}},
    {"lavender", {230, 230, 250}},
    {"lavenderblush", {255, 240, 245}},
    {"lawngreen", {124, 252, 0}},
    {"lemonchiffon", {255, 250, 205}},
    {"lightblue", {173, 216, 230}},
    {"lightcoral", {240, 128, 128}},
    {"lightcyan", {224, 255, 255}},
    {"lightgoldenrodyellow", {250, 250, 210}},
    {"lightgray", {211, 211, 211}},
    {"lightgreen", {144, 238, 144}},
    {"lightgrey", {211, 211, 211}},
    {"lightpink", {255, 182, 193}},
    {"lightsalmon", {255, 160, 122}},
    {"lightseagreen", {32, 178, 170}},
    {"lightskyblue", {135, 206, 250}},
    {"lightslategray", {119, 136, 153}},
    {"lightslategrey", {119, 136, 153}},
    {"lightsteelblue", {176, 196, 222}},
    {"lightyellow", {255, 255, 224}},
    {"lime", {0, 255, 0}},
    {"limegreen", {50, 205, 50}},
    {"linen", {250, 240, 230}},
    {"magenta", {255, 0, 255}},
    {"maroon", {128, 0, 0}},
    {"mediumaquamarine", {102, 205, 170}},
    {"mediumblue", {0, 0, 205}},
    {"mediumorchid", {186, 85, 211}},
    {"mediumpurple", {147, 112, 219}},
    {"mediumseagreen", {60, 179, 113}},
    {"mediumslateblue", {123, 104, 238}},
    {"mediumspringgreen", {0, 250, 154}},
    {"mediumturquoise", {72, 209, 204}},
    {"mediumvioletred", {199, 21, 133}},
    {"midnightblue", {25, 25, 112}},
    {"mintcream", {245, 255, 250}},
    {"mistyrose", {255, 228, 225}},
    {"moccasin", {255, 228, 181}},
    {"navajowhite", {255, 222, 173}},
    {"navy", {0, 0, 128}},
    {"oldlace", {253, 245, 230}},
    {"olive", {128, 128, 0}},
    {"olivedrab", {107, 142, 35}},
    {"orange", {255, 165, 0}},
    {"orangered", {255, 69, 0}},
    {"orchid", {218, 112, 214}},
    {"palegoldenrod", {238, 232, 170}},
    {"palegreen", {152, 251, 152}},
    {"paleturquoise", {175, 238, 238}},
    {"palevioletred", {219, 112, 147}},
    {"papayawhip", {255, 239, 213}},
    {"peachpuff", {255, 218, 185}},
    {"peru", {205, 133, 63}},
    {"pink", {255, 192, 203}},
    {"plum", {221, 160, 221}},
    {"powderblue", {176, 224, 230}},
    {"purple", {128, 0, 128}},
    {"rebeccapurple", {102, 51, 153}},
    {"red", {255, 0, 0}},
    {"rosybrown", {188, 143, 143}},
    {"royalblue", {65, 105, 225}},
    {"saddlebrown", {139, 69, 19}},
    {"salmon", {250, 128, 114}},
    {"sandybrown", {244, 164, 96}},
    {"seagreen", {46, 139, 87}},
    {"seashell", {255, 245, 238}},
    {"sienna", {160, 82, 45}},
    {"silver", {192, 192, 192}},
    {"skyblue", {135, 206, 235}},
    {"slateblue", {106, 90, 205}},
    {"slategray", {112, 128, 144}},
    {"slategrey", {112, 128, 144}},
    {"snow", {255, 250, 250}},
    {"springgreen", {0, 255, 127}},
    {"steelblue", {70, 130, 180}},
    {"tan", {210, 180, 140}},
    {"teal", {0, 128, 128}},
    {"thistle", {216, 191, 216}},
    {"tomato", {255, 99, 71}},
    {"turquoise", {64, 224, 208}},
    {"violet", {238, 130, 238}},
    {"wheat", {245, 222, 179}},
    {"white", {255, 255, 255}},
    {"whitesmoke", {245, 245, 245}},
    {"yellow", {255, 255, 0}},
    {"yellowgreen", {154, 205, 50}},
    // clang-format on
};

auto createMarkerParseConfig() -> pattern::ParseConfig
{
    return {
        // clang-format off
        .AllowedSegments = {
            pattern::SegmentKind::Literal,
            pattern::SegmentKind::ForegroundColorStart,
            pattern::SegmentKind::BackgroundColorStart,
            pattern::SegmentKind::LevelColorStart,
            pattern::SegmentKind::Bold,
            pattern::SegmentKind::Dim,
            pattern::SegmentKind::Italic,
            pattern::SegmentKind::Underline,
            pattern::SegmentKind::Strikethrough,
            pattern::SegmentKind::StyleEnd,
        },
        .RelaxedStyleNesting = true
        // clang-format on
    };
}

constexpr auto toLowerAscii(char ch) noexcept -> char
{
    return (ch >= 'A' && ch <= 'Z') ? static_cast<char>(ch + 32) : ch;
}

constexpr auto equalsIgnoreCase(Str lhs, Str rhs) noexcept -> bool
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (UInt index = 0; index < lhs.size(); ++index) {
        if (toLowerAscii(lhs[index]) != toLowerAscii(rhs[index])) {
            return false;
        }
    }

    return true;
}

auto parseHexDigit(char ch) -> anyhow::Result<UInt8>
{
    if (ch >= '0' && ch <= '9') {
        return static_cast<UInt8>(ch - '0');
    }

    if (ch >= 'a' && ch <= 'f') {
        return static_cast<UInt8>(10 + (ch - 'a'));
    }

    if (ch >= 'A' && ch <= 'F') {
        return static_cast<UInt8>(10 + (ch - 'A'));
    }

    return Err(ANYHOW_FMT("character '{}' is not allowed", ch));
}

auto parseColorSpec(Str spec) -> anyhow::Result<terminal::RGB>
{
    auto trimmed = strings::Trim(spec, [](auto ch) -> bool { return ch == ' ' || ch == '\t'; });
    ENSURE(!trimmed.empty(), "empty colour spec provided");

    if (trimmed.front() == '#') {
        auto hex = trimmed.substr(1);
        if (hex.size() == 6) {
            const auto [redHi, redLo]
                = std::make_pair(VIOLET_TRY(parseHexDigit(hex[0])), VIOLET_TRY(parseHexDigit(hex[1])));

            const auto [greenHi, greenLo]
                = std::make_pair(VIOLET_TRY(parseHexDigit(hex[2])), VIOLET_TRY(parseHexDigit(hex[3])));

            const auto [blueHi, blueLo]
                = std::make_pair(VIOLET_TRY(parseHexDigit(hex[4])), VIOLET_TRY(parseHexDigit(hex[5])));

            return terminal::RGB(
                // clang-format off
                static_cast<UInt8>((redHi << 4) | redLo),
                static_cast<UInt8>((greenHi << 4) | greenLo),
                static_cast<UInt8>((blueHi << 4) | blueLo)
                // clang-format on
            );
        }

        if (hex.size() == 3) {
            const auto red = VIOLET_TRY(parseHexDigit(hex[0]));
            const auto green = VIOLET_TRY(parseHexDigit(hex[1]));
            const auto blue = VIOLET_TRY(parseHexDigit(hex[2]));

            return terminal::RGB(
                // clang-format off
                static_cast<UInt8>((red << 4) | red),
                static_cast<UInt8>((green << 4) | green),
                static_cast<UInt8>((blue << 4) | blue)
                // clang-format on
            );
        }

        return Err(ANYHOW("hexadecimal specification must be in #RGB or #RRGGBB"));
    }

    if (contains(trimmed, ',')) {
        Slice<UInt8, 3> components;
        UInt32 count = 0;
        UInt32 cursor = 0;

        while (cursor <= trimmed.size() && count < 3) {
            // trim spaces or tabs
            while (cursor < trimmed.size() && (trimmed[cursor] == ' ' || trimmed[cursor] == '\t')) {
                ++cursor;
            }

            UInt16 value = 0;
            bool hasDigits = false;
            while (cursor < trimmed.size() && trimmed[cursor] >= '0' && trimmed[cursor] <= '9') {
                value = static_cast<UInt16>((value * 10) + (trimmed[cursor] - '0'));
                hasDigits = true;
                ++cursor;
            }

            if (!hasDigits) {
                return Err(ANYHOW_FMT("expected numeric value in RGB spec [{}]", spec));
            }

            if (value > 255) {
                return Err(ANYHOW("RGB component out of range (0..=255)"));
            }

            components.Emplace(static_cast<UInt8>(value));
            ++count;

            // trim spaces or tabs
            while (cursor < trimmed.size() && (trimmed[cursor] == ' ' || trimmed[cursor] == '\t')) {
                ++cursor;
            }

            if (cursor < trimmed.size() && trimmed[cursor] == ',') {
                ++cursor;
            } else {
                break;
            }
        }

        if (count != 3) {
            return Err(ANYHOW_FMT("RGB spec requires exactly 3 components [{}]", spec));
        }

        while (cursor < trimmed.size() && (trimmed[cursor] == ' ' || trimmed[cursor] == '\t')) {
            ++cursor;
        }

        if (cursor != trimmed.size()) {
            return Err(ANYHOW("unexpected trailing content in RGB spec"));
        }

        return terminal::RGB(components[0], components[1], components[2]);
    }

    for (const auto& [name, rgb]: kCSSColours) {
        if (equalsIgnoreCase(trimmed, name)) {
            return rgb;
        }
    }

    return Err(ANYHOW_FMT("unknown color spec: [{}]", spec));
}

auto getLevelColor(LogLevel level, const LevelColourPalette& palette) noexcept -> terminal::RGB
{
    return palette.Iter()
        .Find([level](LevelColourPalette::value_type kv) -> bool { return kv.first == level; })
        .Map([](LevelColourPalette::value_type kv) -> terminal::RGB { return kv.second; })
        .UnwrapOrDefault();
}

auto isColoursEnabled(const Optional<bool>& useColours) -> bool
{
    return useColours.MapOr(terminal::ColoursEnabled(), std::identity{});
}

auto validateColorSpecs(const pattern::ParsedPattern& parsed) -> anyhow::Result<void>
{
    for (UInt index = 0; index < parsed.Count; ++index) {
        const auto& segment = parsed.Segments[index];
        const bool requiresSpec = segment.Kind == pattern::SegmentKind::ForegroundColorStart
            || segment.Kind == pattern::SegmentKind::BackgroundColorStart;

        const bool optionalSpec = segment.Kind == pattern::SegmentKind::LevelColorStart;

        if (!requiresSpec && !optionalSpec) {
            continue;
        }

        if (!segment.Directive || !segment.Directive->HasSpecification()) {
            continue;
        }

        auto spec = segment.Directive->Specification(parsed.Pattern);
        VIOLET_TRY(parseColorSpec(spec));
    }

    return {};
}

void appendAttributeValue(String& out, AttributeValue value)
{
    value.Match(
        // clang-format off
        [](Mono)                    -> void {},
        [&out](bool value)          -> void { out.append(value ? "true" : "false"); },
        [&out](Int64 value)         -> void { out.append(violet::ToString(value)); },
        [&out](UInt64 value)        -> void { out.append(violet::ToString(value)); },
        [&out](double value)        -> void { out.append(violet::ToString(value)); },
        [&out](const String& value) -> void { out.append(value); }
        // clang-format on
    );
}

} // namespace

auto Pattern::Parse(Str input) -> anyhow::Result<Pattern>
{
    return Parse(input, Config{});
}

auto Pattern::Parse(Str input, Config config) -> anyhow::Result<Pattern>
{
    auto parsed = VIOLET_TRY(pattern::Parse(input));
    VIOLET_TRY_VOID(validateColorSpecs(parsed));

    Optional<pattern::ParsedPattern> prefix;
    if (!config.Markers.first.empty()) {
        auto result = pattern::Parse(config.Markers.first, createMarkerParseConfig());
        if (result.Err()) {
            return Err(ANYHOW("while parsing marker prefix").Context(VIOLET_MOVE(result).Err()));
        }

        VIOLET_TRY_VOID(validateColorSpecs(*result));
        prefix = VIOLET_MOVE(result.Value());
    }

    Optional<pattern::ParsedPattern> suffix;
    if (!config.Markers.second.empty()) {
        auto result = pattern::Parse(config.Markers.second, createMarkerParseConfig());
        if (result.Err()) {
            return Err(ANYHOW("while parsing suffix prefix").Context(VIOLET_MOVE(result).Err()));
        }

        VIOLET_TRY_VOID(validateColorSpecs(*result));
        suffix = VIOLET_MOVE(result.Value());
    }

    return Pattern(VIOLET_MOVE(parsed), VIOLET_MOVE(config), VIOLET_MOVE(prefix), VIOLET_MOVE(suffix));
}

auto Pattern::Azalia() -> Pattern
{
    constexpr auto pattern = "%fg{#868686}[%t{%B %d, %G - %H:%M:%S %p}]%style:end %levelColor[%L{>5}]%style:end "
                             "%fg{#483D8B}«%n [thread %T{hex}]»%style:end  %m    %A";

    Pattern::Config config;
    config.LevelColorMapping = kAzaliaLevelColorPalette;
    config.Markers.first = "%fg{34,34,34}%bold[";
    config.Markers.second = "]%style:end";
    config.Delimiter = "=";

    return Pattern::Parse(pattern, VIOLET_MOVE(config)).Unwrap();
}

auto Pattern::Format(const Record& record) const -> String
{
    String out;
    formatPattern(this->n_parsed, record, out);

    return out;
}

void Pattern::formatPattern(const pattern::ParsedPattern& pattern, const Record& record, String& out) const
{
    for (UInt index = 0; index < pattern.Count; ++index) {
        formatSegment(pattern, pattern.Segments[index], record, out);
    }
}

void Pattern::formatSegment(
    const pattern::ParsedPattern& pattern, const pattern::Segment& segment, const Record& record, String& out) const
{
    switch (segment.Kind) {
    case pattern::SegmentKind::Literal:
        pattern.FormatLiteral(segment, record, out);
        break;

    case pattern::SegmentKind::Timestamp:
        pattern.FormatTimestamp(segment, record, out);
        break;

    case pattern::SegmentKind::Level:
        pattern.FormatLevel(segment, record, out);
        break;

    case pattern::SegmentKind::Message:
        pattern.FormatMessage(segment, record, out);
        break;

    case pattern::SegmentKind::Logger:
        pattern.FormatLogger(segment, record, out);
        break;

    case pattern::SegmentKind::File:
        pattern.FormatFile(segment, record, out);
        break;

    case pattern::SegmentKind::FunctionName:
        pattern.FormatFunctionName(segment, record, out);
        break;

    case pattern::SegmentKind::LineNum:
        pattern.FormatLine(segment, record, out);
        break;

    case pattern::SegmentKind::ColumnNum:
        pattern.FormatColumn(segment, record, out);
        break;

    case pattern::SegmentKind::ThreadId:
        pattern.FormatThreadId(segment, record, out);
        break;

    case pattern::SegmentKind::Attribute:
        formatAttribute(pattern, segment, record, out);
        break;

    case pattern::SegmentKind::AllAttributes:
        this->formatAllAttributes(record, out);
        break;

    case pattern::SegmentKind::ForegroundColorStart:
        emitColour(pattern, segment, /*foreground=*/true, out);
        break;

    case pattern::SegmentKind::BackgroundColorStart:
        emitColour(pattern, segment, /*foreground=*/false, out);
        break;

    case pattern::SegmentKind::LevelColorStart:
        emitLevelColour(pattern, segment, record, out);
        break;

    case pattern::SegmentKind::Bold:
    case pattern::SegmentKind::Dim:
    case pattern::SegmentKind::Italic:
    case pattern::SegmentKind::Underline:
    case pattern::SegmentKind::Strikethrough:
        emitStyle(segment.Kind, out);
        break;

    case pattern::SegmentKind::StyleEnd:
        emitStyleEnd(out);
        break;
    }
}

void Pattern::formatAttribute(
    const pattern::ParsedPattern& pattern, const pattern::Segment& segment, const Record& record, String& out)
{
    if (!segment.Directive.HasValue() || !segment.Directive->HasSpecification()) {
        return;
    }

    auto key = segment.Directive->Specification(pattern.Pattern);
    if (auto ent = record.Fields.Get(key); ent.HasValue()) {
        appendAttributeValue(out, *ent);
    }
}

void Pattern::formatAllAttributes(const Record& record, String& out) const
{
    if (record.Fields.Empty()) {
        return;
    }

    bool first = true;
    for (const auto& [key, value]: record.Fields) {
        if (!first) {
            out.append(" ");
        }

        first = false;
        if (this->n_prefixMarker) {
            formatPattern(this->n_prefixMarker.Value(), record, out);
        } else {
            out.append(this->n_config.Markers.first);
        }

        out.append(key);
        out.append(this->n_config.Delimiter);
        appendAttributeValue(out, value);

        if (this->n_suffixMarker) {
            formatPattern(this->n_suffixMarker.Value(), record, out);
        } else {
            out.append(this->n_config.Markers.second);
        }
    }
}

void Pattern::emitColour(
    const pattern::ParsedPattern& pattern, const pattern::Segment& segment, bool foreground, String& out) const
{
    if (!isColoursEnabled(this->n_config.UseColours)) {
        return;
    }

    if (!segment.Directive.HasValue() || !segment.Directive->HasSpecification()) {
        return;
    }

    auto spec = segment.Directive->Specification(pattern.Pattern);
    auto rgb = parseColorSpec(spec).Unwrap();
    rgb.Foreground = foreground;
    out.append(rgb.Paint());
}

void Pattern::emitLevelColour(
    const pattern::ParsedPattern& pattern, const pattern::Segment& segment, const Record& record, String& out) const
{
    if (!isColoursEnabled(this->n_config.UseColours)) {
        return;
    }

    terminal::RGB rgb;
    if (segment.Directive.HasValue() && segment.Directive->HasSpecification()) {
        auto spec = segment.Directive->Specification(pattern.Pattern);
        rgb = parseColorSpec(spec).Unwrap();
    } else {
        const auto palette = this->n_config.LevelColorMapping.UnwrapOr(kDefaultLevelColorPalette);
        rgb = getLevelColor(record.Level, palette);
    }

    rgb.Foreground = true;
    out.append(rgb.Paint());
}

void Pattern::emitStyle(pattern::SegmentKind kind, String& out) const
{
    if (!isColoursEnabled(this->n_config.UseColours)) {
        return;
    }

    terminal::Style style;
    switch (kind) {
    case pattern::SegmentKind::Bold:
        style.Bold();
        break;

    case pattern::SegmentKind::Dim:
        style.Dim();
        break;

    case pattern::SegmentKind::Italic:
        style.Italic();
        break;

    case pattern::SegmentKind::Underline:
        style.Underline();
        break;

    case pattern::SegmentKind::Strikethrough:
        style.Strikethrough();
        break;

    default:
        return;
    }

    out.append(style.Paint());
}

void Pattern::emitStyleEnd(String& out) const
{
    if (!isColoursEnabled(this->n_config.UseColours)) {
        return;
    }

    out.append(terminal::Style{}.Paint());
}

} // namespace violet::experimental::log::formatter
