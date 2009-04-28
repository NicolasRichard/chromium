/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2006 Apple Computer, Inc.
 *               2006 Alexander Kellett <lypanov@kde.org>
 *               2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 *               2007 Nikolas Zimmermann <zimmermann@kde.org>
 *               2008 Rob Buis <buis@kde.org>
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

#if ENABLE(SVG)
#include "RenderSVGText.h"

#include "FloatConversion.h"
#include "FloatQuad.h"
#include "GraphicsContext.h"
#include "PointerEventsHitRules.h"
#include "RenderLayer.h"
#include "RenderSVGRoot.h"
#include "SVGLengthList.h"
#include "SVGRenderSupport.h"
#include "SVGResourceFilter.h"
#include "SVGRootInlineBox.h"
#include "SVGTextElement.h"
#include "SVGTransformList.h"
#include "SVGURIReference.h"
#include "SimpleFontData.h"

namespace WebCore {

RenderSVGText::RenderSVGText(SVGTextElement* node) 
    : RenderSVGBlock(node)
{
}

IntRect RenderSVGText::clippedOverflowRectForRepaint(RenderBoxModelObject* repaintContainer)
{
    // Return early for any cases where we don't actually paint
    if (style()->visibility() != VISIBLE && !enclosingLayer()->hasVisibleContent())
        return IntRect();

    // Pass our local paint rect to computeRectForRepaint() which will
    // map to parent coords and recurse up the parent chain.
    IntRect repaintRect = enclosingIntRect(repaintRectInLocalCoordinates());
    computeRectForRepaint(repaintContainer, repaintRect);
    return repaintRect;
}

void RenderSVGText::computeRectForRepaint(RenderBoxModelObject* repaintContainer, IntRect& repaintRect, bool fixed)
{
    // Translate to coords in our parent renderer, and then call computeRectForRepaint on our parent
    repaintRect = localToParentTransform().mapRect(repaintRect);
    parent()->computeRectForRepaint(repaintContainer, repaintRect, fixed);
}

bool RenderSVGText::calculateLocalTransform()
{
    TransformationMatrix oldTransform = m_localTransform;
    m_localTransform = static_cast<SVGTextElement*>(node())->animatedLocalTransform();
    return (oldTransform != m_localTransform);
}

void RenderSVGText::layout()
{
    ASSERT(needsLayout());
    
    // FIXME: This is a hack to avoid the RenderBlock::layout() partial repainting code which is not (yet) SVG aware
    setNeedsLayout(true);

    // FIXME: using m_absoluteBounds breaks if containerForRepaint() is not the root
    LayoutRepainter repainter(*this, checkForRepaintDuringLayout(), &m_absoluteBounds);

    // Best guess for a relative starting point
    SVGTextElement* text = static_cast<SVGTextElement*>(node());
    int xOffset = (int)(text->x()->getFirst().value(text));
    int yOffset = (int)(text->y()->getFirst().value(text));
    setLocation(xOffset, yOffset);
    
    calculateLocalTransform();

    RenderBlock::layout();

    m_absoluteBounds = absoluteClippedOverflowRect();

    repainter.repaintAfterLayout();
        
    setNeedsLayout(false);
}

RootInlineBox* RenderSVGText::createRootBox()
{
    RootInlineBox* box = new (renderArena()) SVGRootInlineBox(this);
    box->setIsSVG(true);
    return box;
}

bool RenderSVGText::nodeAtPoint(const HitTestRequest& request, HitTestResult& result, int _x, int _y, int _tx, int _ty, HitTestAction hitTestAction)
{
    PointerEventsHitRules hitRules(PointerEventsHitRules::SVG_TEXT_HITTESTING, style()->pointerEvents());
    bool isVisible = (style()->visibility() == VISIBLE);
    if (isVisible || !hitRules.requireVisible) {
        if ((hitRules.canHitStroke && (style()->svgStyle()->hasStroke() || !hitRules.requireStroke))
            || (hitRules.canHitFill && (style()->svgStyle()->hasFill() || !hitRules.requireFill))) {
            TransformationMatrix totalTransform = absoluteTransform();
            double localX, localY;
            totalTransform.inverse().map(_x, _y, localX, localY);
            FloatPoint hitPoint(_x, _y);
            return RenderBlock::nodeAtPoint(request, result, (int)localX, (int)localY, _tx, _ty, hitTestAction);
        }
    }

    return false;
}

void RenderSVGText::absoluteRects(Vector<IntRect>& rects, int, int, bool)
{
    RenderSVGRoot* root = findSVGRootObject(parent());
    if (!root)
        return;

    FloatPoint absPos = localToAbsolute();

    TransformationMatrix htmlParentCtm = root->RenderBox::absoluteTransform();
 
    // Don't use objectBoundingBox here, as it's unites the selection rects. Makes it hard
    // to spot errors, if there are any using WebInspector. Individually feed them into 'rects'.
    for (InlineRunBox* runBox = firstLineBox(); runBox; runBox = runBox->nextLineBox()) {
        ASSERT(runBox->isInlineFlowBox());

        InlineFlowBox* flowBox = static_cast<InlineFlowBox*>(runBox);
        for (InlineBox* box = flowBox->firstChild(); box; box = box->nextOnLine()) {
            FloatRect boxRect(box->x(), box->y(), box->width(), box->height());
            boxRect.move(narrowPrecisionToFloat(absPos.x() - htmlParentCtm.e()), narrowPrecisionToFloat(absPos.y() - htmlParentCtm.f()));
            // FIXME: broken with CSS transforms
            rects.append(enclosingIntRect(absoluteTransform().mapRect(boxRect)));
        }
    }
}

void RenderSVGText::absoluteQuads(Vector<FloatQuad>& quads, bool)
{
    RenderSVGRoot* root = findSVGRootObject(parent());
    if (!root)
        return;

    FloatPoint absPos = localToAbsolute();

    TransformationMatrix htmlParentCtm = root->RenderBox::absoluteTransform();
 
    // Don't use objectBoundingBox here, as it's unites the selection rects. Makes it hard
    // to spot errors, if there are any using WebInspector. Individually feed them into 'rects'.
    for (InlineRunBox* runBox = firstLineBox(); runBox; runBox = runBox->nextLineBox()) {
        ASSERT(runBox->isInlineFlowBox());

        InlineFlowBox* flowBox = static_cast<InlineFlowBox*>(runBox);
        for (InlineBox* box = flowBox->firstChild(); box; box = box->nextOnLine()) {
            FloatRect boxRect(box->x(), box->y(), box->width(), box->height());
            boxRect.move(narrowPrecisionToFloat(absPos.x() - htmlParentCtm.e()), narrowPrecisionToFloat(absPos.y() - htmlParentCtm.f()));
            // FIXME: broken with CSS transforms
            quads.append(absoluteTransform().mapRect(boxRect));
        }
    }
}

void RenderSVGText::paint(PaintInfo& paintInfo, int, int)
{   
    PaintInfo pi(paintInfo);
    pi.context->save();
    applyTransformToPaintInfo(pi, localToParentTransform());
    RenderBlock::paint(pi, 0, 0);
    pi.context->restore();
}

FloatRect RenderSVGText::objectBoundingBox() const
{
    FloatRect boundingBox;

    for (InlineRunBox* runBox = firstLineBox(); runBox; runBox = runBox->nextLineBox()) {
        ASSERT(runBox->isInlineFlowBox());

        InlineFlowBox* flowBox = static_cast<InlineFlowBox*>(runBox);
        for (InlineBox* box = flowBox->firstChild(); box; box = box->nextOnLine())
            boundingBox.unite(FloatRect(box->x(), box->y(), box->width(), box->height()));
    }

    boundingBox.move(x(), y());
    return boundingBox;
}

FloatRect RenderSVGText::repaintRectInLocalCoordinates() const
{
    FloatRect repaintRect = objectBoundingBox();

    // SVG needs to include the strokeWidth(), not the textStrokeWidth().
    if (style()->svgStyle()->hasStroke()) {
        float strokeWidth = SVGRenderStyle::cssPrimitiveToLength(this, style()->svgStyle()->strokeWidth(), 0.0f);

#if ENABLE(SVG_FONTS)
        const Font& font = style()->font();
        if (font.primaryFont()->isSVGFont()) {
            float scale = font.unitsPerEm() > 0 ? font.size() / font.unitsPerEm() : 0.0f;

            if (scale != 0.0f)
                strokeWidth /= scale;
        }
#endif

        repaintRect.inflate(strokeWidth);
    }

    repaintRect.unite(filterBoundingBoxForRenderer(this));

    return repaintRect;
}

}

#endif // ENABLE(SVG)

// vim:ts=4:noet
