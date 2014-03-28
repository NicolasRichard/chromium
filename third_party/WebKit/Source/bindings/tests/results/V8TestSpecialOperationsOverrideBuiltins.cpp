// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py. DO NOT MODIFY!

#include "config.h"
#include "V8TestSpecialOperationsOverrideBuiltins.h"

#include "RuntimeEnabledFeatures.h"
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

static void initializeScriptWrappableForInterface(TestSpecialOperationsOverrideBuiltins* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestSpecialOperationsOverrideBuiltins::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

void webCoreInitializeScriptWrappableForInterface(WebCore::TestSpecialOperationsOverrideBuiltins* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8TestSpecialOperationsOverrideBuiltins::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestSpecialOperationsOverrideBuiltins::domTemplate, V8TestSpecialOperationsOverrideBuiltins::derefObject, 0, 0, 0, V8TestSpecialOperationsOverrideBuiltins::installPerContextEnabledMethods, 0, WrapperTypeObjectPrototype, false };

namespace TestSpecialOperationsOverrideBuiltinsV8Internal {

template <typename T> void V8_USE(T) { }

static void namedPropertyGetter(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestSpecialOperationsOverrideBuiltins* impl = V8TestSpecialOperationsOverrideBuiltins::toNative(info.Holder());
    AtomicString propertyName = toCoreAtomicString(name);
    String result = impl->anonymousNamedGetter(propertyName);
    if (result.isNull())
        return;
    v8SetReturnValueString(info, result, info.GetIsolate());
}

static void namedPropertyGetterCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    TestSpecialOperationsOverrideBuiltinsV8Internal::namedPropertyGetter(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void namedPropertySetter(v8::Local<v8::String> name, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestSpecialOperationsOverrideBuiltins* impl = V8TestSpecialOperationsOverrideBuiltins::toNative(info.Holder());
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, propertyName, name);
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, propertyValue, v8Value);
    bool result = impl->anonymousNamedSetter(propertyName, propertyValue);
    if (!result)
        return;
    v8SetReturnValue(info, v8Value);
}

static void namedPropertySetterCallback(v8::Local<v8::String> name, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    TestSpecialOperationsOverrideBuiltinsV8Internal::namedPropertySetter(name, v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void namedPropertyQuery(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Integer>& info)
{
    TestSpecialOperationsOverrideBuiltins* impl = V8TestSpecialOperationsOverrideBuiltins::toNative(info.Holder());
    AtomicString propertyName = toCoreAtomicString(name);
    v8::String::Utf8Value namedProperty(name);
    ExceptionState exceptionState(ExceptionState::GetterContext, *namedProperty, "TestSpecialOperationsOverrideBuiltins", info.Holder(), info.GetIsolate());
    bool result = impl->namedPropertyQuery(propertyName, exceptionState);
    if (exceptionState.throwIfNeeded())
        return;
    if (!result)
        return;
    v8SetReturnValueInt(info, v8::None);
}

static void namedPropertyQueryCallback(v8::Local<v8::String> name, const v8::PropertyCallbackInfo<v8::Integer>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    TestSpecialOperationsOverrideBuiltinsV8Internal::namedPropertyQuery(name, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void namedPropertyEnumerator(const v8::PropertyCallbackInfo<v8::Array>& info)
{
    TestSpecialOperationsOverrideBuiltins* impl = V8TestSpecialOperationsOverrideBuiltins::toNative(info.Holder());
    v8::Isolate* isolate = info.GetIsolate();
    Vector<String> names;
    ExceptionState exceptionState(ExceptionState::EnumerationContext, "TestSpecialOperationsOverrideBuiltins", info.Holder(), isolate);
    impl->namedPropertyEnumerator(names, exceptionState);
    if (exceptionState.throwIfNeeded())
        return;
    v8::Handle<v8::Array> v8names = v8::Array::New(isolate, names.size());
    for (size_t i = 0; i < names.size(); ++i)
        v8names->Set(v8::Integer::New(isolate, i), v8String(isolate, names[i]));
    v8SetReturnValue(info, v8names);
}

static void namedPropertyEnumeratorCallback(const v8::PropertyCallbackInfo<v8::Array>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMNamedProperty");
    TestSpecialOperationsOverrideBuiltinsV8Internal::namedPropertyEnumerator(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

} // namespace TestSpecialOperationsOverrideBuiltinsV8Internal

static void configureV8TestSpecialOperationsOverrideBuiltinsTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "TestSpecialOperationsOverrideBuiltins", v8::Local<v8::FunctionTemplate>(), V8TestSpecialOperationsOverrideBuiltins::internalFieldCount,
        0, 0,
        0, 0,
        0, 0,
        isolate);
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED instanceTemplate = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED prototypeTemplate = functionTemplate->PrototypeTemplate();
    functionTemplate->InstanceTemplate()->SetNamedPropertyHandler(TestSpecialOperationsOverrideBuiltinsV8Internal::namedPropertyGetterCallback, TestSpecialOperationsOverrideBuiltinsV8Internal::namedPropertySetterCallback, TestSpecialOperationsOverrideBuiltinsV8Internal::namedPropertyQueryCallback, 0, TestSpecialOperationsOverrideBuiltinsV8Internal::namedPropertyEnumeratorCallback);

    // Custom toString template
    functionTemplate->Set(v8AtomicString(isolate, "toString"), V8PerIsolateData::current()->toStringTemplate());
}

v8::Handle<v8::FunctionTemplate> V8TestSpecialOperationsOverrideBuiltins::domTemplate(v8::Isolate* isolate)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    v8::Local<v8::FunctionTemplate> result = data->existingDOMTemplate(const_cast<WrapperTypeInfo*>(&wrapperTypeInfo));
    if (!result.IsEmpty())
        return result;

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    result = v8::FunctionTemplate::New(isolate, V8ObjectConstructor::isValidConstructorMode);
    configureV8TestSpecialOperationsOverrideBuiltinsTemplate(result, isolate);
    data->setDOMTemplate(const_cast<WrapperTypeInfo*>(&wrapperTypeInfo), result);
    return result;
}

bool V8TestSpecialOperationsOverrideBuiltins::hasInstance(v8::Handle<v8::Value> v8Value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, v8Value);
}

v8::Handle<v8::Object> V8TestSpecialOperationsOverrideBuiltins::findInstanceInPrototypeChain(v8::Handle<v8::Value> v8Value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->findInstanceInPrototypeChain(&wrapperTypeInfo, v8Value);
}

TestSpecialOperationsOverrideBuiltins* V8TestSpecialOperationsOverrideBuiltins::toNativeWithTypeCheck(v8::Isolate* isolate, v8::Handle<v8::Value> value)
{
    return hasInstance(value, isolate) ? fromInternalPointer(v8::Handle<v8::Object>::Cast(value)->GetAlignedPointerFromInternalField(v8DOMWrapperObjectIndex)) : 0;
}

v8::Handle<v8::Object> V8TestSpecialOperationsOverrideBuiltins::createWrapper(PassRefPtr<TestSpecialOperationsOverrideBuiltins> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestSpecialOperationsOverrideBuiltins>(impl.get(), isolate));
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
    V8DOMWrapper::associateObjectWithWrapper<V8TestSpecialOperationsOverrideBuiltins>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8TestSpecialOperationsOverrideBuiltins::derefObject(void* object)
{
    fromInternalPointer(object)->deref();
}

template<>
v8::Handle<v8::Value> toV8NoInline(TestSpecialOperationsOverrideBuiltins* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
