/*
 * Copyright (C) 2003 Apple Computer, Inc.
 *
 * Portions are Copyright (C) 1998 Netscape Communications Corporation.
 *
 * Other contributors:
 *   Robert O'Callahan <roc+@cs.cmu.edu>
 *   David Baron <dbaron@fas.harvard.edu>
 *   Christian Biesinger <cbiesinger@web.de>
 *   Randall Jesup <rjesup@wgate.com>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *   Josh Soref <timeless@mac.com>
 *   Boris Zbarsky <bzbarsky@mit.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 */

#include "render_layer.h"
#include <kdebug.h>
#include <assert.h>
#include "khtmlview.h"
#include "render_block.h"
#include "render_arena.h"
#include "xml/dom_docimpl.h"

#include <qscrollbar.h>
#include <qptrvector.h>

using namespace DOM;
using namespace khtml;

#ifdef APPLE_CHANGES
QScrollBar* RenderLayer::gScrollBar = 0;
#endif

#ifndef NDEBUG
static bool inRenderLayerDetach;
#endif

void
RenderScrollMediator::slotValueChanged(int val)
{
    m_layer->updateScrollPositionFromScrollbars();
}

RenderLayer::RenderLayer(RenderObject* object)
: m_object( object ),
m_parent( 0 ),
m_previous( 0 ),
m_next( 0 ),
m_first( 0 ),
m_last( 0 ),
m_x( 0 ),
m_y( 0 ),
m_width( 0 ),
m_height( 0 ),
m_scrollX( 0 ),
m_scrollY( 0 ),
m_scrollWidth( 0 ),
m_scrollHeight( 0 ),
m_hBar( 0 ),
m_vBar( 0 ),
m_scrollMediator( 0 ),
m_posZOrderList( 0 ),
m_negZOrderList( 0 ),
m_zOrderListsDirty( true )
{
}

RenderLayer::~RenderLayer()
{
    // Child layers will be deleted by their corresponding render objects, so
    // our destructor doesn't have to do anything.
    m_parent = m_previous = m_next = m_first = m_last = 0;
    delete m_hBar;
    delete m_vBar;
    delete m_scrollMediator;
    delete m_posZOrderList;
    delete m_negZOrderList;
}

void RenderLayer::updateLayerPosition()
{
    // The canvas is sized to the docWidth/Height over in RenderCanvas::layout, so we
    // don't need to ever update our layer position here.
    if (renderer()->isCanvas())
        return;
    
    int x = m_object->xPos();
    int y = m_object->yPos();

    if (!m_object->isPositioned()) {
        // We must adjust our position by walking up the render tree looking for the
        // nearest enclosing object with a layer.
        RenderObject* curr = m_object->parent();
        while (curr && !curr->layer()) {
            x += curr->xPos();
            y += curr->yPos();
            curr = curr->parent();
        }
    }

    if (m_object->isRelPositioned())
        static_cast<RenderBox*>(m_object)->relativePositionOffset(x, y);

    // Subtract our parent's scroll offset.
    if (parent())
        parent()->subtractScrollOffset(x, y);
    
    setPos(x,y);

    setWidth(m_object->width());
    setHeight(m_object->height());

    if (!m_object->style()->hidesOverflow()) {
        if (m_object->overflowWidth() > m_object->width())
            setWidth(m_object->overflowWidth());
        if (m_object->overflowHeight() > m_object->height())
            setHeight(m_object->overflowHeight());
    }    
}

RenderLayer *RenderLayer::stackingContext() const
{
    RenderLayer* curr = parent();
    for ( ; curr && !curr->m_object->isCanvas() && !curr->m_object->isRoot() &&
          curr->m_object->style()->hasAutoZIndex();
          curr = curr->parent());
    return curr;
}

RenderLayer*
RenderLayer::enclosingPositionedAncestor() const
{
    RenderLayer* curr = parent();
    for ( ; curr && !curr->m_object->isCanvas() && !curr->m_object->isRoot() &&
         !curr->m_object->isPositioned() && !curr->m_object->isRelPositioned();
         curr = curr->parent());
         
    return curr;
}

#if APPLE_CHANGES
bool
RenderLayer::isTransparent()
{
    return m_object->style()->opacity() < 1.0f;
}

RenderLayer*
RenderLayer::transparentAncestor()
{
    RenderLayer* curr = parent();
    for ( ; curr && curr->m_object->style()->opacity() == 1.0f; curr = curr->parent());
    return curr;
}
#endif

void* RenderLayer::operator new(size_t sz, RenderArena* renderArena) throw()
{
    return renderArena->allocate(sz);
}

void RenderLayer::operator delete(void* ptr, size_t sz)
{
    assert(inRenderLayerDetach);
    
    // Stash size where detach can find it.
    *(size_t *)ptr = sz;
}

void RenderLayer::detach(RenderArena* renderArena)
{
#ifndef NDEBUG
    inRenderLayerDetach = true;
#endif
    delete this;
#ifndef NDEBUG
    inRenderLayerDetach = false;
#endif
    
    // Recover the size left there for us by operator delete and free the memory.
    renderArena->free(*(size_t *)this, this);
}

void RenderLayer::addChild(RenderLayer *child, RenderLayer* beforeChild)
{
    RenderLayer* prevSibling = beforeChild ? beforeChild->previousSibling() : lastChild();
    if (prevSibling) {
        child->setPreviousSibling(prevSibling);
        prevSibling->setNextSibling(child);
    }
    else
        setFirstChild(child);

    if (beforeChild) {
        beforeChild->setPreviousSibling(child);
        child->setNextSibling(beforeChild);
    }
    else
        setLastChild(child);
   
    child->setParent(this);

    // Dirty the z-order list in which we are contained.
    child->stackingContext()->dirtyZOrderLists();
}

RenderLayer* RenderLayer::removeChild(RenderLayer* oldChild)
{
    // remove the child
    if (oldChild->previousSibling())
        oldChild->previousSibling()->setNextSibling(oldChild->nextSibling());
    if (oldChild->nextSibling())
        oldChild->nextSibling()->setPreviousSibling(oldChild->previousSibling());

    if (m_first == oldChild)
        m_first = oldChild->nextSibling();
    if (m_last == oldChild)
        m_last = oldChild->previousSibling();

    // Dirty the z-order list in which we are contained.  When called via the
    // reattachment process in removeOnlyThisLayer, the layer may already be disconnected
    // from the main layer tree, so we need to null-check the |stackingContext| value.
    RenderLayer* stackingContext = oldChild->stackingContext();
    if (stackingContext)
        oldChild->stackingContext()->dirtyZOrderLists();
    
    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);
    oldChild->setParent(0);
    
    return oldChild;
}

void RenderLayer::removeOnlyThisLayer()
{
    if (!m_parent)
        return;
    
    // Remove us from the parent.
    RenderLayer* parent = m_parent;
    RenderLayer* nextSib = nextSibling();
    parent->removeChild(this);
    
    // Now walk our kids and reattach them to our parent.
    RenderLayer* current = m_first;
    while (current) {
        RenderLayer* next = current->nextSibling();
        removeChild(current);
        parent->addChild(current, nextSib);
        current = next;
    }
    
    detach(renderer()->renderArena());
}

void RenderLayer::insertOnlyThisLayer()
{
    if (!m_parent && renderer()->parent()) {
        // We need to connect ourselves when our renderer() has a parent.
        // Find our enclosingLayer and add ourselves.
        RenderLayer* parentLayer = renderer()->parent()->enclosingLayer();
        if (parentLayer)
            parentLayer->addChild(this, 
                                  renderer()->parent()->findNextLayer(parentLayer, renderer()));
    }
    
    // Remove all descendant layers from the hierarchy and add them to the new position.
    for (RenderObject* curr = renderer()->firstChild(); curr; curr = curr->nextSibling())
        curr->moveLayers(m_parent, this);
}

void 
RenderLayer::convertToLayerCoords(const RenderLayer* ancestorLayer, int& x, int& y) const
{
    if (ancestorLayer == this)
        return;
        
    if (m_object->style()->position() == FIXED) {
        // Add in the offset of the view.  We can obtain this by calling
        // absolutePosition() on the RenderCanvas.
        int xOff, yOff;
        m_object->absolutePosition(xOff, yOff, true);
        x += xOff;
        y += yOff;
        return;
    }
 
    RenderLayer* parentLayer;
    if (m_object->style()->position() == ABSOLUTE)
        parentLayer = enclosingPositionedAncestor();
    else
        parentLayer = parent();
    
    if (!parentLayer) return;
    
    parentLayer->convertToLayerCoords(ancestorLayer, x, y);

    x += xPos();
    y += yPos();
}

void
RenderLayer::scrollOffset(int& x, int& y)
{
    x += scrollXOffset();
    y += scrollYOffset();
}

void
RenderLayer::subtractScrollOffset(int& x, int& y)
{
    x -= scrollXOffset();
    y -= scrollYOffset();
}

void
RenderLayer::scrollToOffset(int x, int y, bool updateScrollbars)
{
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    int maxX = m_scrollWidth - m_object->clientWidth();
    int maxY = m_scrollHeight - m_object->clientHeight();
    if (x > maxX) x = maxX;
    if (y > maxY) y = maxY;

    // FIXME: Eventually, we will want to perform a blit.  For now never
    // blit, since the check for blitting is going to be very
    // complicated (since it will involve testing whether our layer
    // is either occluded by another layer or clipped by an enclosing
    // layer or contains fixed backgrounds, etc.).
    m_scrollX = x;
    m_scrollY = y;

    // FIXME: Fire the onscroll DOM event.
    
    // Just schedule a full repaint of our object.
    m_object->repaint(true);
    
    if (updateScrollbars) {
        if (m_hBar)
            m_hBar->setValue(m_scrollX);
        if (m_vBar)
            m_vBar->setValue(m_scrollY);
    }
}

void
RenderLayer::updateScrollPositionFromScrollbars()
{
    bool needUpdate = false;
    int newX = m_scrollX;
    int newY = m_scrollY;
    
    if (m_hBar) {
        newX = m_hBar->value();
        if (newX != m_scrollX)
           needUpdate = true;
    }

    if (m_vBar) {
        newY = m_vBar->value();
        if (newY != m_scrollY)
           needUpdate = true;
    }

    if (needUpdate)
        scrollToOffset(newX, newY, false);
}

void
RenderLayer::setHasHorizontalScrollbar(bool hasScrollbar)
{
    if (hasScrollbar && !m_hBar) {
        QScrollView* scrollView = m_object->element()->getDocument()->view();
        m_hBar = new QScrollBar(Qt::Horizontal, scrollView);
        scrollView->addChild(m_hBar, 0, -50000);
        if (!m_scrollMediator)
            m_scrollMediator = new RenderScrollMediator(this);
        m_scrollMediator->connect(m_hBar, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)));
    }
    else if (!hasScrollbar && m_hBar) {
        m_scrollMediator->disconnect(m_hBar, SIGNAL(valueChanged(int)),
                                     m_scrollMediator, SLOT(slotValueChanged(int)));
        delete m_hBar;
        m_hBar = 0;
    }
}

void
RenderLayer::setHasVerticalScrollbar(bool hasScrollbar)
{
    if (hasScrollbar && !m_vBar) {
        QScrollView* scrollView = m_object->element()->getDocument()->view();
        m_vBar = new QScrollBar(Qt::Vertical, scrollView);
        scrollView->addChild(m_vBar, 0, -50000);
        if (!m_scrollMediator)
            m_scrollMediator = new RenderScrollMediator(this);
        m_scrollMediator->connect(m_vBar, SIGNAL(valueChanged(int)), SLOT(slotValueChanged(int)));
    }
    else if (!hasScrollbar && m_vBar) {
        m_scrollMediator->disconnect(m_vBar, SIGNAL(valueChanged(int)),
                                     m_scrollMediator, SLOT(slotValueChanged(int)));
        delete m_vBar;
        m_vBar = 0;
    }
}

int
RenderLayer::verticalScrollbarWidth()
{
    if (!m_vBar)
        return 0;

    return m_vBar->width();
}

int
RenderLayer::horizontalScrollbarHeight()
{
    if (!m_hBar)
        return 0;

    return m_hBar->height();
}

void
RenderLayer::moveScrollbarsAside()
{
    if (m_hBar)
        m_hBar->move(0, -50000);
    if (m_vBar)
        m_vBar->move(0, -50000);
}

void
RenderLayer::positionScrollbars(const QRect& absBounds)
{
    if (m_vBar) {
        m_vBar->move(absBounds.x()+absBounds.width()-m_object->borderRight()-m_vBar->width(),
                     absBounds.y()+m_object->borderTop());
        m_vBar->resize(m_vBar->width(), absBounds.height() -
                       (m_object->borderTop()+m_object->borderBottom()) -
                       (m_hBar ? m_hBar->height()-1 : 0));
    }

    if (m_hBar) {
        m_hBar->move(absBounds.x()+m_object->borderLeft(),
                     absBounds.y()+absBounds.height()-m_object->borderBottom()-m_hBar->height());
        m_hBar->resize(absBounds.width() - (m_object->borderLeft()+m_object->borderRight()) -
                       (m_vBar ? m_vBar->width()-1 : 0),
                       m_hBar->height());
    }
}

#define LINE_STEP   10
#define PAGE_KEEP   40

void
RenderLayer::checkScrollbarsAfterLayout()
{
    updateLayerPosition();
    
    int rightPos = m_object->rightmostPosition();
    int bottomPos = m_object->lowestPosition();

    int clientWidth = m_object->clientWidth();
    int clientHeight = m_object->clientHeight();
    m_scrollWidth = clientWidth;
    m_scrollHeight = clientHeight;
    
    if (rightPos - m_object->borderLeft() > m_scrollWidth)
        m_scrollWidth = rightPos - m_object->borderLeft();
    if (bottomPos - m_object->borderTop() > m_scrollHeight)
        m_scrollHeight = bottomPos - m_object->borderTop();
    
    bool needHorizontalBar = rightPos > width();
    bool needVerticalBar = bottomPos > height();

    bool haveHorizontalBar = m_hBar;
    bool haveVerticalBar = m_vBar;

    // overflow:scroll should just enable/disable.
    if (m_object->style()->overflow() == OSCROLL) {
        m_hBar->setEnabled(needHorizontalBar);
        m_vBar->setEnabled(needVerticalBar);
    }

    // overflow:auto may need to lay out again if scrollbars got added/removed.
    bool scrollbarsChanged = (m_object->style()->overflow() == OAUTO) &&
        (haveHorizontalBar != needHorizontalBar || haveVerticalBar != needVerticalBar);    
    if (scrollbarsChanged) {
        setHasHorizontalScrollbar(needHorizontalBar);
        setHasVerticalScrollbar(needVerticalBar);
       
        m_object->setNeedsLayout(true);
	if (m_object->isRenderBlock())
            static_cast<RenderBlock*>(m_object)->layoutBlock(true);
        else
            m_object->layout();
	return;
    }

    // Set up the range (and page step/line step).
    if (m_hBar) {
        int pageStep = (clientWidth-PAGE_KEEP);
        if (pageStep < 0) pageStep = clientWidth;
        m_hBar->setSteps(LINE_STEP, pageStep);
#ifdef APPLE_CHANGES
        m_hBar->setKnobProportion(clientWidth, m_scrollWidth);
#else
        m_hBar->setRange(0, m_scrollWidth-clientWidth);
#endif
    }
    if (m_vBar) {
        int pageStep = (clientHeight-PAGE_KEEP);
        if (pageStep < 0) pageStep = clientHeight;
        m_vBar->setSteps(LINE_STEP, pageStep);
#ifdef APPLE_CHANGES
        m_vBar->setKnobProportion(clientHeight, m_scrollHeight);
#else
        m_vBar->setRange(0, m_scrollHeight-clientHeight);
#endif
    }
}

#if APPLE_CHANGES
void
RenderLayer::paintScrollbars(QPainter* p, const QRect& damageRect)
{
    if (m_hBar)
        m_hBar->paint(p, damageRect);
    if (m_vBar)
        m_vBar->paint(p, damageRect);
}
#endif

void
RenderLayer::paint(QPainter *p, const QRect& damageRect, bool selectionOnly)
{
    paintLayer(this, p, damageRect, selectionOnly);
}

static void setClip(QPainter* p, const QRect& paintDirtyRect, const QRect& clipRect)
{
    if (paintDirtyRect == clipRect)
        return;

    p->save();
    
#if APPLE_CHANGES
    p->addClip(clipRect);
#else
    QRect clippedRect = p->xForm(clipRect);
    QRegion creg(clippedRect);
    QRegion old = p->clipRegion();
    if (!old.isNull())
        creg = old.intersect(creg);
    p->setClipRegion(creg);
#endif
    
}

static void restoreClip(QPainter* p, const QRect& paintDirtyRect, const QRect& clipRect)
{
    if (paintDirtyRect == clipRect)
        return;
    p->restore();
}

void
RenderLayer::paintLayer(RenderLayer* rootLayer, QPainter *p,
                        const QRect& paintDirtyRect, bool selectionOnly)
{
    // Calculate the clip rects we should use.
    QRect layerBounds, damageRect, clipRectToApply;
    calculateRects(rootLayer, paintDirtyRect, layerBounds, damageRect, clipRectToApply);
    int x = layerBounds.x();
    int y = layerBounds.y();
                             
    // Ensure our z-order lists are up-to-date.
    updateZOrderLists();

#if APPLE_CHANGES
    // Set our transparency if we need to.
    if (isTransparent())
        p->beginTransparencyLayer(renderer()->style()->opacity());
#endif
    
    // We want to paint our layer, but only if we intersect the damage rect.
    bool shouldPaint = intersectsDamageRect(layerBounds, damageRect);
    if (shouldPaint && !selectionOnly) {
        // Paint our background first, before painting any child layers.
        if (!damageRect.isEmpty()) {
            // Establish the clip used to paint our background.
            setClip(p, paintDirtyRect, damageRect);

            // Paint the background.
            renderer()->paint(p, damageRect.x(), damageRect.y(),
                              damageRect.width(), damageRect.height(),
                              x - renderer()->xPos(), y - renderer()->yPos(),
                              PaintActionElementBackground);

            // Position our scrollbars.
            positionScrollbars(layerBounds);

#if APPLE_CHANGES
            // Our scrollbar widgets paint exactly when we tell them to, so that they work properly with
            // z-index.  We paint after we painted the background/border, so that the scrollbars will
            // sit above the background/border.
            paintScrollbars(p, damageRect);
#endif
            // Restore the clip.
            restoreClip(p, paintDirtyRect, damageRect);
        }
    }

    // Now walk the sorted list of children with negative z-indices.
    if (m_negZOrderList) {
        uint count = m_negZOrderList->count();
        for (uint i = 0; i < count; i++) {
            RenderLayer* child = m_negZOrderList->at(i);
            child->paintLayer(rootLayer, p, paintDirtyRect, selectionOnly);
        }
    }
    
    // Now establish the appropriate clip and paint our child RenderObjects.
    if (shouldPaint && !clipRectToApply.isEmpty()) {
        // Set up the clip used when painting our children.
        setClip(p, paintDirtyRect, clipRectToApply);

        if (selectionOnly)
            renderer()->paint(p, clipRectToApply.x(), clipRectToApply.y(),
                              clipRectToApply.width(), clipRectToApply.height(),
                              x - renderer()->xPos(), y - renderer()->yPos(), PaintActionSelection);
        else {
            renderer()->paint(p, clipRectToApply.x(), clipRectToApply.y(),
                              clipRectToApply.width(), clipRectToApply.height(),
                              x - renderer()->xPos(), y - renderer()->yPos(), PaintActionChildBackgrounds);
            renderer()->paint(p, clipRectToApply.x(), clipRectToApply.y(),
                              clipRectToApply.width(), clipRectToApply.height(),
                              x - renderer()->xPos(), y - renderer()->yPos(), PaintActionFloat);
            renderer()->paint(p, clipRectToApply.x(), clipRectToApply.y(),
                              clipRectToApply.width(), clipRectToApply.height(),
                              x - renderer()->xPos(), y - renderer()->yPos(), PaintActionForeground);
        }

        // Now restore our clip.
        restoreClip(p, paintDirtyRect, clipRectToApply);
    }
    
    // Now walk the sorted list of children with positive z-indices.
    if (m_posZOrderList) {
        uint count = m_posZOrderList->count();
        for (uint i = 0; i < count; i++) {
            RenderLayer* child = m_posZOrderList->at(i);
            child->paintLayer(rootLayer, p, paintDirtyRect, selectionOnly);
        }
    }
    
#if APPLE_CHANGES
    // End our transparency layer
    if (isTransparent())
        p->endTransparencyLayer();
#endif
}

bool
RenderLayer::nodeAtPoint(RenderObject::NodeInfo& info, int x, int y)
{
#if APPLE_CHANGES
    // Clear our our scrollbar variable
    RenderLayer::gScrollBar = 0;
#endif
    
    QRect damageRect(m_x, m_y, width(), height());
    RenderLayer* insideLayer = nodeAtPointForLayer(this, info, x, y, damageRect);

    // Now determine if the result is inside an anchor.
    DOM::NodeImpl* node = info.innerNode();
    while (node) {
        if (node->hasAnchor())
            info.setURLElement(node);
        node = node->parentNode();
    }

    // Next set up the correct :hover/:active state along the new chain.
    updateHoverActiveState(info);

    // Now return whether we were inside this layer (this will always be true for the root
    // layer).
    return insideLayer;
}

RenderLayer*
RenderLayer::nodeAtPointForLayer(RenderLayer* rootLayer, RenderObject::NodeInfo& info,
                                 int xMousePos, int yMousePos, const QRect& hitTestRect)
{
    // Calculate the clip rects we should use.
    QRect layerBounds, bgRect, fgRect;
    calculateRects(rootLayer, hitTestRect, layerBounds, bgRect, fgRect);
    
    // Ensure our z-order lists are up-to-date.
    updateZOrderLists();

    // This variable tracks which layer the mouse ends up being inside.  The minute we find an insideLayer,
    // we are done and can return it.
    RenderLayer* insideLayer = 0;
    
    // Begin by walking our list of positive layers from highest z-index down to the lowest
    // z-index.
    if (m_posZOrderList) {
        uint count = m_posZOrderList->count();
        for (int i = count-1; i >= 0; i--) {
            RenderLayer* child = m_posZOrderList->at(i);
            insideLayer = child->nodeAtPointForLayer(rootLayer, info, xMousePos, yMousePos, hitTestRect);
            if (insideLayer)
                return insideLayer;
        }
    }

    // Next we want to see if the mouse pos is inside the child RenderObjects of the layer.
    if (containsPoint(xMousePos, yMousePos, fgRect) &&
        renderer()->nodeAtPoint(info, xMousePos, yMousePos,
                                layerBounds.x() - renderer()->xPos(),
                                layerBounds.y() - renderer()->yPos(),
                                HitTestChildrenOnly))
        return this;
        
    // Now check our negative z-index children.
    if (m_negZOrderList) {
        uint count = m_negZOrderList->count();
        for (int i = count-1; i >= 0; i--) {
            RenderLayer* child = m_negZOrderList->at(i);
            insideLayer = child->nodeAtPointForLayer(rootLayer, info, xMousePos, yMousePos, hitTestRect);
            if (insideLayer)
                return insideLayer;
        }
    }

    // Next we want to see if the mouse pos is inside this layer but not any of its children.
    if (containsPoint(xMousePos, yMousePos, bgRect) &&
        renderer()->nodeAtPoint(info, xMousePos, yMousePos,
                                layerBounds.x() - renderer()->xPos(),
                                layerBounds.y() - renderer()->yPos(),
                                HitTestSelfOnly))
        return this;

    // No luck.
    return 0;
}

void RenderLayer::calculateClipRects(const RenderLayer* rootLayer, QRect& overflowClipRect,
                                     QRect& posClipRect, QRect& fixedClipRect)
{
    if (parent())
        parent()->calculateClipRects(rootLayer, overflowClipRect, posClipRect, fixedClipRect);
        
    updateLayerPosition(); // For relpositioned layers or non-positioned layers,
                           // we need to keep in sync, since we may have shifted relative
                           // to our parent layer.

    // A fixed object is essentially the root of its containing block hierarchy, so when
    // we encounter such an object, we reset our clip rects to the fixedClipRect.
    if (m_object->style()->position() == FIXED) {
        posClipRect = fixedClipRect;
        overflowClipRect = fixedClipRect;
    }
    else if (m_object->style()->position() == RELATIVE)
        posClipRect = overflowClipRect;
    
    // Update the clip rects that will be passed to child layers.
    if (m_object->hasOverflowClip() || m_object->hasClip()) {
        // This layer establishes a clip of some kind.
        int x = 0;
        int y = 0;
        convertToLayerCoords(rootLayer, x, y);
        
        if (m_object->hasOverflowClip()) {
            QRect newOverflowClip = m_object->getOverflowClipRect(x,y);
            overflowClipRect  = newOverflowClip.intersect(overflowClipRect);
            if (m_object->isPositioned() || m_object->isRelPositioned())
                posClipRect = newOverflowClip.intersect(posClipRect);
        }
        if (m_object->hasClip()) {
            QRect newPosClip = m_object->getClipRect(x,y);
            posClipRect = posClipRect.intersect(newPosClip);
            overflowClipRect = overflowClipRect.intersect(newPosClip);
            fixedClipRect = fixedClipRect.intersect(newPosClip);
        }
    }
}

void RenderLayer::calculateRects(const RenderLayer* rootLayer, const QRect& paintDirtyRect, QRect& layerBounds,
                                 QRect& backgroundRect, QRect& foregroundRect)
{
    QRect overflowClipRect = paintDirtyRect;
    QRect posClipRect = paintDirtyRect;
    QRect fixedClipRect = paintDirtyRect;
    if (parent())
        parent()->calculateClipRects(rootLayer, overflowClipRect, posClipRect, fixedClipRect);

    updateLayerPosition();
    
    int x = 0;
    int y = 0;
    convertToLayerCoords(rootLayer, x, y);
    layerBounds = QRect(x,y,width(),height());

    backgroundRect = m_object->style()->position() == FIXED ? fixedClipRect :
        (m_object->isPositioned() ? posClipRect : overflowClipRect);
    foregroundRect = backgroundRect;
    
    // Update the clip rects that will be passed to child layers.
    if (m_object->hasOverflowClip() || m_object->hasClip()) {
        // This layer establishes a clip of some kind.
        if (m_object->hasOverflowClip())
            foregroundRect = foregroundRect.intersect(m_object->getOverflowClipRect(x,y));
        if (m_object->hasClip()) {
            // Clip applies to *us* as well, so go ahead and update the damageRect.
            QRect newPosClip = m_object->getClipRect(x,y);
            backgroundRect = backgroundRect.intersect(newPosClip);
            foregroundRect = foregroundRect.intersect(newPosClip);
        }

        // If we establish a clip at all, then go ahead and make sure our background
        // rect is intersected with our layer's bounds.
        backgroundRect = backgroundRect.intersect(layerBounds);
    }
}

bool RenderLayer::intersectsDamageRect(const QRect& layerBounds, const QRect& damageRect) const
{
    return (renderer()->isCanvas() || renderer()->isRoot() || renderer()->isBody() ||
            renderer()->hasOverhangingFloats() ||
            (renderer()->isInline() && !renderer()->isReplaced()) ||
            layerBounds.intersects(damageRect));
}

bool RenderLayer::containsPoint(int x, int y, const QRect& damageRect) const
{
    return (renderer()->isCanvas() || renderer()->isRoot() || renderer()->isBody() ||
            renderer()->hasOverhangingFloats() ||
            (renderer()->isInline() && !renderer()->isReplaced()) ||
            damageRect.contains(x, y));
}

// This code has been written to anticipate the addition of CSS3-::outside and ::inside generated
// content (and perhaps XBL).  That's why it uses the render tree and not the DOM tree.
static RenderObject* hoverAncestor(RenderObject* obj)
{
    return (!obj->isInline() && obj->continuation()) ? obj->continuation() : obj->parent();
}

static RenderObject* commonAncestor(RenderObject* obj1, RenderObject* obj2)
{
    if (!obj1 || !obj2)
        return 0;

    for (RenderObject* currObj1 = obj1; currObj1; currObj1 = hoverAncestor(currObj1))
        for (RenderObject* currObj2 = obj2; currObj2; currObj2 = hoverAncestor(currObj2))
            if (currObj1 == currObj2)
                return currObj1;

    return 0;
}

void RenderLayer::updateHoverActiveState(RenderObject::NodeInfo& info)
{
    // We don't update :hover/:active state when the info is marked as readonly.
    if (info.readonly())
        return;
    
    // Check to see if the hovered node has changed.  If not, then we don't need to
    // do anything.  An exception is if we just went from :hover into :hover:active,
    // in which case we need to update to get the new :active state.
    DOM::DocumentImpl* doc = renderer()->document();
    DOM::NodeImpl* oldHoverNode = doc ? doc->hoverNode() : 0;
    DOM::NodeImpl* newHoverNode = info.innerNode();
        
    if (oldHoverNode == newHoverNode && (!oldHoverNode || oldHoverNode->active() == info.active()))
        return;

    // Update our current hover node.
    info.innerNode()->getDocument()->setHoverNode(newHoverNode);
    
    // We have two different objects.  Fetch their renderers.
    RenderObject* oldHoverObj = oldHoverNode ? oldHoverNode->renderer() : 0;
    RenderObject* newHoverObj = info.innerNode() ? info.innerNode()->renderer() : 0;
    
    // Locate the common ancestor render object for the two renderers.
    RenderObject* ancestor = commonAncestor(oldHoverObj, newHoverObj);
    
    // The old hover path only needs to be cleared up to (and not including) the common ancestor;
    for (RenderObject* curr = oldHoverObj; curr && curr != ancestor; curr = hoverAncestor(curr)) {
        curr->setMouseInside(false);
        if (curr->element() && !curr->isText()) {
            bool oldActive = curr->element()->active();
            curr->element()->setActive(false);
            if (curr->style()->affectedByHoverRules() ||
                (curr->style()->affectedByActiveRules() && oldActive))
                curr->element()->setChanged();
        }
    }

    // Now set the hover state for our new object up to the root.
    for (RenderObject* curr = newHoverObj; curr; curr = hoverAncestor(curr)) {
        bool oldInside = curr->mouseInside();
        curr->setMouseInside(true);
        if (curr->element() && !curr->isText()) {
            bool oldActive = curr->element()->active();
            curr->element()->setActive(info.active());
            if ((curr->style()->affectedByHoverRules() && !oldInside) ||
                (curr->style()->affectedByActiveRules() && oldActive != info.active()))
                curr->element()->setChanged();
        }
    }
}

// Sort the buffer from lowest z-index to highest.  The common scenario will have
// most z-indices equal, so we optimize for that case (i.e., the list will be mostly
// sorted already).
static void sortByZOrder(QPtrVector<RenderLayer::RenderLayer>* buffer,
                         QPtrVector<RenderLayer::RenderLayer>* mergeBuffer,
                         uint start, uint end)
{
    if (start >= end)
        return; // Sanity check.

    if (end - start <= 6) {
        // Apply a bubble sort for smaller lists.
        for (uint i = end-1; i > start; i--) {
            bool sorted = true;
            for (uint j = start; j < i; j++) {
                RenderLayer* elt = buffer->at(j);
                RenderLayer* elt2 = buffer->at(j+1);
                if (elt->zIndex() > elt2->zIndex()) {
                    sorted = false;
                    buffer->insert(j, elt2);
                    buffer->insert(j+1, elt);
                }
            }
            if (sorted)
                return;
        }
    }
    else {
        // Peform a merge sort for larger lists.
        uint mid = (start+end)/2;
        sortByZOrder(buffer, mergeBuffer, start, mid);
        sortByZOrder(buffer, mergeBuffer, mid, end);

        RenderLayer* elt = buffer->at(mid-1);
        RenderLayer* elt2 = buffer->at(mid);

        // Handle the fast common case (of equal z-indices).  The list may already
        // be completely sorted.
        if (elt->zIndex() <= elt2->zIndex())
            return;

        // We have to merge sort.  Ensure our merge buffer is big enough to hold
        // all the items.
        mergeBuffer->resize(end - start);
        uint i1 = start;
        uint i2 = mid;

        elt = buffer->at(i1);
        elt2 = buffer->at(i2);

        while (i1 < mid || i2 < end) {
            if (i1 < mid && (i2 == end || elt->zIndex() <= elt2->zIndex())) {
                mergeBuffer->insert(mergeBuffer->count(), elt);
                i1++;
                if (i1 < mid)
                    elt = buffer->at(i1);
            }
            else {
                mergeBuffer->insert(mergeBuffer->count(), elt2);
                i2++;
                if (i2 < end)
                    elt2 = buffer->at(i2);
            }
        }

        for (uint i = start; i < end; i++)
            buffer->insert(i, mergeBuffer->at(i-start));

        mergeBuffer->clear();
    }
}

void RenderLayer::dirtyZOrderLists()
{
    if (m_posZOrderList)
        m_posZOrderList->clear();
    if (m_negZOrderList)
        m_negZOrderList->clear();
    m_zOrderListsDirty = true;
}
    
void RenderLayer::updateZOrderLists()
{
    if (!isStackingContext() || !m_zOrderListsDirty)
        return;
    
    for (RenderLayer* child = firstChild(); child; child = child->nextSibling())
        child->collectLayers(m_posZOrderList, m_negZOrderList);

    // Sort the two lists.
    if (m_posZOrderList) {
        QPtrVector<RenderLayer> mergeBuffer;
        sortByZOrder(m_posZOrderList, &mergeBuffer, 0, m_posZOrderList->count());
    }
    if (m_negZOrderList) {
        QPtrVector<RenderLayer> mergeBuffer;
        sortByZOrder(m_negZOrderList, &mergeBuffer, 0, m_negZOrderList->count());
    }

    m_zOrderListsDirty = false;
}

void RenderLayer::collectLayers(QPtrVector<RenderLayer>*& posBuffer, QPtrVector<RenderLayer>*& negBuffer)
{
    // FIXME: A child render object or layer could override visibility.  Don't remove this
    // optimization though until RenderObject's nodeAtPoint is patched to understand what to do
    // when visibility is overridden by a child.
    if (renderer()->style()->visibility() != VISIBLE)
        return;
    
    // Determine which buffer the child should be in.
    QPtrVector<RenderLayer>*& buffer = (zIndex() >= 0) ? posBuffer : negBuffer;

    // Create the buffer if it doesn't exist yet.
    if (!buffer)
        buffer = new QPtrVector<RenderLayer>();
    
    // Resize by a power of 2 when our buffer fills up.
    if (buffer->count() == buffer->size())
        buffer->resize(2*(buffer->size()+1));

    // Append ourselves at the end of the appropriate buffer.
    buffer->insert(buffer->count(), this);

    // Recur into our children to collect more layers, but only if we don't establish
    // a stacking context.
    if (!isStackingContext()) {
        for (RenderLayer* child = firstChild(); child; child = child->nextSibling())
            child->collectLayers(posBuffer, negBuffer);
    }
}
