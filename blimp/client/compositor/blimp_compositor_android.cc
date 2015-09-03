// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "blimp/client/compositor/blimp_compositor_android.h"

#include <android/native_window_jni.h>

#include "base/command_line.h"
#include "base/memory/scoped_ptr.h"
#include "ui/gfx/geometry/size.h"

namespace {
// The minimum valid content width in a tile.  This is used to make sure the
// width of the content on the rightmost tile isn't a tiny sliver.
const int kMinimumTileContentWidthPixels = 64;
}

namespace blimp {

// static
scoped_ptr<BlimpCompositorAndroid> BlimpCompositorAndroid::Create(
    const gfx::Size& real_size,
    const gfx::Size& size,
    float dp_to_px) {
  gfx::Size device_size(real_size);
  bool real_size_supported = true;
  if (device_size.IsEmpty()) {
    real_size_supported = false;
    device_size = size;
  }
  return make_scoped_ptr(
      new BlimpCompositorAndroid(device_size, real_size_supported, dp_to_px));
}

BlimpCompositorAndroid::BlimpCompositorAndroid(const gfx::Size& size,
                                               bool real_size_supported,
                                               float dp_to_px)
    : BlimpCompositor(dp_to_px),
      portrait_width_(std::min(size.width(), size.height())),
      landscape_width_(std::max(size.width(), size.height())),
      real_size_supported_(real_size_supported),
      window_(nullptr) {}

BlimpCompositorAndroid::~BlimpCompositorAndroid() {
  SetSurface(nullptr, 0 /* null surface */);
}

void BlimpCompositorAndroid::SetSurface(JNIEnv* env, jobject jsurface) {
  if (window_) {
    SetVisible(false);
    ANativeWindow_release(window_);
    window_ = nullptr;
  }

  if (!jsurface)
    return;

  base::android::ScopedJavaLocalFrame scoped_local_reference_frame(env);
  window_ = ANativeWindow_fromSurface(env, jsurface);
  SetVisible(true);
}

gfx::AcceleratedWidget BlimpCompositorAndroid::GetWindow() {
  return window_;
}

void BlimpCompositorAndroid::GenerateLayerTreeSettings(
    cc::LayerTreeSettings* settings) {
  BlimpCompositor::GenerateLayerTreeSettings(settings);

  // Calculate the correct raster tile size to use.  Assuming a square tile.
  DCHECK_EQ(settings->default_tile_size.width(),
            settings->default_tile_size.height());

  int default_tile_size = settings->default_tile_size.width();
  if (real_size_supported_) {
    // Maximum HD dimensions should be 768x1280
    // Maximum FHD dimensions should be 1200x1920
    if (portrait_width_ > 768 || landscape_width_ > 1280)
      default_tile_size = 384;
    if (portrait_width_ > 1200 || landscape_width_ > 1920)
      default_tile_size = 512;

    // Adjust for some resolutions that barely straddle an extra
    // tile when in portrait mode. This helps worst case scroll/raster
    // by not needing a full extra tile for each row.
    int right_tile_width = ((portrait_width_ - 1) % default_tile_size) + 1;
    if (right_tile_width < kMinimumTileContentWidthPixels) {
      // Figure out the new tile count without the small edge tile.
      int full_tile_count = portrait_width_ / default_tile_size;
      DCHECK_GT(full_tile_count, 0);

      // Calculate the ideal new tile width with the new tile count.
      default_tile_size = std::ceil(static_cast<float>(portrait_width_) /
                                    static_cast<float>(full_tile_count));

      // Round up to nearest 32 for GPU efficiency.
      if (default_tile_size & 0x1F)
        default_tile_size = (default_tile_size & ~0x1F) + 32;
    }
  } else {
    // We don't know the exact resolution due to screen controls etc., so this
    // just estimates the values above using tile counts.
    int numTiles = (portrait_width_ * landscape_width_) / (256 * 256);
    if (numTiles > 16)
      default_tile_size = 384;
    if (numTiles >= 40)
      default_tile_size = 512;
  }
  settings->default_tile_size.SetSize(default_tile_size, default_tile_size);
}

}  // namespace blimp
