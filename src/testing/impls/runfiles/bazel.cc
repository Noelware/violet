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
#include <violet/System.h>
#include <violet/Testing/Runfiles.h>
#include <violet/Violet.h>

using rules_cc::cc::runfiles::Runfiles;
using violet::CStr;
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

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static UniquePtr<rules_cc::cc::runfiles::Runfiles> n_runfiles;

// TODO(@auguwu/Noel): Introduce `$VIOLET_TESTING_RUNFILES_TEST_WORKSPACE_OVERRIDE` to
// override this value if it wasn't detected
//
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static String n_testWorkspace = "_main";

constexpr static CStr kWorkspaceEnv = "TEST_WORKSPACE";
constexpr static CStr kRunfilesDirEnv = "RUNFILES_DIR";
constexpr static CStr kWorkspaceOverrideEnv = "VIOLET_TESTING_RUNFILES_WORKSPACE_NAME";

namespace {

auto detectWorkspaceName() -> String
{
    // First, we need to get the repo name to be the main source repository, the
    // way to get that is, to look inside of `$RUNFILES_DIR/_repo_mapping`, which
    // is a symlink and we need to read the file. The contents look like:
    //
    // (example from Noel's machine)
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
    //
    // from Noel's Zenful project:
    //
    // ,dev_floofy_zenful,_main
    // ,googletest,googletest+
    // ,violet,violet+
    // abseil-cpp+,abseil-cpp,abseil-cpp+
    // abseil-cpp+,googletest,googletest+
    // googletest+,abseil-cpp,abseil-cpp+
    // googletest+,googletest,googletest+
    // violet+,absl,abseil-cpp+
    // violet+,googletest,googletest+
    // violet+,org_noelware_violet,violet+
    //
    // `$TEST_WORKSPACE` is also defined, which we can just say
    //
    //    source_location = grep(line, "$TEST_WORKSPACE")
    //
    // and get the first element (`,org_noelware_violet,_main` -> `org_noelware_violet`)
    //
    // This trick also works on Bazel 8.x, but I am not sure if it works on Bazel 7.

    String workspaceName;
    auto workspace = GetEnv(kWorkspaceEnv);
    auto runfilesDir = GetEnv(kRunfilesDirEnv).Map([](const String& value) -> Path { return value; });

    if (workspace && runfilesDir) {
        auto repoMappingPath = runfilesDir->Join("_repo_mapping");
        VIOLET_ASSERT(TryExists(repoMappingPath).Ok(), "repo mapping doesn't exist?");

        auto canon = Canonicalize(repoMappingPath);
        VIOLET_ASSERT(canon.Ok(), "failed to canoncalize (and resolve symlinks) of `_repo_mapping`");

        auto runfilesRepoMapping = File::Open(canon.Value(), OpenOptions{}.Read(true));
        if (runfilesRepoMapping.Ok()) {
            Vec<UInt8> buf(8192);

            auto res = runfilesRepoMapping->Read(buf);
            VIOLET_ASSERT(res.Ok(), "failed to read runfiles repo mapping");
            VIOLET_ASSERT(res.Value() != 0, "read 0 bytes?!");

            String contents(buf.begin(), buf.end());

            // TODO(@auguwu/Noel): make this better somehow
            std::stringstream ss(contents);
            String line;
            while (std::getline(ss, line, '\n')) {
                // If the line ends with `$TEST_WORKSPACE` (usually `_main` but
                // I could be wrong), get the first element of splitting `,` and
                // that is our repository!
                if (line.ends_with(*workspace.Value())) {
                    auto parts = std::ranges::views::split(line, ',');

                    auto it = parts.begin();
                    ++it; // skip the 0th element

                    VIOLET_ASSERT(it != parts.end(), "unreachable point");

                    auto element = *it;
                    workspaceName = String(element.begin(), element.end());

#ifndef VIOLET_RUNFILES_LOGS
                    std::cout << "[violet/testing/runfiles@init] using workspace name '" << workspaceName << "'\n";
                    std::cout << "if this is the wrong workspace, either submit an issue at "
                                 "https://github.com/Noelware/violet/issues/new";

                    std::cout << " or use the `$" << kWorkspaceOverrideEnv
                              << "' environment variable to populate your workspace\n\n";
#endif

                    ++it; // go to the third element for the test workspace
                    auto tws = *it;
                    n_testWorkspace = String(tws.begin(), tws.end());

#ifndef VIOLET_RUNFILES_LOGS
                    std::cout << "[violet/testing/runfiles@init] $TEST_WORKSPACE = " << n_testWorkspace << '\n';
#endif

                    break;
                }
            }
        } else {
#ifndef VIOLET_RUNFILES_LOGS
            std::cerr << "[violet/testing/runfiles@error] failed to read file '" << runfilesDir->ToString()
                      << "' (tests might fail): " << VIOLET_MOVE(runfilesRepoMapping.Error()).ToString() << '\n';
#endif
        }
    } else if (auto name = GetEnv(kWorkspaceOverrideEnv)) {
        workspaceName = VIOLET_MOVE(*name.Value());
    } else {
#ifndef VIOLET_RUNFILES_LOGS
        std::cerr << "[violet/testing/runfiles@warning] unable to collect `$" << kRunfilesDirEnv << "' or `$"
                  << kWorkspaceEnv
                  << "' environment variables, cannot detect workspace name (you can override this with the `$"
                  << kWorkspaceOverrideEnv << "' environment variable)";
#endif
    }

    return workspaceName;
}

} // namespace

void violet::testing::runfiles::Init(CStr argv0)
{
    String workspaceName = detectWorkspaceName();
    String error;
    n_runfiles.reset(rules_cc::cc::runfiles::Runfiles::Create(argv0, workspaceName, &error));

    if (!n_runfiles) {
        auto msg = std::format("failed to build runfiles for test: {}", error);
        VIOLET_ASSERT(false, msg.c_str());
    }
}

auto violet::testing::runfiles::Get(Str path) -> Optional<String>
{
    VIOLET_ASSERT(static_cast<bool>(n_runfiles), "`violet::testing::runfiles::Init` was never called");

    String logicalPath;
    if (!n_testWorkspace.empty()) {
        logicalPath = std::format("{}/{}", n_testWorkspace, path);
    } else {
        logicalPath = path;
    }

    auto logical = n_runfiles->Rlocation(logicalPath);
    VIOLET_ASSERT(!logical.empty(), "bad runfile location");

    auto exists = filesystem::TryExists(static_cast<Str>(logical));
    if (exists.Err()) {
#ifndef NDEBUG
        auto error = VIOLET_MOVE(exists.Error());
        std::cerr << "[violet/testing/runfiles] Unable to check the existence of runfile '" << logical << ": "
                  << error.ToString() << '\n';
#endif

        return Nothing;
    }

    if (!exists.Value()) {
#ifndef NDEBUG
        std::cerr << "[violet/testing/runfiles] runfile [" << logical << "] doesn't exist (from path: " << path
                  << ")\n";
#endif

        return Nothing;
    }

    return logical;
}
