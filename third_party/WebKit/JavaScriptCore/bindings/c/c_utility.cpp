/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */
#include <c_instance.h> 
#include <c_utility.h> 
#include <internal.h>
#include <npruntime_impl.h>
#include <npruntime_priv.h>
#include <runtime.h>
#include <runtime_object.h>
#include <runtime_root.h>
#include <value.h>
#include <NP_jsobject.h>

using namespace KJS;
using namespace KJS::Bindings;

// Requires free() of returned UTF16Chars.
void convertNPStringToUTF16(const NPString *string, NPUTF16 **UTF16Chars, unsigned int *UTF16Length)
{
    convertUTF8ToUTF16(string->UTF8Characters, string->UTF8Length, UTF16Chars, UTF16Length);
}

// Requires free() of returned UTF16Chars.
void convertUTF8ToUTF16(const NPUTF8 *UTF8Chars, int UTF8Length, NPUTF16 **UTF16Chars, unsigned int *UTF16Length)
{
    assert(UTF8Chars);
    
    if (UTF8Length == -1)
        UTF8Length = strlen(UTF8Chars);
        
    CFStringRef stringRef = CFStringCreateWithBytes(NULL, (const UInt8*)UTF8Chars, (CFIndex)UTF8Length, kCFStringEncodingUTF8, false);

    *UTF16Length = (unsigned int)CFStringGetLength(stringRef);
    *UTF16Chars = (NPUTF16 *)malloc(sizeof(NPUTF16) * (*UTF16Length));

    // Convert the string to UTF16.
    CFRange range = { 0, *UTF16Length };
    CFStringGetCharacters(stringRef, range, (UniChar *)*UTF16Chars);
    CFRelease(stringRef);
}

// Variant value must be released with NPReleaseVariantValue()
void coerceValueToNPVariantStringType(ExecState *exec, ValueImp *value, NPVariant *result)
{
    UString ustring = value->toString(exec);
    CString cstring = ustring.UTF8String();
    NPString string = { (const NPUTF8 *)cstring.c_str(), cstring.size() };
    NPN_InitializeVariantWithStringCopy(result, &string);
}

// Variant value must be released with NPReleaseVariantValue()
void convertValueToNPVariant(ExecState *exec, ValueImp *value, NPVariant *result)
{
    Type type = value->type();
    
    if (type == StringType) {
        UString ustring = value->toString(exec);
        CString cstring = ustring.UTF8String();
        NPString string = { (const NPUTF8 *)cstring.c_str(), cstring.size() };
        NPN_InitializeVariantWithStringCopy(result, &string );
    }
    else if (type == NumberType) {
        NPN_InitializeVariantWithDouble(result, value->toNumber(exec));
    }
    else if (type == BooleanType) {
        NPN_InitializeVariantWithBool(result, value->toBoolean(exec));
    }
    else if (type == UnspecifiedType) {
        NPN_InitializeVariantAsUndefined(result);
    }
    else if (type == NullType) {
        NPN_InitializeVariantAsNull(result);
    }
    else if (type == ObjectType) {
        ObjectImp *objectImp = static_cast<ObjectImp*>(value);
        if (objectImp->classInfo() == &RuntimeObjectImp::info) {
            RuntimeObjectImp *imp = static_cast<RuntimeObjectImp *>(value);
            CInstance *instance = static_cast<CInstance*>(imp->getInternalInstance());
            NPN_InitializeVariantWithObject(result, instance->getObject());
        }
	else {

	    Interpreter *originInterpreter = exec->interpreter();
            const Bindings::RootObject *originExecutionContext = rootForInterpreter(originInterpreter);

	    Interpreter *interpreter = 0;
	    if (originInterpreter->isGlobalObject(value)) {
		interpreter = originInterpreter->interpreterForGlobalObject(value);
	    }

	    if (!interpreter)
		interpreter = originInterpreter;
		
            const Bindings::RootObject *executionContext = rootForInterpreter(interpreter);
            if (!executionContext) {
                Bindings::RootObject *newExecutionContext = new Bindings::RootObject(0);
                newExecutionContext->setInterpreter(interpreter);
                executionContext = newExecutionContext;
            }
    
	    NPObject *obj = (NPObject *)exec->interpreter()->createLanguageInstanceForValue(exec, Instance::CLanguage, value->toObject(exec), originExecutionContext, executionContext);
	    NPN_InitializeVariantWithObject(result, obj);
	    _NPN_ReleaseObject(obj);
	}
    }
    else
        NPN_InitializeVariantAsUndefined(result);
}

ValueImp *convertNPVariantToValue(ExecState *exec, const NPVariant *variant)
{
    NPVariantType type = variant->type;

    if (type == NPVariantType_Bool) {
        NPBool aBool;
        if (NPN_VariantToBool(variant, &aBool))
            return jsBoolean(aBool);
        return jsBoolean(false);
    }
    else if (type == NPVariantType_Null) {
        return Null();
    }
    else if (type == NPVariantType_Void) {
        return Undefined();
    }
    else if (type == NPVariantType_Int32) {
        int32_t anInt;
        if (NPN_VariantToInt32(variant, &anInt))
            return Number(anInt);
        return Number(0);
    }
    else if (type == NPVariantType_Double) {
        double aDouble;
        if (NPN_VariantToDouble(variant, &aDouble))
            return Number(aDouble);
        return Number(0);
    }
    else if (type == NPVariantType_String) {
        NPUTF16 *stringValue;
        unsigned int UTF16Length;
        convertNPStringToUTF16(&variant->value.stringValue, &stringValue, &UTF16Length);    // requires free() of returned memory.
        UString resultString((const UChar *)stringValue,UTF16Length);
        free(stringValue);
        return jsString(resultString);
    }
    else if (type == NPVariantType_Object) {
        NPObject *obj = variant->value.objectValue;
        
        if (obj->_class == NPScriptObjectClass) {
            // Get ObjectImp from NP_JavaScriptObject.
            JavaScriptObject *o = (JavaScriptObject *)obj;
            return const_cast<ObjectImp*>(o->imp);
        }
        else {
            //  Wrap NPObject in a CInstance.
            return Instance::createRuntimeObject(Instance::CLanguage, (void *)obj);
        }
    }
    
    return Undefined();
}

