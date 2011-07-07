/*
 * Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
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

#if ENABLE(SVG)
#include "SVGGradientElement.h"

#include "Attribute.h"
#include "CSSStyleSelector.h"
#include "RenderSVGHiddenContainer.h"
#include "RenderSVGPath.h"
#include "RenderSVGResourceLinearGradient.h"
#include "RenderSVGResourceRadialGradient.h"
#include "SVGElementInstance.h"
#include "SVGNames.h"
#include "SVGStopElement.h"
#include "SVGTransformList.h"
#include "SVGTransformable.h"

namespace WebCore {

// Animated property definitions
DEFINE_ANIMATED_ENUMERATION(SVGGradientElement, SVGNames::spreadMethodAttr, SpreadMethod, spreadMethod, SVGSpreadMethodType)
DEFINE_ANIMATED_ENUMERATION(SVGGradientElement, SVGNames::gradientUnitsAttr, GradientUnits, gradientUnits, SVGUnitTypes::SVGUnitType)
DEFINE_ANIMATED_TRANSFORM_LIST(SVGGradientElement, SVGNames::gradientTransformAttr, GradientTransform, gradientTransform)
DEFINE_ANIMATED_STRING(SVGGradientElement, XLinkNames::hrefAttr, Href, href)
DEFINE_ANIMATED_BOOLEAN(SVGGradientElement, SVGNames::externalResourcesRequiredAttr, ExternalResourcesRequired, externalResourcesRequired)

SVGGradientElement::SVGGradientElement(const QualifiedName& tagName, Document* document)
    : SVGStyledElement(tagName, document)
    , m_spreadMethod(SVGSpreadMethodPad)
    , m_gradientUnits(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
{
}

bool SVGGradientElement::isSupportedAttribute(const QualifiedName& attrName)
{
    DEFINE_STATIC_LOCAL(HashSet<QualifiedName>, supportedAttributes, ());
    if (supportedAttributes.isEmpty()) {
        SVGURIReference::addSupportedAttributes(supportedAttributes);
        SVGExternalResourcesRequired::addSupportedAttributes(supportedAttributes);
        supportedAttributes.add(SVGNames::gradientUnitsAttr);
        supportedAttributes.add(SVGNames::gradientTransformAttr);
        supportedAttributes.add(SVGNames::spreadMethodAttr);
    }
    return supportedAttributes.contains(attrName);
}

void SVGGradientElement::parseMappedAttribute(Attribute* attr)
{
    if (!isSupportedAttribute(attr->name())) {
        SVGStyledElement::parseMappedAttribute(attr);
        return;
    }

    if (attr->name() == SVGNames::gradientUnitsAttr) {
        SVGUnitTypes::SVGUnitType propertyValue = SVGPropertyTraits<SVGUnitTypes::SVGUnitType>::fromString(attr->value());
        if (propertyValue > 0)
            setGradientUnitsBaseValue(propertyValue);
        return;
    }

    if (attr->name() == SVGNames::gradientTransformAttr) {
        SVGTransformList newList;
        if (!SVGTransformable::parseTransformAttribute(newList, attr->value()))
            newList.clear();

        detachAnimatedGradientTransformListWrappers(newList.size());
        setGradientTransformBaseValue(newList);
        return;
    }

    if (attr->name() == SVGNames::spreadMethodAttr) {
        SVGSpreadMethodType propertyValue = SVGPropertyTraits<SVGSpreadMethodType>::fromString(attr->value());
        if (propertyValue > 0)
            setSpreadMethodBaseValue(propertyValue);
        return;
    }

    if (SVGURIReference::parseMappedAttribute(attr))
        return;
    if (SVGExternalResourcesRequired::parseMappedAttribute(attr))
        return;

    ASSERT_NOT_REACHED();
}

void SVGGradientElement::svgAttributeChanged(const QualifiedName& attrName)
{
    if (!isSupportedAttribute(attrName)) {
        SVGStyledElement::svgAttributeChanged(attrName);
        return;
    }

    SVGElementInstance::InvalidationGuard invalidationGuard(this);
    
    if (RenderObject* object = renderer())
        object->setNeedsLayout(true);
}

void SVGGradientElement::synchronizeProperty(const QualifiedName& attrName)
{
    if (attrName == anyQName()) {
        synchronizeGradientUnits();
        synchronizeGradientTransform();
        synchronizeSpreadMethod();
        synchronizeExternalResourcesRequired();
        synchronizeHref();
        SVGStyledElement::synchronizeProperty(attrName);
        return;
    }

    if (!isSupportedAttribute(attrName)) {
        SVGStyledElement::synchronizeProperty(attrName);
        return;
    }

    if (attrName == SVGNames::gradientUnitsAttr) {
        synchronizeGradientUnits();
        return;
    }

    if (attrName == SVGNames::gradientTransformAttr) {
        synchronizeGradientTransform();
        return;
    }

    if (attrName == SVGNames::spreadMethodAttr) {
        synchronizeSpreadMethod();
        return;
    }

    if (SVGExternalResourcesRequired::isKnownAttribute(attrName)) {
        synchronizeExternalResourcesRequired();
        return;
    }

    if (SVGURIReference::isKnownAttribute(attrName)) {
        synchronizeHref();
        return;
    }

    ASSERT_NOT_REACHED();
}

void SVGGradientElement::fillPassedAttributeToPropertyTypeMap(AttributeToPropertyTypeMap& attributeToPropertyTypeMap)
{
    SVGStyledElement::fillPassedAttributeToPropertyTypeMap(attributeToPropertyTypeMap);

    attributeToPropertyTypeMap.set(SVGNames::spreadMethodAttr, AnimatedEnumeration);
    attributeToPropertyTypeMap.set(SVGNames::gradientUnitsAttr, AnimatedEnumeration);
    attributeToPropertyTypeMap.set(SVGNames::gradientTransformAttr, AnimatedTransformList);
    attributeToPropertyTypeMap.set(XLinkNames::hrefAttr, AnimatedString);
}
    
void SVGGradientElement::childrenChanged(bool changedByParser, Node* beforeChange, Node* afterChange, int childCountDelta)
{
    SVGStyledElement::childrenChanged(changedByParser, beforeChange, afterChange, childCountDelta);

    if (changedByParser)
        return;

    if (RenderObject* object = renderer())
        object->setNeedsLayout(true);
}

Vector<Gradient::ColorStop> SVGGradientElement::buildStops()
{
    Vector<Gradient::ColorStop> stops;

    float previousOffset = 0.0f;
    for (Node* n = firstChild(); n; n = n->nextSibling()) {
        SVGElement* element = n->isSVGElement() ? static_cast<SVGElement*>(n) : 0;
        if (!element || !element->isGradientStop())
            continue;

        SVGStopElement* stop = static_cast<SVGStopElement*>(element);
        Color color = stop->stopColorIncludingOpacity();

        // Figure out right monotonic offset
        float offset = stop->offset();
        offset = std::min(std::max(previousOffset, offset), 1.0f);
        previousOffset = offset;

        // Extract individual channel values
        // FIXME: Why doesn't ColorStop take a Color and an offset??
        float r, g, b, a;
        color.getRGBA(r, g, b, a);

        stops.append(Gradient::ColorStop(offset, r, g, b, a));
    }

    return stops;
}

}

#endif // ENABLE(SVG)
