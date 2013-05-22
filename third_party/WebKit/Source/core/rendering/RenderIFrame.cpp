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
#include "core/rendering/RenderIFrame.h"

#include "HTMLNames.h"
#include "core/html/HTMLIFrameElement.h"
#include "core/page/Frame.h"
#include "core/page/FrameView.h"
#include "core/rendering/RenderView.h"

namespace WebCore {

using namespace HTMLNames;
    
RenderIFrame::RenderIFrame(Element* element)
    : RenderPart(element)
{
}

bool RenderIFrame::shouldComputeSizeAsReplaced() const
{
    // When we're seamless, we use normal block/box sizing code except when inline.
    return !isSeamless();
}

bool RenderIFrame::isInlineBlockOrInlineTable() const
{
    return isSeamless() && isInline();
}

LayoutUnit RenderIFrame::minPreferredLogicalWidth() const
{
    if (!isSeamless())
        return RenderPart::minPreferredLogicalWidth();

    RenderView* childRoot = contentRootRenderer();
    if (!childRoot)
        return 0;

    return childRoot->minPreferredLogicalWidth() + borderAndPaddingLogicalWidth();
}

LayoutUnit RenderIFrame::maxPreferredLogicalWidth() const
{
    if (!isSeamless())
        return RenderPart::maxPreferredLogicalWidth();

    RenderView* childRoot = contentRootRenderer();
    if (!childRoot)
        return 0;

    return childRoot->maxPreferredLogicalWidth() + borderAndPaddingLogicalWidth();
}

bool RenderIFrame::isSeamless() const
{
    return node() && node()->hasTagName(iframeTag) && static_cast<HTMLIFrameElement*>(node())->shouldDisplaySeamlessly();
}

bool RenderIFrame::requiresLayer() const
{
    return RenderPart::requiresLayer() || style()->resize() != RESIZE_NONE;
}

RenderView* RenderIFrame::contentRootRenderer() const
{
    // FIXME: Is this always a valid cast? What about plugins?
    ASSERT(!widget() || widget()->isFrameView());
    FrameView* childFrameView = toFrameView(widget());
    return childFrameView ? childFrameView->frame()->contentRenderer() : 0;
}

void RenderIFrame::layoutSeamlessly()
{
    updateLogicalWidth();
    // FIXME: Containers set their height to 0 before laying out their kids (as we're doing here)
    // however, this causes FrameView::layout() to add vertical scrollbars, incorrectly inflating
    // the resulting contentHeight(). We'll need to make FrameView::layout() smarter.
    setLogicalHeight(0);
    updateWidgetPosition(); // Tell the Widget about our new width/height (it will also layout the child document).

    // Laying out our kids is normally responsible for adjusting our height, so we set it here.
    // Replaced elements normally do not respect padding, but seamless elements should: we'll add
    // both padding and border to the child's logical height here.
    FrameView* childFrameView = toFrameView(widget());
    if (childFrameView) // Widget should never be null during layout(), but just in case.
        setLogicalHeight(childFrameView->contentsHeight() + borderTop() + borderBottom() + paddingTop() + paddingBottom());
    updateLogicalHeight();

    updateWidgetPosition(); // Notify the Widget of our final height.

    // Assert that the child document did a complete layout.
    RenderView* childRoot = childFrameView ? childFrameView->frame()->contentRenderer() : 0;
    ASSERT(!childFrameView || !childFrameView->layoutPending());
    ASSERT_UNUSED(childRoot, !childRoot || !childRoot->needsLayout());
}

void RenderIFrame::layout()
{
    StackStats::LayoutCheckPoint layoutCheckPoint;
    ASSERT(needsLayout());

    if (isSeamless()) {
        layoutSeamlessly();
        // Do not return so as to share the layer and overflow updates below.
    } else {
        updateLogicalWidth();
        // No kids to layout as a replaced element.
        updateLogicalHeight();
    }

    m_overflow.clear();
    addVisualEffectOverflow();
    updateLayerTransform();

    setNeedsLayout(false);
}

}
