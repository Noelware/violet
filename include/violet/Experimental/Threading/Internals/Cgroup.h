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

#include <violet/Container/Optional.h>
#include <violet/Filesystem/Path.h>

namespace NOELDOC_HIDE violet {
namespace experimental::threading::internals::cgroups {

/// Which cgroup hierarchy to look for.
enum struct Version : UInt8 {
    /// cgroup v1, where the `cpu` controller has its own `cgroup` mount.
    V1,

    /// cgroup v2, the unified `cgroup2` hierarchy.
    V2
};

/// A cgroup hierarchy mount found in `/proc/self/mountinfo`.
///
/// ## Remarks
/// Both fields are views into the `mountinfo` contents they were parsed from.
struct VIOLET_API Mount final {
    /// The path within the hierarchy that is mounted (e.g. `/`, or `/docker/abc` inside a container).
    filesystem::PathRef Root;

    /// Where the hierarchy is mounted (e.g. `/sys/fs/cgroup`).
    filesystem::PathRef MountPoint;

    /// Finds the first mount in the contents of `/proc/self/mountinfo` that matches `version`.
    ///
    /// @param mountinfo the full contents of `/proc/self/mountinfo`
    /// @param version which hierarchy to match
    static auto Find(Str mountinfo, Version version) noexcept -> Optional<Mount>;

    /// Resolves the cgroup `path` from `/proc/self/cgroup` against `mount` into a directory on disk.
    ///
    /// ## Remarks
    /// When the mount's root isn't `/` (as in a container without a cgroup namespace), the root is stripped
    /// from `path`. If `path` lies outside the mounted root, or climbs out of it with a leading `..`, only the
    /// mount point itself is visible, so that is returned.
    ///
    /// @param path the path from [`FindCgroupPath`]
    auto ResolveDirectory(filesystem::PathRef path) const -> filesystem::Path;
};

/// Finds this process's cgroup path for `version` in the contents of `/proc/self/cgroup`.
///
/// Lines are `hierarchy-id:controllers:path`. v2 is the `0::<path>` line; for v1, it's the line whose
/// controller list contains `cpu`.
///
/// @param cgroups the full contents of `/proc/self/cgroup`
/// @param version which hierarchy to look for
VIOLET_API
auto FindCgroupPath(Str cgroups, Version version) noexcept -> Optional<filesystem::PathRef>;

/// Parses a single `/proc/self/mountinfo` line, returning it only if it is the hierarchy that holds
/// the `cpu` controller for `version`.
///
/// ## Remarks
/// Octal escapes in paths (like `\040` for a space) are not decoded; cgroup mount points don't use them
/// in practice.
///
/// ## Example
/// ```cpp
/// auto mount = ParseMount(
///     "30 23 0:26 / /sys/fs/cgroup rw,nosuid shared:4 - cgroup2 cgroup2 rw,nsdelegate", Version::V2);
///
/// VIOLET_ASSERT(mount);
/// VIOLET_ASSERT(mount->MountPoint == "/sys/fs/cgroup");
/// ```
///
/// @param line a single line, without the trailing newline
/// @param version which hierarchy to match
VIOLET_API
auto ParseMount(Str line, Version version) noexcept -> Optional<Mount>;

/// Parses a cgroup v2 `cpu.max` file (`<quota> <period>`, where quota may be `max`) into a CPU count,
/// rounded up.
///
/// @param contents the contents of `cpu.max`
/// @return the number of CPUs, or [`Nothing`] if unlimited or malformed
VIOLET_API
auto ParseCpuMax(Str contents) -> Optional<UInt>;

/// Parses cgroup v1 `cpu.cfs_quota_us` and `cpu.cfs_period_us` files into a CPU count, rounded up.
///
/// @param quota the contents of `cpu.cfs_quota_us` (`-1` means unlimited)
/// @param period the contents of `cpu.cfs_period_us`
/// @return the number of CPUs, or [`Nothing`] if unlimited or malformed
VIOLET_API
auto ParseCfsQuota(Str quota, Str period) -> Optional<UInt>;

/// Walks from `dir` up to `mountPoint`, calling `readLimit` on each directory, and returns the
/// smallest limit found.
///
/// ## Remarks
/// - CPU bandwidth limits are hierarchical: a parent's quota caps every child, so a limit set on any
///   ancestor applies to this process.
/// - If `dir` isn't under `mountPoint`, only `dir` itself is read.
///
/// ## Example
/// ```cpp
/// auto quota = QuotaOf("/sys/fs/cgroup", "/sys/fs/cgroup/a/b", [](PathRef dir) -> Optional<UInt> {
///     return dir == "/sys/fs/cgroup/a" ? Optional<UInt>(2) : Nothing;
/// });
///
/// VIOLET_ASSERT(quota.UnwrapOr(0) == 2);
/// ```
///
/// @param mountPoint the hierarchy's mount point; the walk stops here
/// @param dir the directory to start from, from [`ResolveCgroupDir`]
/// @param readLimit reads the limit for one directory
template<typename Fun>
    requires callable<Fun, filesystem::PathRef> && callable_returns<Fun, Optional<UInt>, filesystem::PathRef>
auto QuotaOf(filesystem::PathRef mountPoint, filesystem::PathRef dir, Fun&& readLimit) -> Optional<UInt>
{
    Optional<UInt> best;
    filesystem::PathRef at = dir;
    Fun getReadLimit = VIOLET_FWD(Fun, readLimit);

    while (true) {
        const Optional<UInt> limit = std::invoke(getReadLimit, at);
        if (limit.HasValue() && (!best.HasValue() || *limit < *best)) {
            best = limit;
        }

        if (at == mountPoint || !at.StartsWith(mountPoint)) {
            return best;
        }

        auto parent = at.Parent();
        if (!parent.HasValue()) {
            return best;
        }

        at = parent.Value();
    }
}

} // namespace experimental::threading::internals::cgroups
} // namespace NOELDOC_HIDE violet
