/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "APIObject.h"
#include <JavaScriptCore/JSBase.h>
#include <wtf/Forward.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>

namespace WebCore {
    class Node;
}

namespace WebKit {

class InjectedBundleScriptWorld;

class InjectedBundleNodeHandle : public APIObject {
public:
    static const Type APIType = TypeBundleNodeHandle;

    static PassRefPtr<InjectedBundleNodeHandle> getOrCreate(JSContextRef context, JSObjectRef object);
    static PassRefPtr<InjectedBundleNodeHandle> getOrCreate(WebCore::Node*);

    ~InjectedBundleNodeHandle();

    WebCore::Node* coreNode() const;

    // Additional DOM Operations
    // Note: These should only be operations that are not exposed to JavaScript.
    void setHTMLInputElementValueForUser(const String&);
    void setHTMLInputElementAutofilled(bool);
    PassRefPtr<InjectedBundleNodeHandle> copyHTMLTableCellElementCellAbove();

private:
    static PassRefPtr<InjectedBundleNodeHandle> create(WebCore::Node*);
    InjectedBundleNodeHandle(WebCore::Node*);

    virtual Type type() const { return APIType; }

    RefPtr<WebCore::Node> m_node;
};

} // namespace WebKit
