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

#include <violet/Experimental/Numeric.h>
#include <violet/Experimental/Threading/Internals/Cgroup.h>
#include <violet/Strings.h>

using namespace violet::strings;

namespace violet::experimental::threading::internals::cgroups {
namespace {

auto hasListItem(Str list, Str item) noexcept -> bool
{
    for (auto ent: Split(list, ',')) {
        if (ent == item) {
            return true;
        }
    }

    return false;
}

auto quotaToCpus(UInt64 quota, UInt64 period) noexcept -> Optional<UInt>
{
    if (quota == 0 || period == 0) {
        return Nothing;
    }

    return static_cast<UInt>((quota / period) + (quota % period != 0 ? 1 : 0));
}

} // namespace

auto Mount::Find(Str mountinfo, Version version) noexcept -> Optional<Mount>
{
    for (auto line: Lines(mountinfo)) {
        auto mount = ParseMount(line, version);
        if (mount.HasValue()) {
            return mount;
        }
    }

    return Nothing;
}

auto Mount::ResolveDirectory(filesystem::PathRef path) const -> filesystem::Path
{
    filesystem::Path dir(this->MountPoint);
    auto relative = path.Data();
    if (!this->Root.Root()) {
        if (!path.StartsWith(this->Root)) {
            return dir;
        }

        relative.remove_prefix(this->Root.Size());
    }

    if (filesystem::PathRef{relative}.StartsWith(filesystem::PathRef{"/.."})) {
        return dir;
    }

    while (relative.starts_with('/')) {
        relative.remove_prefix(1);
    }

    return relative.empty() ? dir : dir / relative;
}

auto FindCgroupPath(Str cgroups, Version version) noexcept -> Optional<filesystem::PathRef>
{
    for (auto line: Lines(cgroups)) {
        auto fields = SplitN<2>(line, ':');
        auto id = fields.Next();
        auto controllers = fields.Next();
        auto path = fields.Next();

        if (!path.HasValue()) {
            continue;
        }

        if (version == Version::V2) {
            if (*id == "0" && controllers->empty()) {
                return path.Value();
            }
        } else if (hasListItem(*controllers, "cpu")) {
            return path.Value();
        }
    }

    return Nothing;
}

auto ParseMount(Str line, Version version) noexcept -> Optional<Mount>
{
    Str root;
    Str mountPoint;
    Str fsType;
    Str superOpts;

    UInt index = 0;
    UInt afterSep = 0;
    bool sawSep = false;

    for (auto field: Split(line, ' ')) {
        if (!sawSep) {
            if (index == 3) {
                root = field;
            } else if (index == 4) {
                mountPoint = field;
            } else if (index >= 6 && field == "-") {
                sawSep = true;
            }

            index++;
            continue;
        }

        if (afterSep == 0) {
            fsType = field;
        } else if (afterSep == 2) {
            superOpts = field;
        }

        afterSep++;
    }

    if (!sawSep || mountPoint.empty()) {
        return Nothing;
    }

    if (version == Version::V2) {
        if (fsType != "cgroup2") {
            return Nothing;
        }
    } else if (fsType != "cgroup" || !hasListItem(superOpts, "cpu")) {
        return Nothing;
    }

    return Some<Mount>(root, mountPoint);
}

auto ParseCpuMax(Str contents) -> Optional<UInt>
{
    auto [quota, period] = SplitOnce(Trim(contents), ' ');
    if (!period.HasValue() || quota == "max") {
        return Nothing;
    }

    auto q1 = numeric::Parse<UInt64>(quota);
    auto p1 = numeric::Parse<UInt64>(*period);
    if (!q1.Ok() || !p1.Ok()) {
        return Nothing;
    }

    return quotaToCpus(q1.Value(), p1.Value());
}

auto ParseCfsQuota(Str quota, Str period) -> Optional<UInt>
{
    auto q1 = numeric::Parse<Int64>(Trim(quota));
    auto p1 = numeric::Parse<UInt64>(Trim(period));
    if (!q1.Ok() || !p1.Ok() || *q1 <= 0) {
        return Nothing;
    }

    return quotaToCpus(static_cast<UInt64>(q1.Value()), p1.Value());
}

} // namespace violet::experimental::threading::internals::cgroups
