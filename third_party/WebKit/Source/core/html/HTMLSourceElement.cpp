/*
 * Copyright (C) 2007, 2008, 2010 Apple Inc. All rights reserved.
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

#include "config.h"
#include "core/html/HTMLSourceElement.h"

#include "HTMLNames.h"
#include "core/events/Event.h"
#include "core/events/ThreadLocalEventNames.h"
#include "core/html/HTMLMediaElement.h"
#include "platform/Logging.h"

using namespace std;

namespace WebCore {

using namespace HTMLNames;

inline HTMLSourceElement::HTMLSourceElement(const QualifiedName& tagName, Document& document)
    : HTMLElement(tagName, document)
    , m_errorEventTimer(this, &HTMLSourceElement::errorEventTimerFired)
{
    LOG(Media, "HTMLSourceElement::HTMLSourceElement - %p", this);
    ASSERT(hasTagName(sourceTag));
    ScriptWrappable::init(this);
}

PassRefPtr<HTMLSourceElement> HTMLSourceElement::create(const QualifiedName& tagName, Document& document)
{
    return adoptRef(new HTMLSourceElement(tagName, document));
}

Node::InsertionNotificationRequest HTMLSourceElement::insertedInto(ContainerNode* insertionPoint)
{
    HTMLElement::insertedInto(insertionPoint);
    Element* parent = parentElement();
    if (parent && parent->isMediaElement())
        toHTMLMediaElement(parentNode())->sourceWasAdded(this);
    return InsertionDone;
}

void HTMLSourceElement::removedFrom(ContainerNode* removalRoot)
{
    Element* parent = parentElement();
    if (!parent && removalRoot->isElementNode())
        parent = toElement(removalRoot);
    if (parent && parent->isMediaElement())
        toHTMLMediaElement(parent)->sourceWasRemoved(this);
    HTMLElement::removedFrom(removalRoot);
}

void HTMLSourceElement::setSrc(const String& url)
{
    setAttribute(srcAttr, url);
}

String HTMLSourceElement::media() const
{
    return getAttribute(mediaAttr);
}

void HTMLSourceElement::setMedia(const String& media)
{
    setAttribute(mediaAttr, media);
}

String HTMLSourceElement::type() const
{
    return getAttribute(typeAttr);
}

void HTMLSourceElement::setType(const String& type)
{
    setAttribute(typeAttr, type);
}

void HTMLSourceElement::scheduleErrorEvent()
{
    LOG(Media, "HTMLSourceElement::scheduleErrorEvent - %p", this);
    if (m_errorEventTimer.isActive())
        return;

    m_errorEventTimer.startOneShot(0);
}

void HTMLSourceElement::cancelPendingErrorEvent()
{
    LOG(Media, "HTMLSourceElement::cancelPendingErrorEvent - %p", this);
    m_errorEventTimer.stop();
}

void HTMLSourceElement::errorEventTimerFired(Timer<HTMLSourceElement>*)
{
    LOG(Media, "HTMLSourceElement::errorEventTimerFired - %p", this);
    dispatchEvent(Event::createCancelable(eventNames().errorEvent));
}

bool HTMLSourceElement::isURLAttribute(const Attribute& attribute) const
{
    return attribute.name() == srcAttr || HTMLElement::isURLAttribute(attribute);
}

}
