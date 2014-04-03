/*
 * Copyright (C) 2010 Julien Chaffraix <jchaffraix@webkit.org>
 * All right reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

#ifndef XMLHttpRequestProgressEventThrottle_h
#define XMLHttpRequestProgressEventThrottle_h

#include "heap/Handle.h"
#include "platform/Timer.h"
#include "wtf/PassRefPtr.h"
#include "wtf/Vector.h"
#include "wtf/text/AtomicString.h"

namespace WebCore {

class Event;
class EventTarget;

enum ProgressEventAction {
    DoNotFlushProgressEvent,
    FlushDeferredProgressEvent,
    FlushProgressEvent
};

// This implements the XHR2 progress event dispatching: "dispatch a progress event called progress
// about every 50ms or for every byte received, whichever is least frequent".
class XMLHttpRequestProgressEventThrottle FINAL : public TimerBase {
public:
    explicit XMLHttpRequestProgressEventThrottle(EventTarget*);
    virtual ~XMLHttpRequestProgressEventThrottle();

    void dispatchProgressEvent(bool lengthComputable, unsigned long long loaded, unsigned long long total);
    void dispatchReadyStateChangeEvent(PassRefPtrWillBeRawPtr<Event>, ProgressEventAction = DoNotFlushProgressEvent);
    void dispatchEvent(PassRefPtrWillBeRawPtr<Event>);
    void dispatchEventAndLoadEnd(const AtomicString&, bool, unsigned long long, unsigned long long);

    void suspend();
    void resume();

private:
    static const double minimumProgressEventDispatchingIntervalInSeconds;

    virtual void fired() OVERRIDE;
    void dispatchDeferredEvents(Timer<XMLHttpRequestProgressEventThrottle>*);
    bool flushDeferredProgressEvent();
    void deliverProgressEvent();

    bool hasEventToDispatch() const;

    // Weak pointer to our XMLHttpRequest object as it is the one holding us.
    EventTarget* m_target;

    bool m_lengthComputable;
    unsigned long long m_loaded;
    unsigned long long m_total;

    bool m_deferEvents;
    RefPtrWillBePersistent<Event> m_deferredProgressEvent;
    WillBePersistentHeapVector<RefPtrWillBeMember<Event> > m_deferredEvents;
    Timer<XMLHttpRequestProgressEventThrottle> m_dispatchDeferredEventsTimer;
};

} // namespace WebCore

#endif // XMLHttpRequestProgressEventThrottle_h
