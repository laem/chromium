// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/platform_shared_memory_region.h"

#include "base/memory/shared_memory_mapping.h"
#include "base/process/process_metrics.h"
#include "base/sys_info.h"
#include "base/test/gtest_util.h"
#include "base/test/test_shared_memory_util.h"
#include "build/build_config.h"
#include "testing/gtest/include/gtest/gtest.h"

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include <mach/mach_vm.h>
#endif

namespace base {
namespace subtle {

const size_t kRegionSize = 1024;

class PlatformSharedMemoryRegionTest : public ::testing::Test {
 public:
  SharedMemoryMapping MapAt(PlatformSharedMemoryRegion* region,
                            off_t offset,
                            size_t bytes) {
    void* memory = nullptr;
    size_t mapped_size = 0;
    if (!region->MapAt(offset, bytes, &memory, &mapped_size))
      return {};

    return SharedMemoryMapping(memory, mapped_size, region->GetGUID());
  }

  void* GetMemory(SharedMemoryMapping* mapping) {
    return mapping->raw_memory_ptr();
  }
};

// Tests that a default constructed region is invalid and produces invalid
// mappings.
TEST_F(PlatformSharedMemoryRegionTest, DefaultConstructedRegionIsInvalid) {
  PlatformSharedMemoryRegion region;
  EXPECT_FALSE(region.IsValid());
  SharedMemoryMapping mapping = MapAt(&region, 0, kRegionSize);
  EXPECT_FALSE(mapping.IsValid());
  PlatformSharedMemoryRegion duplicate = region.Duplicate();
  EXPECT_FALSE(duplicate.IsValid());
  EXPECT_FALSE(region.ConvertToReadOnly());
}

// Tests that creating a region of 0 size returns an invalid region.
TEST_F(PlatformSharedMemoryRegionTest, CreateRegionOfZeroSizeIsInvalid) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(0);
  EXPECT_FALSE(region.IsValid());

  PlatformSharedMemoryRegion region2 =
      PlatformSharedMemoryRegion::CreateUnsafe(0);
  EXPECT_FALSE(region2.IsValid());
}

// Tests that creating a region of size bigger than the integer max value
// returns an invalid region.
TEST_F(PlatformSharedMemoryRegionTest, CreateTooLargeRegionIsInvalid) {
  size_t too_large_region_size =
      static_cast<size_t>(std::numeric_limits<int>::max()) + 1;
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(too_large_region_size);
  EXPECT_FALSE(region.IsValid());

  PlatformSharedMemoryRegion region2 =
      PlatformSharedMemoryRegion::CreateUnsafe(too_large_region_size);
  EXPECT_FALSE(region2.IsValid());
}

// Tests that the platform-specific handle converted to read-only cannot be used
// to perform a writable mapping with low-level system APIs like mmap().
TEST_F(PlatformSharedMemoryRegionTest, ReadOnlyHandleIsNotWritable) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  EXPECT_TRUE(region.ConvertToReadOnly());
  EXPECT_EQ(region.GetMode(), PlatformSharedMemoryRegion::Mode::kReadOnly);
  EXPECT_TRUE(
      CheckReadOnlyPlatformSharedMemoryRegionForTesting(std::move(region)));
}

// Tests that the PassPlatformHandle() call invalidates the region.
TEST_F(PlatformSharedMemoryRegionTest, InvalidAfterPass) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  ignore_result(region.PassPlatformHandle());
  EXPECT_FALSE(region.IsValid());
}

// Tests that the region is invalid after move.
TEST_F(PlatformSharedMemoryRegionTest, InvalidAfterMove) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  PlatformSharedMemoryRegion moved_region = std::move(region);
  EXPECT_FALSE(region.IsValid());
  EXPECT_TRUE(moved_region.IsValid());
}

// Tests that calling Take() with the size parameter equal to zero returns an
// invalid region.
TEST_F(PlatformSharedMemoryRegionTest, TakeRegionOfZeroSizeIsInvalid) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  PlatformSharedMemoryRegion region2 = PlatformSharedMemoryRegion::Take(
      region.PassPlatformHandle(), region.GetMode(), 0, region.GetGUID());
  EXPECT_FALSE(region2.IsValid());
}

// Tests that calling Take() with the size parameter bigger than the integer max
// value returns an invalid region.
TEST_F(PlatformSharedMemoryRegionTest, TakeTooLargeRegionIsInvalid) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  PlatformSharedMemoryRegion region2 = PlatformSharedMemoryRegion::Take(
      region.PassPlatformHandle(), region.GetMode(),
      static_cast<size_t>(std::numeric_limits<int>::max()) + 1,
      region.GetGUID());
  EXPECT_FALSE(region2.IsValid());
}

// Tests that mapping bytes out of the region limits fails.
TEST_F(PlatformSharedMemoryRegionTest, MapAtOutOfTheRegionLimitsTest) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  SharedMemoryMapping mapping = MapAt(&region, 0, region.GetSize() + 1);
  EXPECT_FALSE(mapping.IsValid());
}

// Tests that mapping with a size and offset causing overflow fails.
TEST_F(PlatformSharedMemoryRegionTest, MapAtWithOverflowTest) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(
          SysInfo::VMAllocationGranularity() * 2);
  ASSERT_TRUE(region.IsValid());
  size_t size = std::numeric_limits<size_t>::max();
  size_t offset = SysInfo::VMAllocationGranularity();
  // |size| + |offset| should be below the region size due to overflow but
  // mapping a region with these parameters should be invalid.
  EXPECT_LT(size + offset, region.GetSize());
  SharedMemoryMapping mapping = MapAt(&region, offset, size);
  EXPECT_FALSE(mapping.IsValid());
}

#if defined(OS_POSIX) && !defined(OS_ANDROID) && !defined(OS_FUCHSIA) && \
    !defined(OS_MACOSX)
// Tests that the second handle is closed after a conversion to read-only on
// POSIX.
TEST_F(PlatformSharedMemoryRegionTest,
       ConvertToReadOnlyInvalidatesSecondHandle) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  ASSERT_TRUE(region.ConvertToReadOnly());
  FDPair fds = region.GetPlatformHandle();
  EXPECT_LT(fds.readonly_fd, 0);
}
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
// Tests that protection bits are set correctly for read-only region on MacOS.
TEST_F(PlatformSharedMemoryRegionTest, MapCurrentAndMaxProtectionSetCorrectly) {
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  ASSERT_TRUE(region.ConvertToReadOnly());
  SharedMemoryMapping ro_mapping = MapAt(&region, 0, kRegionSize);
  ASSERT_TRUE(ro_mapping.IsValid());

  vm_region_basic_info_64 basic_info;
  mach_vm_size_t dummy_size = 0;
  void* temp_addr = GetMemory(&ro_mapping);
  MachVMRegionResult result = GetBasicInfo(
      mach_task_self(), &dummy_size,
      reinterpret_cast<mach_vm_address_t*>(&temp_addr), &basic_info);
  EXPECT_EQ(result, MachVMRegionResult::Success);
  EXPECT_EQ(basic_info.protection & VM_PROT_ALL, VM_PROT_READ);
  EXPECT_EQ(basic_info.max_protection & VM_PROT_ALL, VM_PROT_READ);
}
#endif

// Tests that platform handle permissions are checked correctly.
TEST_F(PlatformSharedMemoryRegionTest,
       CheckPlatformHandlePermissionsCorrespondToMode) {
  using Mode = PlatformSharedMemoryRegion::Mode;
  auto check = [](const PlatformSharedMemoryRegion& region,
                  PlatformSharedMemoryRegion::Mode mode) {
    return PlatformSharedMemoryRegion::
        CheckPlatformHandlePermissionsCorrespondToMode(
            region.GetPlatformHandle(), mode, region.GetSize());
  };

  // Check kWritable region.
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  EXPECT_TRUE(check(region, Mode::kWritable));

  // Check kReadOnly region.
  ASSERT_TRUE(region.ConvertToReadOnly());
  EXPECT_TRUE(check(region, Mode::kReadOnly));

  // Check kUnsafe region.
  PlatformSharedMemoryRegion region2 =
      PlatformSharedMemoryRegion::CreateUnsafe(kRegionSize);
  ASSERT_TRUE(region2.IsValid());
  EXPECT_TRUE(check(region2, Mode::kUnsafe));

  // TODO(https://crbug.com/825177): add negative expectations once all
  // platforms implement this check.
}

// Tests that it's impossible to create read-only platform shared memory region.
TEST_F(PlatformSharedMemoryRegionTest, CreateReadOnlyRegionDeathTest) {
#ifdef OFFICIAL_BUILD
  // The official build does not print the reason a CHECK failed.
  const char kErrorRegex[] = "";
#else
  const char kErrorRegex[] =
      "Creating a region in read-only mode will lead to this region being "
      "non-modifiable";
#endif
  EXPECT_DEATH_IF_SUPPORTED(
      PlatformSharedMemoryRegion::Create(
          PlatformSharedMemoryRegion::Mode::kReadOnly, kRegionSize),
      kErrorRegex);
}

// Tests that it's prohibited to duplicate a writable region.
TEST_F(PlatformSharedMemoryRegionTest, DuplicateWritableRegionDeathTest) {
#ifdef OFFICIAL_BUILD
  const char kErrorRegex[] = "";
#else
  const char kErrorRegex[] =
      "Duplicating a writable shared memory region is prohibited";
#endif
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  EXPECT_DEATH_IF_SUPPORTED(region.Duplicate(), kErrorRegex);
}

// Tests that it's prohibited to convert an unsafe region to read-only.
TEST_F(PlatformSharedMemoryRegionTest, UnsafeRegionConvertToReadOnlyDeathTest) {
#ifdef OFFICIAL_BUILD
  const char kErrorRegex[] = "";
#else
  const char kErrorRegex[] =
      "Only writable shared memory region can be converted to read-only";
#endif
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateUnsafe(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  EXPECT_DEATH_IF_SUPPORTED(region.ConvertToReadOnly(), kErrorRegex);
}

// Tests that it's prohibited to convert a read-only region to read-only.
TEST_F(PlatformSharedMemoryRegionTest,
       ReadOnlyRegionConvertToReadOnlyDeathTest) {
#ifdef OFFICIAL_BUILD
  const char kErrorRegex[] = "";
#else
  const char kErrorRegex[] =
      "Only writable shared memory region can be converted to read-only";
#endif
  PlatformSharedMemoryRegion region =
      PlatformSharedMemoryRegion::CreateWritable(kRegionSize);
  ASSERT_TRUE(region.IsValid());
  EXPECT_TRUE(region.ConvertToReadOnly());
  EXPECT_DEATH_IF_SUPPORTED(region.ConvertToReadOnly(), kErrorRegex);
}

}  // namespace subtle
}  // namespace base
