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
//
//! # 🌺💜 `violet/Experimental/Logging/Formatters/Pattern/Segment.h`

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Container/Result.h>
#include <violet/anyhow.h>

namespace violet::experimental::log::formatter::pattern {

/// Controls a directive's output alignment when padded to a minimum width.
///
/// ## Example
/// Given `"INFO"` as the example string (4 characters) with width `8` and default fill (`' '`)
///
/// - [`Alignment::Left`] (`<`):  `"INFO    "`
/// - [`Alignment::Right`] (`>`): `"    INFO"`
/// - [`Alignment::Center] (`^`): `"  INFO  "`
/// - [`Alignment::None`]:        `"INFO"`
enum struct NOELDOC_EXPERIMENTAL_SINCE("current") Alignment : UInt8 {
    /// No alignment was specified. Padding is not applied.
    None,

    /// Content is centered, padding split evenly on both sides. Selected by `^`.
    /// When the total padding is odd, the extra character goes on the right.
    Center,

    /// Content is right-aligned, padding on the left. Selected by `>`.
    Right,

    /// Content is left-aligned, padding on the right. Selected by `<`.
    Left
};

/// Representation of a [`Segment`].
///
/// [`SegmentKind::Literal`] contains verbatim text copied to the output unchanged. All other
/// variants correspond to a `%`-prefixed directive resolved at format time from the log record
/// or terminal state.
///
/// [`Directive`]s fall into four capability groups that determine their optional specification (`{...}`)
/// is parsed. You can use [`GetSegmentCapability`] for all the capabilities.
enum struct NOELDOC_EXPERIMENTAL_SINCE("current") SegmentKind : UInt8 {
    /// Verbatim text from the pattern string. The [`Text`][Segment::Text] on the parent [`Segment`]
    /// delimits the literal span.
    Literal,

    /// `%t` **<~>** timestamp at which the log record was created.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::SpecOnly`] **-** The specification is intepreted as a [`strftime`]-compatible
    /// format string. If omitted, it'll default to `%Y-%m-%d %H:%M:%S`. The special value `iso8601` uses
    /// a fast path for ISO 8601 output.
    ///
    /// Padding is not supported because timestamp output is typically
    /// fixed-width for a given format string.
    ///
    /// ## Examples
    /// - `%t`           **~** `"2026-05-04 14:32:01"`
    /// - `%t{%H:%M:%S}` **~** `"14:32:01"`
    /// - `%t{iso8601}`  **~** `"2026-05-04T14:32:01Z"`
    Timestamp,

    /// `%L` **<~>** log level (e.g., `INFO`, `WARN`)
    ///
    /// ## Capabilities
    /// [`SegmentCapability::SpecAndPadding`]
    ///
    /// ### Specification
    /// - `upper`   **~** uppercase, e.g., `"INFO"`
    /// - `lower`   **~** lowercase, e.g., `"info"`
    /// - `default` **~** the default specification
    ///
    /// ## Examples
    /// - `%L`           **~** `"Info"`
    /// - `%L{<5}`       **~** `"Info "`
    /// - `%L{lower}`    **~** `"info"`
    /// - `%L{upper:<5}` **~** `"INFO "`
    Level,

    /// `%m` **<~>** the log message body.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    Message,

    /// `%n` **<~>** the logger name that produced this record.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::PaddingOnly`]
    ///
    /// ## Examples
    /// - `%n`      **~** `"eousd"`
    /// - `%n{<10}` **~** `"eousd     "`
    Logger,

    /// `%f` **<~>** the source file (either absolute or relative) to where the log record
    /// was produced.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    File,

    /// `%F` **<~>** the function name that was resolved.
    ///
    /// > [!CAUTION]
    /// > **Violet** uses [`violet::SourceLocation`] to resolve source location by default, which is
    /// > compiler-specific. Custom [`SourceLocation`]s can also use their own function names,
    /// > so this is left verbatim and not touched.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    FunctionName,

    /// `%l` **<~>** the line number where the log record was produced. resolves to zero if
    /// not available.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    LineNum,

    /// `%c` **<~>** the column number where the log record was produced. resolves to zero if
    /// not available.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    ColumnNum,

    /// `%T` **<~>** the current thread identifier from [`std::this_thread::get_id()`].
    ///
    /// ## Capabilities
    /// [`SegmentCapability::SpecAndPadding`]
    ///
    /// ### Specifications
    /// - `dec` (default) **~** decimal representation
    /// - `hex`           **~** hexadecimal representation
    ///
    /// ## Examples
    /// - `%T`        **~** `"12345"`
    /// - `%T{hex}`   **~** `"0x12345"`
    /// - `%T{hex:08} **~** `"0000303a"`
    ThreadId,

    /// `%A` **<~>** all key-value attributes attached to the log record.
    ///
    /// You can set the prefix and suffix markers with [`Pattern::Config::Markers`] (first = prefix, second = suffix).
    /// And, you can set the delimiter where `key` and `value` will be placed, i.e, the `=` in `key=value` with
    /// [`Pattern::Config::Delimiter`].
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    AllAttributes,

    /// `%X{key}` **<~>** a single attribute value looked up by key name.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::SpecOnly`]
    ///
    /// ## Examples
    /// - `request.id=%X{request.id}` **~** `request.id=abc-123` (if present)
    /// - `request.id=%X{request.id}` **~** `request.id=`        (if not present)
    Attribute,

    /// `%fg{...}` **<~>** begin foreground colour output.
    ///
    /// If you use the framework's choice, you can set the colour choice with [`terminal::SetColourChoice()`] at
    /// application startup or use [`terminal::ColourChoice`] in your CLI applications to allow flexibility.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::SpecOnly`]
    ///
    /// ## Examples
    /// - `%fg{red}`              **~** can be any [CSS color value] variant.
    /// - `%fg{#c0ffee}`          **~** hexadecimal representation
    /// - `%fg{red, blue, green}` **~** `(red, blue, green)` representation in whole numbers from `0..=255`
    ///
    /// [CSS color value]: https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/named-color
    ForegroundColorStart,

    /// `%bg{...}` **<~>** begin background colour output.
    ///
    /// Look into [`SegmentKind::ForegroundColorStart`] for more information.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::SpecOnly`]
    BackgroundColorStart,

    /// `%levelColor` **<~>** begin a color automatically derived from the log level.
    ///
    /// The level colours can be customized by the specification itself or from [`Pattern::Config::LevelColorMap`],
    /// which accepts [`Array`]\<[`Pair`]\<[`LogLevel`], [`terminal::RGB`]\>, `5`\>.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::SpecOnly`]
    ///
    /// ### Specification
    /// It can be any value that [`SegmentKind::BackgroundColorStart`]/[`SegmentKind::ForegroundColorStart`] support.
    ///
    /// If not provided, it'll use the mapping from [`Pattern::Config::LevelColorMap`].
    LevelColorStart,

    /// `%bold` **<~>** enable bold text output.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    Bold,

    /// `%dim` **<~>** enable dimmed text output.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    Dim,

    /// `%italic` **<~>** enable italic text output.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    Italic,

    /// `%underline` **<~>** enable underlined text output.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    Underline,

    /// `%strikethrough` **<~>** enable strikethrough text output.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    Strikethrough,

    /// `%style:end` **<~>** reset all styling from derived segments to terminal defaults.
    ///
    /// ## Capabilities
    /// [`SegmentCapability::None`]
    ///
    /// Each [`ForegroundColorStart`], [`BackgroundColorStart`], [`LevelColorStart`], and other
    /// style directives must be followed by `%style:end` to close the styled region. Nested styling
    /// is supported, `StyleEnd` pops one level from the style stack. The [`Depth`][Segment::Depth] in
    /// a segment tracks the nesting level.
    StyleEnd
};

/// Describes the capabilities of a segment representation in its optional `{...}` block.
///
/// Each [`SegmentKind`] maps to exactly one capability, which determines how the brace content
/// is parsed by [`Directive::Parse`].
enum struct NOELDOC_EXPERIMENTAL_SINCE("current") SegmentCapability : UInt8 {
    /// No `{...}` block is accepted.
    ///
    /// Used by style directives that take no arguments and by content directives
    /// whose output format is fixed.
    ///
    /// If `{...}` is present, parsing will return an error.
    None,

    /// The entire `{...}` block is interpreted as a format spec.
    ///
    /// No padding syntax is supported. The full brace content is recorded
    /// as the spec and its interpretation depends on the segment kind.
    SpecOnly,

    /// The entire `{...}` block is interpreted as a padding spec.
    PaddingOnly,

    /// The `{...}` block is split at the last `:` into a spec and a padding
    /// descriptor.
    ///
    /// If no `:` is present, the content is disambiguated: if it starts with
    /// an alignment character (`<`, `>`, `^`), a digit, or a fill-align pair,
    /// it is treated as padding; otherwise it is treated as a spec.
    SpecAndPadding
};

/// Returns the [`SegmentCapability`] for a given [`SegmentKind`].
///
/// This determines how the optional `{...}` block following a directive is
/// parsed by [`Directive::Parse`].
constexpr auto GetSegmentCapability(SegmentKind kind) noexcept -> SegmentCapability
{
    switch (kind) {
    case SegmentKind::Literal:
    case SegmentKind::Message:
    case SegmentKind::File:
    case SegmentKind::FunctionName:
    case SegmentKind::LineNum:
    case SegmentKind::ColumnNum:
    case SegmentKind::AllAttributes:
    case SegmentKind::Bold:
    case SegmentKind::Dim:
    case SegmentKind::Italic:
    case SegmentKind::Underline:
    case SegmentKind::Strikethrough:
    case SegmentKind::StyleEnd:
        return SegmentCapability::None;

    case SegmentKind::Timestamp:
    case SegmentKind::Attribute:
    case SegmentKind::ForegroundColorStart:
    case SegmentKind::BackgroundColorStart:
    case SegmentKind::LevelColorStart:
        return SegmentCapability::SpecOnly;

    case SegmentKind::Logger:
        return SegmentCapability::PaddingOnly;

    case SegmentKind::Level:
    case SegmentKind::ThreadId:
        return SegmentCapability::SpecAndPadding;

    default:
        VIOLET_UNREACHABLE();
    }
}

/// A parsed format spec and/or padding descriptor from a directive's `{...}` block.
///
/// Not all directives accept a `{...}` block, and those that do may support only
/// a spec, only padding, or both. See [`SegmentKind`] for per-directive capabilities.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Directive final {
    /// Byte offset of the spec content within the original pattern string.
    UInt16 SpecOffset = 0;

    /// Length in bytes of the spec content. Zero means no spec was provided.
    UInt16 SpecLength = 0;

    /// Minimum output width for padding. Zero means no padding.
    UInt16 Width = 0;

    /// Alignment mode. `None` when no padding is specified.
    enum Alignment Alignment = Alignment::None;

    /// Fill character for padding. Defaults to `' '`.
    char Fill = ' ';

    /// Returns `true` if a spec was provided in the `{...}` block.
    [[nodiscard]] constexpr auto HasSpecification() const noexcept -> bool
    {
        return this->SpecLength > 0;
    }

    /// Returns a non-owning view of the directive specification.
    [[nodiscard]] constexpr auto Specification(Str pattern) const noexcept -> Str
    {
        return pattern.substr(this->SpecOffset, this->SpecLength);
    }

    /// Returns `true` if padding was specified.
    [[nodiscard]] constexpr auto HasPadding() const noexcept -> bool
    {
        return this->Alignment != Alignment::None;
    }

    /// Parses the content between `{` and `}` for a given segment kind.
    ///
    /// @param body   the raw text inside the braces
    /// @param offset byte offset of `body` within the full pattern string.
    /// @param kind   segment kind
    static auto Parse(Str body, UInt32 offset, SegmentKind kind) -> anyhow::Result<Directive>;
};

/// A single parsed segment of a log format pattern.
///
/// A pattern string like `"%fg{green}%bold[%t{%H:%M:%S}]%style:end %L{upper:<5}%end %m"`
/// is decomposed into a flat sequence of `Segment` values. Each segment is
/// either a literal text span or a format directive.
///
/// Segments reference substrings of the original pattern by offset and length
/// (via the `Text` field) rather than owning copies. The pattern string must
/// outlive any use of the parsed segments.
///
/// ## Style Nesting
/// Style directives (`ForegroundColorStart`, `BackgroundColorStart`,
/// `LevelColorStart`, `Bold`, `Dim`, `Italic`, `Underline`, `Strikethrough`) push onto a
/// logical style stack, and `StyleEnd` pops from it. The `Depth` field tracks
/// the current nesting level at each segment, enabling formatters to validate
/// balanced styling or to emit nested ANSI escape sequences correctly.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Segment final {
    /// The kind of content this segment represents.
    SegmentKind Kind;

    /// Substring span within the original pattern string, stored as
    /// `(offset, length)`.
    ///
    /// For `Literal` segments, this delimits the verbatim text to emit.
    /// For directive segments, this field is unused (both values are zero).
    Pair<UInt16, UInt16> Text;

    /// Optional format spec and/or padding descriptor parsed from the
    /// directive's `{...}` block.
    ///
    /// [`Nothing`] for `Literal` segments and for directives that were written
    /// without a `{...}` block.
    Optional<struct Directive> Directive;

    /// The style nesting depth at this segment.
    ///
    /// Incremented by style-start directives (`ForegroundColorStart`,
    /// `BackgroundColorStart`, `LevelColorStart`, `Bold`, `Dim`, `Italic`,
    /// `Underline`) and decremented by `StyleEnd`. Zero means no active
    /// styling context.
    UInt8 Depth = 0;
};

} // namespace violet::experimental::log::formatter::pattern

VIOLET_TO_STRING(violet::experimental::log::formatter::pattern::SegmentKind, sk, {
    using SK = violet::experimental::log::formatter::pattern::SegmentKind;
    switch (sk) {
    case SK::Literal:
        return "literal";

    case SK::Timestamp:
        return "timestamp (%t)";

    case SK::Level:
        return "level (%L)";

    case SK::Message:
        return "message (%m)";

    case SK::Logger:
        return "logger name (%n)";

    case SK::File:
        return "source file (%f)";

    case SK::FunctionName:
        return "function name (%F)";

    case SK::LineNum:
        return "line number (%l)";

    case SK::ColumnNum:
        return "column number (%c)";

    case SK::ThreadId:
        return "thread id (%T)";

    case SK::AllAttributes:
        return "all attributes (%A)";

    case SK::Attribute:
        return "attribute (%X)";

    case SK::ForegroundColorStart:
        return "foreground colour: start (%fg)";

    case SK::BackgroundColorStart:
        return "background colour: start (%bg)";

    case SK::LevelColorStart:
        return "level colour: start (%levelColor)";

    case SK::Bold:
        return "bold (%bold)";

    case SK::Dim:
        return "dim (%dim)";

    case SK::Italic:
        return "italic (%italic)";

    case SK::Underline:
        return "underline (%underline)";

    case SK::Strikethrough:
        return "strikethrough (%strikethrough)";

    case SK::StyleEnd:
        return "style end (%style:end)";
    }
});
