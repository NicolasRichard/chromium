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

#if ENABLE(Condition1) || ENABLE(Condition2)

#include "WebDOMTestInterface.h"

#include "TestInterface.h"
#include "WebExceptionHandler.h"
#include <wtf/GetPtr.h>
#include <wtf/RefPtr.h>

struct WebDOMTestInterface::WebDOMTestInterfacePrivate {
    WebDOMTestInterfacePrivate(WebCore::TestInterface* object = 0)
        : impl(object)
    {
    }

    RefPtr<WebCore::TestInterface> impl;
};

WebDOMTestInterface::WebDOMTestInterface()
    : WebDOMObject()
    , m_impl(0)
{
}

WebDOMTestInterface::WebDOMTestInterface(WebCore::TestInterface* impl)
    : WebDOMObject()
    , m_impl(new WebDOMTestInterfacePrivate(impl))
{
}

WebDOMTestInterface::WebDOMTestInterface(const WebDOMTestInterface& copy)
    : WebDOMObject()
{
    m_impl = copy.impl() ? new WebDOMTestInterfacePrivate(copy.impl()) : 0;
}

WebCore::TestInterface* WebDOMTestInterface::impl() const
{
    return m_impl ? m_impl->impl.get() : 0;
}

WebDOMTestInterface::~WebDOMTestInterface()
{
    delete m_impl;
    m_impl = 0;
}

WebCore::TestInterface* toWebCore(const WebDOMTestInterface& wrapper)
{
    return wrapper.impl();
}

WebDOMTestInterface toWebKit(WebCore::TestInterface* value)
{
    return WebDOMTestInterface(value);
}

#endif // ENABLE(Condition1) || ENABLE(Condition2)
