/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MediaStreamCenterChromium_h
#define MediaStreamCenterChromium_h

#include "core/platform/mediastream/MediaStreamCenter.h"

#include "public/platform/WebMediaStreamCenterClient.h"
#include <wtf/OwnPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/text/WTFString.h>

namespace WebKit {
class WebMediaStream;
class WebMediaStreamCenter;
class WebMediaStreamTrack;
}

namespace WebCore {

class MediaStreamComponent;
class MediaStreamDescriptor;
class MediaStreamSourcesQueryClient;

class MediaStreamCenterChromium : public MediaStreamCenter, public WebKit::WebMediaStreamCenterClient {
public:
    MediaStreamCenterChromium();
    ~MediaStreamCenterChromium();

    // MediaStreamCenter
    virtual void queryMediaStreamSources(PassRefPtr<MediaStreamSourcesQueryClient>) OVERRIDE;
    virtual void didSetMediaStreamTrackEnabled(MediaStreamDescriptor*, MediaStreamComponent*) OVERRIDE;
    virtual bool didAddMediaStreamTrack(MediaStreamDescriptor*, MediaStreamComponent*) OVERRIDE;
    virtual bool didRemoveMediaStreamTrack(MediaStreamDescriptor*, MediaStreamComponent*) OVERRIDE;
    virtual void didStopLocalMediaStream(MediaStreamDescriptor*) OVERRIDE;
    virtual void didCreateMediaStream(MediaStreamDescriptor*) OVERRIDE;

    // WebKit::WebMediaStreamCenterClient
    virtual void stopLocalMediaStream(const WebKit::WebMediaStream&) OVERRIDE;

private:
    OwnPtr<WebKit::WebMediaStreamCenter> m_private;
};

} // namespace WebCore

#endif // MediaStreamCenterChromium_h
