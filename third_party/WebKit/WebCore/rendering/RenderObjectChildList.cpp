/*
 * Copyright (C) 2009 Apple Inc.  All rights reserved.
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
#include "RenderObjectChildList.h"

#include "RenderBlock.h"
#include "RenderCounter.h"
#include "RenderImageGeneratedContent.h"
#include "RenderInline.h"
#include "RenderTextFragment.h"

namespace WebCore {

void RenderObjectChildList::destroyLeftoverChildren()
{
    while (firstChild()) {
        if (firstChild()->isListMarker() || (firstChild()->style()->styleType() == RenderStyle::FIRST_LETTER && !firstChild()->isText()))
            firstChild()->remove();  // List markers are owned by their enclosing list and so don't get destroyed by this container. Similarly, first letters are destroyed by their remaining text fragment.
        else {
            // Destroy any anonymous children remaining in the render tree, as well as implicit (shadow) DOM elements like those used in the engine-based text fields.
            if (firstChild()->element())
                firstChild()->element()->setRenderer(0);
            firstChild()->destroy();
        }
    }
}

static RenderObject* beforeAfterContainer(RenderObject* container, RenderStyle::PseudoId type)
{
    if (type == RenderStyle::BEFORE) {
        RenderObject* first = container;
        do {
            // Skip list markers.
            first = first->firstChild();
            while (first && first->isListMarker())
                first = first->nextSibling();
        } while (first && first->isAnonymous() && first->style()->styleType() == RenderStyle::NOPSEUDO);
        if (first && first->style()->styleType() != type)
            return 0;
        return first;
    }
    if (type == RenderStyle::AFTER) {
        RenderObject* last = container;
        do {
            last = last->lastChild();
        } while (last && last->isAnonymous() && last->style()->styleType() == RenderStyle::NOPSEUDO && !last->isListMarker());
        if (last && last->style()->styleType() != type)
            return 0;
        return last;
    }

    ASSERT_NOT_REACHED();
    return 0;
}

static RenderObject* findBeforeAfterParent(RenderObject* object)
{
    // Only table parts need to search for the :before or :after parent
    if (!(object->isTable() || object->isTableSection() || object->isTableRow()))
        return object;

    RenderObject* beforeAfterParent = object;
    while (beforeAfterParent && !(beforeAfterParent->isText() || beforeAfterParent->isImage()))
        beforeAfterParent = beforeAfterParent->firstChild();
    return beforeAfterParent;
}

static void invalidateCountersInContainer(RenderObject* container)
{
    if (!container)
        return;
    container = findBeforeAfterParent(container);
    if (!container)
        return;
    for (RenderObject* content = container->firstChild(); content; content = content->nextSibling()) {
        if (content->isCounter())
            static_cast<RenderCounter*>(content)->invalidate();
    }
}

void RenderObjectChildList::invalidateCounters(RenderObject* owner)
{
    ASSERT(!owner->documentBeingDestroyed());
    invalidateCountersInContainer(beforeAfterContainer(owner, RenderStyle::BEFORE));
    invalidateCountersInContainer(beforeAfterContainer(owner, RenderStyle::AFTER));
}

void RenderObjectChildList::updateBeforeAfterContent(RenderObject* owner, RenderStyle::PseudoId type, RenderObject* styledObject)
{
    // Double check that the document did in fact use generated content rules.  Otherwise we should not have been called.
    ASSERT(owner->document()->usesBeforeAfterRules());

    // In CSS2, before/after pseudo-content cannot nest.  Check this first.
    if (owner->style()->styleType() == RenderStyle::BEFORE || owner->style()->styleType() == RenderStyle::AFTER)
        return;
    
    if (!styledObject)
        styledObject = owner;

    RenderStyle* pseudoElementStyle = styledObject->getCachedPseudoStyle(type);
    RenderObject* child = beforeAfterContainer(owner, type);

    // Whether or not we currently have generated content attached.
    bool oldContentPresent = child;

    // Whether or not we now want generated content.  
    bool newContentWanted = pseudoElementStyle && pseudoElementStyle->display() != NONE;

    // For <q><p/></q>, if this object is the inline continuation of the <q>, we only want to generate
    // :after content and not :before content.
    if (newContentWanted && type == RenderStyle::BEFORE && owner->isRenderInline() && toRenderInline(owner)->isInlineContinuation())
        newContentWanted = false;

    // Similarly, if we're the beginning of a <q>, and there's an inline continuation for our object,
    // then we don't generate the :after content.
    if (newContentWanted && type == RenderStyle::AFTER && owner->isRenderInline() && toRenderInline(owner)->continuation())
        newContentWanted = false;
    
    // If we don't want generated content any longer, or if we have generated content, but it's no longer
    // identical to the new content data we want to build render objects for, then we nuke all
    // of the old generated content.
    if (!newContentWanted || (oldContentPresent && Node::diff(child->style(), pseudoElementStyle) == Node::Detach)) {
        // Nuke the child. 
        if (child && child->style()->styleType() == type) {
            oldContentPresent = false;
            child->destroy();
            child = (type == RenderStyle::BEFORE) ? owner->virtualChildren()->firstChild() : owner->virtualChildren()->lastChild();
        }
    }

    // If we have no pseudo-element style or if the pseudo-element style's display type is NONE, then we
    // have no generated content and can now return.
    if (!newContentWanted)
        return;

    if (owner->isRenderInline() && !pseudoElementStyle->isDisplayInlineType() && pseudoElementStyle->floating() == FNONE &&
        !(pseudoElementStyle->position() == AbsolutePosition || pseudoElementStyle->position() == FixedPosition))
        // According to the CSS2 spec (the end of section 12.1), the only allowed
        // display values for the pseudo style are NONE and INLINE for inline flows.
        // FIXME: CSS2.1 lifted this restriction, but block display types will crash.
        // For now we at least relax the restriction to allow all inline types like inline-block
        // and inline-table.
        pseudoElementStyle->setDisplay(INLINE);

    if (oldContentPresent) {
        if (child && child->style()->styleType() == type) {
            // We have generated content present still.  We want to walk this content and update our
            // style information with the new pseudo-element style.
            child->setStyle(pseudoElementStyle);

            RenderObject* beforeAfterParent = findBeforeAfterParent(child);
            if (!beforeAfterParent)
                return;

            // Note that if we ever support additional types of generated content (which should be way off
            // in the future), this code will need to be patched.
            for (RenderObject* genChild = beforeAfterParent->firstChild(); genChild; genChild = genChild->nextSibling()) {
                if (genChild->isText())
                    // Generated text content is a child whose style also needs to be set to the pseudo-element style.
                    genChild->setStyle(pseudoElementStyle);
                else if (genChild->isImage()) {
                    // Images get an empty style that inherits from the pseudo.
                    RefPtr<RenderStyle> style = RenderStyle::create();
                    style->inheritFrom(pseudoElementStyle);
                    genChild->setStyle(style.release());
                } else
                    // Must be a first-letter container. updateFirstLetter() will take care of it.
                    ASSERT(genChild->style()->styleType() == RenderStyle::FIRST_LETTER);
            }
        }
        return; // We've updated the generated content. That's all we needed to do.
    }
    
    RenderObject* insertBefore = (type == RenderStyle::BEFORE) ? owner->virtualChildren()->firstChild() : 0;

    // Generated content consists of a single container that houses multiple children (specified
    // by the content property).  This generated content container gets the pseudo-element style set on it.
    RenderObject* generatedContentContainer = 0;
    
    // Walk our list of generated content and create render objects for each.
    for (const ContentData* content = pseudoElementStyle->contentData(); content; content = content->m_next) {
        RenderObject* renderer = 0;
        switch (content->m_type) {
            case CONTENT_NONE:
                break;
            case CONTENT_TEXT:
                renderer = new (owner->renderArena()) RenderTextFragment(owner->document() /* anonymous object */, content->m_content.m_text);
                renderer->setStyle(pseudoElementStyle);
                break;
            case CONTENT_OBJECT: {
                RenderImageGeneratedContent* image = new (owner->renderArena()) RenderImageGeneratedContent(owner->document()); // anonymous object
                RefPtr<RenderStyle> style = RenderStyle::create();
                style->inheritFrom(pseudoElementStyle);
                image->setStyle(style.release());
                if (StyleImage* styleImage = content->m_content.m_image)
                    image->setStyleImage(styleImage);
                renderer = image;
                break;
            }
            case CONTENT_COUNTER:
                renderer = new (owner->renderArena()) RenderCounter(owner->document(), *content->m_content.m_counter);
                renderer->setStyle(pseudoElementStyle);
                break;
        }

        if (renderer) {
            if (!generatedContentContainer) {
                // Make a generated box that might be any display type now that we are able to drill down into children
                // to find the original content properly.
                generatedContentContainer = RenderObject::createObject(owner->document(), pseudoElementStyle);
                generatedContentContainer->setStyle(pseudoElementStyle);
                owner->addChild(generatedContentContainer, insertBefore);
            }
            generatedContentContainer->addChild(renderer);
        }
    }
}

}
