// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_BLINK_WEB_LAYER_IMPL_FIXED_BOUNDS_H_
#define CC_BLINK_WEB_LAYER_IMPL_FIXED_BOUNDS_H_

#include "cc/blink/web_layer_impl.h"
#include "ui/gfx/size.h"
#include "ui/gfx/transform.h"

namespace cc_blink {

// A special implementation of WebLayerImpl for layers that its contents
// need to be automatically scaled when the bounds changes. The compositor
// can efficiently handle the bounds change of such layers if the bounds
// is fixed to a given value and the change of bounds are converted to
// transformation scales.
class WebLayerImplFixedBounds : public WebLayerImpl {
 public:
  CC_BLINK_EXPORT WebLayerImplFixedBounds();
  CC_BLINK_EXPORT explicit WebLayerImplFixedBounds(scoped_refptr<cc::Layer>);
  virtual ~WebLayerImplFixedBounds();

  // WebLayerImpl overrides.
  virtual void invalidateRect(const blink::WebFloatRect& rect) override;
  virtual void invalidateRect(const blink::WebRect& rect) override;
  virtual void setTransformOrigin(
      const blink::WebFloatPoint3D& transform_origin);
  virtual void setBounds(const blink::WebSize& bounds);
  virtual blink::WebSize bounds() const;
  virtual void setTransform(const SkMatrix44& transform);
  virtual SkMatrix44 transform() const;

  CC_BLINK_EXPORT void SetFixedBounds(gfx::Size bounds);

 protected:
  void SetTransformInternal(const gfx::Transform& transform);
  void UpdateLayerBoundsAndTransform();

  gfx::Transform original_transform_;
  gfx::Size original_bounds_;
  gfx::Size fixed_bounds_;

 private:
  DISALLOW_COPY_AND_ASSIGN(WebLayerImplFixedBounds);
};

}  // namespace cc_blink

#endif  // CC_BLINK_WEB_LAYER_IMPL_FIXED_BOUNDS_H_
