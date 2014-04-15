// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MediaValuesCached_h
#define MediaValuesCached_h

#include "core/css/MediaValues.h"

namespace WebCore {

class MediaValuesCached FINAL : public MediaValues {
public:
    struct MediaValuesCachedData {
        // Members variables must be thread safe, since they're copied to the parser thread
        int viewportWidth;
        int viewportHeight;
        int deviceWidth;
        int deviceHeight;
        float devicePixelRatio;
        int colorBitsPerComponent;
        int monochromeBitsPerComponent;
        PointerDeviceType pointer;
        int defaultFontSize;
        int computedFontSize;
        bool hasXHeight;
        double xHeight;
        double zeroWidth;
        bool threeDEnabled;
        bool scanMediaType;
        bool screenMediaType;
        bool printMediaType;
        bool strictMode;
    };

    static PassRefPtr<MediaValues> create(Document&);
    static PassRefPtr<MediaValues> create(LocalFrame*, RenderStyle*);
    static PassRefPtr<MediaValues> create(MediaValuesCachedData&);
    virtual PassRefPtr<MediaValues> copy() const OVERRIDE;
    virtual bool isSafeToSendToAnotherThread() const OVERRIDE;
    virtual bool computeLength(double value, unsigned short type, int& result) const OVERRIDE;

    virtual int viewportWidth() const OVERRIDE;
    virtual int viewportHeight() const OVERRIDE;
    virtual int deviceWidth() const OVERRIDE;
    virtual int deviceHeight() const OVERRIDE;
    virtual float devicePixelRatio() const OVERRIDE;
    virtual int colorBitsPerComponent() const OVERRIDE;
    virtual int monochromeBitsPerComponent() const OVERRIDE;
    virtual PointerDeviceType pointer() const OVERRIDE;
    virtual bool threeDEnabled() const OVERRIDE;
    virtual bool scanMediaType() const OVERRIDE;
    virtual bool screenMediaType() const OVERRIDE;
    virtual bool printMediaType() const OVERRIDE;
    virtual bool strictMode() const OVERRIDE;
    virtual Document* document() const OVERRIDE;
    virtual bool hasValues() const OVERRIDE;

protected:
    MediaValuesCached(LocalFrame*, RenderStyle*);
    MediaValuesCached(const MediaValuesCachedData&);

    MediaValuesCachedData m_data;
};

} // namespace

#endif // MediaValuesCached_h
