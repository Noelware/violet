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

#include <violet/Language/Macros.h>

#if VIOLET_PLATFORM(LINUX)

#include <violet/Experimental/Threading/Internals/Cgroup.h>
#include <violet/Experimental/Threading/Parallelism.h>
#include <violet/Filesystem/File.h>
#include <violet/IO/Experimental/Error.h>
#include <violet/IO/Read.h>

#include <sched.h>
#include <unistd.h>

using namespace violet::filesystem;

namespace violet::experimental::threading {
namespace {

auto getAffinityCount() -> Optional<UInt>
{
    for (UInt ncpus = CPU_SETSIZE; ncpus <= (1U << 20U); ncpus *= 2) {
        auto* set = CPU_ALLOC(ncpus);
        if (set == nullptr) {
            return Nothing;
        }

        UInt size = CPU_ALLOC_SIZE(ncpus);
        CPU_ZERO_S(size, set);

        const auto rc = ::sched_getaffinity(0, size, set);
        auto count = CPU_COUNT_S(size, set);
        const auto error = errno;
        CPU_FREE(set);

        if (rc == 0) {
            return static_cast<UInt>(count);
        }

        if (error != EINVAL) {
            return Nothing;
        }
    }

    return Nothing;
}

auto readFile(PathRef path) -> io::experimental::Result<String>
{
    auto file = VIOLET_TRY(File::Open(path, OpenOptions{}.Read()));
    return VIOLET_TRY(io::ReadToString(file));
}

auto readCgroupLimit(PathRef dir, internals::cgroups::Version version) -> io::experimental::Result<Optional<UInt>>
{
    if (version == internals::cgroups::Version::V2) {
        return internals::cgroups::ParseCpuMax(VIOLET_TRY(readFile(dir / "cpu.max")));
    }

    auto quota = VIOLET_TRY(readFile(dir / "cpu.cfs_quota_us"));
    auto period = VIOLET_TRY(readFile(dir / "cpu.cfs_period_us"));

    return internals::cgroups::ParseCfsQuota(quota, period);
}

auto getCgroupQuota() -> io::experimental::Result<Optional<UInt>>
{
    auto mountinfo = VIOLET_TRY(readFile("/proc/self/mountinfo"));
    auto cgroups = VIOLET_TRY(readFile("/proc/self/cgroup"));

    for (auto version: {internals::cgroups::Version::V2, internals::cgroups::Version::V1}) {
        auto mount = internals::cgroups::Mount::Find(mountinfo, version);
        auto path = internals::cgroups::FindCgroupPath(cgroups, version);
        if (!mount.HasValue() || !path.HasValue()) {
            continue;
        }

        const auto dir = mount->ResolveDirectory(path.Value());
        auto quota = internals::cgroups::QuotaOf(mount->MountPoint, dir,
            [version](auto at) noexcept -> Optional<UInt> { return readCgroupLimit(at, version).UnwrapOrDefault(); });

        if (quota.HasValue()) {
            return quota;
        }
    }

    return Nothing;
}

} // namespace

auto AvailableParallelism() noexcept -> UInt
{
    UInt count = getAffinityCount().UnwrapOr(0);
    if (count == 0) {
        const auto online = ::sysconf(_SC_NPROCESSORS_ONLN);
        count = online > 0 ? static_cast<UInt>(online) : 1;
    }

    if (auto quota = getCgroupQuota().UnwrapOrDefault(); quota.HasValue()) {
        count = std::min(count, *quota);
    }

    return std::max<UInt>(count, 1);
}

} // namespace violet::experimental::threading

#endif
