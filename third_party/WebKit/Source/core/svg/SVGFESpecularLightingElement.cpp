/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Oliver Hunt <oliver@nerget.com>
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

#include "core/svg/SVGFESpecularLightingElement.h"

#include "platform/graphics/filters/FilterEffect.h"
#include "core/rendering/style/RenderStyle.h"
#include "core/svg/SVGElementInstance.h"
#include "core/svg/SVGParserUtilities.h"
#include "core/svg/graphics/filters/SVGFilterBuilder.h"

namespace WebCore {

// Animated property definitions
DEFINE_ANIMATED_STRING(SVGFESpecularLightingElement, SVGNames::inAttr, In1, in1)

BEGIN_REGISTER_ANIMATED_PROPERTIES(SVGFESpecularLightingElement)
    REGISTER_LOCAL_ANIMATED_PROPERTY(in1)
    REGISTER_PARENT_ANIMATED_PROPERTIES(SVGFilterPrimitiveStandardAttributes)
END_REGISTER_ANIMATED_PROPERTIES

inline SVGFESpecularLightingElement::SVGFESpecularLightingElement(Document& document)
    : SVGFilterPrimitiveStandardAttributes(SVGNames::feSpecularLightingTag, document)
    , m_specularConstant(SVGAnimatedNumber::create(this, SVGNames::specularConstantAttr, SVGNumber::create(1)))
    , m_specularExponent(SVGAnimatedNumber::create(this, SVGNames::specularExponentAttr, SVGNumber::create(1)))
    , m_surfaceScale(SVGAnimatedNumber::create(this, SVGNames::surfaceScaleAttr, SVGNumber::create(1)))
    , m_kernelUnitLength(SVGAnimatedNumberOptionalNumber::create(this, SVGNames::surfaceScaleAttr))
{
    ScriptWrappable::init(this);

    addToPropertyMap(m_specularConstant);
    addToPropertyMap(m_specularExponent);
    addToPropertyMap(m_surfaceScale);
    addToPropertyMap(m_kernelUnitLength);
    registerAnimatedPropertiesForSVGFESpecularLightingElement();
}

PassRefPtr<SVGFESpecularLightingElement> SVGFESpecularLightingElement::create(Document& document)
{
    return adoptRef(new SVGFESpecularLightingElement(document));
}

bool SVGFESpecularLightingElement::isSupportedAttribute(const QualifiedName& attrName)
{
    DEFINE_STATIC_LOCAL(HashSet<QualifiedName>, supportedAttributes, ());
    if (supportedAttributes.isEmpty()) {
        supportedAttributes.add(SVGNames::inAttr);
        supportedAttributes.add(SVGNames::specularConstantAttr);
        supportedAttributes.add(SVGNames::specularExponentAttr);
        supportedAttributes.add(SVGNames::surfaceScaleAttr);
        supportedAttributes.add(SVGNames::kernelUnitLengthAttr);
    }
    return supportedAttributes.contains<SVGAttributeHashTranslator>(attrName);
}

void SVGFESpecularLightingElement::parseAttribute(const QualifiedName& name, const AtomicString& value)
{
    if (!isSupportedAttribute(name)) {
        SVGFilterPrimitiveStandardAttributes::parseAttribute(name, value);
        return;
    }

    if (name == SVGNames::inAttr) {
        setIn1BaseValue(value);
        return;
    }

    SVGParsingError parseError = NoError;

    if (name == SVGNames::surfaceScaleAttr)
        m_surfaceScale->setBaseValueAsString(value, parseError);
    else if (name == SVGNames::specularConstantAttr)
        m_specularConstant->setBaseValueAsString(value, parseError);
    else if (name == SVGNames::specularExponentAttr)
        m_specularExponent->setBaseValueAsString(value, parseError);
    else if (name == SVGNames::kernelUnitLengthAttr)
        m_kernelUnitLength->setBaseValueAsString(value, parseError);
    else
        ASSERT_NOT_REACHED();

    reportAttributeParsingError(parseError, name, value);
}

bool SVGFESpecularLightingElement::setFilterEffectAttribute(FilterEffect* effect, const QualifiedName& attrName)
{
    FESpecularLighting* specularLighting = static_cast<FESpecularLighting*>(effect);

    if (attrName == SVGNames::lighting_colorAttr) {
        RenderObject* renderer = this->renderer();
        ASSERT(renderer);
        ASSERT(renderer->style());
        return specularLighting->setLightingColor(renderer->style()->svgStyle()->lightingColor());
    }
    if (attrName == SVGNames::surfaceScaleAttr)
        return specularLighting->setSurfaceScale(m_surfaceScale->currentValue()->value());
    if (attrName == SVGNames::specularConstantAttr)
        return specularLighting->setSpecularConstant(m_specularConstant->currentValue()->value());
    if (attrName == SVGNames::specularExponentAttr)
        return specularLighting->setSpecularExponent(m_specularExponent->currentValue()->value());

    LightSource* lightSource = const_cast<LightSource*>(specularLighting->lightSource());
    SVGFELightElement* lightElement = SVGFELightElement::findLightElement(this);
    ASSERT(lightSource);
    ASSERT(lightElement);

    if (attrName == SVGNames::azimuthAttr)
        return lightSource->setAzimuth(lightElement->azimuth()->currentValue()->value());
    if (attrName == SVGNames::elevationAttr)
        return lightSource->setElevation(lightElement->elevation()->currentValue()->value());
    if (attrName == SVGNames::xAttr)
        return lightSource->setX(lightElement->x()->currentValue()->value());
    if (attrName == SVGNames::yAttr)
        return lightSource->setY(lightElement->y()->currentValue()->value());
    if (attrName == SVGNames::zAttr)
        return lightSource->setZ(lightElement->z()->currentValue()->value());
    if (attrName == SVGNames::pointsAtXAttr)
        return lightSource->setPointsAtX(lightElement->pointsAtX()->currentValue()->value());
    if (attrName == SVGNames::pointsAtYAttr)
        return lightSource->setPointsAtY(lightElement->pointsAtY()->currentValue()->value());
    if (attrName == SVGNames::pointsAtZAttr)
        return lightSource->setPointsAtZ(lightElement->pointsAtZ()->currentValue()->value());
    if (attrName == SVGNames::specularExponentAttr)
        return lightSource->setSpecularExponent(lightElement->specularExponent()->currentValue()->value());
    if (attrName == SVGNames::limitingConeAngleAttr)
        return lightSource->setLimitingConeAngle(lightElement->limitingConeAngle()->currentValue()->value());

    ASSERT_NOT_REACHED();
    return false;
}

void SVGFESpecularLightingElement::svgAttributeChanged(const QualifiedName& attrName)
{
    if (!isSupportedAttribute(attrName)) {
        SVGFilterPrimitiveStandardAttributes::svgAttributeChanged(attrName);
        return;
    }

    SVGElementInstance::InvalidationGuard invalidationGuard(this);

    if (attrName == SVGNames::surfaceScaleAttr
        || attrName == SVGNames::specularConstantAttr
        || attrName == SVGNames::specularExponentAttr
        || attrName == SVGNames::kernelUnitLengthAttr) {
        primitiveAttributeChanged(attrName);
        return;
    }

    if (attrName == SVGNames::inAttr) {
        invalidate();
        return;
    }

    ASSERT_NOT_REACHED();
}

void SVGFESpecularLightingElement::lightElementAttributeChanged(const SVGFELightElement* lightElement, const QualifiedName& attrName)
{
    if (SVGFELightElement::findLightElement(this) != lightElement)
        return;

    // The light element has different attribute names so attrName can identify the requested attribute.
    primitiveAttributeChanged(attrName);
}

PassRefPtr<FilterEffect> SVGFESpecularLightingElement::build(SVGFilterBuilder* filterBuilder, Filter* filter)
{
    FilterEffect* input1 = filterBuilder->getEffectById(AtomicString(in1CurrentValue()));

    if (!input1)
        return 0;

    RefPtr<LightSource> lightSource = SVGFELightElement::findLightSource(this);
    if (!lightSource)
        return 0;

    RenderObject* renderer = this->renderer();
    if (!renderer)
        return 0;

    ASSERT(renderer->style());
    Color color = renderer->style()->svgStyle()->lightingColor();

    RefPtr<FilterEffect> effect = FESpecularLighting::create(filter, color, m_surfaceScale->currentValue()->value(), m_specularConstant->currentValue()->value(),
        m_specularExponent->currentValue()->value(), kernelUnitLengthX()->currentValue()->value(), kernelUnitLengthY()->currentValue()->value(), lightSource.release());
    effect->inputEffects().append(input1);
    return effect.release();
}

}
