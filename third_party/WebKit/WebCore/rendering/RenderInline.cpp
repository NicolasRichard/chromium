/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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
 *
 */

#include "config.h"
#include "RenderInline.h"

#include "Chrome.h"
#include "FloatQuad.h"
#include "GraphicsContext.h"
#include "HitTestResult.h"
#include "Page.h"
#include "RenderArena.h"
#include "RenderBlock.h"
#include "RenderLayer.h"
#include "RenderView.h"
#include "TransformState.h"
#include "VisiblePosition.h"

#if ENABLE(DASHBOARD_SUPPORT)
#include "Frame.h"
#endif

using namespace std;

namespace WebCore {

RenderInline::RenderInline(Node* node)
    : RenderBoxModelObject(node)
    , m_continuation(0)
    , m_lineHeight(-1)
    , m_verticalPosition(PositionUndefined)
{
    setChildrenInline(true);
}

void RenderInline::destroy()
{
    // Make sure to destroy anonymous children first while they are still connected to the rest of the tree, so that they will
    // properly dirty line boxes that they are removed from.  Effects that do :before/:after only on hover could crash otherwise.
    children()->destroyLeftoverChildren();

    // Destroy our continuation before anything other than anonymous children.
    // The reason we don't destroy it before anonymous children is that they may
    // have continuations of their own that are anonymous children of our continuation.
    if (m_continuation) {
        m_continuation->destroy();
        m_continuation = 0;
    }
    
    if (!documentBeingDestroyed()) {
        if (firstLineBox()) {
            // We can't wait for RenderBoxModelObject::destroy to clear the selection,
            // because by then we will have nuked the line boxes.
            // FIXME: The SelectionController should be responsible for this when it
            // is notified of DOM mutations.
            if (isSelectionBorder())
                view()->clearSelection();

            // If line boxes are contained inside a root, that means we're an inline.
            // In that case, we need to remove all the line boxes so that the parent
            // lines aren't pointing to deleted children. If the first line box does
            // not have a parent that means they are either already disconnected or
            // root lines that can just be destroyed without disconnecting.
            if (firstLineBox()->parent()) {
                for (InlineFlowBox* box = firstLineBox(); box; box = box->nextLineBox())
                    box->remove();
            }
        } else if (isInline() && parent())
            parent()->dirtyLinesFromChangedChild(this);
    }

    m_lineBoxes.deleteLineBoxes(renderArena());

    RenderBoxModelObject::destroy();
}

RenderInline* RenderInline::inlineElementContinuation() const
{
    if (!m_continuation || m_continuation->isInline())
        return toRenderInline(m_continuation);
    return toRenderBlock(m_continuation)->inlineElementContinuation();
}

void RenderInline::updateBoxModelInfoFromStyle()
{
    RenderBoxModelObject::updateBoxModelInfoFromStyle();

    setInline(true); // Needed for run-ins, since run-in is considered a block display type.

    // FIXME: Support transforms and reflections on inline flows someday.
    setHasTransform(false);
    setHasReflection(false);    
}

void RenderInline::styleDidChange(StyleDifference diff, const RenderStyle* oldStyle)
{
    RenderBoxModelObject::styleDidChange(diff, oldStyle);

    // Ensure that all of the split inlines pick up the new style. We
    // only do this if we're an inline, since we don't want to propagate
    // a block's style to the other inlines.
    // e.g., <font>foo <h4>goo</h4> moo</font>.  The <font> inlines before
    // and after the block share the same style, but the block doesn't
    // need to pass its style on to anyone else.
    for (RenderInline* currCont = inlineElementContinuation(); currCont; currCont = currCont->inlineElementContinuation()) {
        RenderBoxModelObject* nextCont = currCont->continuation();
        currCont->setContinuation(0);
        currCont->setStyle(style());
        currCont->setContinuation(nextCont);
    }

    m_lineHeight = -1;

    // Update pseudos for :before and :after now.
    if (!isAnonymous() && document()->usesBeforeAfterRules()) {
        children()->updateBeforeAfterContent(this, BEFORE);
        children()->updateBeforeAfterContent(this, AFTER);
    }
}

void RenderInline::addChild(RenderObject* newChild, RenderObject* beforeChild)
{
    if (continuation())
        return addChildToContinuation(newChild, beforeChild);
    return addChildIgnoringContinuation(newChild, beforeChild);
}

static RenderBoxModelObject* nextContinuation(RenderObject* renderer)
{
    if (renderer->isInline() && !renderer->isReplaced())
        return toRenderInline(renderer)->continuation();
    return toRenderBlock(renderer)->inlineElementContinuation();
}

RenderBoxModelObject* RenderInline::continuationBefore(RenderObject* beforeChild)
{
    if (beforeChild && beforeChild->parent() == this)
        return this;

    RenderBoxModelObject* curr = nextContinuation(this);
    RenderBoxModelObject* nextToLast = this;
    RenderBoxModelObject* last = this;
    while (curr) {
        if (beforeChild && beforeChild->parent() == curr) {
            if (curr->firstChild() == beforeChild)
                return last;
            return curr;
        }

        nextToLast = last;
        last = curr;
        curr = nextContinuation(curr);
    }

    if (!beforeChild && !last->firstChild())
        return nextToLast;
    return last;
}

void RenderInline::addChildIgnoringContinuation(RenderObject* newChild, RenderObject* beforeChild)
{
    // Make sure we don't append things after :after-generated content if we have it.
    if (!beforeChild && isAfterContent(lastChild()))
        beforeChild = lastChild();

    if (!newChild->isInline() && !newChild->isFloatingOrPositioned()) {
        // We are placing a block inside an inline. We have to perform a split of this
        // inline into continuations.  This involves creating an anonymous block box to hold
        // |newChild|.  We then make that block box a continuation of this inline.  We take all of
        // the children after |beforeChild| and put them in a clone of this object.
        RefPtr<RenderStyle> newStyle = RenderStyle::create();
        newStyle->inheritFrom(style());
        newStyle->setDisplay(BLOCK);

        RenderBlock* newBox = new (renderArena()) RenderBlock(document() /* anonymous box */);
        newBox->setStyle(newStyle.release());
        RenderBoxModelObject* oldContinuation = continuation();
        setContinuation(newBox);

        // Someone may have put a <p> inside a <q>, causing a split.  When this happens, the :after content
        // has to move into the inline continuation.  Call updateBeforeAfterContent to ensure that our :after
        // content gets properly destroyed.
        bool isLastChild = (beforeChild == lastChild());
        if (document()->usesBeforeAfterRules())
            children()->updateBeforeAfterContent(this, AFTER);
        if (isLastChild && beforeChild != lastChild())
            beforeChild = 0; // We destroyed the last child, so now we need to update our insertion
                             // point to be 0.  It's just a straight append now.

        splitFlow(beforeChild, newBox, newChild, oldContinuation);
        return;
    }

    RenderBoxModelObject::addChild(newChild, beforeChild);

    newChild->setNeedsLayoutAndPrefWidthsRecalc();
}

RenderInline* RenderInline::cloneInline(RenderInline* src)
{
    RenderInline* o = new (src->renderArena()) RenderInline(src->node());
    o->setStyle(src->style());
    return o;
}

void RenderInline::splitInlines(RenderBlock* fromBlock, RenderBlock* toBlock,
                                RenderBlock* middleBlock,
                                RenderObject* beforeChild, RenderBoxModelObject* oldCont)
{
    // Create a clone of this inline.
    RenderInline* clone = cloneInline(this);
    clone->setContinuation(oldCont);

    // Now take all of the children from beforeChild to the end and remove
    // them from |this| and place them in the clone.
    RenderObject* o = beforeChild;
    while (o) {
        RenderObject* tmp = o;
        o = tmp->nextSibling();
        clone->addChildIgnoringContinuation(children()->removeChildNode(this, tmp), 0);
        tmp->setNeedsLayoutAndPrefWidthsRecalc();
    }

    // Hook |clone| up as the continuation of the middle block.
    middleBlock->setContinuation(clone);

    // We have been reparented and are now under the fromBlock.  We need
    // to walk up our inline parent chain until we hit the containing block.
    // Once we hit the containing block we're done.
    RenderBoxModelObject* curr = toRenderBoxModelObject(parent());
    RenderBoxModelObject* currChild = this;
    
    // FIXME: Because splitting is O(n^2) as tags nest pathologically, we cap the depth at which we're willing to clone.
    // There will eventually be a better approach to this problem that will let us nest to a much
    // greater depth (see bugzilla bug 13430) but for now we have a limit.  This *will* result in
    // incorrect rendering, but the alternative is to hang forever.
    unsigned splitDepth = 1;
    const unsigned cMaxSplitDepth = 200; 
    while (curr && curr != fromBlock) {
        ASSERT(curr->isRenderInline());
        if (splitDepth < cMaxSplitDepth) {
            // Create a new clone.
            RenderInline* cloneChild = clone;
            clone = cloneInline(toRenderInline(curr));

            // Insert our child clone as the first child.
            clone->addChildIgnoringContinuation(cloneChild, 0);

            // Hook the clone up as a continuation of |curr|.
            RenderInline* inlineCurr = toRenderInline(curr);
            oldCont = inlineCurr->continuation();
            inlineCurr->setContinuation(clone);
            clone->setContinuation(oldCont);

            // Someone may have indirectly caused a <q> to split.  When this happens, the :after content
            // has to move into the inline continuation.  Call updateBeforeAfterContent to ensure that the inline's :after
            // content gets properly destroyed.
            if (document()->usesBeforeAfterRules())
                inlineCurr->children()->updateBeforeAfterContent(inlineCurr, AFTER);

            // Now we need to take all of the children starting from the first child
            // *after* currChild and append them all to the clone.
            o = currChild->nextSibling();
            while (o) {
                RenderObject* tmp = o;
                o = tmp->nextSibling();
                clone->addChildIgnoringContinuation(inlineCurr->children()->removeChildNode(curr, tmp), 0);
                tmp->setNeedsLayoutAndPrefWidthsRecalc();
            }
        }
        
        // Keep walking up the chain.
        currChild = curr;
        curr = toRenderBoxModelObject(curr->parent());
        splitDepth++;
    }

    // Now we are at the block level. We need to put the clone into the toBlock.
    toBlock->children()->appendChildNode(toBlock, clone);

    // Now take all the children after currChild and remove them from the fromBlock
    // and put them in the toBlock.
    o = currChild->nextSibling();
    while (o) {
        RenderObject* tmp = o;
        o = tmp->nextSibling();
        toBlock->children()->appendChildNode(toBlock, fromBlock->children()->removeChildNode(fromBlock, tmp));
    }
}

void RenderInline::splitFlow(RenderObject* beforeChild, RenderBlock* newBlockBox,
                             RenderObject* newChild, RenderBoxModelObject* oldCont)
{
    RenderBlock* pre = 0;
    RenderBlock* block = containingBlock();
    
    // Delete our line boxes before we do the inline split into continuations.
    block->deleteLineBoxTree();
    
    bool madeNewBeforeBlock = false;
    if (block->isAnonymousBlock() && (!block->parent() || !block->parent()->createsAnonymousWrapper())) {
        // We can reuse this block and make it the preBlock of the next continuation.
        pre = block;
        pre->removePositionedObjects(0);
        block = block->containingBlock();
    } else {
        // No anonymous block available for use.  Make one.
        pre = block->createAnonymousBlock();
        madeNewBeforeBlock = true;
    }

    RenderBlock* post = block->createAnonymousBlock();

    RenderObject* boxFirst = madeNewBeforeBlock ? block->firstChild() : pre->nextSibling();
    if (madeNewBeforeBlock)
        block->children()->insertChildNode(block, pre, boxFirst);
    block->children()->insertChildNode(block, newBlockBox, boxFirst);
    block->children()->insertChildNode(block, post, boxFirst);
    block->setChildrenInline(false);
    
    if (madeNewBeforeBlock) {
        RenderObject* o = boxFirst;
        while (o) {
            RenderObject* no = o;
            o = no->nextSibling();
            pre->children()->appendChildNode(pre, block->children()->removeChildNode(block, no));
            no->setNeedsLayoutAndPrefWidthsRecalc();
        }
    }

    splitInlines(pre, post, newBlockBox, beforeChild, oldCont);

    // We already know the newBlockBox isn't going to contain inline kids, so avoid wasting
    // time in makeChildrenNonInline by just setting this explicitly up front.
    newBlockBox->setChildrenInline(false);

    // We delayed adding the newChild until now so that the |newBlockBox| would be fully
    // connected, thus allowing newChild access to a renderArena should it need
    // to wrap itself in additional boxes (e.g., table construction).
    newBlockBox->addChild(newChild);

    // Always just do a full layout in order to ensure that line boxes (especially wrappers for images)
    // get deleted properly.  Because objects moves from the pre block into the post block, we want to
    // make new line boxes instead of leaving the old line boxes around.
    pre->setNeedsLayoutAndPrefWidthsRecalc();
    block->setNeedsLayoutAndPrefWidthsRecalc();
    post->setNeedsLayoutAndPrefWidthsRecalc();
}

void RenderInline::addChildToContinuation(RenderObject* newChild, RenderObject* beforeChild)
{
    RenderBoxModelObject* flow = continuationBefore(beforeChild);
    ASSERT(!beforeChild || beforeChild->parent()->isRenderBlock() || beforeChild->parent()->isRenderInline());
    RenderBoxModelObject* beforeChildParent = 0;
    if (beforeChild)
        beforeChildParent = toRenderBoxModelObject(beforeChild->parent());
    else {
        RenderBoxModelObject* cont = nextContinuation(flow);
        if (cont)
            beforeChildParent = cont;
        else
            beforeChildParent = flow;
    }

    if (newChild->isFloatingOrPositioned())
        return beforeChildParent->addChildIgnoringContinuation(newChild, beforeChild);

    // A continuation always consists of two potential candidates: an inline or an anonymous
    // block box holding block children.
    bool childInline = newChild->isInline();
    bool bcpInline = beforeChildParent->isInline();
    bool flowInline = flow->isInline();

    if (flow == beforeChildParent)
        return flow->addChildIgnoringContinuation(newChild, beforeChild);
    else {
        // The goal here is to match up if we can, so that we can coalesce and create the
        // minimal # of continuations needed for the inline.
        if (childInline == bcpInline)
            return beforeChildParent->addChildIgnoringContinuation(newChild, beforeChild);
        else if (flowInline == childInline)
            return flow->addChildIgnoringContinuation(newChild, 0); // Just treat like an append.
        else
            return beforeChildParent->addChildIgnoringContinuation(newChild, beforeChild);
    }
}

void RenderInline::paint(PaintInfo& paintInfo, int tx, int ty)
{
    m_lineBoxes.paint(this, paintInfo, tx, ty);
}

void RenderInline::absoluteRects(Vector<IntRect>& rects, int tx, int ty)
{
    if (InlineFlowBox* curr = firstLineBox()) {
        for (; curr; curr = curr->nextLineBox())
            rects.append(IntRect(tx + curr->x(), ty + curr->y(), curr->logicalWidth(), curr->logicalHeight()));
    } else
        rects.append(IntRect(tx, ty, 0, 0));

    if (continuation()) {
        if (continuation()->isBox()) {
            RenderBox* box = toRenderBox(continuation());
            continuation()->absoluteRects(rects, 
                                          tx - containingBlock()->x() + box->x(),
                                          ty - containingBlock()->y() + box->y());
        } else
            continuation()->absoluteRects(rects, tx - containingBlock()->x(), ty - containingBlock()->y());
    }
}

void RenderInline::absoluteQuads(Vector<FloatQuad>& quads)
{
    if (InlineFlowBox* curr = firstLineBox()) {
        for (; curr; curr = curr->nextLineBox()) {
            FloatRect localRect(curr->x(), curr->y(), curr->logicalWidth(), curr->logicalHeight());
            quads.append(localToAbsoluteQuad(localRect));
        }
    } else
        quads.append(localToAbsoluteQuad(FloatRect()));

    if (continuation())
        continuation()->absoluteQuads(quads);
}

int RenderInline::offsetLeft() const
{
    int x = RenderBoxModelObject::offsetLeft();
    if (firstLineBox())
        x += firstLineBox()->x();
    return x;
}

int RenderInline::offsetTop() const
{
    int y = RenderBoxModelObject::offsetTop();
    if (firstLineBox())
        y += firstLineBox()->y();
    return y;
}

static int computeMargin(const RenderInline* renderer, const Length& margin)
{
    if (margin.isAuto())
        return 0;
    if (margin.isFixed())
        return margin.value();
    if (margin.isPercent())
        return margin.calcMinValue(max(0, renderer->containingBlock()->availableLogicalWidth()));
    return 0;
}

int RenderInline::marginLeft() const
{
    if (!style()->isHorizontalWritingMode())
        return 0;
    return computeMargin(this, style()->marginLeft());
}

int RenderInline::marginRight() const
{
    if (!style()->isHorizontalWritingMode())
        return 0;
    return computeMargin(this, style()->marginRight());
}

int RenderInline::marginTop() const
{
    if (style()->isHorizontalWritingMode())
        return 0;
    return computeMargin(this, style()->marginTop());
}

int RenderInline::marginBottom() const
{
    if (style()->isHorizontalWritingMode())
        return 0;
    return computeMargin(this, style()->marginBottom());
}

int RenderInline::marginStart() const
{
    return computeMargin(this, style()->marginStart());
}

int RenderInline::marginEnd() const
{
    return computeMargin(this, style()->marginEnd());
}

const char* RenderInline::renderName() const
{
    if (isRelPositioned())
        return "RenderInline (relative positioned)";
    if (isAnonymous())
        return "RenderInline (generated)";
    if (isRunIn())
        return "RenderInline (run-in)";
    return "RenderInline";
}

bool RenderInline::nodeAtPoint(const HitTestRequest& request, HitTestResult& result,
                                int x, int y, int tx, int ty, HitTestAction hitTestAction)
{
    return m_lineBoxes.hitTest(this, request, result, x, y, tx, ty, hitTestAction);
}

VisiblePosition RenderInline::positionForPoint(const IntPoint& point)
{
    // FIXME: Does not deal with relative positioned inlines (should it?)
    RenderBlock* cb = containingBlock();
    if (firstLineBox()) {
        // This inline actually has a line box.  We must have clicked in the border/padding of one of these boxes.  We
        // should try to find a result by asking our containing block.
        return cb->positionForPoint(point);
    }

    // Translate the coords from the pre-anonymous block to the post-anonymous block.
    int parentBlockX = cb->x() + point.x();
    int parentBlockY = cb->y() + point.y();
    RenderBoxModelObject* c = continuation();
    while (c) {
        RenderBox* contBlock = c->isInline() ? c->containingBlock() : toRenderBlock(c);
        if (c->isInline() || c->firstChild())
            return c->positionForCoordinates(parentBlockX - contBlock->x(), parentBlockY - contBlock->y());
        c = toRenderBlock(c)->inlineElementContinuation();
    }
    
    return RenderBoxModelObject::positionForPoint(point);
}

IntRect RenderInline::linesBoundingBox() const
{
    IntRect result;
    
    // See <rdar://problem/5289721>, for an unknown reason the linked list here is sometimes inconsistent, first is non-zero and last is zero.  We have been
    // unable to reproduce this at all (and consequently unable to figure ot why this is happening).  The assert will hopefully catch the problem in debug
    // builds and help us someday figure out why.  We also put in a redundant check of lastLineBox() to avoid the crash for now.
    ASSERT(!firstLineBox() == !lastLineBox());  // Either both are null or both exist.
    if (firstLineBox() && lastLineBox()) {
        // Return the width of the minimal left side and the maximal right side.
        int leftSide = 0;
        int rightSide = 0;
        for (InlineFlowBox* curr = firstLineBox(); curr; curr = curr->nextLineBox()) {
            if (curr == firstLineBox() || curr->x() < leftSide)
                leftSide = curr->x();
            if (curr == firstLineBox() || curr->x() + curr->logicalWidth() > rightSide)
                rightSide = curr->x() + curr->logicalWidth();
        }
        result.setWidth(rightSide - leftSide);
        result.setX(leftSide);
        result.setHeight(lastLineBox()->y() + lastLineBox()->logicalHeight() - firstLineBox()->y());
        result.setY(firstLineBox()->y());
    }

    return result;
}

IntRect RenderInline::linesVisibleOverflowBoundingBox() const
{
    if (!firstLineBox() || !lastLineBox())
        return IntRect();

    // Return the width of the minimal left side and the maximal right side.
    int leftSide = numeric_limits<int>::max();
    int rightSide = numeric_limits<int>::min();
    for (InlineFlowBox* curr = firstLineBox(); curr; curr = curr->nextLineBox()) {
        leftSide = min(leftSide, curr->leftVisibleOverflow());
        rightSide = max(rightSide, curr->rightVisibleOverflow());
    }

    return IntRect(leftSide, firstLineBox()->topVisibleOverflow(), rightSide - leftSide,
        lastLineBox()->bottomVisibleOverflow() - firstLineBox()->topVisibleOverflow());
}

IntRect RenderInline::clippedOverflowRectForRepaint(RenderBoxModelObject* repaintContainer)
{
    // Only run-ins are allowed in here during layout.
    ASSERT(!view() || !view()->layoutStateEnabled() || isRunIn());

    if (!firstLineBox() && !continuation())
        return IntRect();

    // Find our leftmost position.
    IntRect boundingBox(linesVisibleOverflowBoundingBox());
    int left = boundingBox.x();
    int top = boundingBox.y();

    // Now invalidate a rectangle.
    int ow = style() ? style()->outlineSize() : 0;
    
    // We need to add in the relative position offsets of any inlines (including us) up to our
    // containing block.
    RenderBlock* cb = containingBlock();
    for (RenderObject* inlineFlow = this; inlineFlow && inlineFlow->isRenderInline() && inlineFlow != cb; 
         inlineFlow = inlineFlow->parent()) {
         if (inlineFlow->style()->position() == RelativePosition && inlineFlow->hasLayer())
            toRenderInline(inlineFlow)->layer()->relativePositionOffset(left, top);
    }

    IntRect r(-ow + left, -ow + top, boundingBox.width() + ow * 2, boundingBox.height() + ow * 2);
    if (cb->hasColumns())
        cb->adjustRectForColumns(r);

    if (cb->hasOverflowClip()) {
        // cb->height() is inaccurate if we're in the middle of a layout of |cb|, so use the
        // layer's size instead.  Even if the layer's size is wrong, the layer itself will repaint
        // anyway if its size does change.
        IntRect repaintRect(r);
        repaintRect.move(-cb->layer()->scrolledContentOffset()); // For overflow:auto/scroll/hidden.

        IntRect boxRect(0, 0, cb->layer()->width(), cb->layer()->height());
        r = intersection(repaintRect, boxRect);
    }
    
    // FIXME: need to ensure that we compute the correct repaint rect when the repaint container
    // is an inline.
    if (repaintContainer != this)
        cb->computeRectForRepaint(repaintContainer, r);

    if (ow) {
        for (RenderObject* curr = firstChild(); curr; curr = curr->nextSibling()) {
            if (!curr->isText()) {
                IntRect childRect = curr->rectWithOutlineForRepaint(repaintContainer, ow);
                r.unite(childRect);
            }
        }

        if (continuation() && !continuation()->isInline()) {
            IntRect contRect = continuation()->rectWithOutlineForRepaint(repaintContainer, ow);
            r.unite(contRect);
        }
    }

    return r;
}

IntRect RenderInline::rectWithOutlineForRepaint(RenderBoxModelObject* repaintContainer, int outlineWidth)
{
    IntRect r(RenderBoxModelObject::rectWithOutlineForRepaint(repaintContainer, outlineWidth));
    for (RenderObject* curr = firstChild(); curr; curr = curr->nextSibling()) {
        if (!curr->isText())
            r.unite(curr->rectWithOutlineForRepaint(repaintContainer, outlineWidth));
    }
    return r;
}

void RenderInline::computeRectForRepaint(RenderBoxModelObject* repaintContainer, IntRect& rect, bool fixed)
{
    if (RenderView* v = view()) {
        // LayoutState is only valid for root-relative repainting
        if (v->layoutStateEnabled() && !repaintContainer) {
            LayoutState* layoutState = v->layoutState();
            if (style()->position() == RelativePosition && layer())
                rect.move(layer()->relativePositionOffset());
            rect.move(layoutState->m_paintOffset);
            if (layoutState->m_clipped)
                rect.intersect(layoutState->m_clipRect);
            return;
        }
    }

    if (repaintContainer == this)
        return;

    bool containerSkipped;
    RenderObject* o = container(repaintContainer, &containerSkipped);
    if (!o)
        return;

    IntPoint topLeft = rect.location();

    if (o->isBlockFlow() && style()->position() != AbsolutePosition && style()->position() != FixedPosition) {
        RenderBlock* cb = toRenderBlock(o);
        if (cb->hasColumns()) {
            IntRect repaintRect(topLeft, rect.size());
            cb->adjustRectForColumns(repaintRect);
            topLeft = repaintRect.location();
            rect = repaintRect;
        }
    }

    if (style()->position() == RelativePosition && layer()) {
        // Apply the relative position offset when invalidating a rectangle.  The layer
        // is translated, but the render box isn't, so we need to do this to get the
        // right dirty rect.  Since this is called from RenderObject::setStyle, the relative position
        // flag on the RenderObject has been cleared, so use the one on the style().
        topLeft += layer()->relativePositionOffset();
    }
    
    // FIXME: We ignore the lightweight clipping rect that controls use, since if |o| is in mid-layout,
    // its controlClipRect will be wrong. For overflow clip we use the values cached by the layer.
    if (o->hasOverflowClip()) {
        RenderBox* containerBox = toRenderBox(o);

        // o->height() is inaccurate if we're in the middle of a layout of |o|, so use the
        // layer's size instead.  Even if the layer's size is wrong, the layer itself will repaint
        // anyway if its size does change.
        topLeft -= containerBox->layer()->scrolledContentOffset(); // For overflow:auto/scroll/hidden.

        IntRect repaintRect(topLeft, rect.size());
        IntRect boxRect(0, 0, containerBox->layer()->width(), containerBox->layer()->height());
        rect = intersection(repaintRect, boxRect);
        if (rect.isEmpty())
            return;
    } else
        rect.setLocation(topLeft);

    if (containerSkipped) {
        // If the repaintContainer is below o, then we need to map the rect into repaintContainer's coordinates.
        IntSize containerOffset = repaintContainer->offsetFromAncestorContainer(o);
        rect.move(-containerOffset);
        return;
    }
    
    o->computeRectForRepaint(repaintContainer, rect, fixed);
}

IntSize RenderInline::offsetFromContainer(RenderObject* container, const IntPoint& point) const
{
    ASSERT(container == this->container());

    IntSize offset;    
    if (isRelPositioned())
        offset += relativePositionOffset();

    container->adjustForColumns(offset, point);

    if (container->hasOverflowClip())
        offset -= toRenderBox(container)->layer()->scrolledContentOffset();

    return offset;
}

void RenderInline::mapLocalToContainer(RenderBoxModelObject* repaintContainer, bool fixed, bool useTransforms, TransformState& transformState) const
{
    if (repaintContainer == this)
        return;

    if (RenderView *v = view()) {
        if (v->layoutStateEnabled() && !repaintContainer) {
            LayoutState* layoutState = v->layoutState();
            IntSize offset = layoutState->m_paintOffset;
            if (style()->position() == RelativePosition && layer())
                offset += layer()->relativePositionOffset();
            transformState.move(offset);
            return;
        }
    }

    bool containerSkipped;
    RenderObject* o = container(repaintContainer, &containerSkipped);
    if (!o)
        return;

    IntSize containerOffset = offsetFromContainer(o, roundedIntPoint(transformState.mappedPoint()));

    bool preserve3D = useTransforms && (o->style()->preserves3D() || style()->preserves3D());
    if (useTransforms && shouldUseTransformFromContainer(o)) {
        TransformationMatrix t;
        getTransformFromContainer(o, containerOffset, t);
        transformState.applyTransform(t, preserve3D ? TransformState::AccumulateTransform : TransformState::FlattenTransform);
    } else
        transformState.move(containerOffset.width(), containerOffset.height(), preserve3D ? TransformState::AccumulateTransform : TransformState::FlattenTransform);

    if (containerSkipped) {
        // There can't be a transform between repaintContainer and o, because transforms create containers, so it should be safe
        // to just subtract the delta between the repaintContainer and o.
        IntSize containerOffset = repaintContainer->offsetFromAncestorContainer(o);
        transformState.move(-containerOffset.width(), -containerOffset.height(), preserve3D ? TransformState::AccumulateTransform : TransformState::FlattenTransform);
        return;
    }

    o->mapLocalToContainer(repaintContainer, fixed, useTransforms, transformState);
}

void RenderInline::mapAbsoluteToLocalPoint(bool fixed, bool useTransforms, TransformState& transformState) const
{
    // We don't expect this function to be called during layout.
    ASSERT(!view() || !view()->layoutStateEnabled());

    RenderObject* o = container();
    if (!o)
        return;

    o->mapAbsoluteToLocalPoint(fixed, useTransforms, transformState);

    IntSize containerOffset = offsetFromContainer(o, IntPoint());

    bool preserve3D = useTransforms && (o->style()->preserves3D() || style()->preserves3D());
    if (useTransforms && shouldUseTransformFromContainer(o)) {
        TransformationMatrix t;
        getTransformFromContainer(o, containerOffset, t);
        transformState.applyTransform(t, preserve3D ? TransformState::AccumulateTransform : TransformState::FlattenTransform);
    } else
        transformState.move(-containerOffset.width(), -containerOffset.height(), preserve3D ? TransformState::AccumulateTransform : TransformState::FlattenTransform);
}

void RenderInline::updateDragState(bool dragOn)
{
    RenderBoxModelObject::updateDragState(dragOn);
    if (continuation())
        continuation()->updateDragState(dragOn);
}

void RenderInline::childBecameNonInline(RenderObject* child)
{
    // We have to split the parent flow.
    RenderBlock* newBox = containingBlock()->createAnonymousBlock();
    RenderBoxModelObject* oldContinuation = continuation();
    setContinuation(newBox);
    RenderObject* beforeChild = child->nextSibling();
    children()->removeChildNode(this, child);
    splitFlow(beforeChild, newBox, child, oldContinuation);
}

void RenderInline::updateHitTestResult(HitTestResult& result, const IntPoint& point)
{
    if (result.innerNode())
        return;

    Node* n = node();
    IntPoint localPoint(point);
    if (n) {
        if (isInlineElementContinuation()) {
            // We're in the continuation of a split inline.  Adjust our local point to be in the coordinate space
            // of the principal renderer's containing block.  This will end up being the innerNonSharedNode.
            RenderBlock* firstBlock = n->renderer()->containingBlock();
            
            // Get our containing block.
            RenderBox* block = containingBlock();
            localPoint.move(block->x() - firstBlock->x(), block->y() - firstBlock->y());
        }

        result.setInnerNode(n);
        if (!result.innerNonSharedNode())
            result.setInnerNonSharedNode(n);
        result.setLocalPoint(localPoint);
    }
}

void RenderInline::dirtyLineBoxes(bool fullLayout)
{
    if (fullLayout)
        m_lineBoxes.deleteLineBoxes(renderArena());
    else
        m_lineBoxes.dirtyLineBoxes();
}

InlineFlowBox* RenderInline::createInlineFlowBox() 
{
    return new (renderArena()) InlineFlowBox(this);
}

InlineFlowBox* RenderInline::createAndAppendInlineFlowBox()
{
    InlineFlowBox* flowBox = createInlineFlowBox();
    m_lineBoxes.appendLineBox(flowBox);
    return flowBox;
}

int RenderInline::lineHeight(bool firstLine, bool /*isRootLineBox*/) const
{
    if (firstLine && document()->usesFirstLineRules()) {
        RenderStyle* s = style(firstLine);
        if (s != style())
            return s->computedLineHeight();
    }
    
    if (m_lineHeight == -1)
        m_lineHeight = style()->computedLineHeight();
    
    return m_lineHeight;
}

int RenderInline::verticalPositionFromCache(bool firstLine) const
{
    if (firstLine) // We're only really a first-line style if the document actually uses first-line rules.
        firstLine = document()->usesFirstLineRules();
    int vpos = m_verticalPosition;
    if (m_verticalPosition == PositionUndefined || firstLine) {
        vpos = verticalPosition(firstLine);
        if (!firstLine)
            m_verticalPosition = vpos;
    }
    return vpos;
}

IntSize RenderInline::relativePositionedInlineOffset(const RenderBox* child) const
{
    ASSERT(isRelPositioned());
    if (!isRelPositioned())
        return IntSize();

    // When we have an enclosing relpositioned inline, we need to add in the offset of the first line
    // box from the rest of the content, but only in the cases where we know we're positioned
    // relative to the inline itself.

    IntSize offset;
    int sx;
    int sy;
    if (firstLineBox()) {
        sx = firstLineBox()->x();
        sy = firstLineBox()->y();
    } else {
        sx = layer()->staticX();
        sy = layer()->staticY();
    }

    if (!child->style()->hasStaticX())
        offset.setWidth(sx);
    // This is not terribly intuitive, but we have to match other browsers.  Despite being a block display type inside
    // an inline, we still keep our x locked to the left of the relative positioned inline.  Arguably the correct
    // behavior would be to go flush left to the block that contains the inline, but that isn't what other browsers
    // do.
    else if (!child->style()->isOriginalDisplayInlineType())
        // Avoid adding in the left border/padding of the containing block twice.  Subtract it out.
        offset.setWidth(sx - (child->containingBlock()->borderLeft() + child->containingBlock()->paddingLeft()));

    if (!child->style()->hasStaticY())
        offset.setHeight(sy);

    return offset;
}

void RenderInline::imageChanged(WrappedImagePtr, const IntRect*)
{
    if (!parent())
        return;
        
    // FIXME: We can do better.
    repaint();
}

void RenderInline::addFocusRingRects(Vector<IntRect>& rects, int tx, int ty)
{
    for (InlineFlowBox* curr = firstLineBox(); curr; curr = curr->nextLineBox()) {
        RootInlineBox* root = curr->root();
        int top = max(root->lineTop(), curr->y());
        int bottom = min(root->lineBottom(), curr->y() + curr->logicalHeight());
        IntRect rect(tx + curr->x(), ty + top, curr->logicalWidth(), bottom - top);
        if (!rect.isEmpty())
            rects.append(rect);
    }

    for (RenderObject* curr = firstChild(); curr; curr = curr->nextSibling()) {
        if (!curr->isText() && !curr->isListMarker()) {
            FloatPoint pos(tx, ty);
            // FIXME: This doesn't work correctly with transforms.
            if (curr->hasLayer()) 
                pos = curr->localToAbsolute();
            else if (curr->isBox())
                pos.move(toRenderBox(curr)->x(), toRenderBox(curr)->y());
           curr->addFocusRingRects(rects, pos.x(), pos.y());
        }
    }

    if (continuation()) {
        if (continuation()->isInline())
            continuation()->addFocusRingRects(rects, 
                                              tx - containingBlock()->x() + continuation()->containingBlock()->x(),
                                              ty - containingBlock()->y() + continuation()->containingBlock()->y());
        else
            continuation()->addFocusRingRects(rects, 
                                              tx - containingBlock()->x() + toRenderBox(continuation())->x(),
                                              ty - containingBlock()->y() + toRenderBox(continuation())->y());
    }
}

void RenderInline::paintOutline(GraphicsContext* graphicsContext, int tx, int ty)
{
    if (!hasOutline())
        return;
    
    RenderStyle* styleToUse = style();
    if (styleToUse->outlineStyleIsAuto() || hasOutlineAnnotation()) {
        int ow = styleToUse->outlineWidth();
        Color oc = styleToUse->visitedDependentColor(CSSPropertyOutlineColor);

        Vector<IntRect> focusRingRects;
        addFocusRingRects(focusRingRects, tx, ty);
        if (styleToUse->outlineStyleIsAuto())
            graphicsContext->drawFocusRing(focusRingRects, ow, styleToUse->outlineOffset(), oc);
        else
            addPDFURLRect(graphicsContext, unionRect(focusRingRects));
    }

    if (styleToUse->outlineStyleIsAuto() || styleToUse->outlineStyle() == BNONE)
        return;

    Vector<IntRect> rects;

    rects.append(IntRect());
    for (InlineFlowBox* curr = firstLineBox(); curr; curr = curr->nextLineBox()) {
        RootInlineBox* root = curr->root();
        int top = max(root->lineTop(), curr->y());
        int bottom = min(root->lineBottom(), curr->y() + curr->logicalHeight());
        rects.append(IntRect(curr->x(), top, curr->logicalWidth(), bottom - top));
    }
    rects.append(IntRect());

    for (unsigned i = 1; i < rects.size() - 1; i++)
        paintOutlineForLine(graphicsContext, tx, ty, rects.at(i - 1), rects.at(i), rects.at(i + 1));
}

void RenderInline::paintOutlineForLine(GraphicsContext* graphicsContext, int tx, int ty,
                                       const IntRect& lastline, const IntRect& thisline, const IntRect& nextline)
{
    RenderStyle* styleToUse = style();
    int ow = styleToUse->outlineWidth();
    EBorderStyle os = styleToUse->outlineStyle();
    Color oc = styleToUse->visitedDependentColor(CSSPropertyOutlineColor);

    int offset = style()->outlineOffset();

    int t = ty + thisline.y() - offset;
    int l = tx + thisline.x() - offset;
    int b = ty + thisline.bottom() + offset;
    int r = tx + thisline.right() + offset;
    
    // left edge
    drawLineForBoxSide(graphicsContext,
               l - ow,
               t - (lastline.isEmpty() || thisline.x() < lastline.x() || (lastline.right() - 1) <= thisline.x() ? ow : 0),
               l,
               b + (nextline.isEmpty() || thisline.x() <= nextline.x() || (nextline.right() - 1) <= thisline.x() ? ow : 0),
               BSLeft,
               oc, os,
               (lastline.isEmpty() || thisline.x() < lastline.x() || (lastline.right() - 1) <= thisline.x() ? ow : -ow),
               (nextline.isEmpty() || thisline.x() <= nextline.x() || (nextline.right() - 1) <= thisline.x() ? ow : -ow));
    
    // right edge
    drawLineForBoxSide(graphicsContext,
               r,
               t - (lastline.isEmpty() || lastline.right() < thisline.right() || (thisline.right() - 1) <= lastline.x() ? ow : 0),
               r + ow,
               b + (nextline.isEmpty() || nextline.right() <= thisline.right() || (thisline.right() - 1) <= nextline.x() ? ow : 0),
               BSRight,
               oc, os,
               (lastline.isEmpty() || lastline.right() < thisline.right() || (thisline.right() - 1) <= lastline.x() ? ow : -ow),
               (nextline.isEmpty() || nextline.right() <= thisline.right() || (thisline.right() - 1) <= nextline.x() ? ow : -ow));
    // upper edge
    if (thisline.x() < lastline.x())
        drawLineForBoxSide(graphicsContext,
                   l - ow,
                   t - ow,
                   min(r+ow, (lastline.isEmpty() ? 1000000 : tx + lastline.x())),
                   t ,
                   BSTop, oc, os,
                   ow,
                   (!lastline.isEmpty() && tx + lastline.x() + 1 < r + ow) ? -ow : ow);
    
    if (lastline.right() < thisline.right())
        drawLineForBoxSide(graphicsContext,
                   max(lastline.isEmpty() ? -1000000 : tx + lastline.right(), l - ow),
                   t - ow,
                   r + ow,
                   t ,
                   BSTop, oc, os,
                   (!lastline.isEmpty() && l - ow < tx + lastline.right()) ? -ow : ow,
                   ow);
    
    // lower edge
    if (thisline.x() < nextline.x())
        drawLineForBoxSide(graphicsContext,
                   l - ow,
                   b,
                   min(r + ow, !nextline.isEmpty() ? tx + nextline.x() + 1 : 1000000),
                   b + ow,
                   BSBottom, oc, os,
                   ow,
                   (!nextline.isEmpty() && tx + nextline.x() + 1 < r + ow) ? -ow : ow);
    
    if (nextline.right() < thisline.right())
        drawLineForBoxSide(graphicsContext,
                   max(!nextline.isEmpty() ? tx + nextline.right() : -1000000, l - ow),
                   b,
                   r + ow,
                   b + ow,
                   BSBottom, oc, os,
                   (!nextline.isEmpty() && l - ow < tx + nextline.right()) ? -ow : ow,
                   ow);
}

#if ENABLE(DASHBOARD_SUPPORT)
void RenderInline::addDashboardRegions(Vector<DashboardRegionValue>& regions)
{
    // Convert the style regions to absolute coordinates.
    if (style()->visibility() != VISIBLE)
        return;

    const Vector<StyleDashboardRegion>& styleRegions = style()->dashboardRegions();
    unsigned i, count = styleRegions.size();
    for (i = 0; i < count; i++) {
        StyleDashboardRegion styleRegion = styleRegions[i];

        IntRect linesBoundingBox = this->linesBoundingBox();
        int w = linesBoundingBox.width();
        int h = linesBoundingBox.height();

        DashboardRegionValue region;
        region.label = styleRegion.label;
        region.bounds = IntRect(linesBoundingBox.x() + styleRegion.offset.left().value(),
                                linesBoundingBox.y() + styleRegion.offset.top().value(),
                                w - styleRegion.offset.left().value() - styleRegion.offset.right().value(),
                                h - styleRegion.offset.top().value() - styleRegion.offset.bottom().value());
        region.type = styleRegion.type;

        RenderObject* container = containingBlock();
        if (!container)
            container = this;

        region.clip = region.bounds;
        container->computeAbsoluteRepaintRect(region.clip);
        if (region.clip.height() < 0) {
            region.clip.setHeight(0);
            region.clip.setWidth(0);
        }

        FloatPoint absPos = container->localToAbsolute();
        region.bounds.setX(absPos.x() + region.bounds.x());
        region.bounds.setY(absPos.y() + region.bounds.y());

        if (frame()) {
            float pageScaleFactor = frame()->page()->chrome()->scaleFactor();
            if (pageScaleFactor != 1.0f) {
                region.bounds.scale(pageScaleFactor);
                region.clip.scale(pageScaleFactor);
            }
        }

        regions.append(region);
    }
}
#endif

} // namespace WebCore
