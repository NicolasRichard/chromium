/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JSInspectedObjectWrapper_h
#define JSInspectedObjectWrapper_h

#include "JSQuarantinedObjectWrapper.h"

namespace WebCore {

    class JSInspectedObjectWrapper : public JSQuarantinedObjectWrapper {
    public:
        static JSC::JSValue* wrap(JSC::ExecState* unwrappedExec, JSC::JSValue* unwrappedValue);
        virtual ~JSInspectedObjectWrapper();

        static const JSC::ClassInfo s_info;

    private:
        JSInspectedObjectWrapper(JSC::ExecState* unwrappedExec, JSC::JSObject* unwrappedObject, JSC::JSObject* wrappedPrototype);
        JSInspectedObjectWrapper(JSC::ExecState* unwrappedExec, JSC::JSObject* unwrappedObject, PassRefPtr<JSC::StructureID>);

        virtual bool allowsGetProperty() const { return true; }
        virtual bool allowsSetProperty() const { return true; }
        virtual bool allowsDeleteProperty() const { return true; }
        virtual bool allowsConstruct() const { return true; }
        virtual bool allowsHasInstance() const { return true; }
        virtual bool allowsCallAsFunction() const { return true; }
        virtual bool allowsGetPropertyNames() const { return true; }

        virtual JSC::JSValue* prepareIncomingValue(JSC::ExecState* unwrappedExec, JSC::JSValue* unwrappedValue) const;
        virtual JSC::JSValue* wrapOutgoingValue(JSC::ExecState* unwrappedExec, JSC::JSValue* unwrappedValue) const { return wrap(unwrappedExec, unwrappedValue); }

        virtual const JSC::ClassInfo* classInfo() const { return &s_info; }
    };

} // namespace WebCore

#endif // JSInspectedObjectWrapper_h
