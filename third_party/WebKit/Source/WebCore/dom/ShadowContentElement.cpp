/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
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

#include "config.h"
#include "ShadowContentElement.h"

#include "HTMLNames.h"
#include "ShadowContentSelector.h"

namespace WebCore {

void ShadowInclusion::append(PassRefPtr<ShadowInclusion> next)
{
    ASSERT(!m_next);
    ASSERT(!next->previous());
    m_next = next;
    m_next->m_previous = this;
}

void ShadowInclusion::unlink()
{
    ASSERT(!m_previous); // Can be called only for a head.
    RefPtr<ShadowInclusion> item = this;
    while (item) {
        ASSERT(!item->previous());
        RefPtr<ShadowInclusion> nextItem = item->m_next;
        item->m_next.clear();
        if (nextItem)
            nextItem->m_previous.clear();
        item = nextItem;
    }
}

ShadowInclusionList::ShadowInclusionList()
{
}

ShadowInclusionList::~ShadowInclusionList()
{
    ASSERT(isEmpty());
}

ShadowInclusion* ShadowInclusionList::find(Node* content) const
{
    for (ShadowInclusion* item = first(); item; item = item->next()) {
        if (content == item->content())
            return item;
    }
    
    return 0;
}

void ShadowInclusionList::clear()
{
    if (isEmpty()) {
        ASSERT(!m_last);
        return;
    }

    m_first->unlink();
    m_first.clear();
    m_last.clear();
}

void ShadowInclusionList::append(PassRefPtr<ShadowInclusion> child)
{
    if (isEmpty()) {
        ASSERT(!m_last);
        m_first = m_last = child;
        return;
    }

    m_last->append(child);
    m_last = m_last->next();
}


PassRefPtr<ShadowContentElement> ShadowContentElement::create(Document* document)
{
    DEFINE_STATIC_LOCAL(QualifiedName, tagName, (nullAtom, "webkitShadowContent", HTMLNames::divTag.namespaceURI()));
    return adoptRef(new ShadowContentElement(tagName, document));
}

ShadowContentElement::ShadowContentElement(const QualifiedName& name, Document* document)
    : StyledElement(name, document, CreateHTMLElement)
{
}

ShadowContentElement::~ShadowContentElement()
{
}

void ShadowContentElement::attach()
{
    ASSERT(!firstChild()); // Currently doesn't support any light child.
    StyledElement::attach();
    if (ShadowContentSelector* selector = ShadowContentSelector::currentInstance()) {
        selector->willAttachContentFor(this);
        selector->selectInclusion(&m_inclusions);
        for (ShadowInclusion* inclusion = m_inclusions.first(); inclusion; inclusion = inclusion->next())
            inclusion->content()->detach();
        for (ShadowInclusion* inclusion = m_inclusions.first(); inclusion; inclusion = inclusion->next())
            inclusion->content()->attach();
        selector->didAttachContent();
    }
}

void ShadowContentElement::detach()
{
    m_inclusions.clear();
    StyledElement::detach();
}

bool ShadowContentElement::shouldInclude(Node*)
{
    return true;
}


}
