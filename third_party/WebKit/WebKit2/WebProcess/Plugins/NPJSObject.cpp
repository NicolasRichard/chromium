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

#include "NPJSObject.h"

#include "NPRuntimeObjectMap.h"
#include "NPRuntimeUtilities.h"
#include "PluginView.h"
#include <JavaScriptCore/JSObject.h>
#include <WebCore/Frame.h>  
#include <WebCore/IdentifierRep.h>
#include <WebCore/PlatformString.h>

using namespace JSC;
using namespace WebCore;

namespace WebKit {

NPJSObject* NPJSObject::create(NPRuntimeObjectMap* objectMap, JSObject* jsObject)
{
    NPJSObject* npJSObject = toNPJSObject(createNPObject(0, npClass()));
    npJSObject->initialize(objectMap, jsObject);

    return npJSObject;
}

NPJSObject::NPJSObject()
    : m_objectMap(0)
{
}

NPJSObject::~NPJSObject()
{
    m_objectMap->npJSObjectDestroyed(this);
}

bool NPJSObject::isNPJSObject(NPObject* npObject)
{
    return npObject->_class == npClass();
}

void NPJSObject::initialize(NPRuntimeObjectMap* objectMap, JSObject* jsObject)
{
    ASSERT(!m_objectMap);
    ASSERT(!m_jsObject);

    m_objectMap = objectMap;
    m_jsObject = jsObject;
}

static Identifier identifierFromIdentifierRep(ExecState* exec, IdentifierRep* identifierRep)
{
    ASSERT(identifierRep->isString());

    const char* string = identifierRep->string();
    int length = strlen(string);

    return Identifier(exec, String::fromUTF8WithLatin1Fallback(string, length).impl());
}

bool NPJSObject::hasProperty(NPIdentifier identifier)
{
    IdentifierRep* identifierRep = static_cast<IdentifierRep*>(identifier);
    
    ExecState* exec = m_objectMap->globalExec();
    if (!exec)
        return false;
    
    bool result;
    if (identifierRep->isString())
        result = m_jsObject->hasProperty(exec, identifierFromIdentifierRep(exec, identifierRep));
    else
        result = m_jsObject->hasProperty(exec, identifierRep->number());

    exec->clearException();

    return result;
}

bool NPJSObject::getProperty(NPIdentifier identifier, NPVariant* result)
{
    // FIXME: Implement.
    return false;
}

NPClass* NPJSObject::npClass()
{
    static NPClass npClass = {
        NP_CLASS_STRUCT_VERSION,
        NP_Allocate,
        NP_Deallocate,
        0, 
        0,
        0,
        0,
        NP_HasProperty,
        NP_GetProperty,
        0,
        0,
        0,
        0
    };

    return &npClass;
}
    
NPObject* NPJSObject::NP_Allocate(NPP npp, NPClass* npClass)
{
    ASSERT_UNUSED(npp, !npp);

    return new NPJSObject;
}

void NPJSObject::NP_Deallocate(NPObject* npObject)
{
    NPJSObject* npJSObject = toNPJSObject(npObject);
    delete npJSObject;
}

bool NPJSObject::NP_HasProperty(NPObject* npObject, NPIdentifier propertyName)
{
    return toNPJSObject(npObject)->hasProperty(propertyName);
}
    
bool NPJSObject::NP_GetProperty(NPObject* npObject, NPIdentifier propertyName, NPVariant* result)
{
    return toNPJSObject(npObject)->getProperty(propertyName, result);
}

} // namespace WebKit
