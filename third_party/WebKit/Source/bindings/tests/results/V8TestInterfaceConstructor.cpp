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
#include "V8TestInterfaceConstructor.h"

#include "RuntimeEnabledFeatures.h"
#include "V8TestInterfaceEmpty.h"
#include "bindings/v8/Dictionary.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "core/frame/UseCounter.h"
#include "platform/TraceEvent.h"
#include "wtf/GetPtr.h"
#include "wtf/RefPtr.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(TestInterfaceConstructor* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::setTypeInfoInObject(object, &V8TestInterfaceConstructor::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

// In ScriptWrappable::init, the use of a local function declaration has an issue on Windows:
// the local declaration does not pick up the surrounding namespace. Therefore, we provide this function
// in the global namespace.
// (More info on the MSVC bug here: http://connect.microsoft.com/VisualStudio/feedback/details/664619/the-namespace-of-local-function-declarations-in-c)
void webCoreInitializeScriptWrappableForInterface(WebCore::TestInterfaceConstructor* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8TestInterfaceConstructor::wrapperTypeInfo = { gin::kEmbedderBlink, V8TestInterfaceConstructor::domTemplate, V8TestInterfaceConstructor::derefObject, 0, 0, 0, V8TestInterfaceConstructor::installPerContextEnabledMethods, 0, WrapperTypeObjectPrototype };

namespace TestInterfaceConstructorV8Internal {

template <typename T> void V8_USE(T) { }

static void constructor1(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ConstructionContext, "TestInterfaceConstructor", info.Holder(), info.GetIsolate());
    ExecutionContext* context = currentExecutionContext(info.GetIsolate());
    Document& document = *toDocument(currentExecutionContext(info.GetIsolate()));
    RefPtr<TestInterfaceConstructor> impl = TestInterfaceConstructor::create(context, document, exceptionState);
    v8::Handle<v8::Object> wrapper = info.Holder();
    if (exceptionState.throwIfNeeded())
        return;

    V8DOMWrapper::associateObjectWithWrapper<V8TestInterfaceConstructor>(impl.release(), &V8TestInterfaceConstructor::wrapperTypeInfo, wrapper, info.GetIsolate(), WrapperConfiguration::Dependent);
    v8SetReturnValue(info, wrapper);
}

static void constructor2(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    ExceptionState exceptionState(ExceptionState::ConstructionContext, "TestInterfaceConstructor", info.Holder(), info.GetIsolate());
    V8TRYCATCH_VOID(double, doubleArg, static_cast<double>(info[0]->NumberValue()));
    V8TRYCATCH_FOR_V8STRINGRESOURCE_VOID(V8StringResource<>, stringArg, info[1]);
    V8TRYCATCH_VOID(TestInterfaceEmpty*, testInterfaceEmptyArg, V8TestInterfaceEmpty::hasInstance(info[2], info.GetIsolate()) ? V8TestInterfaceEmpty::toNative(v8::Handle<v8::Object>::Cast(info[2])) : 0);
    V8TRYCATCH_VOID(Dictionary, dictionaryArg, Dictionary(info[3], info.GetIsolate()));
    if (!dictionaryArg.isUndefinedOrNull() && !dictionaryArg.isObject()) {
        exceptionState.throwTypeError("parameter 4 ('dictionaryArg') is not an object.");
        exceptionState.throwIfNeeded();
        return;
    }
    V8TRYCATCH_VOID(Vector<String>, sequenceStringArg, toNativeArray<String>(info[4], 5, info.GetIsolate()));
    V8TRYCATCH_VOID(Dictionary, optionalDictionaryArg, Dictionary(info[5], info.GetIsolate()));
    if (!optionalDictionaryArg.isUndefinedOrNull() && !optionalDictionaryArg.isObject()) {
        exceptionState.throwTypeError("parameter 6 ('optionalDictionaryArg') is not an object.");
        exceptionState.throwIfNeeded();
        return;
    }
    V8TRYCATCH_VOID(TestInterfaceEmpty*, optionalTestInterfaceEmptyArg, V8TestInterfaceEmpty::hasInstance(info[6], info.GetIsolate()) ? V8TestInterfaceEmpty::toNative(v8::Handle<v8::Object>::Cast(info[6])) : 0);
    ExecutionContext* context = currentExecutionContext(info.GetIsolate());
    Document& document = *toDocument(currentExecutionContext(info.GetIsolate()));
    RefPtr<TestInterfaceConstructor> impl = TestInterfaceConstructor::create(context, document, doubleArg, stringArg, testInterfaceEmptyArg, dictionaryArg, sequenceStringArg, optionalDictionaryArg, optionalTestInterfaceEmptyArg, exceptionState);
    v8::Handle<v8::Object> wrapper = info.Holder();
    if (exceptionState.throwIfNeeded())
        return;

    V8DOMWrapper::associateObjectWithWrapper<V8TestInterfaceConstructor>(impl.release(), &V8TestInterfaceConstructor::wrapperTypeInfo, wrapper, info.GetIsolate(), WrapperConfiguration::Dependent);
    v8SetReturnValue(info, wrapper);
}

static void constructor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (((info.Length() == 0))) {
        TestInterfaceConstructorV8Internal::constructor1(info);
        return;
    }
    if (((info.Length() == 5) && (V8TestInterfaceEmpty::hasInstance(info[2], info.GetIsolate())) && (info[3]->IsObject()) && (info[4]->IsArray())) || ((info.Length() == 6) && (V8TestInterfaceEmpty::hasInstance(info[2], info.GetIsolate())) && (info[3]->IsObject()) && (info[4]->IsArray()) && (info[5]->IsObject())) || ((info.Length() == 7) && (V8TestInterfaceEmpty::hasInstance(info[2], info.GetIsolate())) && (info[3]->IsObject()) && (info[4]->IsArray()) && (info[5]->IsObject()) && (V8TestInterfaceEmpty::hasInstance(info[6], info.GetIsolate())))) {
        TestInterfaceConstructorV8Internal::constructor2(info);
        return;
    }
    throwTypeError(ExceptionMessages::failedToConstruct("TestInterfaceConstructor", "No matching constructor signature."), info.GetIsolate());
}

} // namespace TestInterfaceConstructorV8Internal

void V8TestInterfaceConstructor::constructorCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "DOMConstructor");
    UseCounter::count(activeExecutionContext(info.GetIsolate()), UseCounter::TestFeature);
    if (!info.IsConstructCall()) {
        throwTypeError(ExceptionMessages::failedToConstruct("TestInterfaceConstructor", "Please use the 'new' operator, this DOM object constructor cannot be called as a function."), info.GetIsolate());
        return;
    }

    if (ConstructorMode::current() == ConstructorMode::WrapExistingObject) {
        v8SetReturnValue(info, info.Holder());
        return;
    }

    TestInterfaceConstructorV8Internal::constructor(info);
}

static void configureV8TestInterfaceConstructorTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "TestInterfaceConstructor", v8::Local<v8::FunctionTemplate>(), V8TestInterfaceConstructor::internalFieldCount,
        0, 0,
        0, 0,
        0, 0,
        isolate, currentWorldType);
    functionTemplate->SetCallHandler(V8TestInterfaceConstructor::constructorCallback);
    functionTemplate->SetLength(0);
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED instanceTemplate = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> ALLOW_UNUSED prototypeTemplate = functionTemplate->PrototypeTemplate();

    // Custom toString template
    functionTemplate->Set(v8AtomicString(isolate, "toString"), V8PerIsolateData::current()->toStringTemplate());
}

v8::Handle<v8::FunctionTemplate> V8TestInterfaceConstructor::domTemplate(v8::Isolate* isolate, WrapperWorldType currentWorldType)
{
    V8PerIsolateData* data = V8PerIsolateData::from(isolate);
    V8PerIsolateData::TemplateMap::iterator result = data->templateMap(currentWorldType).find(&wrapperTypeInfo);
    if (result != data->templateMap(currentWorldType).end())
        return result->value.newLocal(isolate);

    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "BuildDOMTemplate");
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(isolate, V8ObjectConstructor::isValidConstructorMode);
    configureV8TestInterfaceConstructorTemplate(templ, isolate, currentWorldType);
    data->templateMap(currentWorldType).add(&wrapperTypeInfo, UnsafePersistent<v8::FunctionTemplate>(isolate, templ));
    return handleScope.Escape(templ);
}

bool V8TestInterfaceConstructor::hasInstance(v8::Handle<v8::Value> jsValue, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstanceInMainWorld(&wrapperTypeInfo, jsValue)
        || V8PerIsolateData::from(isolate)->hasInstanceInNonMainWorld(&wrapperTypeInfo, jsValue);
}

v8::Handle<v8::Object> V8TestInterfaceConstructor::createWrapper(PassRefPtr<TestInterfaceConstructor> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8TestInterfaceConstructor>(impl.get(), isolate));
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
    V8DOMWrapper::associateObjectWithWrapper<V8TestInterfaceConstructor>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8TestInterfaceConstructor::derefObject(void* object)
{
    fromInternalPointer(object)->deref();
}

template<>
v8::Handle<v8::Value> toV8NoInline(TestInterfaceConstructor* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
