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
//! # 🌺💜 `violet/Experimental/Logging/Formatters/Pattern.h`

#pragma once

#include <violet/Container/Optional.h>
#include <violet/Experimental/Logging/Formatter.h>
#include <violet/Experimental/Logging/Formatters/Pattern/Parser.h>
#include <violet/Experimental/Logging/LogRecord.h>
#include <violet/Experimental/Slice.h>
#include <violet/Support/Terminal.h>
#include <violet/anyhow.h>

namespace violet::experimental::log::formatter {

/// [`Formatter`] driven by a parsed pattern string.
///
/// **Pattern** iterates a pre-parsed sequence of [`Segment`]s,
/// resolving each directive against the provided [`LogRecord`] and the current
/// terminal capabilities.
struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Pattern final: public Formatter {
    struct VIOLET_API NOELDOC_EXPERIMENTAL_SINCE("current") Config final {
        /// Force-toggle colour output. If [`Nothing`] is used, the formatter
        /// defers to [`terminal::ColoursEnabled()`].
        Optional<bool> UseColours = Nothing;

        /// Custom level-to-colour mapping for `%levelColor`. If not provided,
        /// a built-in mapping is used.
        Optional<Slice<Pair<LogLevel, terminal::RGB>, 6>> LevelColorMapping = Nothing;

        /// Prefix and suffix markers for `%A` (all attributes).
        /// Default: `{"[", "]"}`.
        ///
        /// Each marker is itself parsed as a pattern, so it may contain
        /// style directives (e.g., `%bold`/`%style:end`).
        Pair<String, String> Markers{"[", "]"};

        /// Key-value delimiter for attributes. Default: `"="`.
        String Delimiter = "=";
    };

    /// Parses a pattern string and constructs a `Pattern` formatter with the default configuration.
    static auto Parse(Str input) -> anyhow::Result<Pattern>;

    /// Parses a pattern string and constructs a `Pattern` formatter.
    static auto Parse(Str input, Config config) -> anyhow::Result<Pattern>;

    /// Returns a [`Pattern`] that resembles Azalia's output
    ///
    /// See:
    /// <https://github.com/Noelware/azalia/blob/61ab79859a014cd7c2c45bbdd9d4925a6a896bc0/crates/log/src/writers/default.rs>
    static auto Azalia() -> Pattern;

    NOELDOC_SEE("violet::experimental::log::Formatter::Format(const violet::experimental::log::Record&)")
    [[nodiscard]] auto Format(const Record& record) const -> String override;

private:
    VIOLET_EXPLICIT Pattern(pattern::ParsedPattern parsed, Config config, Optional<pattern::ParsedPattern> prefix,
        Optional<pattern::ParsedPattern> suffix)
        : n_parsed(VIOLET_MOVE(parsed))
        , n_config(VIOLET_MOVE(config))
        , n_prefixMarker(VIOLET_MOVE(prefix))
        , n_suffixMarker(VIOLET_MOVE(suffix))
    {
    }

    void formatPattern(const pattern::ParsedPattern& pattern, const Record& record, String& out) const;

    void formatSegment(const pattern::ParsedPattern& pattern, const pattern::Segment& segment, const Record& record,
        String& out) const;

    static void formatAttribute(
        const pattern::ParsedPattern& pattern, const pattern::Segment& segment, const Record& record, String& out);

    void formatAllAttributes(const Record& record, String& out) const;

    void emitColour(
        const pattern::ParsedPattern& pattern, const pattern::Segment& segment, bool foreground, String& out) const;

    void emitLevelColour(const pattern::ParsedPattern& pattern, const pattern::Segment& segment, const Record& record,
        String& out) const;

    void emitStyle(pattern::SegmentKind kind, String& out) const;
    void emitStyleEnd(String& out) const;

    pattern::ParsedPattern n_parsed;
    Config n_config;
    Optional<pattern::ParsedPattern> n_prefixMarker;
    Optional<pattern::ParsedPattern> n_suffixMarker;
};

} // namespace violet::experimental::log::formatter
