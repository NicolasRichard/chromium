/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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
 */

#include "config.h"

#if ENABLE(FILTERS)
#include "FEOffset.h"

#include "Filter.h"
#include "GraphicsContext.h"

namespace WebCore {

FEOffset::FEOffset(float dx, float dy)
    : FilterEffect()
    , m_dx(dx)
    , m_dy(dy)
{
}

PassRefPtr<FEOffset> FEOffset::create(float dx, float dy)
{
    return adoptRef(new FEOffset(dx, dy));
}

float FEOffset::dx() const
{
    return m_dx;
}

void FEOffset::setDx(float dx)
{
    m_dx = dx;
}

float FEOffset::dy() const
{
    return m_dy;
}

void FEOffset::setDy(float dy)
{
    m_dy = dy;
}

void FEOffset::determineAbsolutePaintRect(Filter* filter)
{
    FloatRect paintRect = inputEffect(0)->absolutePaintRect();
    paintRect.move(filter->applyHorizontalScale(m_dx), filter->applyVerticalScale(m_dy));
    paintRect.intersect(maxEffectRect());
    setAbsolutePaintRect(enclosingIntRect(paintRect));
}

void FEOffset::apply(Filter* filter)
{
    FilterEffect* in = inputEffect(0);
    in->apply(filter);
    if (!in->resultImage())
        return;

    GraphicsContext* filterContext = effectContext(filter);
    if (!filterContext)
        return;

    setIsAlphaImage(in->isAlphaImage());

    FloatRect drawingRegion = drawingRegionOfInputImage(in->absolutePaintRect());
    drawingRegion.move(filter->applyHorizontalScale(m_dx), filter->applyVerticalScale(m_dy));
    filterContext->drawImageBuffer(in->resultImage(), ColorSpaceDeviceRGB, drawingRegion);
}

void FEOffset::dump()
{
}

TextStream& FEOffset::externalRepresentation(TextStream& ts, int indent) const
{
    writeIndent(ts, indent);
    ts << "[feOffset"; 
    FilterEffect::externalRepresentation(ts);
    ts << " dx=\"" << dx() << "\" dy=\"" << dy() << "\"]\n";
    inputEffect(0)->externalRepresentation(ts, indent + 1);
    return ts;
}

} // namespace WebCore

#endif // ENABLE(FILTERS)
