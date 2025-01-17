// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/shared_memory_mapping.h"

#include <utility>

#include "base/logging.h"
#include "base/memory/shared_memory_tracker.h"
#include "base/unguessable_token.h"
#include "build/build_config.h"

#if defined(OS_POSIX)
#include <sys/mman.h>
#endif

#if defined(OS_WIN)
#include <aclapi.h>
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include <mach/mach_vm.h>
#include "base/mac/mach_logging.h"
#endif

#if defined(OS_FUCHSIA)
#include <zircon/process.h>
#include <zircon/status.h>
#include <zircon/syscalls.h>
#endif

namespace base {

SharedMemoryMapping::SharedMemoryMapping() = default;

SharedMemoryMapping::SharedMemoryMapping(SharedMemoryMapping&& mapping)
    : memory_(mapping.memory_), size_(mapping.size_), guid_(mapping.guid_) {
  mapping.memory_ = nullptr;
}

SharedMemoryMapping& SharedMemoryMapping::operator=(
    SharedMemoryMapping&& mapping) {
  Unmap();
  memory_ = mapping.memory_;
  size_ = mapping.size_;
  guid_ = mapping.guid_;
  mapping.memory_ = nullptr;
  return *this;
}

SharedMemoryMapping::~SharedMemoryMapping() {
  Unmap();
}

SharedMemoryMapping::SharedMemoryMapping(void* memory,
                                         size_t size,
                                         const UnguessableToken& guid)
    : memory_(memory), size_(size), guid_(guid) {
  SharedMemoryTracker::GetInstance()->IncrementMemoryUsage(*this);
}

void SharedMemoryMapping::Unmap() {
  if (!IsValid())
    return;

  SharedMemoryTracker::GetInstance()->DecrementMemoryUsage(*this);
#if defined(OS_WIN)
  if (!UnmapViewOfFile(memory_))
    DPLOG(ERROR) << "UnmapViewOfFile";
#elif defined(OS_FUCHSIA)
  uintptr_t addr = reinterpret_cast<uintptr_t>(memory_);
  zx_status_t status = zx_vmar_unmap(zx_vmar_root_self(), addr, size_);
  DLOG_IF(ERROR, status != ZX_OK)
      << "zx_vmar_unmap failed: " << zx_status_get_string(status);
#elif defined(OS_MACOSX) && !defined(OS_IOS)
  kern_return_t kr = mach_vm_deallocate(
      mach_task_self(), reinterpret_cast<mach_vm_address_t>(memory_), size_);
  MACH_DLOG_IF(ERROR, kr != KERN_SUCCESS, kr) << "mach_vm_deallocate";
#else
  if (munmap(memory_, size_) < 0)
    DPLOG(ERROR) << "munmap";
#endif
}

ReadOnlySharedMemoryMapping::ReadOnlySharedMemoryMapping() = default;
ReadOnlySharedMemoryMapping::ReadOnlySharedMemoryMapping(
    ReadOnlySharedMemoryMapping&&) = default;
ReadOnlySharedMemoryMapping& ReadOnlySharedMemoryMapping::operator=(
    ReadOnlySharedMemoryMapping&&) = default;
ReadOnlySharedMemoryMapping::ReadOnlySharedMemoryMapping(
    void* address,
    size_t size,
    const UnguessableToken& guid)
    : SharedMemoryMapping(address, size, guid) {}

WritableSharedMemoryMapping::WritableSharedMemoryMapping() = default;
WritableSharedMemoryMapping::WritableSharedMemoryMapping(
    WritableSharedMemoryMapping&&) = default;
WritableSharedMemoryMapping& WritableSharedMemoryMapping::operator=(
    WritableSharedMemoryMapping&&) = default;
WritableSharedMemoryMapping::WritableSharedMemoryMapping(
    void* address,
    size_t size,
    const UnguessableToken& guid)
    : SharedMemoryMapping(address, size, guid) {}

}  // namespace base
