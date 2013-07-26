/*
 * Copyright (C) 2010 Google Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 *
 */

#ifndef WorkerEventQueue_h
#define WorkerEventQueue_h

#include "core/dom/EventQueue.h"
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/PassOwnPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class Event;
class Node;
class ScriptExecutionContext;

class WorkerEventQueue : public EventQueue {
    WTF_MAKE_FAST_ALLOCATED;
public:

    static PassOwnPtr<WorkerEventQueue> create(ScriptExecutionContext*);
    virtual ~WorkerEventQueue();

    // EventQueue
    virtual bool enqueueEvent(PassRefPtr<Event>) OVERRIDE;
    virtual bool cancelEvent(Event*) OVERRIDE;
    virtual void close() OVERRIDE;

private:
    explicit WorkerEventQueue(ScriptExecutionContext*);
    void removeEvent(Event*);

    ScriptExecutionContext* m_scriptExecutionContext;
    bool m_isClosed;

    class EventDispatcherTask;
    typedef HashMap<RefPtr<Event>, EventDispatcherTask*> EventTaskMap;
    EventTaskMap m_eventTaskMap;
};

}

#endif // WorkerEventQueue_h
