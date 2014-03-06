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
#include "V8TestSpecialOperationsCustom.h"

#include "RuntimeEnabledFeatures.h"
#include "V8Node.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8HiddenValue.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "platform/TraceEvent.h"
#include "wtf/GetPtr.h"
#include "wtf/RefPtr.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(TestSpecialOperationsCustom* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestSpecialOperationsCustom::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

// In ScriptWrappable::init, the use of a local function declaration has an issue on Windows:
// the local declaration does not pick up the surrounding namespace. Therefore, we provide this function
// in the global namespace.
// (More info on the MSVC bug here: http://connect.microsoft.com/VisualStudio/feedback/details/664619/the-namespace-of-local-function-declarations-in-c)
void webCoreInitializeScriptWrappableForInterface(WebCore::TestSpecialOperationsCustom* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8TestSpecialOperationsCustom::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestSpecialOperationsCustom::domTemplate, V8TestSpecialOperationsCustom::derefObject, 0, 0, 0, V8TestSpecialOperationsCustom::installPerContextEnabledMethods, 0, WrapperTypeObjectPrototype, false };

namespace TestSpecialOperationsCustomV8Internal {

template <typename T> void V8_USE(T) { }

static void indexedPropertyGetterCallback(uint32_t index, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMIndexedProperty");
    V8TestSpecialOperationsCustom::indexedPropertyGetterCustom(index, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void indexedPropertySetterCallback(uint32_t index, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMIndexedProperty");
    V8TestSpecialOperationsCustom::indexedPropertySetterCustom(index, jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void indexedPropertyDeleterCallback(uint32_t index, const v8::PropertyCallbackInfo<v8::Boolean>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMIndexedProperty");
    V8TestSpecialOperationsCustom::indexedPropertyDeleterCustom(index, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void namedPropertyGetterCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    V8TestSpecialOperationsCustom::namedPropertyGetterCustom(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void namedPropertySetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    V8TestSpecialOperationsCustom::namedPropertySetterCustom(name, jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void namedPropertyQueryCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Integer>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    V8TestSpecialOperationsCustom::namedPropertyQueryCustom(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void namedPropertyDeleterCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Boolean>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    V8TestSpecialOperationsCustom::namedPropertyDeleterCustom(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void namedPropertyEnumeratorCallback(const v8::PropertyCallbackInfo<v8::Array>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    V8TestSpecialOperationsCustom::namedPropertyEnumeratorCustom(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

} // namespace TestSpecialOperationsCustomV8Internal

static void configureV8TestSpecialOperationsCustomTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "TestSpecialOperationsCustom", v8::Local<v8::FunctionTemplate>(), V8TestSpecialOperationsCustom::internalFieldCount,
        0, 0,
        0, 0,
        0, 0,
        isolate, currentWorldType);
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED instanceTemplate = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED prototypeTemplate = functionTemplate->PrototypeTemplate();
    functionTemplate->InstanceTemplate()->SetIndexedPropertyHandler(TestSpecialOperationsCustomV8Internal::indexedPropertyGetterCallback, TestSpecialOperationsCustomV8Internal::indexedPropertySetterCallback, 0, TestSpecialOperationsCustomV8Internal::indexedPropertyDeleterCallback, indexedPropertyEnumerator<TestSpecialOperationsCustom>);
    functionTemplate->InstanceTemplate()->SetNamedPropertyHandler(TestSpecialOperationsCustomV8Internal::namedPropertyGetterCallback, TestSpecialOperationsCustomV8Internal::namedPropertySetterCallback, TestSpecialOperationsCustomV8Internal::namedPropertyQueryCallback, TestSpecialOperationsCustomV8Internal::namedPropertyDeleterCallback, TestSpecialOperationsCustomV8Internal::namedPropertyEnumeratorCallback);

    // Custom toString template
    functionTemplate->Set(v8AtomicString(isolate, "toString"), V8PerIsolateData::current()->toStringTemplate());
}

v8::Handle<v8::FunctionTemplate> V8TestSpecialOperationsCustom::domTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&wrapperTypeInfo);
    if (result != data->templateMap(currentWorldType).end())
        return result->value.newLocal(isolate);

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, V8ObjectConstructor::isValidConstructorMode);
    configureV8TestSpecialOperationsCustomTemplate(templ, isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&wrapperTypeInfo, UnsafePersistent<v8::FunctionTemplate>(isolate, templ));
    return templ;
}

bool V8TestSpecialOperationsCustom::hasInstance(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue);
}

v8::Handle<v8::Object> V8TestSpecialOperationsCustom::findInstanceInPrototypeChain(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->findInstanceInPrototypeChain(&wrapperTypeInfo, jsValue);
}

TestSpecialOperationsCustom* V8TestSpecialOperationsCustom::toNativeWithTypeCheck(v8::Isolate* isolate, v8::Handle<v8::Value> value)
{
    return hasInstance(value, isolate) ? fromInternalPointer(v8::Handle<v8::Object>::Cast(value)->GetAlignedPointerFromInternalField(v8DOMWrapperObjectIndex)) : 0;
}

v8::Handle<v8::Object> V8TestSpecialOperationsCustom::createWrapper(PassRefPtr<TestSpecialOperationsCustom> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestSpecialOperationsCustom>(impl.get(), isolate));
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
    V8DOMWrapper::associateObjectWithWrapper<V8TestSpecialOperationsCustom>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8TestSpecialOperationsCustom::derefObject(void* object)
{
    fromInternalPointer(object)->deref();
}

template<>
v8::Handle<v8::Value> toV8NoInline(TestSpecialOperationsCustom* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
