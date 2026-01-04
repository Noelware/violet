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

#include <violet/System.h>
#include <violet/System/Which.h>
#include <violet/Violet.h>

using violet::CStr;

#ifdef VIOLET_UNIX
static inline const char kSeparator = ':';
#elif defined(VIOLET_WINDOWS)
static inline const char kSeparator = ';';
#endif

auto violet::sys::Which(Str, const WhichConfig& config) -> io::Result<filesystem::Path>
{
    auto path = sys::GetEnv(config.PathEnv);
    if (!path) {
        return Err(VIOLET_IO_ERROR(
            InvalidData, String, std::format("`$PATH` environment (set to {}) was not found", config.PathEnv)));
    }

    // TODO(@auguwu/Noel): should `$PATH` computation be cached?
    Vec<String> paths;
    std::stringstream ss(*path);
    String current;

    while (std::getline(ss, current, kSeparator)) {
        paths.push_back(current);
    }

    // Now that we have our paths, let's iterate!
    return { "/" };
}
