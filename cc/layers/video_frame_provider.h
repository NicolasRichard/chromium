// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_LAYERS_VIDEO_FRAME_PROVIDER_H_
#define CC_LAYERS_VIDEO_FRAME_PROVIDER_H_

#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "cc/base/cc_export.h"

namespace media {
class VideoFrame;
}

namespace cc {

// VideoFrameProvider and VideoFrameProvider::Client define the relationship by
// which video frames are exchanged between a provider and client.
//
// Threading notes: This class may be used in a multithreaded manner. However,
// if the Client implementation calls GetCurrentFrame()/PutCurrentFrame() from
// one thread, the provider must ensure that all client methods (except
// StopUsingProvider()) are called from that thread (typically the compositor
// thread).
class CC_EXPORT VideoFrameProvider {
 public:
  class CC_EXPORT Client {
   public:
    // The provider will call this method to tell the client to stop using it.
    // StopUsingProvider() may be called from any thread. The client should
    // block until it has PutCurrentFrame() any outstanding frames.
    virtual void StopUsingProvider() = 0;

    // Notifies the client that it should start or stop making regular
    // UpdateCurrentFrame() calls to the provider. No further calls to
    // UpdateCurrentFrame() should be made once StopRendering() returns.
    //
    // Callers should use these methods to indicate when it expects and no
    // longer expects (respectively) to have new frames for the client. Clients
    // may use this information for power conservation.
    virtual void StartRendering() = 0;
    virtual void StopRendering() = 0;

    // Notifies the client that GetCurrentFrame() will return new data.
    // TODO(dalecurtis): Nuke this once VideoFrameProviderClientImpl is using a
    // BeginFrameObserver based approach. http://crbug.com/336733
    virtual void DidReceiveFrame() = 0;

    // Notifies the client of a new UV transform matrix to be used.
    virtual void DidUpdateMatrix(const float* matrix) = 0;

   protected:
    virtual ~Client() {}
  };

  // May be called from any thread, but there must be some external guarantee
  // that the provider is not destroyed before this call returns.
  virtual void SetVideoFrameProviderClient(Client* client) = 0;

  // Called by the client on a regular interval. Returns true if a new frame
  // will be available via GetCurrentFrame() which should be displayed within
  // the presentation interval [|deadline_min|, |deadline_max|].
  //
  // Implementations may use this to drive frame acquisition from underlying
  // sources, so it must be called by clients before calling GetCurrentFrame().
  virtual bool UpdateCurrentFrame(base::TimeTicks deadline_min,
                                  base::TimeTicks deadline_max) = 0;

  // Returns the current frame, which may have been updated by a recent call to
  // UpdateCurrentFrame(). A call to this method does not ensure that the frame
  // will be rendered. A subsequent call to PutCurrentFrame() must be made if
  // the frame is expected to be rendered.
  //
  // Clients should call this in response to UpdateCurrentFrame() returning true
  // or in response to a DidReceiveFrame() call.
  //
  // TODO(dalecurtis): Remove text about DidReceiveFrame() once the old path
  // has been removed. http://crbug.com/439548
  virtual scoped_refptr<media::VideoFrame> GetCurrentFrame() = 0;

  // Indicates that the last frame returned via GetCurrentFrame() is expected to
  // be rendered. Must only occur after a previous call to GetCurrentFrame().
  virtual void PutCurrentFrame() = 0;

 protected:
  virtual ~VideoFrameProvider() {}
};

}  // namespace cc

#endif  // CC_LAYERS_VIDEO_FRAME_PROVIDER_H_
