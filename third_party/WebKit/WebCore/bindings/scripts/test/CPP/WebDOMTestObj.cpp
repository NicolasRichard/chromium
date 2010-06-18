/*
 * This file is part of the WebKit open source project.
 * This file has been generated by generate-bindings.pl. DO NOT MODIFY!
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "WebDOMTestObj.h"

#include "AtomicString.h"
#include "HTMLNames.h"
#include "KURL.h"
#include "SerializedScriptValue.h"
#include "TestObj.h"
#include "WebDOMString.h"
#include "WebExceptionHandler.h"
#include "WebNativeEventListener.h"
#include <wtf/GetPtr.h>
#include <wtf/RefPtr.h>

struct WebDOMTestObj::WebDOMTestObjPrivate {
    WebDOMTestObjPrivate(WebCore::TestObj* object = 0)
        : impl(object)
    {
    }

    RefPtr<WebCore::TestObj> impl;
};

WebDOMTestObj::WebDOMTestObj()
    : WebDOMObject()
    , m_impl(0)
{
}

WebDOMTestObj::WebDOMTestObj(WebCore::TestObj* impl)
    : WebDOMObject()
    , m_impl(new WebDOMTestObjPrivate(impl))
{
}

WebDOMTestObj::WebDOMTestObj(const WebDOMTestObj& copy)
    : WebDOMObject()
{
    m_impl = copy.impl() ? new WebDOMTestObjPrivate(copy.impl()) : 0;
}

WebCore::TestObj* WebDOMTestObj::impl() const
{
    return m_impl ? m_impl->impl.get() : 0;
}

WebDOMTestObj::~WebDOMTestObj()
{
    delete m_impl;
    m_impl = 0;
}

int WebDOMTestObj::readOnlyIntAttr() const
{
    if (!impl())
        return 0;

    return impl()->readOnlyIntAttr();
}

WebDOMString WebDOMTestObj::readOnlyStringAttr() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->readOnlyStringAttr());
}

WebDOMTestObj WebDOMTestObj::readOnlyTestObjAttr() const
{
    if (!impl())
        return WebDOMTestObj();

    return toWebKit(WTF::getPtr(impl()->readOnlyTestObjAttr()));
}

int WebDOMTestObj::intAttr() const
{
    if (!impl())
        return 0;

    return impl()->intAttr();
}

void WebDOMTestObj::setIntAttr(int newIntAttr)
{
    if (!impl())
        return;

    impl()->setIntAttr(newIntAttr);
}

long long WebDOMTestObj::longLongAttr() const
{
    if (!impl())
        return 0;

    return impl()->longLongAttr();
}

void WebDOMTestObj::setLongLongAttr(long long newLongLongAttr)
{
    if (!impl())
        return;

    impl()->setLongLongAttr(newLongLongAttr);
}

unsigned long long WebDOMTestObj::unsignedLongLongAttr() const
{
    if (!impl())
        return 0;

    return impl()->unsignedLongLongAttr();
}

void WebDOMTestObj::setUnsignedLongLongAttr(unsigned long long newUnsignedLongLongAttr)
{
    if (!impl())
        return;

    impl()->setUnsignedLongLongAttr(newUnsignedLongLongAttr);
}

WebDOMString WebDOMTestObj::stringAttr() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->stringAttr());
}

void WebDOMTestObj::setStringAttr(const WebDOMString& newStringAttr)
{
    if (!impl())
        return;

    impl()->setStringAttr(newStringAttr);
}

WebDOMTestObj WebDOMTestObj::testObjAttr() const
{
    if (!impl())
        return WebDOMTestObj();

    return toWebKit(WTF::getPtr(impl()->testObjAttr()));
}

void WebDOMTestObj::setTestObjAttr(const WebDOMTestObj& newTestObjAttr)
{
    if (!impl())
        return;

    impl()->setTestObjAttr(toWebCore(newTestObjAttr));
}

WebDOMString WebDOMTestObj::reflectedStringAttr() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->getAttribute(WebCore::HTMLNames::reflectedstringattrAttr));
}

void WebDOMTestObj::setReflectedStringAttr(const WebDOMString& newReflectedStringAttr)
{
    if (!impl())
        return;

    impl()->setAttribute(WebCore::HTMLNames::reflectedstringattrAttr, newReflectedStringAttr);
}

int WebDOMTestObj::reflectedIntegralAttr() const
{
    if (!impl())
        return 0;

    return impl()->getIntegralAttribute(WebCore::HTMLNames::reflectedintegralattrAttr);
}

void WebDOMTestObj::setReflectedIntegralAttr(int newReflectedIntegralAttr)
{
    if (!impl())
        return;

    impl()->setIntegralAttribute(WebCore::HTMLNames::reflectedintegralattrAttr, newReflectedIntegralAttr);
}

bool WebDOMTestObj::reflectedBooleanAttr() const
{
    if (!impl())
        return false;

    return impl()->hasAttribute(WebCore::HTMLNames::reflectedbooleanattrAttr);
}

void WebDOMTestObj::setReflectedBooleanAttr(bool newReflectedBooleanAttr)
{
    if (!impl())
        return;

    impl()->setBooleanAttribute(WebCore::HTMLNames::reflectedbooleanattrAttr, newReflectedBooleanAttr);
}

WebDOMString WebDOMTestObj::reflectedURLAttr() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->getURLAttribute(WebCore::HTMLNames::reflectedurlattrAttr));
}

void WebDOMTestObj::setReflectedURLAttr(const WebDOMString& newReflectedURLAttr)
{
    if (!impl())
        return;

    impl()->setAttribute(WebCore::HTMLNames::reflectedurlattrAttr, newReflectedURLAttr);
}

WebDOMString WebDOMTestObj::reflectedStringAttr() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->getAttribute(WebCore::HTMLNames::customContentStringAttrAttr));
}

void WebDOMTestObj::setReflectedStringAttr(const WebDOMString& newReflectedStringAttr)
{
    if (!impl())
        return;

    impl()->setAttribute(WebCore::HTMLNames::customContentStringAttrAttr, newReflectedStringAttr);
}

int WebDOMTestObj::reflectedCustomIntegralAttr() const
{
    if (!impl())
        return 0;

    return impl()->getIntegralAttribute(WebCore::HTMLNames::customContentIntegralAttrAttr);
}

void WebDOMTestObj::setReflectedCustomIntegralAttr(int newReflectedCustomIntegralAttr)
{
    if (!impl())
        return;

    impl()->setIntegralAttribute(WebCore::HTMLNames::customContentIntegralAttrAttr, newReflectedCustomIntegralAttr);
}

bool WebDOMTestObj::reflectedCustomBooleanAttr() const
{
    if (!impl())
        return false;

    return impl()->hasAttribute(WebCore::HTMLNames::customContentBooleanAttrAttr);
}

void WebDOMTestObj::setReflectedCustomBooleanAttr(bool newReflectedCustomBooleanAttr)
{
    if (!impl())
        return;

    impl()->setBooleanAttribute(WebCore::HTMLNames::customContentBooleanAttrAttr, newReflectedCustomBooleanAttr);
}

WebDOMString WebDOMTestObj::reflectedURLAttr() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->getURLAttribute(WebCore::HTMLNames::customContentURLAttrAttr));
}

void WebDOMTestObj::setReflectedURLAttr(const WebDOMString& newReflectedURLAttr)
{
    if (!impl())
        return;

    impl()->setAttribute(WebCore::HTMLNames::customContentURLAttrAttr, newReflectedURLAttr);
}

int WebDOMTestObj::attrWithGetterException() const
{
    if (!impl())
        return 0;

    WebCore::ExceptionCode ec = 0;
    int result = impl()->attrWithGetterException(ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
    return result;
}

void WebDOMTestObj::setAttrWithGetterException(int newAttrWithGetterException)
{
    if (!impl())
        return;

    WebCore::ExceptionCode ec = 0;
    impl()->setAttrWithGetterException(newAttrWithGetterException, ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
}

int WebDOMTestObj::attrWithSetterException() const
{
    if (!impl())
        return 0;

    return impl()->attrWithSetterException();
}

void WebDOMTestObj::setAttrWithSetterException(int newAttrWithSetterException)
{
    if (!impl())
        return;

    WebCore::ExceptionCode ec = 0;
    impl()->setAttrWithSetterException(newAttrWithSetterException, ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
}

WebDOMString WebDOMTestObj::stringAttrWithGetterException() const
{
    if (!impl())
        return WebDOMString();

    WebCore::ExceptionCode ec = 0;
    WebDOMString result = impl()->stringAttrWithGetterException(ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
    return static_cast<const WebCore::String&>(result);
}

void WebDOMTestObj::setStringAttrWithGetterException(const WebDOMString& newStringAttrWithGetterException)
{
    if (!impl())
        return;

    WebCore::ExceptionCode ec = 0;
    impl()->setStringAttrWithGetterException(newStringAttrWithGetterException, ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
}

WebDOMString WebDOMTestObj::stringAttrWithSetterException() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->stringAttrWithSetterException());
}

void WebDOMTestObj::setStringAttrWithSetterException(const WebDOMString& newStringAttrWithSetterException)
{
    if (!impl())
        return;

    WebCore::ExceptionCode ec = 0;
    impl()->setStringAttrWithSetterException(newStringAttrWithSetterException, ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
}

WebDOMString WebDOMTestObj::scriptStringAttr() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->scriptStringAttr());
}

#if ENABLE(Condition1)
int WebDOMTestObj::conditionalAttr1() const
{
    if (!impl())
        return 0;

    return impl()->conditionalAttr1();
}

void WebDOMTestObj::setConditionalAttr1(int newConditionalAttr1)
{
    if (!impl())
        return;

    impl()->setConditionalAttr1(newConditionalAttr1);
}

#endif
#if ENABLE(Condition1) && ENABLE(Condition2)
int WebDOMTestObj::conditionalAttr2() const
{
    if (!impl())
        return 0;

    return impl()->conditionalAttr2();
}

void WebDOMTestObj::setConditionalAttr2(int newConditionalAttr2)
{
    if (!impl())
        return;

    impl()->setConditionalAttr2(newConditionalAttr2);
}

#endif
#if ENABLE(Condition1) || ENABLE(Condition2)
int WebDOMTestObj::conditionalAttr3() const
{
    if (!impl())
        return 0;

    return impl()->conditionalAttr3();
}

void WebDOMTestObj::setConditionalAttr3(int newConditionalAttr3)
{
    if (!impl())
        return;

    impl()->setConditionalAttr3(newConditionalAttr3);
}

#endif
int WebDOMTestObj::description() const
{
    if (!impl())
        return 0;

    return impl()->description();
}

int WebDOMTestObj::id() const
{
    if (!impl())
        return 0;

    return impl()->id();
}

void WebDOMTestObj::setId(int newId)
{
    if (!impl())
        return;

    impl()->setId(newId);
}

WebDOMString WebDOMTestObj::hash() const
{
    if (!impl())
        return WebDOMString();

    return static_cast<const WebCore::String&>(impl()->hash());
}

void WebDOMTestObj::voidMethod()
{
    if (!impl())
        return;

    impl()->voidMethod();
}

void WebDOMTestObj::voidMethodWithArgs(int intArg, const WebDOMString& strArg, const WebDOMTestObj& objArg)
{
    if (!impl())
        return;

    impl()->voidMethodWithArgs(intArg, strArg, toWebCore(objArg));
}

int WebDOMTestObj::intMethod()
{
    if (!impl())
        return 0;

    return impl()->intMethod();
}

int WebDOMTestObj::intMethodWithArgs(int intArg, const WebDOMString& strArg, const WebDOMTestObj& objArg)
{
    if (!impl())
        return 0;

    return impl()->intMethodWithArgs(intArg, strArg, toWebCore(objArg));
}

WebDOMTestObj WebDOMTestObj::objMethod()
{
    if (!impl())
        return WebDOMTestObj();

    return toWebKit(WTF::getPtr(impl()->objMethod()));
}

WebDOMTestObj WebDOMTestObj::objMethodWithArgs(int intArg, const WebDOMString& strArg, const WebDOMTestObj& objArg)
{
    if (!impl())
        return WebDOMTestObj();

    return toWebKit(WTF::getPtr(impl()->objMethodWithArgs(intArg, strArg, toWebCore(objArg))));
}

WebDOMTestObj WebDOMTestObj::methodThatRequiresAllArgs(const WebDOMString& strArg, const WebDOMTestObj& objArg)
{
    if (!impl())
        return WebDOMTestObj();

    return toWebKit(WTF::getPtr(impl()->methodThatRequiresAllArgs(strArg, toWebCore(objArg))));
}

WebDOMTestObj WebDOMTestObj::methodThatRequiresAllArgsAndThrows(const WebDOMString& strArg, const WebDOMTestObj& objArg)
{
    if (!impl())
        return WebDOMTestObj();

    WebCore::ExceptionCode ec = 0;
    WebDOMTestObj result = toWebKit(WTF::getPtr(impl()->methodThatRequiresAllArgsAndThrows(strArg, toWebCore(objArg), ec)));
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
    return result;
}

void WebDOMTestObj::serializedValue(const WebDOMString& serializedArg)
{
    if (!impl())
        return;

    impl()->serializedValue(WebCore::SerializedScriptValue::create(WebCore::String(serializedArg)));
}

void WebDOMTestObj::methodWithException()
{
    if (!impl())
        return;

    WebCore::ExceptionCode ec = 0;
    impl()->methodWithException(ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
}

void WebDOMTestObj::addEventListener(const WebDOMString& type, const WebDOMEventListener& listener, bool useCapture)
{
    if (!impl())
        return;

    impl()->addEventListener(type, toWebCore(listener), useCapture);
}

void WebDOMTestObj::removeEventListener(const WebDOMString& type, const WebDOMEventListener& listener, bool useCapture)
{
    if (!impl())
        return;

    impl()->removeEventListener(type, toWebCore(listener), useCapture);
}

void WebDOMTestObj::withDynamicFrame()
{
    if (!impl())
        return;

    impl()->withDynamicFrame();
}

void WebDOMTestObj::withDynamicFrameAndArg(int intArg)
{
    if (!impl())
        return;

    impl()->withDynamicFrameAndArg(intArg);
}

void WebDOMTestObj::withDynamicFrameAndOptionalArg(int intArg, int optionalArg)
{
    if (!impl())
        return;

    impl()->withDynamicFrameAndOptionalArg(intArg, optionalArg);
}

void WebDOMTestObj::withScriptStateVoid()
{
    if (!impl())
        return;

    impl()->withScriptStateVoid();
}

WebDOMTestObj WebDOMTestObj::withScriptStateObj()
{
    if (!impl())
        return WebDOMTestObj();

    return toWebKit(WTF::getPtr(impl()->withScriptStateObj()));
}

void WebDOMTestObj::withScriptStateVoidException()
{
    if (!impl())
        return;

    WebCore::ExceptionCode ec = 0;
    impl()->withScriptStateVoidException(ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
}

WebDOMTestObj WebDOMTestObj::withScriptStateObjException()
{
    if (!impl())
        return WebDOMTestObj();

    WebCore::ExceptionCode ec = 0;
    WebDOMTestObj result = toWebKit(WTF::getPtr(impl()->withScriptStateObjException(ec)));
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
    return result;
}

void WebDOMTestObj::withScriptExecutionContext()
{
    if (!impl())
        return;

    impl()->withScriptExecutionContext();
}

void WebDOMTestObj::methodWithOptionalArg(int opt)
{
    if (!impl())
        return;

    impl()->methodWithOptionalArg(opt);
}

void WebDOMTestObj::methodWithNonOptionalArgAndOptionalArg(int nonOpt, int opt)
{
    if (!impl())
        return;

    impl()->methodWithNonOptionalArgAndOptionalArg(nonOpt, opt);
}

void WebDOMTestObj::methodWithNonOptionalArgAndTwoOptionalArgs(int nonOpt, int opt1, int opt2)
{
    if (!impl())
        return;

    impl()->methodWithNonOptionalArgAndTwoOptionalArgs(nonOpt, opt1, opt2);
}

WebCore::TestObj* toWebCore(const WebDOMTestObj& wrapper)
{
    return wrapper.impl();
}

WebDOMTestObj toWebKit(WebCore::TestObj* value)
{
    return WebDOMTestObj(value);
}
