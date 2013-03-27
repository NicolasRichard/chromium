// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/image/image_png_rep.h"

#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/size.h"

namespace gfx {

ImagePNGRep::ImagePNGRep()
    : raw_data(NULL),
      scale_factor(ui::SCALE_FACTOR_NONE) {
}

ImagePNGRep::ImagePNGRep(const scoped_refptr<base::RefCountedMemory>& data,
                         ui::ScaleFactor data_scale_factor)
    : raw_data(data),
      scale_factor(data_scale_factor) {
}

ImagePNGRep::~ImagePNGRep() {
}

gfx::Size ImagePNGRep::Size() const {
  // The only way to get the width and height of a raw PNG stream, at least
  // using the gfx::PNGCodec API, is to decode the whole thing.
  CHECK(raw_data.get());
  SkBitmap bitmap;
  if (!gfx::PNGCodec::Decode(raw_data->front(), raw_data->size(),
                             &bitmap)) {
    LOG(ERROR) << "Unable to decode PNG.";
    return gfx::Size(0, 0);
  }
  return gfx::Size(bitmap.width(), bitmap.height());
}

}  // namespace gfx
