/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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

// This file has been auto-generated by code_generator_v8.pm. DO NOT MODIFY!

#include "config.h"
#include "V8TestInterfaceEventTarget.h"

#include "RuntimeEnabledFeatures.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "platform/TraceEvent.h"
#include "wtf/GetPtr.h"
#include "wtf/RefPtr.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(TestInterfaceEventTarget* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestInterfaceEventTarget::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

// In ScriptWrappable::init, the use of a local function declaration has an issue on Windows:
// the local declaration does not pick up the surrounding namespace. Therefore, we provide this function
// in the global namespace.
// (More info on the MSVC bug here: http://connect.microsoft.com/VisualStudio/feedback/details/664619/the-namespace-of-local-function-declarations-in-c)
void webCoreInitializeScriptWrappableForInterface(WebCore::TestInterfaceEventTarget* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8TestInterfaceEventTarget::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestInterfaceEventTarget::domTemplate, V8TestInterfaceEventTarget::derefObject, 0, V8TestInterfaceEventTarget::toEventTarget, 0, V8TestInterfaceEventTarget::installPerContextEnabledMethods, &V8EventTarget::wrapperTypeInfo, WrapperTypeObjectPrototype };

namespace TestInterfaceEventTargetV8Internal {

template <typename T> void V8_USE(T) { }

} // namespace TestInterfaceEventTargetV8Internal

const WrapperTypeInfo V8TestInterfaceEventTargetConstructor::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestInterfaceEventTargetConstructor::domTemplate, V8TestInterfaceEventTarget::derefObject, 0, V8TestInterfaceEventTarget::toEventTarget, 0, V8TestInterfaceEventTarget::installPerContextEnabledMethods, 0, WrapperTypeObjectPrototype };

static void V8TestInterfaceEventTargetConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (!info.IsConstructCall()) {
        throwTypeError(ExceptionMessages::failedToConstruct("Name", "Please use the 'new' operator, this DOM object constructor cannot be called as a function."), info.GetIsolate());
        return;
    }

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject) {
        v8SetReturnValue(info, info.Holder());
        return;
    }

    Document* document = currentDocument(info.GetIsolate());
    ASSERT(document);

    // Make sure the document is added to the DOM Node map. Otherwise, the TestInterfaceEventTarget instance
    // may end up being the only node in the map and get garbage-collected prematurely.
    toV8(document, info.Holder(), info.GetIsolate());

    RefPtr<TestInterfaceEventTarget> impl = TestInterfaceEventTarget::createForJSConstructor(*document);
    v8::Handle<v8::Object> wrapper = info.Holder();

    V8DOMWrapper::associateObjectWithWrapper<V8TestInterfaceEventTarget>(impl.release(), &V8TestInterfaceEventTargetConstructor::wrapperTypeInfo, wrapper, info.GetIsolate(), WrapperConfiguration::Dependent);
    v8SetReturnValue(info, wrapper);
}

v8::Handle<v8::FunctionTemplate> V8TestInterfaceEventTargetConstructor::domTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    // This is only for getting a unique pointer which we can pass to privateTemplate.
    static int privateTemplateUniqueKey;
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    v8::Local<v8::FunctionTemplate> result = data->privateTemplateIfExists(currentWorldType, &privateTemplateUniqueKey);
    if (!result.IsEmpty())
        return result;

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    v8::EscapableHandleScope scope(isolate);
    result = v8::FunctionTemplate::New(isolate, V8TestInterfaceEventTargetConstructorCallback);

    v8::Local<v8::ObjectTemplate> instanceTemplate = result->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount(V8TestInterfaceEventTarget::internalFieldCount);
    result->SetClassName(v8AtomicString(isolate, "TestInterfaceEventTarget"));
    result->Inherit(V8TestInterfaceEventTarget::domTemplate(isolate, currentWorldType));
    data->setPrivateTemplate(currentWorldType, &privateTemplateUniqueKey, result);

    return scope.Escape(result);
}

static void configureV8TestInterfaceEventTargetTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "TestInterfaceEventTarget", V8EventTarget::domTemplate(isolate, currentWorldType), V8TestInterfaceEventTarget::internalFieldCount,
        0, 0,
        0, 0,
        0, 0,
        isolate, currentWorldType);
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED instanceTemplate = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED prototypeTemplate = functionTemplate->PrototypeTemplate();

    // Custom toString template
    functionTemplate->Set(v8AtomicString(isolate, "toString"), V8PerIsolateData::current()->toStringTemplate());
}

v8::Handle<v8::FunctionTemplate> V8TestInterfaceEventTarget::domTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&wrapperTypeInfo);
    if (result != data->templateMap(currentWorldType).end())
        return result->value.newLocal(isolate);

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, V8ObjectConstructor::isValidConstructorMode);
    configureV8TestInterfaceEventTargetTemplate(templ, isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&wrapperTypeInfo, UnsafePersistent<v8::FunctionTemplate>(isolate, templ));
    return handleScope.Escape(templ);
}

bool V8TestInterfaceEventTarget::hasInstance(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstanceInMainWorld(&wrapperTypeInfo, jsValue)
        || V8PerIsolateData::from(isolate)->hasInstanceInNonMainWorld(&wrapperTypeInfo, jsValue);
}

EventTarget* V8TestInterfaceEventTarget::toEventTarget(v8::Handle<v8::Object> object)
{
    return toNative(object);
}

v8::Handle<v8::Object> V8TestInterfaceEventTarget::createWrapper(PassRefPtr<TestInterfaceEventTarget> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestInterfaceEventTarget>(impl.get(), isolate));
    if (ScriptWrappable::wrapperCanBeStoredInObject(impl.get())) {
        const WrapperTypeInfo* actualInfo = ScriptWrappable::getTypeInfoFromObject(impl.get());
        // Might be a XXXConstructor::wrapperTypeInfo instead of an XXX::wrapperTypeInfo. These will both have
        // the same object de-ref functions, though, so use that as the basis of the check.
        RELEASE_ASSERT_WITH_SECURITY_IMPLICATION(actualInfo->derefObjectFunction == wrapperTypeInfo.derefObjectFunction);
    }

    v8::Handle<v8::Object> wrapper = V8DOMWrapper::createWrapper(creationContext, &wrapperTypeInfo, toInternalPointer(impl.get()), isolate);
    if (UNLIKELY(wrapper.IsEmpty()))
        return wrapper;

    installPerContextEnabledProperties(wrapper, impl.get(), isolate);
    V8DOMWrapper::associateObjectWithWrapper<V8TestInterfaceEventTarget>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8TestInterfaceEventTarget::derefObject(void* object)
{
    fromInternalPointer(object)->deref();
}

template<>
v8::Handle<v8::Value> toV8NoInline(TestInterfaceEventTarget* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
