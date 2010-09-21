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

#ifndef WebIDBObjectStore_h
#define WebIDBObjectStore_h

#include "WebCommon.h"
#include "WebDOMStringList.h"
#include "WebIDBCallbacks.h"
#include "WebString.h"

namespace WebKit {

class WebIDBKeyRange;

// See comment in WebIndexedDatabase for a high level overview these classes.
class WebIDBObjectStore {
public:
    virtual ~WebIDBObjectStore() { }

    virtual WebString name() const
    {
        WEBKIT_ASSERT_NOT_REACHED();
        return WebString();
    }
    virtual WebString keyPath() const
    {
        WEBKIT_ASSERT_NOT_REACHED();
        return WebString();
    }
    virtual WebDOMStringList indexNames() const
    {
        WEBKIT_ASSERT_NOT_REACHED();
        return WebDOMStringList();
    }

    // FIXME: Remove the default parameters for transactionIds.
    virtual void get(const WebIDBKey& key, WebIDBCallbacks*, int transactionId = 0) { WEBKIT_ASSERT_NOT_REACHED(); }
    virtual void put(const WebSerializedScriptValue& value, const WebIDBKey& key, bool addOnly, WebIDBCallbacks*, int transactionId = 0) { WEBKIT_ASSERT_NOT_REACHED(); }
    virtual void remove(const WebIDBKey& key, WebIDBCallbacks*, int transactionId = 0) { WEBKIT_ASSERT_NOT_REACHED(); }
    virtual void createIndex(const WebString& name, const WebString& keyPath, bool unique, WebIDBCallbacks*) { WEBKIT_ASSERT_NOT_REACHED(); }
    // Transfers ownership of the WebIDBIndex to the caller.
    virtual WebIDBIndex* index(const WebString& name)
    {
        WEBKIT_ASSERT_NOT_REACHED();
        return 0;
    }
    virtual void removeIndex(const WebString& name, WebIDBCallbacks*) { WEBKIT_ASSERT_NOT_REACHED(); }
    virtual void openCursor(const WebIDBKeyRange&, unsigned short direction, WebIDBCallbacks*, int transactionId = 0) { WEBKIT_ASSERT_NOT_REACHED(); }
    // FIXME: finish.
};

} // namespace WebKit

#endif // WebIDBObjectStore_h
