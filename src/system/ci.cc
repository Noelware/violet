// 🌺💜 Violet: Extended C++ standard library
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
//
// This is a port of the `is_ci` Rust crate (https://crates.io/crates/is_ci) for
// the `Noelware.Violet.Terminal` library.

#include <violet/System.h>
#include <violet/System/CI.h>

// clang-format off
constexpr const violet::Array<violet::CStr, 34> kProviders = {
    "CI_NAME",
    "GITHUB_ACTION",
    "GITLAB_CI",
    "NETLIFY",
    "TRAVIS",
    "CODEBUILD_SRC_DIR",
    "BUILDER_OUTPUT",
    "GITLAB_DEPLOYMENT",
    "NOW_GITHUB_DEPLOYMENT",
    "NOW_BUILDER",
    "BITBUCKET_DEPLOYMENT",
    "GERRIT_PROJECT",
    "SYSTEM_TEAMFOUNDATIONCOLLECTIONURI",
    "BITRISE_IO",
    "BUDDY_WORKSPACE_ID",
    "BUILDKITE",
    "CIRRUS_CI",
    "APPVEYOR",
    "CIRCLECI",
    "SEMAPHORE",
    "DRONE",
    "DSARI",
    "TDDIUM",
    "STRIDER",
    "TASKCLUSTER_ROOT_URL",
    "JENKINS_URL",
    "bamboo.buildKey",
    "GO_PIPELINE_NAME",
    "HUDSON_URL",
    "MAGNUM",
    "NEVERCODE",
    "RENDER",
    "SAIL_CI",
    "SHIPPABLE"
};
// clang-format on

auto violet::sys::ContinuousIntegration() noexcept -> bool
{
    if (auto var = sys::GetEnv("CI")) {
        auto res = Str(*var);
        return res == "true" || res == "1" || res == "woodpecker";
    }

    if (auto node = sys::GetEnv("NODE")) {
        auto res = Str(*node);
        return res.ends_with("//heroku/node/bin/node");
    }

    const auto check = [](CStr key) -> bool { return sys::GetEnv(key).HasValue(); };
    for (const auto& key: kProviders) {
        if (check(key)) {
            return true;
        }
    }

    return false;
}
