/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Matt Lilek <webkit@mattlilek.com>
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#if ENABLE(DOM_STORAGE)

#include "InspectorDOMStorageResource.h"

#include "Document.h"
#include "Frame.h"
#include "InspectorFrontend.h"
#include "JSONObject.h"
#include "ScriptObjectQuarantine.h"
#include "Storage.h"

using namespace JSC;

namespace WebCore {

InspectorDOMStorageResource::InspectorDOMStorageResource(Storage* domStorage, bool isLocalStorage, Frame* frame)
    : m_domStorage(domStorage)
    , m_isLocalStorage(isLocalStorage)
    , m_frame(frame)
    , m_scriptObjectCreated(false)
{
}

bool InspectorDOMStorageResource::isSameHostAndType(Frame* frame, bool isLocalStorage) const
{
    return equalIgnoringCase(m_frame->document()->securityOrigin()->host(), frame->document()->securityOrigin()->host()) && m_isLocalStorage == isLocalStorage;
}

void InspectorDOMStorageResource::bind(InspectorFrontend* frontend)
{
    if (m_scriptObjectCreated)
        return;

    JSONObject jsonObject = frontend->newJSONObject();
    ScriptObject domStorage;
    if (!getQuarantinedScriptObject(m_frame.get(), m_domStorage.get(), domStorage))
        return;
    jsonObject.set("domStorage", domStorage);
    jsonObject.set("host", m_frame->document()->securityOrigin()->host());
    jsonObject.set("isLocalStorage", m_isLocalStorage);
    if (frontend->addDOMStorage(jsonObject))
        m_scriptObjectCreated = true;
}

void InspectorDOMStorageResource::unbind()
{
    m_scriptObjectCreated = false;
}

} // namespace WebCore

#endif
