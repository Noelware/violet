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

#include "rules_cc/cc/runfiles/runfiles.h"

#include <violet/Container/Optional.h>
#include <violet/Filesystem.h>
#include <violet/IO/Read.h>
#include <violet/Strings.h>
#include <violet/System.h>
#include <violet/Testing/Runfiles.h>

#ifdef VIOLET_RUNFILES_LOGS
#include <violet/Print.h>
#endif

using violet::CStr;
using violet::Nothing;
using violet::Optional;
using violet::Pair;
using violet::String;
using violet::UInt8;
using violet::UniquePtr;
using violet::Vec;
using violet::filesystem::Canonicalize;
using violet::filesystem::File;
using violet::filesystem::OpenOptions;
using violet::filesystem::Path;
using violet::filesystem::TryExists;
using violet::sys::GetEnv;

constexpr static CStr kTestWorkspaceEnv = "TEST_WORKSPACE";
constexpr static CStr kRunfilesDirEnv = "RUNFILES_DIR";
constexpr static CStr kDeprecatedWorkspaceOverrideEnv = "VIOLET_TESTING_RUNFILES_WORKSPACE_NAME";
constexpr static CStr kWorkspaceOverrideEnv = "VIOLET_TESTING_RUNFILES_WORKSPACE";

#ifdef VIOLET_RUNFILES_LOGS
#define println(fmt, ...)                                                                                              \
    violet::Println(                                                                                                   \
        "[violet/testing/runfiles][{}:{}] {}", __FILE__, __LINE__, std::format(fmt __VA_OPT__(, ) __VA_ARGS__))

#define printerr(fmt, ...)                                                                                             \
    violet::PrintErrln(                                                                                                \
        "[violet/testing/runfiles][{}:{}] {}", __FILE__, __LINE__, std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#else
#define println(...)
#define printerr(...)
#endif

namespace {

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
UniquePtr<rules_cc::cc::runfiles::Runfiles> n_runfiles;
Optional<String> n_workspace;
Optional<String> n_testWorkspace;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

constexpr auto stringToPath(const String& str) -> Path
{
    return str;
}

auto collectRepositoryMapping() -> Vec<Pair<String, String>>
{
    auto runfilesDir = GetEnv(kRunfilesDirEnv).Map(stringToPath);
    if (!runfilesDir.HasValue()) {
        printerr("missing `${}` environment variable, therefore runfiles framework is useless", kRunfilesDirEnv);
        return { };
    }

    auto repoMappingFile = runfilesDir->Join("_repo_mapping");
    auto repoMappingFileExists = TryExists(repoMappingFile);
    if (repoMappingFileExists.Err()) {
        printerr(
            "failed to find repository mapping [{}]: {}", repoMappingFile, VIOLET_MOVE(repoMappingFileExists).Error());

        return { };
    }

    if (!*repoMappingFileExists) {
        printerr("repository mapping [{}] doesn't exist", repoMappingFile);
        return { };
    }

    auto canonRepoMapping = Canonicalize(repoMappingFile);
    if (canonRepoMapping.Err()) {
        printerr("failed to canonicalize repository mapping [{}]: {}", repoMappingFile,
            VIOLET_MOVE(canonRepoMapping).Error());

        return { };
    }

    Vec<Pair<String, String>> workspaces;
    auto mapping = File::Open(*canonRepoMapping, OpenOptions{ }.Read());
    if (mapping.Ok()) {
        auto contents = violet::io::ReadToString(*mapping);
        VIOLET_ASSERT0(contents.Ok());

        for (const auto& line: violet::strings::Lines(*contents)) {
            if (line.empty()) {
                continue;
            }

            auto parts = violet::strings::SplitN<2>(line, ',');
            VIOLET_ASSERT0(parts.Next().HasValue()); // skip the first entry since we don't care about it

            auto first = parts.Next();
            VIOLET_ASSERT0(first.HasValue());

            auto second = parts.Next();
            VIOLET_ASSERT0(second.HasValue());

            workspaces.emplace_back(*first, *second);
        }
    } else {
        printerr("failed to open file [{}]: {}", canonRepoMapping, VIOLET_MOVE(mapping).Error());
    }

    return workspaces;
}

auto detectWorkspace() -> Optional<String>
{
    // First, we would need to get the repository name from `$RUNFILES_DIR/_repo_mapping`,
    // which is a file that shows all the repository mappings Bazel uses. For example,
    // something like this would be the `_repo_mapping` file:
    //
    // ```
    // ,absl,abseil-cpp+
    // ,googletest,googletest+
    // ,org_noelware_violet,_main
    // ,rules_cc,rules_cc+
    // abseil-cpp+,abseil-cpp,abseil-cpp+
    // abseil-cpp+,googletest,googletest+
    // abseil-cpp+,rules_cc,rules_cc+
    // bazel_tools,rules_cc,rules_cc+
    // bazel_tools+xcode_configure_extension+local_config_xcode,rules_cc,rules_cc+
    // googletest+,abseil-cpp,abseil-cpp+
    // googletest+,googletest,googletest+
    // googletest+,rules_cc,rules_cc+
    // rules_cc+,rules_cc,rules_cc+
    // rules_cc++cc_configure_extension+local_config_cc,rules_cc,rules_cc+
    // ```
    //
    // Bazel sets `$TEST_WORKSPACE` when we are invoked with a `cc_test` binary,
    // which we can just do:
    //
    // ```pseudo
    // val file = readFile("$RUNFILES_DIR/_repo_mapping")
    // for l in file.lines() {
    //     if (val line = line.Grep("$TEST_WORKSPACE")) {
    //         return line.Extract(delim: ",", index: 2);
    //     }
    // }
    //
    // return Nothing;
    // ```
    //
    // If we are called from `Init(argv[0], /*inTestContext=/*false)`, then the caller
    // should use `violet::testing::runfiles::SetWorkspaceName("<ws>")`, which, it is
    // usually `_main`.
    if (n_workspace.HasValue()) {
        return n_workspace;
    }

    auto workspace = GetEnv(kTestWorkspaceEnv);
    if (!workspace.HasValue() && !n_workspace.HasValue()) {
        printerr("missing `${}` environment variable, you will need to rely on "
                 "`violet::testing::runfiles::SetWorkspaceName`",
            kTestWorkspaceEnv);

        return Nothing;
    }

    println("using `$TEST_WORKSPACE` (from Bazel): {}", *workspace);

    if (auto name = GetEnv(kDeprecatedWorkspaceOverrideEnv)) {
        printerr("warning: environment variable `${}` is deprecated in favour of `${}`, this will be removed in a "
                 "26.06 release",
            kDeprecatedWorkspaceOverrideEnv, kWorkspaceOverrideEnv);

        return name;
    }

    if (auto name = GetEnv(kWorkspaceOverrideEnv)) {
        return name;
    }

    for (const auto& [repo, ws]: collectRepositoryMapping()) {
        if (ws.ends_with(workspace.Value())) {
            n_workspace = repo;

            println("using workspace name: {}", n_workspace);
            println("--> if this is wrong, you can submit an issue if it's on our end at: "
                    "https://github.com/Noelware/violet/issues/new?labels=%22Noelware.Violet.Testing%22&type=Bug");

            println("--> you can also set the `${}` environment variable as well", kWorkspaceOverrideEnv);

            n_testWorkspace = ws;
            break;
        }
    }

    return n_workspace;
}

} // namespace

auto violet::testing::runfiles::WorkspaceName() -> Optional<String>
{
    if (n_workspace.HasValue()) {
        return n_workspace;
    }

    static std::once_flag flag;
    std::call_once(flag, [] -> void {
        if (auto ws = detectWorkspace()) {
            n_workspace = ws;
        }
    });

    return n_workspace;
}

void violet::testing::runfiles::SetWorkspaceName(Str ws) noexcept
{
    n_workspace = ws;
}

void violet::testing::runfiles::Init(CStr argv0)
{
    String error;
    String workspaceName = WorkspaceName().UnwrapOrDefault();
    n_runfiles.reset(rules_cc::cc::runfiles::Runfiles::Create(argv0, workspaceName, &error));

    VIOLET_ASSERT(n_runfiles != nullptr, std::format("failed to build runfiles: {}", error));
}

auto violet::testing::runfiles::Get(Str path) -> Optional<String>
{
    VIOLET_ASSERT0(n_runfiles != nullptr);

    String logicalPath = n_testWorkspace.MapOr(
        String(path), [path](String& ws) -> String { return std::format("{}/{}", VIOLET_MOVE(ws), path); });

    auto logical = n_runfiles->Rlocation(logicalPath);
    VIOLET_ASSERT(!logical.empty(), "bad runfile location");

    auto exists = TryExists(static_cast<Str>(logical));
    if (exists.Err()) {
        printerr("unable to check existence of runfile [{}]: {}", logical, VIOLET_MOVE(exists).Error());
        return Nothing;
    }

    if (!*exists) {
        printerr("runfile [{}] doesn't exist (path={})", logical, path);
        return Nothing;
    }

    return logical;
}
