/*
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
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
#include "EventHandler.h"

#include "Cursor.h"
#include "Document.h"
#include "EventNames.h"
#include "FloatPoint.h"
#include "FocusController.h"
#include "FrameLoader.h"
#include "FrameQt.h"
#include "FrameTree.h"
#include "FrameView.h"
#include "HTMLFrameSetElement.h"
#include "HitTestRequest.h"
#include "HitTestResult.h"
#include "KeyboardEvent.h"
#include "MouseEventWithHitTestResults.h"
#include "Page.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformScrollBar.h"
#include "PlatformWheelEvent.h"
#include "RenderWidget.h"

namespace WebCore {

using namespace EventNames;

#define notImplemented() qDebug("FIXME: UNIMPLEMENTED: %s:%d (%s)", __FILE__, __LINE__, __FUNCTION__)

static bool isKeyboardOptionTab(KeyboardEvent* event)
{
    return event
        && (event->type() == keydownEvent || event->type() == keypressEvent)
        && event->altKey()
        && event->keyIdentifier() == "U+000009";
}

bool EventHandler::tabsToLinks(KeyboardEvent* event) const
{
    return isKeyboardOptionTab(event);
}

bool EventHandler::tabsToAllControls(KeyboardEvent* event) const
{
    bool handlingOptionTab = isKeyboardOptionTab(event);
    
    return handlingOptionTab;
}

void EventHandler::freeClipboard()
{
    notImplemented();
}

void EventHandler::focusDocumentView()
{
    Page* page = m_frame->page();
    if (page)
        page->focusController()->setFocusedFrame(m_frame);
}

bool EventHandler::passWidgetMouseDownEventToWidget(const MouseEventWithHitTestResults& event)
{
    // Figure out which view to send the event to.
    RenderObject* target = event.targetNode() ? event.targetNode()->renderer() : 0;
    if (!target || !target->isWidget())
        return false;

    return passMouseDownEventToWidget(static_cast<RenderWidget*>(target)->widget());
}

bool EventHandler::passWidgetMouseDownEventToWidget(RenderWidget* renderWidget)
{
    return passMouseDownEventToWidget(renderWidget->widget());
}

bool EventHandler::passMouseDownEventToWidget(Widget* widget)
{
    notImplemented();
    return false;
}

bool EventHandler::lastEventIsMouseUp() const
{
    //this is some hack for mac. it shouldn't be platform specific at all
    return false;
}
    
bool EventHandler::dragHysteresisExceeded(const FloatPoint& floatDragViewportLocation) const
{
    notImplemented();
    return false;
}

bool EventHandler::handleDrag(const MouseEventWithHitTestResults& event)
{
    //notImplemented();
    return false;
}

bool EventHandler::handleMouseUp(const MouseEventWithHitTestResults&)
{
    //i don't know what this does. looks like more mac code disguised as
    //  platform stuff
    return true;
}

bool EventHandler::passSubframeEventToSubframe(MouseEventWithHitTestResults& event, Frame* subframe)
{
    notImplemented();
    return false;
}

bool EventHandler::passWheelEventToWidget(Widget* widget)
{
    notImplemented();
    return false;
}

// Called as we walk up the element chain for nodes with CSS property -webkit-user-drag == auto
bool EventHandler::shouldDragAutoNode(Node* node, const IntPoint& point) const
{
    notImplemented();
    return false;
}

// returns if we should continue "default processing", i.e., whether eventhandler canceled
bool EventHandler::dispatchDragSrcEvent(const AtomicString& eventType, const PlatformMouseEvent& event)
{
    notImplemented();
    return false;
}

bool EventHandler::passMousePressEventToSubframe(MouseEventWithHitTestResults& mev, Frame* subframe)
{
    return passSubframeEventToSubframe(mev, subframe);
}

bool EventHandler::passMouseMoveEventToSubframe(MouseEventWithHitTestResults& mev, Frame* subframe)
{
    return passSubframeEventToSubframe(mev, subframe);
}

bool EventHandler::passMouseReleaseEventToSubframe(MouseEventWithHitTestResults& mev, Frame* subframe)
{
    return passSubframeEventToSubframe(mev, subframe);
}

bool EventHandler::passWheelEventToSubframe(PlatformWheelEvent&, Frame* subframe)
{
    return passWheelEventToWidget(subframe->view());
}

bool EventHandler::passMousePressEventToScrollbar(MouseEventWithHitTestResults&, PlatformScrollbar* scrollbar)
{
    return passWheelEventToWidget(scrollbar);
}

}
