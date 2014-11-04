// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/gpu/client/gpu_memory_buffer_impl.h"

#include "content/common/gpu/client/gpu_memory_buffer_impl_ozone_native_buffer.h"
#include "content/common/gpu/client/gpu_memory_buffer_impl_shared_memory.h"

namespace content {

// static
void GpuMemoryBufferImpl::Create(const gfx::Size& size,
                                 Format format,
                                 Usage usage,
                                 int client_id,
                                 const CreationCallback& callback) {
  if (GpuMemoryBufferImplOzoneNativeBuffer::IsConfigurationSupported(format,
                                                                     usage)) {
    GpuMemoryBufferImplOzoneNativeBuffer::Create(
        size, format, client_id, callback);
    return;
  }

  if (GpuMemoryBufferImplSharedMemory::IsConfigurationSupported(
          size, format, usage)) {
    GpuMemoryBufferImplSharedMemory::Create(size, format, callback);
    return;
  }

  callback.Run(scoped_ptr<GpuMemoryBufferImpl>());
}

// static
void GpuMemoryBufferImpl::AllocateForChildProcess(
    const gfx::Size& size,
    Format format,
    Usage usage,
    base::ProcessHandle child_process,
    int child_client_id,
    const AllocationCallback& callback) {
  if (GpuMemoryBufferImplOzoneNativeBuffer::IsConfigurationSupported(format,
                                                                     usage)) {
    GpuMemoryBufferImplOzoneNativeBuffer::AllocateForChildProcess(
        size, format, child_client_id, callback);
    return;
  }

  if (GpuMemoryBufferImplSharedMemory::IsConfigurationSupported(
          size, format, usage)) {
    GpuMemoryBufferImplSharedMemory::AllocateForChildProcess(
        size, format, child_process, callback);
    return;
  }

  callback.Run(gfx::GpuMemoryBufferHandle());
}

// static
void GpuMemoryBufferImpl::DeletedByChildProcess(
    gfx::GpuMemoryBufferType type,
    const gfx::GpuMemoryBufferId& id,
    base::ProcessHandle child_process,
    int child_client_id,
    uint32_t sync_point) {
  switch (type) {
    case gfx::SHARED_MEMORY_BUFFER:
      break;
    case gfx::OZONE_NATIVE_BUFFER:
      if (id.secondary_id != child_client_id) {
        LOG(ERROR)
            << "Child attempting to delete GpuMemoryBuffer it does not own";
      } else {
        GpuMemoryBufferImplOzoneNativeBuffer::DeletedByChildProcess(id,
                                                                    sync_point);
      }
      break;
    default:
      NOTREACHED();
  }
}

// static
scoped_ptr<GpuMemoryBufferImpl> GpuMemoryBufferImpl::CreateFromHandle(
    const gfx::GpuMemoryBufferHandle& handle,
    const gfx::Size& size,
    Format format,
    const DestructionCallback& callback) {
  switch (handle.type) {
    case gfx::SHARED_MEMORY_BUFFER:
      return GpuMemoryBufferImplSharedMemory::CreateFromHandle(
          handle, size, format, callback);
    case gfx::OZONE_NATIVE_BUFFER:
      return GpuMemoryBufferImplOzoneNativeBuffer::CreateFromHandle(
          handle, size, format, callback);
    default:
      return scoped_ptr<GpuMemoryBufferImpl>();
  }
}

}  // namespace content
