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
#if ENABLE(Condition1) || ENABLE(Condition2)
#include "V8TestSerializedScriptValueInterface.h"

#include "RuntimeEnabledFeatures.h"
#include "bindings/v8/ExceptionMessages.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/ScriptController.h"
#include "bindings/v8/ScriptState.h"
#include "bindings/v8/ScriptValue.h"
#include "bindings/v8/SerializedScriptValue.h"
#include "bindings/v8/V8Binding.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8DOMWrapper.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "platform/TraceEvent.h"
#include "wtf/UnusedParam.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(TestSerializedScriptValueInterface* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestSerializedScriptValueInterface::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

// In ScriptWrappable::init, the use of a local function declaration has an issue on Windows:
// the local declaration does not pick up the surrounding namespace. Therefore, we provide this function
// in the global namespace.
// (More info on the MSVC bug here: http://connect.microsoft.com/VisualStudio/feedback/details/664619/the-namespace-of-local-function-declarations-in-c)
void webCoreInitializeScriptWrappableForInterface(WebCore::TestSerializedScriptValueInterface* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8TestSerializedScriptValueInterface::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestSerializedScriptValueInterface::GetTemplate, V8TestSerializedScriptValueInterface::derefObject, 0, 0, 0, V8TestSerializedScriptValueInterface::installPerContextEnabledMethods, 0, WrapperTypeObjectPrototype };

namespace TestSerializedScriptValueInterfaceV8Internal {

template <typename T> void V8_USE(T) { }

static void valueAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    v8SetReturnValue(info, imp->value() ? imp->value()->deserialize() : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void valueAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::valueAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void valueAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    ExceptionState exceptionState(ExceptionState::SetterContext, "value", "TestSerializedScriptValueInterface", info.Holder(), info.GetIsolate());
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    V8TRYCATCH_VOID(RefPtr<SerializedScriptValue>, cppValue, SerializedScriptValue::create(jsValue, info.GetIsolate()));
    imp->setValue(WTF::getPtr(cppValue));
}

static void valueAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestSerializedScriptValueInterfaceV8Internal::valueAttributeSetter(jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void readonlyValueAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    v8SetReturnValue(info, imp->readonlyValue() ? imp->readonlyValue()->deserialize() : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void readonlyValueAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::readonlyValueAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void dirtySerializedValueAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::String> propertyName = v8::String::NewFromUtf8(info.GetIsolate(), "dirtySerializedValue", v8::String::kInternalizedString);
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    if (!imp->isValueDirty()) {
        v8::Handle<v8::Value> jsValue = info.Holder()->GetHiddenValue(propertyName);
        if (!jsValue.IsEmpty()) {
            v8SetReturnValue(info, jsValue);
            return;
        }
    }
    RefPtr<SerializedScriptValue> serialized = imp->dirtySerializedValue();
    ScriptValue jsValue = serialized ? serialized->deserialize() : v8::Handle<v8::Value>(v8::Null(info.GetIsolate()));
    info.Holder()->SetHiddenValue(propertyName, jsValue);
    v8SetReturnValue(info, jsValue);
}

static void dirtySerializedValueAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::dirtySerializedValueAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void dirtySerializedValueAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    ExceptionState exceptionState(ExceptionState::SetterContext, "dirtySerializedValue", "TestSerializedScriptValueInterface", info.Holder(), info.GetIsolate());
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    V8TRYCATCH_VOID(RefPtr<SerializedScriptValue>, cppValue, SerializedScriptValue::create(jsValue, info.GetIsolate()));
    imp->setDirtySerializedValue(WTF::getPtr(cppValue));
    info.Holder()->DeleteHiddenValue(v8::String::NewFromUtf8(info.GetIsolate(), "dirtySerializedValue", v8::String::kInternalizedString)); // Invalidate the cached value.
}

static void dirtySerializedValueAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestSerializedScriptValueInterfaceV8Internal::dirtySerializedValueAttributeSetter(jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void dirtyScriptValueAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::String> propertyName = v8::String::NewFromUtf8(info.GetIsolate(), "dirtyScriptValue", v8::String::kInternalizedString);
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    if (!imp->isValueDirty()) {
        v8::Handle<v8::Value> jsValue = info.Holder()->GetHiddenValue(propertyName);
        if (!jsValue.IsEmpty()) {
            v8SetReturnValue(info, jsValue);
            return;
        }
    }
    ScriptValue jsValue = imp->dirtyScriptValue();
    info.Holder()->SetHiddenValue(propertyName, jsValue.v8Value());
    v8SetReturnValue(info, jsValue.v8Value());
}

static void dirtyScriptValueAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::dirtyScriptValueAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void dirtyScriptValueAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    ExceptionState exceptionState(ExceptionState::SetterContext, "dirtyScriptValue", "TestSerializedScriptValueInterface", info.Holder(), info.GetIsolate());
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    V8TRYCATCH_VOID(ScriptValue, cppValue, ScriptValue(jsValue, info.GetIsolate()));
    imp->setDirtyScriptValue(cppValue);
    info.Holder()->DeleteHiddenValue(v8::String::NewFromUtf8(info.GetIsolate(), "dirtyScriptValue", v8::String::kInternalizedString)); // Invalidate the cached value.
}

static void dirtyScriptValueAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestSerializedScriptValueInterfaceV8Internal::dirtyScriptValueAttributeSetter(jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void cachedValueCallWithAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::String> propertyName = v8::String::NewFromUtf8(info.GetIsolate(), "cachedValueCallWith", v8::String::kInternalizedString);
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    if (!imp->isValueDirty()) {
        v8::Handle<v8::Value> jsValue = info.Holder()->GetHiddenValue(propertyName);
        if (!jsValue.IsEmpty()) {
            v8SetReturnValue(info, jsValue);
            return;
        }
    }
    ScriptState* currentState = ScriptState::current();
    if (!currentState)
        return v8Undefined();
    ScriptState& state = *currentState;
    RefPtr<SerializedScriptValue> serialized = imp->cachedValueCallWith(&state);
    ScriptValue jsValue = serialized ? serialized->deserialize() : v8::Handle<v8::Value>(v8::Null(info.GetIsolate()));
    info.Holder()->SetHiddenValue(propertyName, jsValue);
    v8SetReturnValue(info, jsValue);
}

static void cachedValueCallWithAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    TestSerializedScriptValueInterfaceV8Internal::cachedValueCallWithAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

static void cachedValueCallWithAttributeSetter(v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    ExceptionState exceptionState(ExceptionState::SetterContext, "cachedValueCallWith", "TestSerializedScriptValueInterface", info.Holder(), info.GetIsolate());
    TestSerializedScriptValueInterface* imp = V8TestSerializedScriptValueInterface::toNative(info.Holder());
    V8TRYCATCH_VOID(RefPtr<SerializedScriptValue>, cppValue, SerializedScriptValue::create(jsValue, info.GetIsolate()));
    ScriptState* currentState = ScriptState::current();
    if (!currentState)
        return;
    ScriptState& state = *currentState;
    imp->setCachedValueCallWith(&state, WTF::getPtr(cppValue));
    if (state.hadException())
        throwError(state.exception(), info.GetIsolate());
    info.Holder()->DeleteHiddenValue(v8::String::NewFromUtf8(info.GetIsolate(), "cachedValueCallWith", v8::String::kInternalizedString)); // Invalidate the cached value.
}

static void cachedValueCallWithAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> jsValue, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    TestSerializedScriptValueInterfaceV8Internal::cachedValueCallWithAttributeSetter(jsValue, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "Execution");
}

} // namespace TestSerializedScriptValueInterfaceV8Internal

static const V8DOMConfiguration::AttributeConfiguration V8TestSerializedScriptValueInterfaceAttributes[] = {
    {"value", TestSerializedScriptValueInterfaceV8Internal::valueAttributeGetterCallback, TestSerializedScriptValueInterfaceV8Internal::valueAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"readonlyValue", TestSerializedScriptValueInterfaceV8Internal::readonlyValueAttributeGetterCallback, 0, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"dirtySerializedValue", TestSerializedScriptValueInterfaceV8Internal::dirtySerializedValueAttributeGetterCallback, TestSerializedScriptValueInterfaceV8Internal::dirtySerializedValueAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"dirtyScriptValue", TestSerializedScriptValueInterfaceV8Internal::dirtyScriptValueAttributeGetterCallback, TestSerializedScriptValueInterfaceV8Internal::dirtyScriptValueAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"cachedValueCallWith", TestSerializedScriptValueInterfaceV8Internal::cachedValueCallWithAttributeGetterCallback, TestSerializedScriptValueInterfaceV8Internal::cachedValueCallWithAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
};

static v8::Handle<v8::FunctionTemplate> ConfigureV8TestSerializedScriptValueInterfaceTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "TestSerializedScriptValueInterface", v8::Local<v8::FunctionTemplate>(), V8TestSerializedScriptValueInterface::internalFieldCount,
        V8TestSerializedScriptValueInterfaceAttributes, WTF_ARRAY_LENGTH(V8TestSerializedScriptValueInterfaceAttributes),
        0, 0,
        0, 0,
        isolate, currentWorldType);
    UNUSED_PARAM(defaultSignature);
    v8::Local<v8::ObjectTemplate> instanceTemplate = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> prototypeTemplate = functionTemplate->PrototypeTemplate();
    UNUSED_PARAM(instanceTemplate);
    UNUSED_PARAM(prototypeTemplate);

    // Custom toString template
    functionTemplate->Set(v8::String::NewFromUtf8(isolate, "toString", v8::String::kInternalizedString), V8PerIsolateData::current()->toStringTemplate());
    return functionTemplate;
}

v8::Handle<v8::FunctionTemplate> V8TestSerializedScriptValueInterface::GetTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&wrapperTypeInfo);
    if (result != data->templateMap(currentWorldType).end())
        return result->value.newLocal(isolate);

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::FunctionTemplate> templ =
        ConfigureV8TestSerializedScriptValueInterfaceTemplate(data->rawTemplate(&wrapperTypeInfo, currentWorldType), isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&wrapperTypeInfo, UnsafePersistent<v8::FunctionTemplate>(isolate, templ));
    return handleScope.Escape(templ);
}

bool V8TestSerializedScriptValueInterface::hasInstance(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue, currentWorldType);
}

bool V8TestSerializedScriptValueInterface::hasInstanceInAnyWorld(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue, MainWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue, IsolatedWorld)
        || V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, jsValue, WorkerWorld);
}

v8::Handle<v8::Object> V8TestSerializedScriptValueInterface::createWrapper(PassRefPtr<TestSerializedScriptValueInterface> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestSerializedScriptValueInterface>(impl.get(), isolate));
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
    V8DOMWrapper::associateObjectWithWrapper<V8TestSerializedScriptValueInterface>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8TestSerializedScriptValueInterface::derefObject(void* object)
{
    fromInternalPointer(object)->deref();
}

template<>
v8::Handle<v8::Value> toV8NoInline(TestSerializedScriptValueInterface* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
#endif // ENABLE(Condition1) || ENABLE(Condition2)
