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

#include <gtest/gtest.h>
#include <violet/Experimental/Threading/Internals/Cgroup.h>

namespace violet::experimental::threading::internals::cgroups {
namespace {

// A hybrid systemd host: the v2 unified hierarchy plus v1 controller mounts.
constexpr Str kHybridMountInfo
    = "22 1 259:2 / / rw,relatime shared:1 - ext4 /dev/nvme0n1p2 rw\n"
      "29 23 0:25 / /sys/fs/cgroup/systemd rw,nosuid shared:3 - cgroup cgroup rw,xattr,name=systemd\n"
      "30 23 0:26 / /sys/fs/cgroup/unified rw,nosuid,nodev,noexec shared:4 - cgroup2 cgroup2 rw,nsdelegate\n"
      "31 23 0:27 / /sys/fs/cgroup/cpuset rw,nosuid shared:5 - cgroup cgroup rw,cpuset\n"
      "32 23 0:28 / /sys/fs/cgroup/cpu,cpuacct rw,nosuid shared:6 - cgroup cgroup rw,cpu,cpuacct\n";

// Inside a container without a cgroup namespace: the hierarchy is mounted from the container's own cgroup.
constexpr Str kContainerMountInfo
    = "600 500 0:50 / / rw,relatime master:200 - overlay overlay rw,lowerdir=/l,upperdir=/u,workdir=/w\n"
      "610 600 0:26 /docker/abc /sys/fs/cgroup ro,nosuid,nodev,noexec master:4 - cgroup2 cgroup rw,nsdelegate\n";

} // namespace

TEST(CgroupMount, ParsesV2Mount)
{
    auto mount
        = ParseMount("30 23 0:26 / /sys/fs/cgroup rw,nosuid shared:4 - cgroup2 cgroup2 rw,nsdelegate", Version::V2);

    ASSERT_TRUE(mount);
    EXPECT_EQ(mount->Root, "/");
    EXPECT_EQ(mount->MountPoint, "/sys/fs/cgroup");
}

TEST(CgroupMount, ParsesV1CpuMount)
{
    auto mount = ParseMount(
        "32 23 0:28 / /sys/fs/cgroup/cpu,cpuacct rw,nosuid shared:6 - cgroup cgroup rw,cpu,cpuacct", Version::V1);

    ASSERT_TRUE(mount);
    EXPECT_EQ(mount->Root, "/");
    EXPECT_EQ(mount->MountPoint, "/sys/fs/cgroup/cpu,cpuacct");
}

TEST(CgroupMount, ParsesMountWithoutOptionalFields)
{
    auto mount = ParseMount("30 23 0:26 / /sys/fs/cgroup rw - cgroup2 cgroup2 rw", Version::V2);

    ASSERT_TRUE(mount);
    EXPECT_EQ(mount->MountPoint, "/sys/fs/cgroup");
}

TEST(CgroupMount, ParsesMountWithManyOptionalFields)
{
    auto mount = ParseMount(
        "1100 1090 0:28 /docker/abc /sys/fs/cgroup/cpu ro shared:6 master:3 propagate_from:2 - cgroup cgroup rw,cpu",
        Version::V1);

    ASSERT_TRUE(mount);
    EXPECT_EQ(mount->Root, "/docker/abc");
    EXPECT_EQ(mount->MountPoint, "/sys/fs/cgroup/cpu");
}

TEST(CgroupMount, DashInMountPathIsNotTheSeparator)
{
    // field 4 is "-"; the separator is only looked for after the mount options (field 6 onward)
    auto mount = ParseMount("30 23 0:26 / - rw shared:4 - cgroup2 cgroup2 rw", Version::V2);

    ASSERT_TRUE(mount);
    EXPECT_EQ(mount->MountPoint, "-");
}

TEST(CgroupMount, RejectsV1MountsWithoutCpuController)
{
    EXPECT_FALSE(ParseMount("31 23 0:27 / /sys/fs/cgroup/cpuset rw - cgroup cgroup rw,cpuset", Version::V1));

    // "cpuacct" is its own controller, not "cpu"
    EXPECT_FALSE(ParseMount("33 23 0:29 / /sys/fs/cgroup/cpuacct rw - cgroup cgroup rw,cpuacct", Version::V1));
    EXPECT_FALSE(
        ParseMount("29 23 0:25 / /sys/fs/cgroup/systemd rw - cgroup cgroup rw,xattr,name=systemd", Version::V1));
}

TEST(CgroupMount, RejectsMountsOfTheOtherVersion)
{
    EXPECT_FALSE(ParseMount("30 23 0:26 / /sys/fs/cgroup rw - cgroup2 cgroup2 rw", Version::V1));
    EXPECT_FALSE(ParseMount("32 23 0:28 / /sys/fs/cgroup/cpu rw - cgroup cgroup rw,cpu", Version::V2));
}

TEST(CgroupMount, RejectsNonCgroupMounts)
{
    EXPECT_FALSE(ParseMount("22 1 259:2 / / rw - ext4 /dev/nvme0n1p2 rw", Version::V2));
    EXPECT_FALSE(ParseMount("22 1 259:2 / / rw - ext4 /dev/nvme0n1p2 rw,cpu", Version::V1));
}

TEST(CgroupMount, RejectsMalformedLines)
{
    EXPECT_FALSE(ParseMount("", Version::V2));
    EXPECT_FALSE(ParseMount("garbage", Version::V2));

    // no " - " separator
    EXPECT_FALSE(ParseMount("30 23 0:26 / /sys/fs/cgroup rw cgroup2", Version::V2));

    // truncated before the mount point
    EXPECT_FALSE(ParseMount("30 23 0:26 /", Version::V2));
}

TEST(CgroupMount, FindsV2MountOnHybridHost)
{
    auto mount = Mount::Find(kHybridMountInfo, Version::V2);

    ASSERT_TRUE(mount);
    EXPECT_EQ(mount->MountPoint, "/sys/fs/cgroup/unified");
}

TEST(CgroupMount, FindsV1CpuMountOnHybridHost)
{
    // skips `name=systemd` and `cpuset`, which come first
    auto mount = Mount::Find(kHybridMountInfo, Version::V1);

    ASSERT_TRUE(mount);
    EXPECT_EQ(mount->MountPoint, "/sys/fs/cgroup/cpu,cpuacct");
}

TEST(CgroupMount, FindsContainerMountWithNonRootRoot)
{
    auto mount = Mount::Find(kContainerMountInfo, Version::V2);

    ASSERT_TRUE(mount);
    EXPECT_EQ(mount->Root, "/docker/abc");
    EXPECT_EQ(mount->MountPoint, "/sys/fs/cgroup");
}

TEST(CgroupMount, MissingMountIsNothing)
{
    EXPECT_FALSE(Mount::Find(kContainerMountInfo, Version::V1));
    EXPECT_FALSE(Mount::Find("22 1 259:2 / / rw - ext4 /dev/nvme0n1p2 rw\n", Version::V2));
    EXPECT_FALSE(Mount::Find("", Version::V2));
}

TEST(CgroupMount, ResolvesDirAgainstRootMount)
{
    Mount mount = {
        .Root = "/",
        .MountPoint = "/sys/fs/cgroup",
    };

    EXPECT_EQ(mount.ResolveDirectory("/user.slice/a.scope"), "/sys/fs/cgroup/user.slice/a.scope");
    EXPECT_EQ(mount.ResolveDirectory("/"), "/sys/fs/cgroup");
}

TEST(CgroupMount, ResolvesDirAgainstContainerRoot)
{
    Mount mount = {
        .Root = "/docker/abc",
        .MountPoint = "/sys/fs/cgroup/cpu",
    };

    EXPECT_EQ(mount.ResolveDirectory("/docker/abc"), "/sys/fs/cgroup/cpu");
    EXPECT_EQ(mount.ResolveDirectory("/docker/abc/child"), "/sys/fs/cgroup/cpu/child");
}

TEST(CgroupMount, PathOutsideRootResolvesToMountPoint)
{
    Mount mount = {
        .Root = "/docker/abc",
        .MountPoint = "/sys/fs/cgroup/cpu",
    };

    // shares a string prefix but not a path component
    EXPECT_EQ(mount.ResolveDirectory("/docker/abcdef"), "/sys/fs/cgroup/cpu");
    EXPECT_EQ(mount.ResolveDirectory("/system.slice"), "/sys/fs/cgroup/cpu");
}

TEST(CgroupMount, EscapedPathResolvesToMountPoint)
{
    Mount mount = {
        .Root = "/",
        .MountPoint = "/sys/fs/cgroup",
    };

    EXPECT_EQ(mount.ResolveDirectory("/../../user.slice"), "/sys/fs/cgroup");

    // `..foo` is a normal name, not an escape
    EXPECT_EQ(mount.ResolveDirectory("/..foo"), "/sys/fs/cgroup/..foo");
}

TEST(CgroupMount, ResolvesFoundMountAndPathEndToEnd)
{
    auto mount = Mount::Find(kContainerMountInfo, Version::V2);
    auto path = FindCgroupPath("0::/docker/abc/worker\n", Version::V2);

    ASSERT_TRUE(mount);
    ASSERT_TRUE(path);
    EXPECT_EQ(mount->ResolveDirectory(path.Value()), "/sys/fs/cgroup/worker");
}

} // namespace violet::experimental::threading::internals::cgroups
