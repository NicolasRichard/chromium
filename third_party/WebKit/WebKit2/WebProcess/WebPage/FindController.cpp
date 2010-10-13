/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "FindController.h"

#include "BackingStore.h"
#include "FindPageOverlay.h"
#include "WebPage.h"
#include "WebPageProxyMessages.h"
#include "WebProcess.h"
#include <WebCore/Frame.h>
#include <WebCore/FrameView.h>
#include <WebCore/GraphicsContext.h>
#include <WebCore/Page.h>

using namespace WebCore;

namespace WebKit {

FindController::FindController(WebPage* webPage)
    : m_webPage(webPage)
    , m_findPageOverlay(0)
    , m_isShowingFindIndicator(false)
{
}

static Frame* frameWithSelection(Page* page)
{
    for (Frame* frame = page->mainFrame(); frame; frame = frame->tree()->traverseNext()) {
        if (frame->selection()->isRange())
            return frame;
    }

    return 0;
}

void FindController::findString(const String& string, FindDirection findDirection, FindOptions findOptions, unsigned maxNumMatches)
{
    TextCaseSensitivity caseSensitivity = findOptions & FindOptionsCaseInsensitive ? TextCaseInsensitive : TextCaseSensitive;
    bool found = m_webPage->corePage()->findString(string, caseSensitivity,
                                                   findDirection == FindDirectionForward ? WebCore::FindDirectionForward : WebCore::FindDirectionBackward,
                                                   findOptions & FindOptionsWrapAround);

    Frame* selectedFrame = frameWithSelection(m_webPage->corePage());

    bool shouldShowOverlay = false;

    if (!found) {
        // We didn't find the string, clear all text matches.
        m_webPage->corePage()->unmarkAllTextMatches();

        // And clear the selection.
        if (selectedFrame)
            selectedFrame->selection()->clear();

        resetFindIndicator();
    } else {
        shouldShowOverlay = findOptions & FindOptionsShowOverlay;

        if (shouldShowOverlay) {
            unsigned numMatches = m_webPage->corePage()->markAllMatchesForText(string, caseSensitivity, false, maxNumMatches);

            // Check if we have more matches than allowed.
            if (numMatches > maxNumMatches)
                shouldShowOverlay = false;
        }

        if (!(findOptions & FindOptionsShowFindIndicator) || !updateFindIndicator(selectedFrame)) {
            // Either we shouldn't show the find indicator, or we couldn't update it.
            resetFindIndicator();
        }
    }

    if (!shouldShowOverlay) {
        if (m_findPageOverlay) {
            // Get rid of the overlay.
            m_webPage->uninstallPageOverlay();
        }
        
        ASSERT(!m_findPageOverlay);
        return;
    }

    if (!m_findPageOverlay) {
        OwnPtr<FindPageOverlay> findPageOverlay = FindPageOverlay::create(this);
        m_findPageOverlay = findPageOverlay.get();
        m_webPage->installPageOverlay(findPageOverlay.release());
    } else {
        // The page overlay needs to be repainted.
        m_findPageOverlay->setNeedsDisplay();
    }
}

void FindController::hideFindUI()
{
    // FIXME: Implement.
}

void FindController::findPageOverlayDestroyed()
{
    ASSERT(m_findPageOverlay);
    m_findPageOverlay = 0;
}

bool FindController::updateFindIndicator(Frame* selectedFrame)
{
    if (!selectedFrame)
        return false;

    IntRect selectionRect = enclosingIntRect(selectedFrame->selection()->bounds());
    Vector<FloatRect> textRects;
    selectedFrame->selection()->getClippedVisibleTextRectangles(textRects);

    // Create a backing store and paint the find indicator text into it.
    RefPtr<BackingStore> findIndicatorTextBackingStore = BackingStore::createSharable(selectionRect.size());
    OwnPtr<GraphicsContext> graphicsContext = findIndicatorTextBackingStore->createGraphicsContext();

    graphicsContext->translate(-selectionRect.x(), -selectionRect.y());
    selectedFrame->view()->setPaintBehavior(PaintBehaviorSelectionOnly | PaintBehaviorForceBlackText | PaintBehaviorFlattenCompositingLayers);
    selectedFrame->document()->updateLayout();
    
    graphicsContext->clip(selectionRect);
    selectedFrame->view()->paint(graphicsContext.get(), selectionRect);
    selectedFrame->view()->setPaintBehavior(PaintBehaviorNormal);
    
    SharedMemory::Handle handle;
    if (!findIndicatorTextBackingStore->createHandle(handle))
        return false;

    // We want the selection rect in window coordinates.
    IntRect selectionRectInWindowCoordinates = selectedFrame->view()->contentsToWindow(selectionRect);

    // We want the text rects in selection rect coordinates.
    Vector<FloatRect> textRectsInSelectionRectCoordinates;
    
    for (size_t i = 0; i < textRects.size(); ++i) {
        IntRect textRectInSelectionRectCoordinates = selectedFrame->view()->contentsToWindow(enclosingIntRect(textRects[i]));
        textRectInSelectionRectCoordinates.move(-selectionRectInWindowCoordinates.x(), -selectionRectInWindowCoordinates.y());

        textRectsInSelectionRectCoordinates.append(textRectInSelectionRectCoordinates);
    }            
    
    WebProcess::shared().connection()->send(Messages::WebPageProxy::SetFindIndicator(selectionRectInWindowCoordinates, textRectsInSelectionRectCoordinates, handle), m_webPage->pageID());
    m_isShowingFindIndicator = true;

    return true;
}

void FindController::resetFindIndicator()
{
    if (!m_isShowingFindIndicator)
        return;

    SharedMemory::Handle handle;
    WebProcess::shared().connection()->send(Messages::WebPageProxy::SetFindIndicator(FloatRect(), Vector<FloatRect>(), handle), m_webPage->pageID());
    m_isShowingFindIndicator = false;
}


} // namespace WebKit
