/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003, 2006, 2007 Apple Inc. All rights reserved.
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

#ifndef RenderBox_h
#define RenderBox_h

#include "RenderBoxModelObject.h"
#include "RenderOverflow.h"
#include "ScrollTypes.h"

namespace WebCore {

struct PaintInfo;

enum LogicalWidthType { LogicalWidth, MinLogicalWidth, MaxLogicalWidth };

enum OverlayScrollbarSizeRelevancy { IgnoreOverlayScrollbarSize, IncludeOverlayScrollbarSize };

class RenderBox : public RenderBoxModelObject {
public:
    RenderBox(Node*);
    virtual ~RenderBox();

    // Use this with caution! No type checking is done!
    RenderBox* firstChildBox() const;
    RenderBox* lastChildBox() const;

    LayoutUnit x() const { return m_frameRect.x(); }
    LayoutUnit y() const { return m_frameRect.y(); }
    LayoutUnit width() const { return m_frameRect.width(); }
    LayoutUnit height() const { return m_frameRect.height(); }

    void setX(LayoutUnit x) { m_frameRect.setX(x); }
    void setY(LayoutUnit y) { m_frameRect.setY(y); }
    void setWidth(LayoutUnit width) { m_frameRect.setWidth(width); }
    void setHeight(LayoutUnit height) { m_frameRect.setHeight(height); }

    LayoutUnit logicalLeft() const { return style()->isHorizontalWritingMode() ? x() : y(); }
    LayoutUnit logicalRight() const { return logicalLeft() + logicalWidth(); }
    LayoutUnit logicalTop() const { return style()->isHorizontalWritingMode() ? y() : x(); }
    LayoutUnit logicalBottom() const { return logicalTop() + logicalHeight(); }
    LayoutUnit logicalWidth() const { return style()->isHorizontalWritingMode() ? width() : height(); }
    LayoutUnit logicalHeight() const { return style()->isHorizontalWritingMode() ? height() : width(); }

    void setLogicalLeft(LayoutUnit left)
    {
        if (style()->isHorizontalWritingMode())
            setX(left);
        else
            setY(left);
    }
    void setLogicalTop(LayoutUnit top)
    {
        if (style()->isHorizontalWritingMode())
            setY(top);
        else
            setX(top);
    }
    void setLogicalLocation(const LayoutPoint& location)
    {
        if (style()->isHorizontalWritingMode())
            setLocation(location);
        else
            setLocation(location.transposedPoint());
    }
    void setLogicalWidth(LayoutUnit size)
    {
        if (style()->isHorizontalWritingMode())
            setWidth(size);
        else
            setHeight(size);
    }
    void setLogicalHeight(LayoutUnit size)
    {
        if (style()->isHorizontalWritingMode())
            setHeight(size);
        else
            setWidth(size);
    }
    void setLogicalSize(const LayoutSize& size)
    {
        if (style()->isHorizontalWritingMode())
            setSize(size);
        else
            setSize(size.transposedSize());
    }

    LayoutPoint location() const { return m_frameRect.location(); }
    LayoutSize locationOffset() const { return LayoutSize(x(), y()); }
    LayoutSize size() const { return m_frameRect.size(); }

    void setLocation(const LayoutPoint& location) { m_frameRect.setLocation(location); }
    
    void setSize(const LayoutSize& size) { m_frameRect.setSize(size); }
    void move(LayoutUnit dx, LayoutUnit dy) { m_frameRect.move(dx, dy); }

    LayoutRect frameRect() const { return m_frameRect; }
    void setFrameRect(const LayoutRect& rect) { m_frameRect = rect; }

    LayoutRect borderBoxRect() const { return LayoutRect(0, 0, width(), height()); }
    virtual LayoutRect borderBoundingBox() const { return borderBoxRect(); } 

    // The content area of the box (excludes padding and border).
    LayoutRect contentBoxRect() const { return LayoutRect(borderLeft() + paddingLeft(), borderTop() + paddingTop(), contentWidth(), contentHeight()); }
    // The content box in absolute coords. Ignores transforms.
    LayoutRect absoluteContentBox() const;
    // The content box converted to absolute coords (taking transforms into account).
    FloatQuad absoluteContentQuad() const;

    // Bounds of the outline box in absolute coords. Respects transforms
    virtual IntRect outlineBoundsForRepaint(RenderBoxModelObject* /*repaintContainer*/, IntPoint* cachedOffsetToRepaintContainer) const;
    virtual void addFocusRingRects(Vector<IntRect>&, const IntPoint&);

    // Use this with caution! No type checking is done!
    RenderBox* previousSiblingBox() const;
    RenderBox* nextSiblingBox() const;
    RenderBox* parentBox() const;

    // Visual and layout overflow are in the coordinate space of the box.  This means that they aren't purely physical directions.
    // For horizontal-tb and vertical-lr they will match physical directions, but for horizontal-bt and vertical-rl, the top/bottom and left/right
    // respectively are flipped when compared to their physical counterparts.  For example minX is on the left in vertical-lr,
    // but it is on the right in vertical-rl.
    IntRect layoutOverflowRect() const { return m_overflow ? m_overflow->layoutOverflowRect() : clientBoxRect(); }
    int minYLayoutOverflow() const { return m_overflow? m_overflow->minYLayoutOverflow() : borderTop(); }
    int maxYLayoutOverflow() const { return m_overflow ? m_overflow->maxYLayoutOverflow() : borderTop() + clientHeight(); }
    int minXLayoutOverflow() const { return m_overflow ? m_overflow->minXLayoutOverflow() : borderLeft(); }
    int maxXLayoutOverflow() const { return m_overflow ? m_overflow->maxXLayoutOverflow() : borderLeft() + clientWidth(); }
    IntSize maxLayoutOverflow() const { return IntSize(maxXLayoutOverflow(), maxYLayoutOverflow()); }
    int logicalLeftLayoutOverflow() const { return style()->isHorizontalWritingMode() ? minXLayoutOverflow() : minYLayoutOverflow(); }
    int logicalRightLayoutOverflow() const { return style()->isHorizontalWritingMode() ? maxXLayoutOverflow() : maxYLayoutOverflow(); }
    
    IntRect visualOverflowRect() const { return m_overflow ? m_overflow->visualOverflowRect() : borderBoxRect(); }
    int minYVisualOverflow() const { return m_overflow? m_overflow->minYVisualOverflow() : 0; }
    int maxYVisualOverflow() const { return m_overflow ? m_overflow->maxYVisualOverflow() : height(); }
    int minXVisualOverflow() const { return m_overflow ? m_overflow->minXVisualOverflow() : 0; }
    int maxXVisualOverflow() const { return m_overflow ? m_overflow->maxXVisualOverflow() : width(); }
    int logicalLeftVisualOverflow() const { return style()->isHorizontalWritingMode() ? minXVisualOverflow() : minYVisualOverflow(); }
    int logicalRightVisualOverflow() const { return style()->isHorizontalWritingMode() ? maxXVisualOverflow() : maxYVisualOverflow(); }
    
    void addLayoutOverflow(const IntRect&);
    void addVisualOverflow(const IntRect&);
    
    void addShadowOverflow();
    void addOverflowFromChild(RenderBox* child) { addOverflowFromChild(child, IntSize(child->x(), child->y())); }
    void addOverflowFromChild(RenderBox* child, const IntSize& delta);
    void clearLayoutOverflow();
    
    void updateLayerTransform();

    void blockDirectionOverflow(bool isLineHorizontal, int& logicalTopLayoutOverflow, int& logicalBottomLayoutOverflow,
                                int& logicalTopVisualOverflow, int& logicalBottomVisualOverflow);

    LayoutUnit contentWidth() const { return clientWidth() - paddingLeft() - paddingRight(); }
    LayoutUnit contentHeight() const { return clientHeight() - paddingTop() - paddingBottom(); }
    LayoutUnit contentLogicalWidth() const { return style()->isHorizontalWritingMode() ? contentWidth() : contentHeight(); }
    LayoutUnit contentLogicalHeight() const { return style()->isHorizontalWritingMode() ? contentHeight() : contentWidth(); }

    // IE extensions. Used to calculate offsetWidth/Height.  Overridden by inlines (RenderFlow)
    // to return the remaining width on a given line (and the height of a single line).
    virtual LayoutUnit offsetWidth() const { return width(); }
    virtual LayoutUnit offsetHeight() const { return height(); }

    // More IE extensions.  clientWidth and clientHeight represent the interior of an object
    // excluding border and scrollbar.  clientLeft/Top are just the borderLeftWidth and borderTopWidth.
    LayoutUnit clientLeft() const { return borderLeft(); }
    LayoutUnit clientTop() const { return borderTop(); }
    LayoutUnit clientWidth() const;
    LayoutUnit clientHeight() const;
    LayoutUnit clientLogicalWidth() const { return style()->isHorizontalWritingMode() ? clientWidth() : clientHeight(); }
    LayoutUnit clientLogicalHeight() const { return style()->isHorizontalWritingMode() ? clientHeight() : clientWidth(); }
    LayoutUnit clientLogicalBottom() const { return borderBefore() + clientLogicalHeight(); }
    LayoutRect clientBoxRect() const { return LayoutRect(clientLeft(), clientTop(), clientWidth(), clientHeight()); }

    // scrollWidth/scrollHeight will be the same as clientWidth/clientHeight unless the
    // object has overflow:hidden/scroll/auto specified and also has overflow.
    // scrollLeft/Top return the current scroll position.  These methods are virtual so that objects like
    // textareas can scroll shadow content (but pretend that they are the objects that are
    // scrolling).
    virtual int scrollLeft() const;
    virtual int scrollTop() const;
    virtual int scrollWidth() const;
    virtual int scrollHeight() const;
    virtual void setScrollLeft(int);
    virtual void setScrollTop(int);

    virtual LayoutUnit marginTop() const { return m_marginTop; }
    virtual LayoutUnit marginBottom() const { return m_marginBottom; }
    virtual LayoutUnit marginLeft() const { return m_marginLeft; }
    virtual LayoutUnit marginRight() const { return m_marginRight; }
    void setMarginTop(LayoutUnit margin) { m_marginTop = margin; }
    void setMarginBottom(LayoutUnit margin) { m_marginBottom = margin; }
    void setMarginLeft(LayoutUnit margin) { m_marginLeft = margin; }
    void setMarginRight(LayoutUnit margin) { m_marginRight = margin; }
    virtual LayoutUnit marginBefore() const;
    virtual LayoutUnit marginAfter() const;
    virtual LayoutUnit marginStart() const;
    virtual LayoutUnit marginEnd() const;
    void setMarginStart(LayoutUnit);
    void setMarginEnd(LayoutUnit);
    void setMarginBefore(LayoutUnit);
    void setMarginAfter(LayoutUnit);

    // The following five functions are used to implement collapsing margins.
    // All objects know their maximal positive and negative margins.  The
    // formula for computing a collapsed margin is |maxPosMargin| - |maxNegmargin|.
    // For a non-collapsing box, such as a leaf element, this formula will simply return
    // the margin of the element.  Blocks override the maxMarginBefore and maxMarginAfter
    // methods.
    enum MarginSign { PositiveMargin, NegativeMargin };
    virtual bool isSelfCollapsingBlock() const { return false; }
    virtual int collapsedMarginBefore() const { return marginBefore(); }
    virtual int collapsedMarginAfter() const { return marginAfter(); }

    virtual void absoluteRects(Vector<IntRect>&, const IntPoint& accumulatedOffset);
    virtual void absoluteQuads(Vector<FloatQuad>&);
    
    IntRect reflectionBox() const;
    int reflectionOffset() const;
    // Given a rect in the object's coordinate space, returns the corresponding rect in the reflection.
    IntRect reflectedRect(const IntRect&) const;

    virtual void layout();
    virtual void paint(PaintInfo&, const LayoutPoint&);
    virtual bool nodeAtPoint(const HitTestRequest&, HitTestResult&, const IntPoint& pointInContainer, const IntPoint& accumulatedOffset, HitTestAction);

    virtual void destroy();

    virtual int minPreferredLogicalWidth() const;
    virtual int maxPreferredLogicalWidth() const;

    int overrideSize() const;
    int overrideWidth() const;
    int overrideHeight() const;
    virtual void setOverrideSize(int);

    virtual IntSize offsetFromContainer(RenderObject*, const IntPoint&) const;
    
    LayoutUnit computeBorderBoxLogicalWidth(LayoutUnit width) const;
    LayoutUnit computeBorderBoxLogicalHeight(LayoutUnit height) const;
    LayoutUnit computeContentBoxLogicalWidth(LayoutUnit width) const;
    LayoutUnit computeContentBoxLogicalHeight(LayoutUnit height) const;

    virtual void borderFitAdjust(IntRect&) const { } // Shrink the box in which the border paints if border-fit is set.

    // Resolve auto margins in the inline direction of the containing block so that objects can be pushed to the start, middle or end
    // of the containing block.
    void computeInlineDirectionMargins(RenderBlock* containingBlock, int containerWidth, int childWidth);

    // Used to resolve margins in the containing block's block-flow direction.
    void computeBlockDirectionMargins(RenderBlock* containingBlock);

    void positionLineBox(InlineBox*);

    virtual InlineBox* createInlineBox();
    void dirtyLineBoxes(bool fullLayout);

    // For inline replaced elements, this function returns the inline box that owns us.  Enables
    // the replaced RenderObject to quickly determine what line it is contained on and to easily
    // iterate over structures on the line.
    InlineBox* inlineBoxWrapper() const { return m_inlineBoxWrapper; }
    void setInlineBoxWrapper(InlineBox* boxWrapper) { m_inlineBoxWrapper = boxWrapper; }
    void deleteLineBoxWrapper();

    virtual IntRect clippedOverflowRectForRepaint(RenderBoxModelObject* repaintContainer);
    virtual void computeRectForRepaint(RenderBoxModelObject* repaintContainer, IntRect&, bool fixed = false);

    virtual void repaintDuringLayoutIfMoved(const LayoutRect&);

    virtual int containingBlockLogicalWidthForContent() const;
    int perpendicularContainingBlockLogicalHeight() const;
    
    virtual void computeLogicalWidth();
    virtual void computeLogicalHeight();

    bool stretchesToViewport() const
    {
        return document()->inQuirksMode() && style()->logicalHeight().isAuto() && !isFloatingOrPositioned() && (isRoot() || isBody());
    }

    virtual IntSize intrinsicSize() const { return IntSize(); }
    int intrinsicLogicalWidth() const { return style()->isHorizontalWritingMode() ? intrinsicSize().width() : intrinsicSize().height(); }
    int intrinsicLogicalHeight() const { return style()->isHorizontalWritingMode() ? intrinsicSize().height() : intrinsicSize().width(); }

    // Whether or not the element shrinks to its intrinsic width (rather than filling the width
    // of a containing block).  HTML4 buttons, <select>s, <input>s, legends, and floating/compact elements do this.
    bool sizesToIntrinsicLogicalWidth(LogicalWidthType) const;
    virtual bool stretchesToMinIntrinsicLogicalWidth() const { return false; }

    LayoutUnit computeLogicalWidthUsing(LogicalWidthType, LayoutUnit availableLogicalWidth);
    LayoutUnit computeLogicalHeightUsing(const Length& height);
    LayoutUnit computeReplacedLogicalWidthUsing(Length width) const;
    LayoutUnit computeReplacedLogicalWidthRespectingMinMaxWidth(LayoutUnit logicalWidth, bool includeMaxWidth = true) const;
    LayoutUnit computeReplacedLogicalHeightUsing(Length height) const;
    LayoutUnit computeReplacedLogicalHeightRespectingMinMaxHeight(LayoutUnit logicalHeight) const;

    virtual LayoutUnit computeReplacedLogicalWidth(bool includeMaxWidth = true) const;
    virtual LayoutUnit computeReplacedLogicalHeight() const;

    LayoutUnit computePercentageLogicalHeight(const Length& height);

    // Block flows subclass availableWidth to handle multi column layout (shrinking the width available to children when laying out.)
    virtual LayoutUnit availableLogicalWidth() const { return contentLogicalWidth(); }
    LayoutUnit availableLogicalHeight() const;
    LayoutUnit availableLogicalHeightUsing(const Length&) const;
    
    // There are a few cases where we need to refer specifically to the available physical width and available physical height.
    // Relative positioning is one of those cases, since left/top offsets are physical.
    LayoutUnit availableWidth() const { return style()->isHorizontalWritingMode() ? availableLogicalWidth() : availableLogicalHeight(); }
    LayoutUnit availableHeight() const { return style()->isHorizontalWritingMode() ? availableLogicalHeight() : availableLogicalWidth(); }

    virtual int verticalScrollbarWidth() const;
    int horizontalScrollbarHeight() const;
    int scrollbarLogicalHeight() const { return style()->isHorizontalWritingMode() ? horizontalScrollbarHeight() : verticalScrollbarWidth(); }
    virtual bool scroll(ScrollDirection, ScrollGranularity, float multiplier = 1, Node** stopNode = 0);
    virtual bool logicalScroll(ScrollLogicalDirection, ScrollGranularity, float multiplier = 1, Node** stopNode = 0);
    bool canBeScrolledAndHasScrollableArea() const;
    virtual bool canBeProgramaticallyScrolled() const;
    virtual void autoscroll();
    virtual void stopAutoscroll() { }
    virtual void panScroll(const IntPoint&);
    bool hasAutoVerticalScrollbar() const { return hasOverflowClip() && (style()->overflowY() == OAUTO || style()->overflowY() == OOVERLAY); }
    bool hasAutoHorizontalScrollbar() const { return hasOverflowClip() && (style()->overflowX() == OAUTO || style()->overflowX() == OOVERLAY); }
    bool scrollsOverflow() const { return scrollsOverflowX() || scrollsOverflowY(); }
    bool scrollsOverflowX() const { return hasOverflowClip() && (style()->overflowX() == OSCROLL || hasAutoHorizontalScrollbar()); }
    bool scrollsOverflowY() const { return hasOverflowClip() && (style()->overflowY() == OSCROLL || hasAutoVerticalScrollbar()); }
    
    virtual IntRect localCaretRect(InlineBox*, int caretOffset, int* extraWidthToEndOfLine = 0);

    virtual IntRect overflowClipRect(const IntPoint& location, OverlayScrollbarSizeRelevancy = IgnoreOverlayScrollbarSize);
    IntRect clipRect(const IntPoint& location);
    virtual bool hasControlClip() const { return false; }
    virtual IntRect controlClipRect(const IntPoint&) const { return IntRect(); }
    bool pushContentsClip(PaintInfo&, const IntPoint& accumulatedOffset);
    void popContentsClip(PaintInfo&, PaintPhase originalPhase, const IntPoint& accumulatedOffset);

    virtual void paintObject(PaintInfo&, const LayoutPoint&) { ASSERT_NOT_REACHED(); }
    virtual void paintBoxDecorations(PaintInfo&, const LayoutPoint&);
    virtual void paintMask(PaintInfo&, const LayoutPoint&);
    virtual void imageChanged(WrappedImagePtr, const IntRect* = 0);

    // Called when a positioned object moves but doesn't necessarily change size.  A simplified layout is attempted
    // that just updates the object's position. If the size does change, the object remains dirty.
    bool tryLayoutDoingPositionedMovementOnly()
    {
        int oldWidth = width();
        computeLogicalWidth();
        // If we shrink to fit our width may have changed, so we still need full layout.
        if (oldWidth != width())
            return false;
        computeLogicalHeight();
        return true;
    }

    IntRect maskClipRect();

    virtual VisiblePosition positionForPoint(const IntPoint&);

    void removeFloatingOrPositionedChildFromBlockLists();
    
    RenderLayer* enclosingFloatPaintingLayer() const;
    
    virtual int firstLineBoxBaseline() const { return -1; }
    virtual int lastLineBoxBaseline() const { return -1; }

    bool shrinkToAvoidFloats() const;
    virtual bool avoidsFloats() const;

    virtual void markForPaginationRelayoutIfNeeded() { }

    bool isWritingModeRoot() const { return !parent() || parent()->style()->writingMode() != style()->writingMode(); }
    
    virtual int lineHeight(bool firstLine, LineDirectionMode, LinePositionMode = PositionOnContainingLine) const;
    virtual int baselinePosition(FontBaseline, bool firstLine, LineDirectionMode, LinePositionMode = PositionOnContainingLine) const;

    enum FlippingAdjustment { ChildToParentFlippingAdjustment, ParentToChildFlippingAdjustment };
    IntPoint flipForWritingMode(const RenderBox* child, const IntPoint&, FlippingAdjustment) const;
    int flipForWritingMode(int position) const; // The offset is in the block direction (y for horizontal writing modes, x for vertical writing modes).
    IntPoint flipForWritingMode(const IntPoint&) const;
    IntPoint flipForWritingModeIncludingColumns(const IntPoint&) const;
    IntSize flipForWritingMode(const IntSize&) const;
    void flipForWritingMode(IntRect&) const;
    FloatPoint flipForWritingMode(const FloatPoint&) const;
    void flipForWritingMode(FloatRect&) const;
    IntSize locationOffsetIncludingFlipping() const;

    IntRect logicalVisualOverflowRectForPropagation(RenderStyle*) const;
    IntRect visualOverflowRectForPropagation(RenderStyle*) const;
    IntRect logicalLayoutOverflowRectForPropagation(RenderStyle*) const;
    IntRect layoutOverflowRectForPropagation(RenderStyle*) const;

    RenderOverflow* hasRenderOverflow() const { return m_overflow.get(); }

    virtual bool needsPreferredWidthsRecalculation() const;
    virtual void computeIntrinsicRatioInformation(FloatSize& /* intrinsicRatio */, bool& /* isPercentageIntrinsicSize */) const { }

protected:
    virtual void styleWillChange(StyleDifference, const RenderStyle* newStyle);
    virtual void styleDidChange(StyleDifference, const RenderStyle* oldStyle);
    virtual void updateBoxModelInfoFromStyle();

    void paintFillLayer(const PaintInfo&, const Color&, const FillLayer*, const LayoutRect&, BackgroundBleedAvoidance, CompositeOperator, RenderObject* backgroundObject);
    void paintFillLayers(const PaintInfo&, const Color&, const FillLayer*, const LayoutRect&, BackgroundBleedAvoidance = BackgroundBleedNone, CompositeOperator = CompositeSourceOver, RenderObject* backgroundObject = 0);

    void paintMaskImages(const PaintInfo&, const LayoutRect&);

#if PLATFORM(MAC)
    void paintCustomHighlight(const IntPoint&, const AtomicString& type, bool behindText);
#endif

    void computePositionedLogicalWidth();
    
    virtual bool shouldComputeSizeAsReplaced() const { return isReplaced() && !isInlineBlockOrInlineTable(); }

    virtual void mapLocalToContainer(RenderBoxModelObject* repaintContainer, bool fixed, bool useTransforms, TransformState&, bool* wasFixed = 0) const;
    virtual void mapAbsoluteToLocalPoint(bool fixed, bool useTransforms, TransformState&) const;

    void paintRootBoxFillLayers(const PaintInfo&);

private:
    bool includeVerticalScrollbarSize() const;
    bool includeHorizontalScrollbarSize() const;

    // Returns true if we did a full repaint
    bool repaintLayerRectsForImage(WrappedImagePtr image, const FillLayer* layers, bool drawingBackground);
   
    int containingBlockLogicalWidthForPositioned(const RenderBoxModelObject* containingBlock, bool checkForPerpendicularWritingMode = true) const;
    int containingBlockLogicalHeightForPositioned(const RenderBoxModelObject* containingBlock, bool checkForPerpendicularWritingMode = true) const;

    void computePositionedLogicalHeight();
    void computePositionedLogicalWidthUsing(Length logicalWidth, const RenderBoxModelObject* containerBlock, TextDirection containerDirection,
                                            int containerLogicalWidth, int bordersPlusPadding,
                                            Length logicalLeft, Length logicalRight, Length marginLogicalLeft, Length marginLogicalRight,
                                            int& logicalWidthValue, int& marginLogicalLeftValue, int& marginLogicalRightValue, int& logicalLeftPos);
    void computePositionedLogicalHeightUsing(Length logicalHeight, const RenderBoxModelObject* containerBlock,
                                             int containerLogicalHeight, int bordersPlusPadding,
                                             Length logicalTop, Length logicalBottom, Length marginLogicalTop, Length marginLogicalBottom,
                                             int& logicalHeightValue, int& marginLogicalTopValue, int& marginLogicalBottomValue, int& logicalTopPos);

    void computePositionedLogicalHeightReplaced();
    void computePositionedLogicalWidthReplaced();

    // This function calculates the minimum and maximum preferred widths for an object.
    // These values are used in shrink-to-fit layout systems.
    // These include tables, positioned objects, floats and flexible boxes.
    virtual void computePreferredLogicalWidths() { setPreferredLogicalWidthsDirty(false); }

    BackgroundBleedAvoidance determineBackgroundBleedAvoidance(GraphicsContext*) const;

private:
    // The width/height of the contents + borders + padding.  The x/y location is relative to our container (which is not always our parent).
    LayoutRect m_frameRect;

protected:
    LayoutUnit m_marginLeft;
    LayoutUnit m_marginRight;
    LayoutUnit m_marginTop;
    LayoutUnit m_marginBottom;

    // The preferred logical width of the element if it were to break its lines at every possible opportunity.
    int m_minPreferredLogicalWidth;
    
    // The preferred logical width of the element if it never breaks any lines at all.
    int m_maxPreferredLogicalWidth;

    // For inline replaced elements, the inline box that owns us.
    InlineBox* m_inlineBoxWrapper;

    // Our overflow information.
    OwnPtr<RenderOverflow> m_overflow;

private:
    // Used to store state between styleWillChange and styleDidChange
    static bool s_hadOverflowClip;
};

inline RenderBox* toRenderBox(RenderObject* object)
{ 
    ASSERT(!object || object->isBox());
    return static_cast<RenderBox*>(object);
}

inline const RenderBox* toRenderBox(const RenderObject* object)
{ 
    ASSERT(!object || object->isBox());
    return static_cast<const RenderBox*>(object);
}

// This will catch anyone doing an unnecessary cast.
void toRenderBox(const RenderBox*);

inline RenderBox* RenderBox::previousSiblingBox() const
{
    return toRenderBox(previousSibling());
}

inline RenderBox* RenderBox::nextSiblingBox() const
{ 
    return toRenderBox(nextSibling());
}

inline RenderBox* RenderBox::parentBox() const
{
    return toRenderBox(parent());
}

inline RenderBox* RenderBox::firstChildBox() const
{
    return toRenderBox(firstChild());
}

inline RenderBox* RenderBox::lastChildBox() const
{
    return toRenderBox(lastChild());
}

} // namespace WebCore

#endif // RenderBox_h
