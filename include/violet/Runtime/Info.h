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

#pragma once

#include <violet/Experimental/SmolString.h>

namespace violet {

/// Full numeric version identifier.
///
/// The version integer is encoded into four components in decimal digits:
/// ```
/// YYMMPPBB
/// ││││││││
/// ││││││└┘└┘── build increment (only in devbuilds, always 0 on release builds)
/// ││││└┘────── patch increment
/// ││└┘──────── current month + 1 (i.e, 02 = January, 03 = Feburary, etc)
/// └┘────────── year   (e.g. 26)
/// ```
constexpr static const int VERSION = VIOLET_VERSION;

/// The major year component (e.g. `26`).
constexpr static const int YEAR = VIOLET_VERSION / 1'000'000;

/// The month component (`1` ~ `12`).
constexpr static const int MONTH = (VIOLET_VERSION / 10000) % 100;

/// The patch component; value of `0` indicates no patch.
constexpr static const int PATCH = (VIOLET_VERSION / 100) % 100;

/// The build component; only available in dev builds.
constexpr static const int BUILD = VIOLET_VERSION % 100;

#ifdef VIOLET_DEVBUILD
/// Returns **true** if this is a development build.
constexpr static const bool DEVBUILD = true;
#else
/// Returns **true** if this is a development build.
constexpr static const bool DEVBUILD = false;
#endif

/// Returns the library version as a human-readable string.
///
/// The format follows a calendar-versioning scheme:
///
/// ```
/// YEAR.MONTH[.PATCH][-dev[.BUILD]]
/// ```
///
/// - `PATCH` is omitted when zero.
/// - The `-dev` suffix and optional `.BUILD` are appended only when
///   `VIOLET_DEVBUILD` is defined.
///
/// # Examples
///
/// Given `VIOLET_VERSION = 2026'06'03'00`:
///
/// ```cpp
/// // Release build
/// Version() == "2026.06.03"
///
/// // Dev build (VIOLET_DEVBUILD defined), BUILD == 0
/// Version() == "2026.06.03-dev"
/// ```
///
/// Given `VIOLET_VERSION = 2026'01'00'05` with `VIOLET_DEVBUILD`:
///
/// ```cpp
/// Version() == "2026.01-dev.5"
/// ```
constexpr auto Version() noexcept -> experimental::SmolString<256>
{
    using namespace std::string_view_literals;

    experimental::SmolString<256> smol;
    smol.AppendFormatted("{}.{:02}", YEAR, MONTH);

    if (PATCH > 0) {
        smol.AppendFormatted(".{:02}", PATCH);
    }

#ifdef VIOLET_DEVBUILD
    smol.Append("-dev");

    if (BUILD > 0) {
        smol.AppendFormatted(".{}", BUILD);
    }
#endif

    return smol;
}

} // namespace violet
