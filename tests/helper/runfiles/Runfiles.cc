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

#include "tests/helper/runfiles/Runfiles.h"

#ifdef BAZEL
#include "rules_cc/cc/runfiles/runfiles.h"
#else
#include <violet/System.h>
#endif

#include <violet/Filesystem.h>
#include <violet/Violet.h>

#ifdef BAZEL
namespace {

violet::UniquePtr<rules_cc::cc::runfiles::Runfiles> n_runfiles;

} // namespace
#endif

void violet::testing::runfiles::Init()
{
#ifdef BAZEL
    violet::String* error = nullptr;
    n_runfiles.reset(rules_cc::cc::runfiles::Runfiles::CreateForTest("", error));
    if (!n_runfiles) {
        auto msg = std::format("failed to build runfiles for test: {}", *error);
        VIOLET_ASSERT(false, msg.c_str());
    }
#endif
}

constexpr static violet::CStr kWorkspace = "org_noelware_violet";

auto violet::testing::runfiles::Get(violet::Str path) -> violet::Optional<violet::String>
{
#ifdef BAZEL
    VIOLET_ASSERT(static_cast<bool>(n_runfiles), "`violet::testing::runfiles::Init()` was never called");

    auto logical = n_runfiles->Rlocation(std::format("{}/{}", kWorkspace, path.data()));
    VIOLET_ASSERT(!logical.empty(), "bad runfile location");

    auto result = violet::filesystem::TryExists(static_cast<violet::Str>(logical));
    if (result.Err()) {
#ifndef NDEBUG
        auto error = VIOLET_MOVE(result.Error());
        std::cout << "checking the existence of file '" << logical << "' failed: " << error.ToString()
                  << "; failing!\n";
#endif

        return Nothing;
    }

    if (!result.Value()) {
        return Nothing;
    }

    return logical;
#elif defined(CMAKE)
    if (auto env = violet::sys::GetEnv("CMAKE_BINARY_DIR")) {
        return Some<String>(std::format("{}/{}", *env, path));
    }

    return Nothing;
#elif defined(MESON)
    if (auto env = violet::sys::GetEnv("MESON_BUILD_DIR")) {
        return Some<String>(std::format("{}/{}", *env, path));
    }

    return Nothing;
#else
    return Nothing;
#endif
}
