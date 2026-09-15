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

#include <violet/Experimental/Logging/Formatters/Pattern/Parser.h>
#include <violet/Strings.h>

#include <algorithm>
#include <thread>

namespace violet::experimental::log::formatter::pattern {
namespace {
struct NamedColour final {
    Str Name;
    struct terminal::RGB RGB;
};

struct Keyword final {
    Str Name;
    SegmentKind Kind;
};

inline constexpr Slice<Keyword, 9> kKeywords = {
    Keyword{.Name = "strikethrough", .Kind = SegmentKind::Strikethrough},
    {.Name = "levelColor", .Kind = SegmentKind::LevelColorStart},
    {.Name = "style:end", .Kind = SegmentKind::StyleEnd},
    {.Name = "underline", .Kind = SegmentKind::Underline},
    {.Name = "italic", .Kind = SegmentKind::Italic},
    {.Name = "bold", .Kind = SegmentKind::Bold},
    {.Name = "dim", .Kind = SegmentKind::Dim},
    {.Name = "fg", .Kind = SegmentKind::ForegroundColorStart},
    {.Name = "bg", .Kind = SegmentKind::BackgroundColorStart},
};

auto logLevelToString(LogLevel level, Str spec) -> String
{
    auto toUpper = [](char ch) -> char { return static_cast<char>(::toupper(static_cast<unsigned char>(ch))); };
    auto toLower = [](char ch) -> char { return static_cast<char>(::tolower(static_cast<unsigned char>(ch))); };

    auto levelStr = violet::ToString(level);
    std::ranges::transform(levelStr, levelStr.begin(), spec == "upper" ? toUpper : toLower);

    return levelStr;
}

auto isWordBoundary(Str rest, UInt end) -> bool
{
    if (end >= rest.size()) {
        return true;
    }

    char ch = rest[end];
    return ch == '{' || ch == '%' || ch == ' ' || ch == '\t' || ch == '\n' || ch == '[' || ch == ']' || ch == '('
        || ch == ')' || ch == '}' || ch == ':';
}

auto tryMatchKeyword(Str rest) -> Pair<UInt32, SegmentKind>
{
    for (const auto& kw: kKeywords) {
        if (rest.starts_with(kw.Name) && isWordBoundary(rest, kw.Name.size())) {
            return {static_cast<UInt32>(kw.Name.size()), kw.Kind};
        }
    }

    return {0, SegmentKind::Literal};
}

auto isStyleStart(SegmentKind kind) -> bool
{
    switch (kind) {
    case SegmentKind::ForegroundColorStart:
    case SegmentKind::BackgroundColorStart:
    case SegmentKind::LevelColorStart:
    case SegmentKind::Bold:
    case SegmentKind::Dim:
    case SegmentKind::Italic:
    case SegmentKind::Underline:
    case SegmentKind::Strikethrough:
        return true;

    default:
        return false;
    }
}

void applyPadding(Str content, const Directive& directive, String& out)
{
    if (!directive.HasPadding() || content.size() >= directive.Width) {
        out.append(content);
        return;
    }

    auto total = directive.Width - static_cast<UInt16>(content.size());
    switch (directive.Alignment) {
    case Alignment::Left:
        out.append(content);
        out.append(total, directive.Fill);
        break;

    case Alignment::Right:
        out.append(total, directive.Fill);
        out.append(content);
        break;

    case Alignment::Center: {
        auto left = total / 2;
        auto right = total - left;

        out.append(left, directive.Fill);
        out.append(content);
        out.append(right, directive.Fill);
    } break;

    default:
        return;
    }
}

} // namespace

auto Parse(Str input, const ParseConfig& config) noexcept -> anyhow::Result<ParsedPattern>
{
    ParsedPattern result;
    result.Pattern = input;

    UInt32 in = 0;
    UInt32 literalStart = 0;
    bool inLiteral = false;
    UInt8 depth = 0;
    auto len = static_cast<UInt32>(input.size());

    auto flushLiteral = [&](UInt32 end) -> anyhow::Result<void> {
        if (inLiteral && end > literalStart) {
            if (result.Count >= result.Segments.Size()) {
                return Err(ANYHOW_FMT("pattern exceeded maximum segment count ({})", result.Segments.Size()));
            }

            result.Segments.Emplace(Segment{
                // clang-format off
                .Kind = SegmentKind::Literal,
                .Text = {static_cast<UInt16>(literalStart), static_cast<UInt64>(end - literalStart)},
                .Directive = Nothing,
                .Depth = depth
                // clang-format on
            });

            ++result.Count;
        }

        inLiteral = false;
        return {};
    };

    while (in < len) {
        if (input[in] != '%' || in + 1 >= len) {
            if (!inLiteral) {
                literalStart = in;
                inLiteral = true;
            }

            ++in;
            continue;
        }

        VIOLET_TRY_VOID(flushLiteral(in));

        auto rest = input.substr(in + 1);
        auto [kwLen, kwKind] = tryMatchKeyword(rest);

        constexpr auto kInvalidSegment = std::numeric_limits<std::underlying_type_t<SegmentKind>>::max();
        auto kind = static_cast<SegmentKind>(kInvalidSegment);
        if (kwLen > 0) {
            kind = kwKind;
            in += 1 + kwLen;
        } else {
            char spec = input[in + 1];
            in += 2;

            switch (spec) {
            case 'L':
                kind = SegmentKind::Level;
                break;

            case 'm':
                kind = SegmentKind::Message;
                break;

            case 'T':
                kind = SegmentKind::ThreadId;
                break;

            case 't':
                kind = SegmentKind::Timestamp;
                break;

            case 'n':
                kind = SegmentKind::Logger;
                break;

            case 'f':
                kind = SegmentKind::File;
                break;

            case 'l':
                kind = SegmentKind::LineNum;
                break;

            case 'c':
                kind = SegmentKind::ColumnNum;
                break;

            case 'F':
                kind = SegmentKind::FunctionName;
                break;

            case 'A':
                kind = SegmentKind::AllAttributes;
                break;

            case 'X':
                kind = SegmentKind::Attribute;
                break;

            case '%': {
                if (result.Count >= result.Segments.Size()) {
                    return Err(ANYHOW_FMT("pattern exceeded maximum segment count ({})", result.Segments.Size()));
                }

                result.Segments.Emplace(Segment{
                    // clang-format off
                    .Kind = SegmentKind::Literal,
                    .Text = {static_cast<UInt16>(in - 1), 1},
                    .Directive = Nothing,
                    .Depth = depth
                    // clang-format on
                });

                ++result.Count;
                literalStart = in;
                continue;
            }

            default:
                return Err(ANYHOW_FMT("unknown format directive: [%{}]", spec));
            }
        }

        if (static_cast<std::underlying_type_t<SegmentKind>>(kind) == kInvalidSegment) {
            return Err(ANYHOW_FMT("unknown or unhandled segment reached"));
        }

        if (!config.AllowedSegments.Empty() && !config.AllowedSegments.Contains(kind)) {
            return Err(ANYHOW_FMT("segment [{}] is not allowed", kind));
        }

        Optional<Directive> directive;
        if (in < len && input[in] == '{') {
            UInt32 open = in + 1;
            UInt32 braceDepth = 1;
            UInt32 ending = open;

            while (ending < len && braceDepth > 0) {
                if (input[ending] == '{') {
                    ++braceDepth;
                } else if (input[ending] == '}') {
                    --braceDepth;
                }

                ++ending;
            }

            if (braceDepth != 0) {
                return Err(ANYHOW("unterminated `{' in format directive"));
            }

            UInt32 close = ending - 1;
            auto body = input.substr(open, close - open);
            in = ending;

            directive = VIOLET_TRY(Directive::Parse(body, open, kind));
        }

        if (auto cap = GetSegmentCapability(kind);
            cap == SegmentCapability::SpecOnly || kind == SegmentKind::Attribute) {
            if (!directive.HasValue() || !directive->HasSpecification()) {
                if (kind == SegmentKind::Attribute) {
                    return Err(ANYHOW("`%X` format directive requires a key spec (e.g. `%X{request.id}`)"));
                }

                if (kind == SegmentKind::ForegroundColorStart || kind == SegmentKind::BackgroundColorStart) {
                    return Err(ANYHOW("%fg/%bg requires a spec (e.g., `%fg{red}`)"));
                }
            }
        }

        if (isStyleStart(kind)) {
            ++depth;
        } else if (kind == SegmentKind::StyleEnd) {
            if (depth == 0) {
                if (!config.RelaxedStyleNesting) {
                    return Err(ANYHOW("`%style:end` with no matching style directive"));
                }
            } else {
                --depth;
            }
        }

        if (result.Count >= result.Segments.Size()) {
            return Err(ANYHOW_FMT("pattern exceeded maximum segment count ({})", result.Segments.Size()));
        }

        result.Segments.Emplace(Segment{
            // clang-format off
            .Kind = kind,
            .Text = {0, 0},
            .Directive = VIOLET_MOVE(directive),
            .Depth = depth
            // clang-format on
        });

        ++result.Count;
        literalStart = in;
    }

    VIOLET_TRY_VOID(flushLiteral(len));
    if (depth != 0 && !config.RelaxedStyleNesting) {
        return Err(ANYHOW_FMT("pattern has {} unclosed style directive(s); missing `%style:end`", depth));
    }

    return result;
}

void ParsedPattern::FormatSegments(const Record& record, String& out) const
{
    for (UInt i = 0; i < this->Count; ++i) {
        this->FormatSegment(this->Segments[i], record, out);
    }
}

#define FORMAT(Name)                                                                                                   \
    void ParsedPattern::VIOLET_CONCAT(Format, Name)(                                                                   \
        [[maybe_unused]] const Segment& segment, [[maybe_unused]] const Record& record, String& out) const

FORMAT(Segment)
{
#define dispatch(kind)                                                                                                 \
    case SegmentKind::kind:                                                                                            \
        this->VIOLET_CONCAT(Format, kind)(segment, record, out);                                                       \
        break

#define dispatch1(kind, fn)                                                                                            \
    case SegmentKind::kind:                                                                                            \
        this->VIOLET_CONCAT(Format, fn)(segment, record, out);                                                         \
        break

    switch (segment.Kind) {
        dispatch(Literal);
        dispatch(Timestamp);
        dispatch(Message);
        dispatch(Logger);
        dispatch(File);
        dispatch(FunctionName);
        dispatch1(LineNum, Line);
        dispatch1(ColumnNum, Column);
        dispatch(ThreadId);
        dispatch(Attribute);

    default:
        return;
    }

#undef dispatch
#undef dispatch1
}

FORMAT(Literal)
{
    const auto [offset, len] = segment.Text;
    out.append(this->Pattern.substr(offset, len));
}

FORMAT(Timestamp)
{
    Str spec;
    if (segment.Directive.HasValue() && segment.Directive->HasSpecification()) {
        spec = segment.Directive->Specification(this->Pattern);
    }

    if (spec.empty()) {
        spec = "%Y-%m-%d %H:%M:%S";
    }

    if (spec == "iso8601") {
        out.append(record.Timestamp.IntoISO8601());
        return;
    }

    const auto tt = chrono::TimePoint::clock_type::to_time_t(record.Timestamp.ToStd());
    struct tm tm{};
    ::localtime_r(&tt, &tm);

    String specStr(spec);
    Slice<char, 128> buf{};

    auto length = ::strftime(buf.Data(), buf.Size(), specStr.c_str(), &tm);
    VIOLET_ASSERT(length > 0, "timestamp format specification produced output longer than 128 bytes");

    out.append({buf.Data(), length});
}

FORMAT(Level)
{
    Str spec;
    if (segment.Directive && segment.Directive->HasSpecification()) {
        spec = segment.Directive->Specification(this->Pattern);
    }

    auto text = logLevelToString(record.Level, spec);
    if (segment.Directive && segment.Directive->HasPadding()) {
        applyPadding(text, *segment.Directive, out);
    } else {
        out.append(text);
    }
}

FORMAT(Message)
{
    out.append(record.Message);
}

FORMAT(Logger)
{
    if (segment.Directive && segment.Directive->HasPadding()) {
        applyPadding(record.Logger, segment.Directive.Value(), out);
    } else {
        out.append(record.Logger);
    }
}

FORMAT(File)
{
    out.append(record.Location.File);
}

FORMAT(FunctionName)
{
    out.append(record.Location.Function);
}

FORMAT(Line)
{
    out.append(violet::ToString(record.Location.Line));
}

FORMAT(Column)
{
    out.append(violet::ToString(record.Location.Column));
}

FORMAT(ThreadId)
{
    Str spec;
    if (segment.Directive && segment.Directive->HasSpecification()) {
        spec = segment.Directive->Specification(this->Pattern);
    }

    constexpr std::hash<std::thread::id> hasher{};
    auto id = std::this_thread::get_id();
    if (spec == "hex") {
        out.append(std::format("0x{:x}", hasher(id)));

        return;
    }

    out.append(std::format("{:d}", hasher(id)));
}

FORMAT(Attribute)
{
    if (!segment.Directive || !segment.Directive->HasSpecification()) {
        return;
    }

    auto key = segment.Directive->Specification(this->Pattern);
    auto ent = record.Fields.Get(key);
    if (!ent.HasValue()) {
        return;
    }

    ent->Match(
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

} // namespace violet::experimental::log::formatter::pattern
