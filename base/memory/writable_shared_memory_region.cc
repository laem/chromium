// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/writable_shared_memory_region.h"

#include <utility>

#include "base/memory/shared_memory.h"
#include "build/build_config.h"

namespace base {

// static
WritableSharedMemoryRegion WritableSharedMemoryRegion::Create(size_t size) {
  subtle::PlatformSharedMemoryRegion handle =
      subtle::PlatformSharedMemoryRegion::CreateWritable(size);

  return WritableSharedMemoryRegion(std::move(handle));
}

// static
WritableSharedMemoryRegion WritableSharedMemoryRegion::Deserialize(
    subtle::PlatformSharedMemoryRegion handle) {
  return WritableSharedMemoryRegion(std::move(handle));
}

// static
subtle::PlatformSharedMemoryRegion
WritableSharedMemoryRegion::TakeHandleForSerialization(
    WritableSharedMemoryRegion region) {
  return std::move(region.handle_);
}

// static
ReadOnlySharedMemoryRegion WritableSharedMemoryRegion::ConvertToReadOnly(
    WritableSharedMemoryRegion region) {
  subtle::PlatformSharedMemoryRegion handle = std::move(region.handle_);
  if (!handle.ConvertToReadOnly())
    return {};

  return ReadOnlySharedMemoryRegion::Deserialize(std::move(handle));
}

WritableSharedMemoryRegion::WritableSharedMemoryRegion() = default;
WritableSharedMemoryRegion::WritableSharedMemoryRegion(
    WritableSharedMemoryRegion&& region) = default;
WritableSharedMemoryRegion& WritableSharedMemoryRegion::operator=(
    WritableSharedMemoryRegion&& region) = default;
WritableSharedMemoryRegion::~WritableSharedMemoryRegion() = default;

WritableSharedMemoryMapping WritableSharedMemoryRegion::Map() {
  return MapAt(0, handle_.GetSize());
}

WritableSharedMemoryMapping WritableSharedMemoryRegion::MapAt(off_t offset,
                                                              size_t size) {
  if (!IsValid())
    return {};

  void* memory = nullptr;
  size_t mapped_size = 0;
  if (!handle_.MapAt(offset, size, &memory, &mapped_size))
    return {};

  return WritableSharedMemoryMapping(memory, mapped_size, handle_.GetGUID());
}

bool WritableSharedMemoryRegion::IsValid() const {
  return handle_.IsValid();
}

WritableSharedMemoryRegion::WritableSharedMemoryRegion(
    subtle::PlatformSharedMemoryRegion handle)
    : handle_(std::move(handle)) {
  CHECK_EQ(handle_.GetMode(),
           subtle::PlatformSharedMemoryRegion::Mode::kWritable);
}

}  // namespace base
