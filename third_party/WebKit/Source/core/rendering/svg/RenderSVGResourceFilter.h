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

#ifndef RenderSVGResourceFilter_h
#define RenderSVGResourceFilter_h

#include "core/platform/graphics/FloatRect.h"
#include "core/platform/graphics/ImageBuffer.h"
#include "core/rendering/svg/RenderSVGResourceContainer.h"
#include "core/svg/SVGFilterElement.h"
#include "core/svg/SVGUnitTypes.h"
#include "core/svg/graphics/filters/SVGFilter.h"
#include "core/svg/graphics/filters/SVGFilterBuilder.h"

#include <wtf/OwnPtr.h>
#include <wtf/PassOwnPtr.h>
#include <wtf/RefPtr.h>

namespace WebCore {

struct FilterData {
    WTF_MAKE_FAST_ALLOCATED;
public:
    enum FilterDataState { PaintingSource, Applying, Built, CycleDetected, MarkedForRemoval };

    FilterData()
        : savedContext(0)
        , state(PaintingSource)
    {
    }

    RefPtr<SVGFilter> filter;
    RefPtr<SVGFilterBuilder> builder;
    OwnPtr<ImageBuffer> sourceGraphicBuffer;
    GraphicsContext* savedContext;
    AffineTransform shearFreeAbsoluteTransform;
    FloatRect boundaries;
    FloatRect drawingRegion;
    FloatSize scale;
    FilterDataState state;
};

class GraphicsContext;

class RenderSVGResourceFilter FINAL : public RenderSVGResourceContainer {
public:
    RenderSVGResourceFilter(SVGFilterElement*);
    virtual ~RenderSVGResourceFilter();

    virtual const char* renderName() const { return "RenderSVGResourceFilter"; }
    virtual bool isSVGResourceFilter() const OVERRIDE { return true; }

    virtual void removeAllClientsFromCache(bool markForInvalidation = true);
    virtual void removeClientFromCache(RenderObject*, bool markForInvalidation = true);

    virtual bool applyResource(RenderObject*, RenderStyle*, GraphicsContext*&, unsigned short resourceMode);
    virtual void postApplyResource(RenderObject*, GraphicsContext*&, unsigned short resourceMode, const Path*, const RenderSVGShape*);

    virtual FloatRect resourceBoundingBox(RenderObject*);

    PassRefPtr<SVGFilterBuilder> buildPrimitives(SVGFilter*);

    SVGUnitTypes::SVGUnitType filterUnits() const { return static_cast<SVGFilterElement*>(node())->filterUnits(); }
    SVGUnitTypes::SVGUnitType primitiveUnits() const { return static_cast<SVGFilterElement*>(node())->primitiveUnits(); }

    void primitiveAttributeChanged(RenderObject*, const QualifiedName&);

    virtual RenderSVGResourceType resourceType() const { return s_resourceType; }
    static RenderSVGResourceType s_resourceType;

    FloatRect drawingRegion(RenderObject*) const;
private:
    bool fitsInMaximumImageSize(const FloatSize&, FloatSize&);

    HashMap<RenderObject*, FilterData*> m_filter;
};

inline RenderSVGResourceFilter* toRenderSVGFilter(RenderObject* object)
{
    ASSERT_WITH_SECURITY_IMPLICATION(!object || object->isSVGResourceFilter());
    return static_cast<RenderSVGResourceFilter*>(object);
}

}

#endif
