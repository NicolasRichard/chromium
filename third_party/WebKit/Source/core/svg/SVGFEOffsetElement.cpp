/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
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

#include "core/svg/SVGFEOffsetElement.h"

#include "SVGNames.h"
#include "platform/graphics/filters/FilterEffect.h"
#include "core/svg/SVGElementInstance.h"
#include "core/svg/graphics/filters/SVGFilterBuilder.h"

namespace WebCore {

// Animated property definitions
DEFINE_ANIMATED_STRING(SVGFEOffsetElement, SVGNames::inAttr, In1, in1)

BEGIN_REGISTER_ANIMATED_PROPERTIES(SVGFEOffsetElement)
    REGISTER_LOCAL_ANIMATED_PROPERTY(in1)
    REGISTER_PARENT_ANIMATED_PROPERTIES(SVGFilterPrimitiveStandardAttributes)
END_REGISTER_ANIMATED_PROPERTIES

inline SVGFEOffsetElement::SVGFEOffsetElement(Document& document)
    : SVGFilterPrimitiveStandardAttributes(SVGNames::feOffsetTag, document)
    , m_dx(SVGAnimatedNumber::create(this, SVGNames::dxAttr, SVGNumber::create()))
    , m_dy(SVGAnimatedNumber::create(this, SVGNames::dyAttr, SVGNumber::create()))
{
    ScriptWrappable::init(this);

    addToPropertyMap(m_dx);
    addToPropertyMap(m_dy);
    registerAnimatedPropertiesForSVGFEOffsetElement();
}

PassRefPtr<SVGFEOffsetElement> SVGFEOffsetElement::create(Document& document)
{
    return adoptRef(new SVGFEOffsetElement(document));
}

bool SVGFEOffsetElement::isSupportedAttribute(const QualifiedName& attrName)
{
    DEFINE_STATIC_LOCAL(HashSet<QualifiedName>, supportedAttributes, ());
    if (supportedAttributes.isEmpty()) {
        supportedAttributes.add(SVGNames::inAttr);
        supportedAttributes.add(SVGNames::dxAttr);
        supportedAttributes.add(SVGNames::dyAttr);
    }
    return supportedAttributes.contains<SVGAttributeHashTranslator>(attrName);
}

void SVGFEOffsetElement::parseAttribute(const QualifiedName& name, const AtomicString& value)
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

    if (name == SVGNames::dxAttr)
        m_dx->setBaseValueAsString(value, parseError);
    else if (name == SVGNames::dyAttr)
        m_dy->setBaseValueAsString(value, parseError);
    else
        ASSERT_NOT_REACHED();

    reportAttributeParsingError(parseError, name, value);
}

void SVGFEOffsetElement::svgAttributeChanged(const QualifiedName& attrName)
{
    if (!isSupportedAttribute(attrName)) {
        SVGFilterPrimitiveStandardAttributes::svgAttributeChanged(attrName);
        return;
    }

    SVGElementInstance::InvalidationGuard invalidationGuard(this);

    if (attrName == SVGNames::inAttr || attrName == SVGNames::dxAttr || attrName == SVGNames::dyAttr) {
        invalidate();
        return;
    }

    ASSERT_NOT_REACHED();
}

PassRefPtr<FilterEffect> SVGFEOffsetElement::build(SVGFilterBuilder* filterBuilder, Filter* filter)
{
    FilterEffect* input1 = filterBuilder->getEffectById(AtomicString(in1CurrentValue()));

    if (!input1)
        return 0;

    RefPtr<FilterEffect> effect = FEOffset::create(filter, m_dx->currentValue()->value(), m_dy->currentValue()->value());
    effect->inputEffects().append(input1);
    return effect.release();
}

}
