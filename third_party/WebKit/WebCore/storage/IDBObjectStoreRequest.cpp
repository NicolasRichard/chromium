/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
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
#include "IDBObjectStoreRequest.h"

#include "DOMStringList.h"
#include "IDBAny.h"
#include "IDBIndexRequest.h"
#include "ScriptExecutionContext.h"
#include "SerializedScriptValue.h"
#include <wtf/UnusedParam.h>

#if ENABLE(INDEXED_DATABASE)

namespace WebCore {

IDBObjectStoreRequest::IDBObjectStoreRequest(ScriptExecutionContext* context, PassRefPtr<IDBObjectStore> idbObjectStore) 
    : m_objectStore(idbObjectStore)
    , m_scriptExecutionContext(context)
{
    m_this = IDBAny::create();
    m_this->set(this);
}

String IDBObjectStoreRequest::name() const
{
    return m_objectStore->name();
}

String IDBObjectStoreRequest::keyPath() const
{
    return m_objectStore->keyPath();
}

PassRefPtr<DOMStringList> IDBObjectStoreRequest::indexNames() const
{
    return m_objectStore->indexNames();
}

PassRefPtr<IDBRequest> IDBObjectStoreRequest::get(PassRefPtr<SerializedScriptValue> key)
{
    // FIXME: implement
    UNUSED_PARAM(key);
    return 0;
}

PassRefPtr<IDBRequest> IDBObjectStoreRequest::add(PassRefPtr<SerializedScriptValue> value, PassRefPtr<SerializedScriptValue> key)
{
    // FIXME: implement
    UNUSED_PARAM(value);
    UNUSED_PARAM(key);
    return 0;
}

PassRefPtr<IDBRequest> IDBObjectStoreRequest::modify(PassRefPtr<SerializedScriptValue> value, PassRefPtr<SerializedScriptValue> key)
{
    // FIXME: implement
    UNUSED_PARAM(value);
    UNUSED_PARAM(key);
    return 0;
}

PassRefPtr<IDBRequest> IDBObjectStoreRequest::addOrModify(PassRefPtr<SerializedScriptValue> value, PassRefPtr<SerializedScriptValue> key)
{
    // FIXME: implement
    UNUSED_PARAM(value);
    UNUSED_PARAM(key);
    return 0;
}

PassRefPtr<IDBRequest> IDBObjectStoreRequest::remove(PassRefPtr<SerializedScriptValue> key)
{
    // FIXME: implement
    UNUSED_PARAM(key);
    return 0;
}

PassRefPtr<IDBRequest> IDBObjectStoreRequest::createIndex(const String& name, const String& keyPath, bool unique) const
{
    // FIXME: Implement.
    UNUSED_PARAM(name);
    UNUSED_PARAM(keyPath);
    UNUSED_PARAM(unique);
    return 0;
}

PassRefPtr<IDBIndexRequest> IDBObjectStoreRequest::index(const String& name) const
{
    return IDBIndexRequest::create(m_objectStore->index(name));
}

PassRefPtr<IDBRequest> IDBObjectStoreRequest::removeIndex(const String& name) const
{
    // FIXME: Implement.
    UNUSED_PARAM(name);
    return 0;
}

} // namespace WebCore

#endif // ENABLE(INDEXED_DATABASE)
