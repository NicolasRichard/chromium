// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_VIDEO_RENDERER_SINK_H_
#define MEDIA_BASE_VIDEO_RENDERER_SINK_H_

#include "base/basictypes.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/time/time.h"
#include "media/base/media_export.h"
#include "media/base/video_frame.h"

namespace media {

// VideoRendererSink is an interface representing the end-point for rendered
// video frames.  An implementation is expected to periodically call Render() on
// a callback object.
class MEDIA_EXPORT VideoRendererSink {
 public:
  class RenderCallback {
   public:
    // Returns a VideoFrame for rendering which should be displayed within the
    // presentation interval [|deadline_min|, |deadline_max|].  Returns NULL if
    // no frame or no new frame (since the last Render() call) is available for
    // rendering within the requested interval.  Intervals are expected to be
    // regular, contiguous, and monotonically increasing.  Irregular intervals
    // may affect the rendering decisions made by the underlying callback.
    virtual scoped_refptr<VideoFrame> Render(base::TimeTicks deadline_min,
                                             base::TimeTicks deadline_max) = 0;

    // Called by the sink when a VideoFrame previously returned via Render() was
    // not actually rendered.  Must be called before the next Render() call.
    virtual void OnFrameDropped() = 0;

    virtual ~RenderCallback() {}
  };

  // Starts video rendering.  See RenderCallback for more details.  Must be
  // called to receive Render() callbacks.  Callbacks may start immediately, so
  // |callback| must be ready to receive callbacks before calling Start().
  virtual void Start(RenderCallback* callback) = 0;

  // Stops video rendering, waits for any outstanding calls to the |callback|
  // given to Start() to complete before returning.  No new calls to |callback|
  // will be issued after this method returns.  May be used as a means of power
  // conservation by the sink implementation, so clients should call this
  // liberally if no new frames are expected.
  virtual void Stop() = 0;

  // Instead of using a callback driven rendering path, allow clients to paint
  // frames as they see fit without regard for the compositor.
  // TODO(dalecurtis): This should be nuked once experiments show the new path
  // is amazing and the old path is not! http://crbug.com/439548
  virtual void PaintFrameUsingOldRenderingPath(
      const scoped_refptr<VideoFrame>& frame) = 0;

  virtual ~VideoRendererSink() {}
};

}  // namespace media

#endif  // MEDIA_BASE_VIDEO_RENDERER_SINK_H_
