// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_RENDERER_HOST_RENDER_MESSAGE_FILTER_H_
#define CONTENT_BROWSER_RENDERER_HOST_RENDER_MESSAGE_FILTER_H_

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "base/memory/linked_ptr.h"
#include "base/memory/shared_memory.h"
#include "base/sequenced_task_runner_helpers.h"
#include "base/strings/string16.h"
#include "build/build_config.h"
#include "cc/resources/shared_bitmap_manager.h"
#include "content/common/host_discardable_shared_memory_manager.h"
#include "content/common/host_shared_bitmap_manager.h"
#include "content/public/browser/browser_message_filter.h"
#include "ipc/message_filter.h"
#include "media/audio/audio_parameters.h"
#include "media/base/channel_layout.h"
#include "third_party/WebKit/public/web/WebPopupType.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/gpu_memory_buffer.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/surface/transport_dib.h"

#if defined(OS_MACOSX)
#include <IOSurface/IOSurface.h>
#endif

#if defined(OS_ANDROID)
#include "base/threading/worker_pool.h"
#endif

class GURL;
struct FontDescriptor;
struct ViewHostMsg_CreateWindow_Params;

namespace blink {
struct WebScreenInfo;
}

namespace base {
class ProcessMetrics;
class SharedMemory;
class TaskRunner;
}

namespace gfx {
struct GpuMemoryBufferHandle;
}

namespace media {
class AudioManager;
struct MediaLogEvent;
}

namespace net {
class URLRequestContext;
class URLRequestContextGetter;
}

namespace content {
class BrowserContext;
class DOMStorageContextWrapper;
class MediaInternals;
class RenderWidgetHelper;
class ResourceContext;
class ResourceDispatcherHostImpl;
struct Referrer;

// This class filters out incoming IPC messages for the renderer process on the
// IPC thread.
class CONTENT_EXPORT RenderMessageFilter : public BrowserMessageFilter {
 public:
  // Create the filter.
  RenderMessageFilter(int render_process_id,
                      BrowserContext* browser_context,
                      net::URLRequestContextGetter* request_context,
                      RenderWidgetHelper* render_widget_helper,
                      media::AudioManager* audio_manager,
                      MediaInternals* media_internals,
                      DOMStorageContextWrapper* dom_storage_context);

  // BrowserMessageFilter methods:
  bool OnMessageReceived(const IPC::Message& message) override;
  void OnDestruct() const override;
  void OverrideThreadForMessage(const IPC::Message& message,
                                BrowserThread::ID* thread) override;
  base::TaskRunner* OverrideTaskRunnerForMessage(
      const IPC::Message& message) override;

  int render_process_id() const { return render_process_id_; }

  // Returns the correct net::URLRequestContext depending on what type of url is
  // given.
  // Only call on the IO thread.
  net::URLRequestContext* GetRequestContextForURL(const GURL& url);

 protected:
  ~RenderMessageFilter() override;

  // This method will be overridden by TestSaveImageFromDataURL class for test.
  virtual void DownloadUrl(int render_view_id,
                           int render_frame_id,
                           const GURL& url,
                           const Referrer& referrer,
                           const base::string16& suggested_name,
                           const bool use_prompt) const;

 private:
  friend class BrowserThread;
  friend class base::DeleteHelper<RenderMessageFilter>;

  void OnGetProcessMemorySizes(size_t* private_bytes, size_t* shared_bytes);
  void OnCreateWindow(const ViewHostMsg_CreateWindow_Params& params,
                      int* route_id,
                      int* main_frame_route_id,
                      int* surface_id,
                      int64* cloned_session_storage_namespace_id);
  void OnCreateWidget(int opener_id,
                      blink::WebPopupType popup_type,
                      int* route_id,
                      int* surface_id);
  void OnCreateFullscreenWidget(int opener_id,
                                int* route_id,
                                int* surface_id);

  void OnGenerateRoutingID(int* route_id);
  void OnDownloadUrl(int render_view_id,
                     int render_frame_id,
                     const GURL& url,
                     const Referrer& referrer,
                     const base::string16& suggested_name);
  void OnSaveImageFromDataURL(int render_view_id,
                              int render_frame_id,
                              const std::string& url_str);

  void OnGetAudioHardwareConfig(media::AudioParameters* input_params,
                                media::AudioParameters* output_params);

#if defined(OS_WIN)
  // Used to look up the monitor color profile.
  void OnGetMonitorColorProfile(std::vector<char>* profile);
#endif

  // Used to ask the browser to allocate a block of shared memory for the
  // renderer to send back data in, since shared memory can't be created
  // in the renderer on POSIX due to the sandbox.
  void AllocateSharedMemoryOnFileThread(uint32 buffer_size,
                                        IPC::Message* reply_msg);
  void OnAllocateSharedMemory(uint32 buffer_size, IPC::Message* reply_msg);
  void AllocateSharedBitmapOnFileThread(uint32 buffer_size,
                                        const cc::SharedBitmapId& id,
                                        IPC::Message* reply_msg);
  void OnAllocateSharedBitmap(uint32 buffer_size,
                              const cc::SharedBitmapId& id,
                              IPC::Message* reply_msg);
  void OnAllocatedSharedBitmap(size_t buffer_size,
                               const base::SharedMemoryHandle& handle,
                               const cc::SharedBitmapId& id);
  void OnDeletedSharedBitmap(const cc::SharedBitmapId& id);
  void OnResolveProxy(const GURL& url, IPC::Message* reply_msg);

  // Browser side discardable shared memory allocation.
  void AllocateLockedDiscardableSharedMemoryOnFileThread(
      uint32 size,
      DiscardableSharedMemoryId id,
      IPC::Message* reply_message);
  void OnAllocateLockedDiscardableSharedMemory(uint32 size,
                                               DiscardableSharedMemoryId id,
                                               IPC::Message* reply_message);
  void DeletedDiscardableSharedMemoryOnFileThread(DiscardableSharedMemoryId id);
  void OnDeletedDiscardableSharedMemory(DiscardableSharedMemoryId id);

  void OnCacheableMetadataAvailable(const GURL& url,
                                    base::Time expected_response_time,
                                    const std::vector<char>& data);
  void OnMediaLogEvents(const std::vector<media::MediaLogEvent>&);

  bool CheckBenchmarkingEnabled() const;
  bool CheckPreparsedJsCachingEnabled() const;

#if defined(OS_ANDROID)
  void OnWebAudioMediaCodec(base::SharedMemoryHandle encoded_data_handle,
                            base::FileDescriptor pcm_output,
                            uint32_t data_size);
#endif

  void OnAllocateGpuMemoryBuffer(gfx::GpuMemoryBufferId id,
                                 uint32 width,
                                 uint32 height,
                                 gfx::BufferFormat format,
                                 gfx::BufferUsage usage,
                                 IPC::Message* reply);
  void GpuMemoryBufferAllocated(IPC::Message* reply,
                                const gfx::GpuMemoryBufferHandle& handle);
  void OnDeletedGpuMemoryBuffer(gfx::GpuMemoryBufferId id,
                                uint32 sync_point);

  // Cached resource request dispatcher host, guaranteed to be non-null. We do
  // not own it; it is managed by the BrowserProcess, which has a wider scope
  // than we do.
  ResourceDispatcherHostImpl* resource_dispatcher_host_;

  HostSharedBitmapManagerClient bitmap_manager_client_;

  // Contextual information to be used for requests created here.
  scoped_refptr<net::URLRequestContextGetter> request_context_;

  // The ResourceContext which is to be used on the IO thread.
  ResourceContext* resource_context_;

  scoped_refptr<RenderWidgetHelper> render_widget_helper_;

  scoped_refptr<DOMStorageContextWrapper> dom_storage_context_;

  int render_process_id_;

  media::AudioManager* audio_manager_;
  MediaInternals* media_internals_;

  DISALLOW_COPY_AND_ASSIGN(RenderMessageFilter);
};

}  // namespace content

#endif  // CONTENT_BROWSER_RENDERER_HOST_RENDER_MESSAGE_FILTER_H_
